#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "FRC2011_Robot.h"
	#include "Swerve_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);  //We may limit swerve to this
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*													Swerve_Robot::DrivingModule														*/
/***********************************************************************************************************************************/

Swerve_Robot::DrivingModule::DrivingModule(const char EntityName[],Robot_Control_Interface *robot_control) : m_ModuleName(EntityName),
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
	m_Drive.SetIntendedPosition(m_IntendedDriveVelocity);

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
Swerve_Robot::Swerve_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders) : 
	Swerve_Drive(EntityName), m_RobotControl(robot_control), 
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

	m_WheelDimensions=RobotProps->GetWheelDimensions();
	for (size_t i=0;i<4;i++)
	{
		DrivingModule::DrivingModule_Props props;
		props.Swivel_Props=&RobotProps->GetSwivelProps();
		props.Drive_Props=&RobotProps->GetDriveProps();
		m_DrivingModule[i]->Initialize(em,&props);
	}
}
void Swerve_Robot::ResetPos()
{
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
		double SwivelDirection=GetIntendedVelocitiesFromIndex(i+4);  //this is either the intended direction or the reverse of it
		const Ship_1D &Swivel=m_DrivingModule[i]->GetSwivel();
		const double LastSwivelDirection=Swivel.GetPos_m();
		double DistanceToIntendedSwivel=LastSwivelDirection-SwivelDirection;
		NormalizeRotation(DistanceToIntendedSwivel);
		DistanceToIntendedSwivel=fabs(DistanceToIntendedSwivel);

		bool IsReverse=false;
		//TODO see if I can get range to work
		//if ((DistanceToIntendedSwivel>PI_2) || (SwivelDirection>Swivel.GetMaxRange()) || (SwivelDirection<Swivel.GetMinRange()))
		if (DistanceToIntendedSwivel>PI_2)
		//if (false)
		{
			SwivelDirection+=PI;
			NormalizeRotation(SwivelDirection);
			IsReverse=true;
			//recalculate with the reversed value
			DistanceToIntendedSwivel=LastSwivelDirection-SwivelDirection;
		}
		m_DrivingModule[i]->SetIntendedSwivelDirection(SwivelDirection);
		const double IntendedSpeed=GetIntendedVelocitiesFromIndex(i);
		double VelocityToUse=IsReverse?-IntendedSpeed:IntendedSpeed;

		//To minimize error only apply the Y component amount to the velocity
		//The less the difference between the current and actual swivel direction the greater the full amount can be applied
		//restrict to half pi to avoid negative feedback
		VelocityToUse=DistanceToIntendedSwivel<PI_2?cos(DistanceToIntendedSwivel)*VelocityToUse:0.0;

		m_DrivingModule[i]->SetIntendedDriveVelocity(VelocityToUse);
		m_DrivingModule[i]->TimeChange(dTime_s);

		const double CurrentVelocity=m_DrivingModule[i]->GetDrive().GetPos_m();
		const double CurrentSwivelDirection=Swivel.GetPos_m();
		if (i==0)
			DOUT4("S= %f %f V= %f %f",CurrentSwivelDirection,SwivelDirection,CurrentVelocity,VelocityToUse);

		//Now to grab and update the actual swerve velocities
		//Note: using GetIntendedVelocities() is a lesser stress for debug purposes
		#if 0
		m_Swerve_Robot_Velocities.Velocity.AsArray[i+4]=CurrentSwivelDirection;
		m_Swerve_Robot_Velocities.Velocity.AsArray[i]=CurrentVelocity;
		#else
		m_Swerve_Robot_Velocities=GetIntendedVelocities();
		#endif
	}

	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
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
	bool ret=false;
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
	return ret;
}

double Swerve_Robot::RPS_To_LinearVelocity(double RPS)
{
	return RPS * c_MotorToWheelGearRatio * M_PI * c_WheelDiameter; 
}

//void Swerve_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
//{
//	m_VoltageOverride=false;
//	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
//			m_VoltageOverride=true;
//}

void Swerve_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	//double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	//m_RobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);
}

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot_Properties														*/
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
	false, //Seems I cannot get the range to work properly
	-DEG_2_RAD(130.0),DEG_2_RAD(130.0)
	),
	m_DriveProps(
	"Drive",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//These should match the settings in the script
	2.916,   //Max Speed (This is linear movement speed)
	10.0,10.0, //ACCEL, BRAKE  (These can be ignored)
	300.0,300.0, //Max Acceleration Forward/Reverse (make these as fast as possible without damaging chain or motor)
	Ship_1D_Properties::eSimpleMotor,
	false	//No limit ever!
	),
	m_WheelDimensions(0.6477,0.9525)
{
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
	m_Tread->setText(L"\"");
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
	Vec2d TreadRotPos(0.0,cos(m_Rotation)-0.3);
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
		props.m_Wheel_Diameter=c_WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
	__super::Initialize(em,props);
}

void Swerve_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<4;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-GetAtt_r());
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
	__super::TimeChange(dTime_s);
	for (size_t i=0;i<4;i++)
	{
		//TODO GetIntendedVelocities for intended UI
		m_Wheel[i].SetSwivel(GetSwerveVelocitiesFromIndex(i+4));
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		const double LinearVelocity=GetSwerveVelocitiesFromIndex(i);
		const double PixelHackScale=m_Wheel[i].GetFontSize()/10.0;  //scale the wheels to be pixel aesthetic
		const double RPS=LinearVelocity /  (PI * c_WheelDiameter * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i].AddRotation(AngularVelocity*dTime_s);
	}
}
