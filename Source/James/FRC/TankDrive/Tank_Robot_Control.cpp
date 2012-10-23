
#include "WPILib.h"

#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "../Base/Script.h"
#include "../Common/Entity_Properties.h"
#include "../Common/Physics_1D.h"
#include "../Common/Physics_2D.h"
#include "../Common/Entity2D.h"
#include "../Common/Goal.h"
#include "../Common/Ship_1D.h"
#include "../Common/Ship.h"
#include "../Common/Vehicle_Drive.h"
#include "../Common/PIDController.h"
#include "../Common/AI_Base_Controller.h"
#include "Tank_Robot.h"
#include "../Base/Joystick.h"
#include "../Base/JoystickBinder.h"
#include "../Common/InOut_Interface.h"
#include "Tank_Robot_Control.h"
#include "../Common/Robot_Control_Interface.h"
#include "../Common/UI_Controller.h"
#include "../Common/PIDController.h"
#include "../Common/InOut_Interface.h"
#include "../Common/Debug.h"

using namespace Framework::Base;
#undef __DisableTankDrive__

  /***********************************************************************************************************************************/
 /*														Tank_Robot_Control															*/
/***********************************************************************************************************************************/


void Tank_Robot_Control::SetSafety(bool UseSafety)
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

Tank_Robot_Control::Tank_Robot_Control(bool UseSafety) :
	m_1(1),m_2(2),m_3(3),m_4(4),
	m_RobotDrive(&m_1,&m_2,&m_3,&m_4),
	//m_RobotDrive(1,2,3,4),  //default Jaguar instantiation
	m_LeftEncoder(3,4),m_RightEncoder(1,2),m_dTime_s(0.0)
{
	//ResetPos();  may need this later
	SetSafety(UseSafety);
	const double EncoderPulseRate=(1.0/360.0);
	m_LeftEncoder.SetDistancePerPulse(EncoderPulseRate),m_RightEncoder.SetDistancePerPulse(EncoderPulseRate);
	m_LeftEncoder.Start(),m_RightEncoder.Start();
}

Tank_Robot_Control::~Tank_Robot_Control() 
{
	m_LeftEncoder.Stop(),m_RightEncoder.Stop();  //TODO Move for autonomous mode only
	m_RobotDrive.SetSafetyEnabled(false);
}

void Tank_Robot_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Tank_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Tank_Robot_Properties *robot_props=static_cast<const Tank_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	//This will copy all the props
	m_TankRobotProps=robot_props->GetTankRobotProps();
	m_LeftEncoder.SetReverseDirection(m_TankRobotProps.LeftEncoderReversed);
	m_RightEncoder.SetReverseDirection(m_TankRobotProps.RightEncoderReversed);
}

double Tank_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_TankRobotProps.MotorToWheelGearRatio * M_PI * m_TankRobotProps.WheelDiameter; 
}

void Tank_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=0.0,RightVelocity=0.0;
	//double LeftRate=m_LeftEncoder.GetRate2(m_dTime_s);
	double LeftRate=m_LeftEncoder.GetRate();
	LeftRate=m_KalFilter_EncodeLeft(LeftRate);
	LeftRate=m_Averager_EncoderLeft.GetAverage(LeftRate);
	LeftRate=IsZero(LeftRate)?0.0:LeftRate;

	//double RightRate=m_RightEncoder.GetRate2(m_dTime_s);
	double RightRate=m_RightEncoder.GetRate();
	RightRate=m_KalFilter_EncodeRight(RightRate);
	RightRate=m_Averager_EncodeRight.GetAverage(RightRate);
	RightRate=IsZero(RightRate)?0.0:RightRate;
	
	//Quick test of using GetRate() vs. GetRate2()
	#if 0
	if ((LeftRate>0.0)||(RightRate>0.0))
		printf("l1=%.1f l2=%.1f r1=%.1f r2=%.1f\n",m_LeftEncoder.GetRate(),LeftRate,m_RightEncoder.GetRate(),RightRate);
	#endif
	
	LeftVelocity=RPS_To_LinearVelocity(LeftRate);
	RightVelocity=RPS_To_LinearVelocity(RightRate);
	Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", LeftVelocity,RightVelocity);
	//Dout(m_TankRobotProps.Feedback_DiplayRow, "l=%.1f r=%.1f", m_LeftEncoder.GetRate()/3.0,m_RightEncoder.GetRate()/3.0);
}

//This is kept simple and straight forward, as it should be generic enough to work with multiple robots
void Tank_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	#if 0
	float right=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
	float left=DriverStation::GetInstance()->GetAnalogIn(2) - 1.0;
	m_RobotDrive.SetLeftRightMotorOutputs(right,left);
	return;
	#endif

	//For now leave this disabled... should not need to script this
	Dout(2, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);
	
	#ifdef __DisableTankDrive__
	m_RobotDrive.SetLeftRightMotorOutputs(0.0,0.0);  //pacify the watchdog
	return;
	#endif
	
	if (!m_TankRobotProps.ReverseSteering)
	{
		m_RobotDrive.SetLeftRightMotorOutputs(
				(float)(LeftVoltage * m_TankRobotProps.VoltageScalar),
				(float)(RightVoltage * m_TankRobotProps.VoltageScalar));
	}
	else
	{
		m_RobotDrive.SetLeftRightMotorOutputs(
				(float)(RightVoltage * m_TankRobotProps.VoltageScalar),
				(float)(LeftVoltage * m_TankRobotProps.VoltageScalar));
	}
}

void Tank_Robot_Control::Tank_Drive_Control_TimeChange(double dTime_s)
{
	m_dTime_s=dTime_s;
}

