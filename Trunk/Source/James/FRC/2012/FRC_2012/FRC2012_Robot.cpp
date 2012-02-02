#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/AI_Base_Controller.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2012_Robot.h"

using namespace Framework::Base;
using namespace std;

#define __OpenLoopAll__

const double Pi=M_PI;
const double Pi2=M_PI*2.0;
const double PI_2 = 1.57079632679489661923;

namespace Base=Framework::Base;

  /***********************************************************************************************************************************/
 /*														FRC_2012_Robot::Turret														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Turret::Turret(Rotary_Control_Interface *robot_control,FRC_2012_Robot *parent) : 
	Rotary_Linear("Turret",robot_control,eTurret),m_pParent(parent)
{
}

void FRC_2012_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//These should be disabled for closed loop... placed here for late binding (mostly for AI tester environment)
		#ifdef __OpenLoopAll__
		SetPotentiometerSafety(true);  //enable for auto open loop
		#endif
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Turret::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
}

void FRC_2012_Robot::Turret::TimeChange(double dTime_s)
{
	if ((m_pParent->m_IsTargeting)&&(IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer())
	{
		Vec2D Target=m_pParent->m_TargetOffset;
		Target-=m_pParent->GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);
		AngleToUse-=m_pParent->GetAtt_r();
		SetIntendedPosition(NormalizeRotation2(AngleToUse));
		//TODO factor in velocity once we have our ball velocity (to solve for time)
	}
	__super::TimeChange(dTime_s);
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
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//These should be disabled for closed loop... placed here for late binding (mostly for AI tester environment)
		#ifdef __OpenLoopAll__
		SetPotentiometerSafety(true);  //enable for auto open loop
		#endif
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

FRC_2012_Robot::PowerWheels::PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Angular("PowerWheels",robot_control,ePowerWheels,eActive),m_pParent(pParent),m_IsRunning(false)
{
}

void FRC_2012_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		//These should be disabled for closed loop... placed here for late binding (mostly for AI tester environment)
		#ifdef __OpenLoopAll__
		SetEncoderSafety(true);  //enable for auto open loop
		#endif
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
	if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested()))
	{
		//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
		//first get the range from 0 - 1
		double positive_range = (Velocity * 0.5) + 0.5;
		positive_range=positive_range>0.01?positive_range:0.0;
		const double minRange=20.0 * Pi;
		const double maxRange=40.0 * Pi;
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
 /*												FRC_2012_Robot::BallConveyorSystem													*/
/***********************************************************************************************************************************/

FRC_2012_Robot::BallConveyorSystem::BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_LowerConveyor("LowerConveyor",robot_control,eLowerConveyor),m_MiddleConveyor("MiddleConveyor",robot_control,eMiddleConveyor),
	m_FireConveyor("FireConveyor",robot_control,eFireConveyor),
		m_Grip(false),m_Squirt(false),m_Fire(false)
{
	//This are always open loop as there is no encoder and this is specified by default
}

void FRC_2012_Robot::BallConveyorSystem::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_LowerConveyor.Initialize(em,props);
	m_MiddleConveyor.Initialize(em,props);
	m_FireConveyor.Initialize(em,props);
}

void FRC_2012_Robot::BallConveyorSystem::TimeChange(double dTime_s)
{
	const bool LowerSensor=m_pParent->m_RobotControl->GetBoolSensorState(eLowerConveyor_Sensor);
	const bool MiddleSensor=m_pParent->m_RobotControl->GetBoolSensorState(eMiddleConveyor_Sensor);
	const bool FireSensor=m_pParent->m_RobotControl->GetBoolSensorState(eFireConveyor_Sensor);
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetRotary_Properties().PrecisionTolerance;
	//Only fire when the wheel has reached its aiming speed
	bool Fire=m_Fire && PowerWheelReachedTolerance;

	//This assumes the motors are in the same orientation: 
	double LowerAcceleration=((m_Grip & (!LowerSensor)) || (LowerSensor & (!MiddleSensor))) | m_Squirt | Fire ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_LowerConveyor.SetCurrentLinearAcceleration(LowerAcceleration);

	double MiddleAcceleration= ((LowerSensor & (!MiddleSensor)) || (MiddleSensor & (!FireSensor))) |  m_Squirt | Fire  ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_MiddleConveyor.SetCurrentLinearAcceleration(MiddleAcceleration);

	double FireAcceleration= (MiddleSensor & (!FireSensor)) | m_Squirt | Fire  ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_FireConveyor.SetCurrentLinearAcceleration(FireAcceleration);

	m_LowerConveyor.AsEntity1D().TimeChange(dTime_s);
	m_MiddleConveyor.AsEntity1D().TimeChange(dTime_s);
	m_FireConveyor.AsEntity1D().TimeChange(dTime_s);
}

//This is the manual override, but probably not used if we use spike as it would be wasteful to have a analog control for this
void FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_LowerConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_MiddleConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_FireConveyor.SetRequestedVelocity_FromNormalized(Velocity);
}

void FRC_2012_Robot::BallConveyorSystem::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_MiddleConveyor.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//Ball_SetCurrentVelocity is the manual override
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/

//TODO get the physical measurements of the game for this
const FRC_2012_Robot::Vec2D c_TargetBasePosition=FRC_2012_Robot::Vec2D(0.0,3.0);
const double c_TargetBaseHeight=2.0;

FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control), m_Turret(robot_control,this),m_PitchRamp(robot_control),
		m_PowerWheels(this,robot_control),m_BallConveyorSystem(this,robot_control),m_IsTargeting(false)
{
	m_IsTargeting=true;  //testing
}

void FRC_2012_Robot::Initialize(Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=static_cast<const FRC_2012_Robot_Properties *>(props);
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
	m_BallConveyorSystem.Initialize(em,RobotProps?&RobotProps->GetConveyorProps():NULL);
}
void FRC_2012_Robot::ResetPos()
{
	__super::ResetPos();
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	m_PowerWheels.ResetPos();
	m_BallConveyorSystem.ResetPos();
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	m_TargetOffset=c_TargetBasePosition;
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.AsEntity1D().TimeChange(dTime_s);
	m_PitchRamp.AsEntity1D().TimeChange(dTime_s);
	m_PowerWheels.AsEntity1D().TimeChange(dTime_s);
	m_BallConveyorSystem.TimeChange(dTime_s);
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

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_PowerWheels.BindAdditionalEventControls(Bind);
	m_BallConveyorSystem.BindAdditionalEventControls(Bind);
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
	DEG_2_RAD(45-3),DEG_2_RAD(65+3) //add padding for quick response time (as close to limits will slow it down)
	),
	m_PowerWheelProps(
	"PowerWheels",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	40 * Pi,   //Max Speed (rounded as we need not have precision)
	60.0,60.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	60.0,60.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Properties::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	),
	m_ConveyorProps(
	"Conveyor",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Properties::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	)

{
	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		//Was originally 0.4953 19.5 width for 2011
		//Now is 0.517652 20.38 according to Parker  (not too worried about the length)
		props.WheelDimensions=Vec2D(0.517652,0.6985); //27.5 x 20.38
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
	{
		Rotary_Props props=m_ConveyorProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.01; //we need good precision
		m_ConveyorProps.RoteryProps()=props;
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2012_UI_Controller														*/
/***********************************************************************************************************************************/
#define __2011Joysticks__
#undef __AirFlo__
#undef __UsingXTerminator__

FRC_2012_UI_Controller::FRC_2012_UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller) : UI_Controller(joy,base_controller)
{
	using namespace Framework::UI;
	#if 0
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Joystick_SetCurrentSpeed_2",true,1.0,0.1,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",false,1.0,0.1,true,"Joystick_1");
	#endif

	#ifdef __2011Joysticks__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Turret_SetCurrentVelocity",false,1.0,0.1,true,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"PowerWheels_SetCurrentVelocity",false,1.0,0.1,false,"Joystick_2");
	
	joy.AddJoy_Button_Default( 0,"Ball_Fire",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Ball_Grip",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 6,"Ball_Squirt",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 10,"PowerWheels_IsRunning",true,false,"Joystick_2");
	#endif
}
