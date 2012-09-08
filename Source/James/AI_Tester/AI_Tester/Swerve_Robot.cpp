#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	//Note we can remove the tank drive train includes once we have a swerve robot control
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "FRC2011_Robot.h"
	#include "Swerve_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*													Swerve_Robot::DrivingModule														*/
/***********************************************************************************************************************************/

Swerve_Robot::DrivingModule::DrivingModule(const char EntityName[],Swerve_Drive_Control_Interface *robot_control) : m_ModuleName(EntityName),
	m_SwivelName("Swivel"),m_DriveName("Drive"),m_Swivel(m_SwivelName.c_str()),m_Drive(m_DriveName.c_str()),
	m_IntendedSwivelDirection(0.0),m_IntendedDriveVelocity(0.0),
	m_RobotControl(robot_control)
{
}

void Swerve_Robot::DrivingModule::Initialize(GG_Framework::Base::EventMap& em,const DrivingModule_Props *props)
{
	m_Swivel.Initialize(em,props->Swivel_Props);
	m_Drive.Initialize(em,props->Drive_Props);
}

void Swerve_Robot::DrivingModule::TimeChange(double dTime_s)
{
	//manage the swivel angle and drive velocity
	m_Swivel.SetIntendedPosition(m_IntendedSwivelDirection);
	m_Drive.SetRequestedVelocity(m_IntendedDriveVelocity);

	//TODO determine why this sticks past 1.5
	//m_Swivel.SetMatchVelocity(m_IntendedSwivelDirection);
	//m_Drive.SetMatchVelocity(m_IntendedDriveVelocity);

	//TODO manage voltage
	//Update the swivel and drive times
	Entity1D &swivel_entity=m_Swivel;  //This gets around keeping time change protected in derived classes
	swivel_entity.TimeChange(dTime_s);
	Entity1D &drive_entity=m_Drive;  //This gets around keeping time change protected in derived classes
	drive_entity.TimeChange(dTime_s);
}

  /***********************************************************************************************************************************/
 /*															Swerve_Robot															*/
/***********************************************************************************************************************************/
Swerve_Robot::Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool UseEncoders) : 
	Ship_Tester(EntityName), m_SwerveDrive(this), m_RobotControl(robot_control), 
	m_UsingEncoders(UseEncoders) //,m_VoltageOverride(false),m_UseDeadZoneSkip(true)
{
	const char * const ModuleName[]=
	{
		"ModuleLF","ModuleRF","ModuleLR","ModuleRR"
	};
	for (size_t i=0;i<4;i++)
		m_DrivingModule[i]=new DrivingModule(ModuleName[i],m_RobotControl);

	//m_UsingEncoders=true; //testing
}

Swerve_Robot::~Swerve_Robot()
{
	for (size_t i=0;i<4;i++)
	{
		delete m_DrivingModule[i];
		m_DrivingModule[i]=NULL;
	}
}

void Swerve_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const Swerve_Robot_Properties *RobotProps=dynamic_cast<const Swerve_Robot_Properties *>(props);
	if (RobotProps)
	{
		//This will copy all the props
		m_SwerveRobotProps=RobotProps->GetSwerveRobotProps();
		m_WheelDimensions=RobotProps->GetWheelDimensions();
		for (size_t i=0;i<4;i++)
		{
			DrivingModule::DrivingModule_Props props;
			props.Swivel_Props=&RobotProps->GetSwivelProps();
			props.Drive_Props=&RobotProps->GetDriveProps();
			m_DrivingModule[i]->Initialize(em,&props);
		}
	}
}

void Swerve_Robot::ResetPos()
{
	m_SwerveDrive.ResetPos();
	__super::ResetPos();
	m_RobotControl->Reset_Encoders();
	for (size_t i=0;i<4;i++)
	{
		m_DrivingModule[i]->ResetPos();
		m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=0.0;
		m_Swerve_Robot_Velocities.Velocity.AsArray[i]=0.0;

	}
}

void Swerve_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	//Now the new UpdateVelocities was just called... work with these intended velocities
	for (size_t i=0;i<4;i++)
	{
		const double IntendedDirection=m_SwerveDrive.GetIntendedVelocitiesFromIndex(i+4);
		double SwivelDirection=IntendedDirection;  //this is either the intended direction or the reverse of it
		const Ship_1D &Swivel=m_DrivingModule[i]->GetSwivel();
		//This is normalized implicitly
		const double LastSwivelDirection=Swivel.GetPos_m();
		double DistanceToIntendedSwivel=fabs(NormalizeRotation2(LastSwivelDirection-SwivelDirection));

		if ((DistanceToIntendedSwivel>PI_2) || 
			(Swivel.GetUsingRange() &&
			 ((SwivelDirection>Swivel.GetMaxRange()) || (SwivelDirection<Swivel.GetMinRange()))) 
			)
		{
			SwivelDirection=NormalizeRotation2(SwivelDirection+PI);
			if (Swivel.GetUsingRange())
			{
				double TestIntendedFlipped=NormalizeRotation2(IntendedDirection+PI);
				//If we flipped because of a huge delta check that the reverse position is in range... and flip it back if it exceed the range
				if ((SwivelDirection>Swivel.GetMaxRange()) || (SwivelDirection<Swivel.GetMinRange()) ||
					(TestIntendedFlipped>Swivel.GetMaxRange()) || (TestIntendedFlipped<Swivel.GetMinRange()))
				{
					SwivelDirection+=PI;
					NormalizeRotation(SwivelDirection);
				}
			}
		}
		//Note the velocity is checked once before the time change here, and once after for the current
		//Only apply swivel adjustments if we have significant movement (this matters in targeting tests)
		if ((fabs(LocalForce[0])>1.5)||(fabs(LocalForce[1])>1.5)||(fabs(m_DrivingModule[i]->GetDrive().GetPhysics().GetVelocity()) > 0.05))
			m_DrivingModule[i]->SetIntendedSwivelDirection(SwivelDirection);
		const double IntendedSpeed=m_SwerveDrive.GetIntendedVelocitiesFromIndex(i);

		//To minimize error only apply the Y component amount to the velocity
		//The less the difference between the current and actual swivel direction the greater the full amount can be applied
		double VelocityToUse=cos(DistanceToIntendedSwivel)*IntendedSpeed;

		m_DrivingModule[i]->SetIntendedDriveVelocity(VelocityToUse);
		m_DrivingModule[i]->TimeChange(dTime_s);

		const double CurrentVelocity=m_DrivingModule[i]->GetDrive().GetPhysics().GetVelocity();
		const double CurrentSwivelDirection=Swivel.GetPos_m();
		//if (i==0)
		//	DOUT4("S= %f %f V= %f %f",CurrentSwivelDirection,SwivelDirection,CurrentVelocity,VelocityToUse);

		//Now to grab and update the actual swerve velocities
		//Note: using GetIntendedVelocities() is a lesser stress for debug purposes
		#if 1
		m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=CurrentSwivelDirection;
		m_Swerve_Robot_Velocities.Velocity.AsArray[i]=CurrentVelocity;
		#else
		m_Swerve_Robot_Velocities=GetIntendedVelocities();
		#endif
	}

	m_SwerveDrive.InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Swerve_Drive_Control_TimeChange(dTime_s);
	//TODO add encoder support here
	//{
	//	//Display encoders without applying calibration
	//	double Encoder_LeftVelocity,Encoder_RightVelocity;
	//	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	//}
	__super::TimeChange(dTime_s);

}

bool Swerve_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	//TODO we'll want to get the measured velocities which should work (or possibly use) like the Swerve_Drive code
	return m_SwerveDrive.InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);

	//bool ret=false;
	//if (m_UsingEncoders)
	//{
	//	Vec2d computedVelocity=m_Physics.GetLinearVelocity();
	//	//double computedAngularVelocity=m_Physics.GetAngularVelocity();
	//	m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
	//	//m_Physics.SetAngularVelocity(m_EncoderHeading);
	//	m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	//	//We must set this back so that the PID can compute the entire error
	//	m_Physics.SetLinearVelocity(computedVelocity);
	//	//m_Physics.SetAngularVelocity(computedAngularVelocity);
	//	ret=true;
	//}
	//return ret;
}

//void Swerve_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
//{
//	m_VoltageOverride=false;
//	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
//			m_VoltageOverride=true;
//}

void Swerve_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	m_SwerveDrive.UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	//double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	//m_RobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);
}

void Swerve_Robot::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	m_SwerveDrive.ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	//We are not going to use these interpolated values in the control (it would corrupt it)... however we can monitor them here, or choose to
	//view them here as needed
	Vec2D force;
	double torque;
	InterpolateThrusterChanges(force,torque,dTime_s);
	__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
}

  /***********************************************************************************************************************************/
 /*														Swerve_Robot_Properties														*/
/***********************************************************************************************************************************/

Swerve_Robot_Properties::Swerve_Robot_Properties() : m_SwivelProps(
	"Swivel",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	18.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	60.0,60.0, //Max Acceleration Forward/Reverse (try to tune to the average turning speed to minimize error on PID)
	Ship_1D_Properties::eSwivel,
	//true,	//Using the range:  for now assuming a 1:1 using a potentiometer with 270 degrees and 10 degrees of padding = 130 degrees each way
	false,  //Or not use the range... it appears it is possible to have unlimited range using the Vishear encoders
	-DEG_2_RAD(130.0),DEG_2_RAD(130.0),
	true  //this is definitely angular
	),
	m_DriveProps(
	"Drive",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//These should match the settings in the script
	2.916,   //Max Speed (This is linear movement speed)
	//10, //TODO find out why I need 8 for turns
	10.0,10.0, //ACCEL, BRAKE  (These can be ignored)
	//Make these as fast as possible without damaging chain or motor
	//These must be fast enough to handle rotation angular acceleration
	60.0,60.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eSimpleMotor,
	false	//No limit ever!
	)
{
	Swerve_Robot_Props props;
	memset(&props,0,sizeof(Swerve_Robot_Props));

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side (can only go 390 degrees a second)
	//props.WheelDimensions=Vec2D(0.3758,0.53);  //This is trimmed down to turn 514 degrees a second with a 2.914 speed

	//const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);  //We may limit swerve to this
	//const double c_GearHeightOffset=1.397;  //55 inches
	//const double c_MotorToWheelGearRatio=12.0/36.0;

	const double c_WheelDiameter=0.1524;  //6 inches
	props.WheelDiameter=c_WheelDiameter;
	props.Polynomial_Wheel[0]=0.0;
	props.Polynomial_Wheel[1]=1.0;
	props.Polynomial_Wheel[2]=0.0;
	props.Polynomial_Wheel[3]=0.0;
	props.Polynomial_Wheel[4]=0.0;
	*props.Polynomial_Swivel=*props.Polynomial_Wheel;
	props.Wheel_PID[0]=props.Swivel_PID[0]=100.0; //setting to a safe 1.0
	props.InputLatency=0.0;
	props.HeadingLatency=0.0;
	props.MotorToWheelGearRatio=1.0;  //most-likely this will be overridden
	props.VoltageScalar=1.0;  //May need to be reversed
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.IsOpen_Wheel=true;  //Always true by default until control is fully functional
	props.IsOpen_Swivel=true;
	for (size_t i=0;i<4;i++)
	{
		props.PID_Console_Dump_Wheel[i]=false;  //Always false unless you want to analyze PID (only one system at a time!)
		props.PID_Console_Dump_Swivel[i]=false;  //Always false unless you want to analyze PID (only one system at a time!)
		props.MaxSpeedOffset[i]=0.0;
		props.EncoderReversed_Wheel[i]=false;
		props.EncoderReversed_Swivel[i]=false;
	}
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.ReverseSteering=false;
	props.DriveTo_ForceDegradeScalar=1.0;
	m_SwerveRobotProps=props;
}


  /***********************************************************************************************************************************/
 /*														Swerve_Robot_Control														*/
/***********************************************************************************************************************************/

Swerve_Robot_Control::Swerve_Robot_Control() : m_DisplayVoltage(true)
{
	for (size_t i=0;i<4;i++)
	{
		m_EncoderVoltage[i]=0;
		m_PotentiometerVoltage[i]=0;
	}
}
void Swerve_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Swerve_Robot_Properties *robot_props=dynamic_cast<const Swerve_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();

		//This will copy all the props
		m_SwerveRobotProps=robot_props->GetSwerveRobotProps();
		//We'll try to construct the props to match our properties
		//Note: for max accel it needs to be powerful enough to handle curve equations
		//Ship_1D_Properties props("SwerveEncoder",2.0,0.0,m_RobotMaxSpeed,1.0,1.0,robot_props->GetMaxAccelForward() * 3.0,robot_props->GetMaxAccelReverse() * 3.0);
		for (size_t i=0;i<4;i++)
		{
			m_Encoders[i].Initialize(&robot_props->GetDriveProps());
			m_Encoders[i].SetReverseDirection(m_SwerveRobotProps.EncoderReversed_Wheel[i]);
			m_Potentiometers[i].Initialize(&robot_props->GetSwivelProps());
			//TODO add reverse direction support for potentiometers
			//m_Potentiometers[i].SetReverseDirection(m_SwerveRobotProps.EncoderReversed_Swivel[i]);
		}
	}
}

void Swerve_Robot_Control::Reset_Encoders()
{
	//Yup... method driven ;)
	for (size_t i=0;i<Swerve_Robot::eNoSwerveRobotSpeedControllerDevices;i++)
		Reset_Rotary(i);
}

void Swerve_Robot_Control::Swerve_Drive_Control_TimeChange(double dTime_s)
{
	for (size_t i=0;i<4;i++)
	{
		m_Encoders[i].SetTimeDelta(dTime_s);
		m_Potentiometers[i].SetTimeDelta(dTime_s);
	}
	if (m_DisplayVoltage)
	{
		//display voltages
		DOUT2("fl=%.2f fr=%.2f rl=%.2f rr=%.2f\n",m_EncoderVoltage[Swerve_Robot::eWheel_FL],m_EncoderVoltage[Swerve_Robot::eWheel_FR],
			m_EncoderVoltage[Swerve_Robot::eWheel_RL],m_EncoderVoltage[Swerve_Robot::eWheel_RR]);
	}
}
void Swerve_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		case Swerve_Robot::eWheel_FR:
		case Swerve_Robot::eWheel_RL:
		case Swerve_Robot::eWheel_RR:
			m_Encoders[index].ResetPos();
			break;
		case Swerve_Robot::eSwivel_FL:
		case Swerve_Robot::eSwivel_FR:
		case Swerve_Robot::eSwivel_RL:
		case Swerve_Robot::eSwivel_RR:
			m_Potentiometers[index-4].ResetPos();
			break;
	}
}

double Swerve_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		case Swerve_Robot::eWheel_FR:
		case Swerve_Robot::eWheel_RL:
		case Swerve_Robot::eWheel_RR:
			result=m_Encoders[index].GetEncoderVelocity();
			break;
		case Swerve_Robot::eSwivel_FL:
		case Swerve_Robot::eSwivel_FR:
		case Swerve_Robot::eSwivel_RL:
		case Swerve_Robot::eSwivel_RR:
			result=NormalizeRotation2(m_Potentiometers[index-4].GetPotentiometerCurrentPosition());
			break;
	}
	return result;
}

void Swerve_Robot_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	switch (index)
	{
	case Swerve_Robot::eWheel_FL:
	case Swerve_Robot::eWheel_FR:
	case Swerve_Robot::eWheel_RL:
	case Swerve_Robot::eWheel_RR:
		//if (m_SlowWheel) Voltage=0.0;
		m_EncoderVoltage[index]=Voltage;
		m_Encoders[index].UpdateEncoderVoltage(Voltage);
		m_Encoders[index].TimeChange();

		break;
	case Swerve_Robot::eSwivel_FL:
	case Swerve_Robot::eSwivel_FR:
	case Swerve_Robot::eSwivel_RL:
	case Swerve_Robot::eSwivel_RR:
		{
			size_t i=index-4;
			m_PotentiometerVoltage[i]=Voltage;
			m_Potentiometers[i].UpdatePotentiometerVoltage(Voltage);
			m_Potentiometers[i].TimeChange();  //have this velocity immediately take effect
		}
		break;
	}
}

double Swerve_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_SwerveRobotProps.MotorToWheelGearRatio * M_PI * m_SwerveRobotProps.WheelDiameter; 
}


  /***************************************************************************************************************/
 /*													Wheel_UI													*/
/***************************************************************************************************************/

void Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Swivel=0.0;
	m_Rotation=0.0;
}

void Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);
	m_Front= new osgText::Text;
	m_Front->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Front->setCharacterSize(m_UIParent->GetFontSize());
	m_Front->setFontResolution(10,10);
	m_Front->setPosition(position);
	m_Front->setAlignment(osgText::Text::CENTER_CENTER);
	m_Front->setText(L"U");
	m_Front->setUpdateCallback(m_UIParent);

	m_Back= new osgText::Text;
	m_Back->setColor(osg::Vec4(1.0,0.0,0.0,1.0));
	m_Back->setCharacterSize(m_UIParent->GetFontSize());
	m_Back->setFontResolution(10,10);
	m_Back->setPosition(position);
	m_Back->setAlignment(osgText::Text::CENTER_CENTER);
	m_Back->setText(L"U");
	m_Back->setUpdateCallback(m_UIParent);

	m_Tread= new osgText::Text;
	m_Tread->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
	m_Tread->setCharacterSize(m_UIParent->GetFontSize());
	m_Tread->setFontResolution(10,10);
	m_Tread->setPosition(position);
	m_Tread->setAlignment(osgText::Text::CENTER_CENTER);
	//m_Tread->setText(L"\"");
	m_Tread->setText(L"__");
	m_Tread->setUpdateCallback(m_UIParent);

}

void Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
	{
		if (m_Front.valid()) geode->addDrawable(m_Front);
		if (m_Back.valid()) geode->addDrawable(m_Back);
		if (m_Tread.valid()) geode->addDrawable(m_Tread);
	}
	else
	{
		if (m_Front.valid()) geode->removeDrawable(m_Front);
		if (m_Back.valid()) geode->removeDrawable(m_Back);
		if (m_Tread.valid()) geode->removeDrawable(m_Tread);
	}
}

void Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	const double FS=m_UIParent->GetFontSize();
	Vec2d FrontSwivel(0.0,0.5);
	Vec2d BackSwivel(0.0,-0.5);
	//Vec2d TreadRotPos(0.0,cos(m_Rotation)-0.3);
	Vec2d TreadRotPos(sin(m_Rotation)*((fabs(m_Swivel)>PI_2)?0.5:-0.5),(cos(m_Rotation)*.8)+0.5);
	FrontSwivel=GlobalToLocal(m_Swivel,FrontSwivel);
	BackSwivel=GlobalToLocal(m_Swivel,BackSwivel);
	TreadRotPos=GlobalToLocal(m_Swivel,TreadRotPos);

	const Vec2d frontOffset(m_props.m_Offset[0]+FrontSwivel[0],m_props.m_Offset[1]+FrontSwivel[1]);
	const Vec2d backOffset(m_props.m_Offset[0]+BackSwivel[0],m_props.m_Offset[1]+BackSwivel[1]);
	const Vec2d TreadOffset(m_props.m_Offset[0]+TreadRotPos[0],m_props.m_Offset[1]+TreadRotPos[1]);

	const Vec2d FrontLocalOffset=GlobalToLocal(Heading,frontOffset);
	const Vec2d BackLocalOffset=GlobalToLocal(Heading,backOffset);
	const Vec2d TreadLocalOffset=GlobalToLocal(Heading,TreadOffset);
	const osg::Vec3 frontPos(parent_pos[0]+(FrontLocalOffset[0]*FS),parent_pos[1]+(FrontLocalOffset[1]*FS),parent_pos[2]);
	const osg::Vec3 backPos (parent_pos[0]+( BackLocalOffset[0]*FS),parent_pos[1]+( BackLocalOffset[1]*FS),parent_pos[2]);
	const osg::Vec3 TreadPos (parent_pos[0]+( TreadLocalOffset[0]*FS),parent_pos[1]+( TreadLocalOffset[1]*FS),parent_pos[2]);

	const double TreadColor=((sin(-m_Rotation) + 1.0)/2.0) * 0.8 + 0.2;
	m_Tread->setColor(osg::Vec4(TreadColor,TreadColor,TreadColor,1.0));

	if (m_Front.valid())
	{
		m_Front->setPosition(frontPos);
		m_Front->setRotation(FromLW_Rot_Radians(PI+Heading+m_Swivel,0.0,0.0));
	}
	if (m_Back.valid())
	{
		m_Back->setPosition(backPos);
		m_Back->setRotation(FromLW_Rot_Radians(Heading+m_Swivel,0.0,0.0));
	}
	if (m_Tread.valid())
	{
		m_Tread->setPosition(TreadPos);
		m_Tread->setRotation(FromLW_Rot_Radians(Heading+m_Swivel,0.0,0.0));
	}
}

void Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Front.valid())	m_Front->setCharacterSize(SizeToUse);
	if (m_Back.valid()) m_Back->setCharacterSize(SizeToUse);
	if (m_Tread.valid()) m_Tread->setCharacterSize(SizeToUse);
}

void Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

  /***************************************************************************************************************/
 /*												Swerve_Robot_UI													*/
/***************************************************************************************************************/
void Swerve_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].UI_Init(parent);
}

void Swerve_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	Vec2D Offsets[4]=
	{
		Vec2D(-1.6, 2.0),
		Vec2D( 1.6, 2.0),
		Vec2D(-1.6,-2.0),
		Vec2D( 1.6,-2.0),
	};
	for (size_t i=0;i<4;i++)
	{
		Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=m_SwerveRobot->GetSwerveRobotProps().WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
}

void Swerve_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<4;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-(m_SwerveRobot->GetAtt_r()));
}

void Swerve_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].Text_SizeToUse(SizeToUse);
}

void Swerve_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<4;i++)
		m_Wheel[i].UpdateScene(geode,AddOrRemove);
}

void Swerve_Robot_UI::TimeChange(double dTime_s)
{
	Swerve_Robot &_=*m_SwerveRobot;
	for (size_t i=0;i<4;i++)
	{
		//TODO GetIntendedVelocities for intended UI
		m_Wheel[i].SetSwivel(_.GetSwerveVelocitiesFromIndex(i+4));
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		const double LinearVelocity=_.GetSwerveVelocitiesFromIndex(i);
		const double PixelHackScale=m_Wheel[i].GetFontSize()/10.0;  //scale the wheels to be pixel aesthetic
		const double RPS=LinearVelocity /  (PI * _.GetSwerveRobotProps().WheelDiameter * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i].AddRotation(AngularVelocity*dTime_s);
	}
}
