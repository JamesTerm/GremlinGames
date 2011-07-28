#define __EncoderHack__
#undef  __ShowPotentiometerReadings__
#undef  __ShowEncoderReadings__

#include "WPILib.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
#include "Robot_Tank.h"
#include "AI_Base_Controller.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "PIDController.h"
#include "InOut_Interface.h"
#include "FRC2011_Robot.h"

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*														Driver_Station_Joystick														*/
/***********************************************************************************************************************************/

size_t Driver_Station_Joystick::GetNoJoysticksFound() 
{
	return m_NoJoysticks;
}

bool Driver_Station_Joystick::read_joystick (size_t nr, JoyState &Info)
{
	//First weed out numbers not in range
	int Number=(int)nr;
	Number-=m_StartingPort;
	bool ret=false;
	nr++;  //DOH the number selection is cardinal!  :(
	if ((Number>=0) && (Number<m_NoJoysticks))
	{
		memset(&Info,0,sizeof(JoyState));  //zero the memory
		//The axis selection is also ordinal
		Info.lX=m_ds->GetStickAxis(nr,1);
		Info.lY=m_ds->GetStickAxis(nr,2);
		Info.lZ=m_ds->GetStickAxis(nr,3);
		Info.lRx=m_ds->GetStickAxis(nr,4);
		Info.lRy=m_ds->GetStickAxis(nr,5);
		Info.ButtonBank[0]=m_ds->GetStickButtons(nr);
		ret=true;
	}
	return ret;
}

Driver_Station_Joystick::Driver_Station_Joystick(int NoJoysticks,int StartingPort) : m_NoJoysticks(NoJoysticks), m_StartingPort(StartingPort)
{
	m_ds = DriverStation::GetInstance();
	Framework::Base::IJoystick::JoystickInfo common;
	common.ProductName="Joystick_1";
	common.InstanceName="Driver_Station";
	common.JoyCapFlags=
		JoystickInfo::fX_Axis|JoystickInfo::fY_Axis|JoystickInfo::fZ_Axis|
		JoystickInfo::fX_Rot|JoystickInfo::fY_Rot;
	common.nSliderCount=0;
	common.nPOVCount=0;
	common.nButtonCount=12;
	common.bPresent=true;
	m_JoyInfo.push_back(common);
	//Go ahead and add other inputs
	common.ProductName="Joystick_2";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_3";
	m_JoyInfo.push_back(common);
	common.ProductName="Joystick_4";
	m_JoyInfo.push_back(common);
}

Driver_Station_Joystick::~Driver_Station_Joystick()
{
}

  /***********************************************************************************************************************************/
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

void Robot_Control::SetSafety(bool UseSafety)
{
	if (UseSafety)
	{
		//I'm giving a whole second before the timeout kicks in... I do not want false positives!
		m_RobotDrive.SetExpiration(1.0);
		m_RobotDrive.SetSafetyEnabled(true);
	}
	else
		m_RobotDrive.SetSafetyEnabled(false);
}


void Robot_Control::ResetPos()
{
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(7))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
	//m_Camera=&AxisCamera::GetInstance();
	//Wait(8.000);
}

Robot_Control::Robot_Control(bool UseSafety) : m_RobotDrive(1,2,3,4),m_ArmMotor(5,6),m_Compress(5,2),
	m_OnElbow(6),m_OffElbow(5),m_OnClaw(4),m_OffClaw(3),m_OnDeploy(2),m_OffDeploy(1),
	m_LeftEncoder(3,4),m_RightEncoder(1,2),m_Potentiometer(1),m_Camera(NULL)
{
	ResetPos();
	SetSafety(UseSafety);
	const double EncoderPulseRate=(1.0/360.0);
	m_LeftEncoder.SetDistancePerPulse(EncoderPulseRate),m_RightEncoder.SetDistancePerPulse(EncoderPulseRate);
	m_LeftEncoder.Start(),m_RightEncoder.Start();
}

Robot_Control::~Robot_Control() 
{
	m_LeftEncoder.Stop(),m_RightEncoder.Stop();  //TODO Move for autonomous mode only
	m_RobotDrive.SetSafetyEnabled(false);
	m_Compress.Stop();
	m_Camera=NULL;  //We don't own this, but I do wish to treat it like we do
}

void Robot_Control::Reset_Arm()
{
	m_KalFilter_Arm.Reset();
}

void Robot_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=static_cast<const FRC_2011_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
}

void Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=0.0,RightVelocity=0.0;
	double LeftRate=m_LeftEncoder.GetRate();
	//LeftRate=m_KalFilter_EncodeLeft(LeftRate);
	double RightRate=m_RightEncoder.GetRate();
	//RightRate=m_KalFilter_EncodeRight(RightRate);
	LeftVelocity=FRC_2011_Robot::RPS_To_LinearVelocity(LeftRate);
	RightVelocity=FRC_2011_Robot::RPS_To_LinearVelocity(RightRate);
	#ifdef __EncoderHack__
	LeftVelocity=RightVelocity;  //Unfortunately the left encoder is not working remove once 
	#endif
	#ifdef __ShowEncoderReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line4, "l=%.1f r=%.1f", LeftVelocity,RightVelocity);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line4, "l=%.1f r=%.1f", m_LeftEncoder.GetRate()/3.0,m_RightEncoder.GetRate()/3.0);	
	#endif
}

const double c_rMotorDriveForward_DeadZone=0.110;
const double c_rMotorDriveReverse_DeadZone=0.04;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.115;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	#if 0
	float right=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
	float left=DriverStation::GetInstance()->GetAnalogIn(2) - 1.0;
	m_RobotDrive.SetLeftRightMotorOutputs(right,left);
	return;
	#endif

	#if 1
	//Eliminate the deadzone
	if (LeftVoltage>0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveForward_Range) + c_lMotorDriveForward_DeadZone;
	else if (LeftVoltage < 0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveReverse_Range) - c_lMotorDriveReverse_DeadZone;

	if (RightVoltage>0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveForward_Range) + c_rMotorDriveForward_DeadZone;
	else if (RightVoltage < 0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveReverse_Range) - c_rMotorDriveReverse_DeadZone;
	#endif
	
	//Unfortunately the actual wheels are reversed
	#ifdef __ShowEncoderReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line3, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);
	//printf("l=%.1f r=%.1f\n", LeftVoltage,RightVoltage);
	#endif

	m_RobotDrive.SetLeftRightMotorOutputs((float)(RightVoltage),(float)(LeftVoltage));
}

//const double c_Arm_DeadZone=0.150;  //was 0.085 for cut off
const double c_Arm_DeadZone=0.085;  //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

void Robot_Control::UpdateArmVoltage(double Voltage)
{
	#if 0
	float ToUse=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
	m_ArmMotor.SetLeftRightMotorOutputs(ToUse,ToUse);
	return;
	#endif

	//TODO determine why the deadzone code has adverse results
	//Eliminate the deadzone
	//Voltage=(Voltage * c_Arm_Range) + ((Voltage>0.0) ? c_Arm_DeadZone : -c_Arm_DeadZone);
	
	//This prevents the motor from over heating when it is close enough to its destination
	//if (fabs(Voltage)<=c_Arm_DeadZone)
	//	Voltage=0.0;
	
	//DOUT4("Arm=%f",Velocity/m_ArmMaxSpeed);
	//Note: client code needs to check the levels are correct!
	m_ArmMotor.SetLeftRightMotorOutputs(Voltage,Voltage);  //always the same velocity for both!
	#ifdef __ShowPotentiometerReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->PrintfLine(DriverStationLCD::kUser_Line4, "ArmVolt=%f ", Voltage);
	#endif
}

double Robot_Control::GetArmCurrentPosition()
{	
	double raw_value = (double)m_Potentiometer.GetAverageValue();
	//raw_value = m_KalFilter_Arm(raw_value);  //apply the Kalman filter
	//Note the value is inverted with the negative operator
	double ret=-FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(raw_value);
	//I may keep these on as they should be useful feedback
	#ifdef __ShowPotentiometerReadings__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	double height=FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(ret);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %.1fft %.1fin", RAD_2_DEG(ret),height*3.2808399,height*39.3700787);
	lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %f %.1fft ", RAD_2_DEG(ret),height,height*3.2808399);
	//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: Pot=%.1f ", raw_value);
	#endif
	return ret;
}




  /***********************************************************************************************************************************/
 /*														Robot_Control_2011															*/
/***********************************************************************************************************************************/


void Robot_Control_2011::CloseSolenoid(size_t index,bool Close)
{
	//virtual void OpenDeploymentDoor(bool Open) {m_DeployDoor.SetAngle(Open?Servo::GetMaxAngle():Servo::GetMinAngle());}
	//virtual void ReleaseLazySusan(bool Release) {m_LazySusan.SetAngle(Release?Servo::GetMaxAngle():Servo::GetMinAngle());}

	switch (index)
	{
		case FRC_2011_Robot::eDeployment:
			printf("CloseDeploymentDoor=%d\n",Close);
			m_OnDeploy.Set(Close),m_OffDeploy.Set(!Close);
			break;
		case FRC_2011_Robot::eClaw:
			printf("CloseClaw=%d\n",Close);
			m_OnClaw.Set(Close),m_OffClaw.Set(!Close);
			break;
		case FRC_2011_Robot::eElbow:
			printf("CloseElbow=%d\n",Close);
			m_OnElbow.Set(Close),m_OffElbow.Set(!Close);
			break;
	}
}






