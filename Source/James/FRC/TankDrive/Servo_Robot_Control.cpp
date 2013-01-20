
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
#include "Servo_Robot_Control.h"
#include "../Common/Robot_Control_Interface.h"
#include "../Common/UI_Controller.h"
#include "../Common/PIDController.h"
#include "../Common/InOut_Interface.h"
#include "../Common/Debug.h"

#ifdef __UsingTestingKit__

using namespace Framework::Base;

  /***********************************************************************************************************************************/
 /*														Servo_Robot_Control															*/
/***********************************************************************************************************************************/


Servo_Robot_Control::Servo_Robot_Control(bool UseSafety) :
	m_YawControl(0),
	m_dTime_s(0.0)
{
	//ResetPos();  may need this later
}

Servo_Robot_Control::~Servo_Robot_Control() 
{
}

void Servo_Robot_Control::Reset_Encoders()
{
	//m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Servo_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Tank_Robot_Properties *robot_props=static_cast<const Tank_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	//This will copy all the props
	m_TankRobotProps=robot_props->GetTankRobotProps();
	//m_LeftEncoder.SetReverseDirection(m_TankRobotProps.LeftEncoderReversed);
	//m_RightEncoder.SetReverseDirection(m_TankRobotProps.RightEncoderReversed);
}

double Servo_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_TankRobotProps.MotorToWheelGearRatio * M_PI * m_TankRobotProps.WheelDiameter; 
}

void Servo_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=0.0,RightVelocity=0.0;
	double LeftRate=0.0;
	double RightRate=0.0;
	
	LeftVelocity=RPS_To_LinearVelocity(LeftRate);
	RightVelocity=RPS_To_LinearVelocity(RightRate);
	Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", LeftVelocity,RightVelocity);
	//Dout(m_TankRobotProps.Feedback_DiplayRow, "l=%.1f r=%.1f", m_LeftEncoder.GetRate()/3.0,m_RightEncoder.GetRate()/3.0);
}

//This is kept simple and straight forward, as it should be generic enough to work with multiple robots
void Servo_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	//For now leave this disabled... should not need to script this
	Dout(2, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);
	
	//if (!m_TankRobotProps.ReverseSteering)
}

void Servo_Robot_Control::Tank_Drive_Control_TimeChange(double dTime_s)
{
	m_dTime_s=dTime_s;
}

#endif
