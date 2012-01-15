#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "FRC2012_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*														FRC_2012_Robot::Turret														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Turret::Turret(Rotary_Control_Interface *robot_control) : Rotary_Linear("Turret",robot_control,eTurret)
{
}

void FRC_2012_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	GG_Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::PitchRamp::PitchRamp(Rotary_Control_Interface *robot_control) : Rotary_Linear("PitchRamp",robot_control,ePitchRamp)
{
}

void FRC_2012_Robot::PitchRamp::SetIntendedPosition(double Position)
{
	//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
	//first get the range from 0 - 1
	double positive_range = (Position * 0.5) + 0.5;
	//positive_range=positive_range>0.01?positive_range:0.0;
	const double minRange=DEG_2_RAD(45);
	const double maxRange=DEG_2_RAD(65);
	const double Scale=(maxRange-minRange) / maxRange;
	Position=(positive_range * Scale) + minRange;

	//DOUT5("Test=%f",RAD_2_DEG(Position));
	__super::SetIntendedPosition(Position);
}

void FRC_2012_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	GG_Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::PowerWheels														*/
/***********************************************************************************************************************************/

FRC_2012_Robot::PowerWheels::PowerWheels(Rotary_Control_Interface *robot_control) : Rotary_Angular("PowerWheels",robot_control,ePowerWheels),
	m_IsRunning(false)
{
}

void FRC_2012_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	GG_Framework::Base::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
	else
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
}

void FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized(double Velocity) 
{
	if (m_IsRunning)
	{
		//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
		//first get the range from 0 - 1
		double positive_range = (Velocity * 0.5) + 0.5;
		positive_range=positive_range>0.01?positive_range:0.0;
		const double minRange=10.0 * Pi;
		const double maxRange=20.0 * Pi;
		const double Scale=(maxRange-minRange) / MAX_SPEED;
		const double Offset=minRange/MAX_SPEED;
		Velocity=(positive_range * Scale) + Offset;
		//DOUT5("%f",Velocity);
		__super::SetRequestedVelocity_FromNormalized(Velocity);
	}
	else
		__super::SetRequestedVelocity_FromNormalized(0.0);
}

  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/
FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control), m_Turret(robot_control),m_PitchRamp(robot_control),
		m_PowerWheels(robot_control)
{
}

void FRC_2012_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
}
void FRC_2012_Robot::ResetPos()
{
	__super::ResetPos();
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	m_PowerWheels.ResetPos();
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	Entity1D &turret_entity=m_Turret;  //This gets around keeping time change protected in derived classes
	turret_entity.TimeChange(dTime_s);
	Entity1D &pitch_entity=m_PitchRamp;
	pitch_entity.TimeChange(dTime_s);
	Entity1D &pw_entity=m_PowerWheels;
	pw_entity.TimeChange(dTime_s);
}

const double c_rMotorDriveForward_DeadZone=0.02;
const double c_rMotorDriveReverse_DeadZone=0.02;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.02;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void FRC_2012_Robot::ComputeDeadZone(double &LeftVoltage,double &RightVoltage)
{
	//Eliminate the deadzone
	if (LeftVoltage>0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveForward_Range) + c_lMotorDriveForward_DeadZone;
	else if (LeftVoltage < 0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveReverse_Range) - c_lMotorDriveReverse_DeadZone;

	if (RightVoltage>0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveForward_Range) + c_rMotorDriveForward_DeadZone;
	else if (RightVoltage < 0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveReverse_Range) - c_rMotorDriveReverse_DeadZone;
}


void FRC_2012_Robot::BindAdditionalEventControls(bool Bind)
{
	//Entity2D::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	//if (Bind)
	//	em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2012_Robot::CloseDeploymentDoor);
	//else
	//	em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2012_Robot::CloseDeploymentDoor);

	Ship_1D &TurretEntity_Access=m_Turret;
	TurretEntity_Access.BindAdditionalEventControls(Bind);
	Ship_1D &PitchEntity_Access=m_PitchRamp;
	PitchEntity_Access.BindAdditionalEventControls(Bind);
	Ship_1D &pw_Access=m_PowerWheels;
	pw_Access.BindAdditionalEventControls(Bind);
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			{
				//	printf("Arm=%f\n",Voltage);
				//DOUT3("Arm Voltage=%f",Voltage);
				m_TurretVoltage=Voltage;
				m_Turret_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Turret_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePitchRamp:
			{
				//	printf("Arm=%f\n",Voltage);
				//DOUT3("Arm Voltage=%f",Voltage);
				m_PitchRampVoltage=Voltage;
				m_Pitch_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Pitch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePowerWheels:
			m_PowerWheelVoltage=Voltage;
			m_PowerWheel_Enc.UpdateEncoderVoltage(Voltage);
			m_PowerWheel_Enc.TimeChange();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
	}
}

FRC_2012_Robot_Control::FRC_2012_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_TurretVoltage(0.0),m_PowerWheelVoltage(0.0)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
}

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	m_KalFilter_Arm.Reset();
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);

	assert(robot_props);

	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
	m_Turret_Pot.Initialize(&robot_props->GetTurretProps());
	m_Pitch_Pot.Initialize(&robot_props->GetPitchRampProps());
	m_PowerWheel_Enc.Initialize(&robot_props->GetPowerWheelProps());
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Turret_Pot.SetTimeDelta(dTime_s);
	m_Pitch_Pot.SetTimeDelta(dTime_s);
	m_PowerWheel_Enc.SetTimeDelta(dTime_s);
	//display voltages
	DOUT2("l=%f r=%f t=%f pi=%f pw=%f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
		m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage);
}


double FRC_2012_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
		
			result=m_Turret_Pot.GetPotentiometerCurrentPosition();
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			//DOUT4 ("pot=%f",result);
			break;
		case FRC_2012_Robot::ePitchRamp:

			result=m_Pitch_Pot.GetPotentiometerCurrentPosition();
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			DOUT4 ("pitch=%f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePowerWheels:
			result=m_PowerWheel_Enc.GetEncoderVelocity();
			//DOUT4 ("vel=%f",result);
			break;
	}
	return result;
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2012_Robot_Properties::FRC_2012_Robot_Properties()  : m_TurretProps(
	"Turret",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eSwivel,
	true,	//Using the range
	-Pi,Pi
	),
	m_PitchRampProps(
	"Pitch",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eRobotArm,
	true,	//Using the range
	DEG_2_RAD(45),DEG_2_RAD(65)
	),
	m_PowerWheelProps(
	"PowerWheels",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	20 * PI,   //Max Speed (rounded as we need not have precision)
	60.0,60.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	60.0,60.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Properties::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	)
{
	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
	}
	{
		Rotary_Props props=m_TurretProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_TurretProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PitchRampProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_PitchRampProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PowerWheelProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.01; //we need good precision
		m_PowerWheelProps.RoteryProps()=props;
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2012_Turret_UI															*/
/***********************************************************************************************************************************/

void FRC_2012_Turret_UI::Initialize(Entity2D::EventMap& em, const Turret_Properties *props)
{
	if (props)
		m_props=*props;
	else
		m_props.YOffset=2.0;
}
void FRC_2012_Turret_UI::UI_Init(Actor_Text *parent)
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Turret= new osgText::Text;
	m_Turret->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Turret->setCharacterSize(m_UIParent->GetFontSize());
	m_Turret->setFontResolution(10,10);
	m_Turret->setPosition(position);
	m_Turret->setAlignment(osgText::Text::CENTER_CENTER);
	m_Turret->setText(L"\\/\n|\n||\n||\n( )");
	m_Turret->setUpdateCallback(m_UIParent);
}
void FRC_2012_Turret_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	FRC_2012_Control_Interface *turret_access=m_RobotControl;
	double HeadingToUse=Heading+(-turret_access->GetRotaryCurrentPorV(FRC_2012_Robot::eTurret));
	const double FS=m_UIParent->GetFontSize();
	const Vec2d TurretOffset(0,m_props.YOffset);
	const Vec2d TurretLocalOffset=GlobalToLocal(HeadingToUse ,TurretOffset);
	const osg::Vec3 TurretPos (parent_pos[0]+( TurretLocalOffset[0]*FS),parent_pos[1]+( TurretLocalOffset[1]*FS),parent_pos[2]);

	//const char *TeamName=m_UIParent->GetEntityProperties_Interface()->GetTeamName();
	//if (strcmp(TeamName,"red")==0)
	//	m_Turret->setColor(osg::Vec4(1.0f,0.0f,0.5f,1.0f));  //This is almost magenta (easier to see)
	//else if (strcmp(TeamName,"blue")==0)
	//	m_Turret->setColor(osg::Vec4(0.0f,0.5f,1.0f,1.0f));  //This is almost cyan (easier to see too)

	if (m_Turret.valid())
	{
		m_Turret->setPosition(TurretPos);
		m_Turret->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}

}
void FRC_2012_Turret_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Turret.valid()) m_Turret->setCharacterSize(SizeToUse);
}
void FRC_2012_Turret_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Turret.valid()) geode->addDrawable(m_Turret);
	else
		if (m_Turret.valid()) geode->removeDrawable(m_Turret);
}
  /***************************************************************************************************************/
 /*											FRC_2012_Power_Wheel_UI												*/
/***************************************************************************************************************/

void FRC_2012_Power_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	if (props)
		m_props=*props;
	else
		m_props.m_Offset=Vec2d(0,0);
}

void FRC_2012_Power_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Wheel= new osgText::Text;
	m_Wheel->setColor(osg::Vec4(1.0,0.0,0.5,1.0));
	m_Wheel->setCharacterSize(m_UIParent->GetFontSize());
	m_Wheel->setFontResolution(10,10);
	m_Wheel->setPosition(position);
	m_Wheel->setAlignment(osgText::Text::CENTER_CENTER);
	m_Wheel->setText(L"|");
	m_Wheel->setUpdateCallback(m_UIParent);
}

void FRC_2012_Power_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Wheel.valid()) geode->addDrawable(m_Wheel);
	else
		if (m_Wheel.valid()) geode->removeDrawable(m_Wheel);
}

void FRC_2012_Power_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	double HeadingToUse=Heading+m_Rotation;
	const double FS=m_UIParent->GetFontSize();
	const Vec2d WheelOffset(m_props.m_Offset[0],m_props.m_Offset[1]);
	const Vec2d WheelLocalOffset=GlobalToLocal(Heading,WheelOffset);
	const osg::Vec3 WheelPos (parent_pos[0]+( WheelLocalOffset[0]*FS),parent_pos[1]+( WheelLocalOffset[1]*FS),parent_pos[2]);

	if (m_Wheel.valid())
	{
		m_Wheel->setPosition(WheelPos);
		m_Wheel->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}
}

void FRC_2012_Power_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Wheel.valid()) m_Wheel->setCharacterSize(SizeToUse);
}

void FRC_2012_Power_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

void FRC_2012_Power_Wheel_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double NormalizedVelocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::ePowerWheels) / (20.0 * Pi);
	NormalizedVelocity-=0.2;
	if (NormalizedVelocity<0.0)
		NormalizedVelocity=0.0;

	//Scale down the rotation to something easy to gauge in UI
	AddRotation((NormalizedVelocity * 18) * dTime_s);
}

  /***************************************************************************************************************/
 /*												FRC_2012_Robot_UI												*/
/***************************************************************************************************************/

FRC_2012_Robot_UI::FRC_2012_Robot_UI(const char EntityName[]) : FRC_2012_Robot(EntityName,this),FRC_2012_Robot_Control(),
		m_TankUI(this),m_TurretUI(this),m_PowerWheelUI(this)
{
}

void FRC_2012_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
	m_PowerWheelUI.TimeChange(dTime_s);
}
void FRC_2012_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
	m_TurretUI.Initialize(em);
	m_PowerWheelUI.Initialize(em);
}

void FRC_2012_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
	m_TurretUI.UI_Init(parent);
	m_PowerWheelUI.UI_Init(parent);
}
void FRC_2012_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
	m_TurretUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_PowerWheelUI.update(nv,draw,parent_pos,-GetAtt_r());
}
void FRC_2012_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
	m_TurretUI.Text_SizeToUse(SizeToUse);
	m_PowerWheelUI.Text_SizeToUse(SizeToUse);
}
void FRC_2012_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
	m_TurretUI.UpdateScene(geode,AddOrRemove);
	m_PowerWheelUI.UpdateScene(geode,AddOrRemove);
}
