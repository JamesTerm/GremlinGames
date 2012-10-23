#include "WPILib.h"

#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "../Common/Entity_Properties.h"
#include "../Common/Physics_1D.h"
#include "../Common/Physics_2D.h"
#include "../Common/Entity2D.h"
#include "../Common/Goal.h"
#include "../Common/Ship_1D.h"
#include "../Common/Ship.h"
#include "../Common/AI_Base_Controller.h"
#include "../Common/Vehicle_Drive.h"
#include "../Common/PIDController.h"
#include "../Base/Joystick.h"
#include "../Base/JoystickBinder.h"
#include "../Common/Robot_Control_Interface.h"
#include "../Common/InOut_Interface.h"
#include "../Common/Rotary_System.h"
#include "Swerve_Robot.h"
#include "Nona_Robot.h"
#include "Tank_Tester.h"
#include "../Common/Debug.h"

using namespace Framework::Base;
using namespace std;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;

#ifdef __DebugLUA__
#define __ShowLCD__
#endif

  /***********************************************************************************************************************************/
 /*														Tank_Tester_Control															*/
/***********************************************************************************************************************************/


void Tank_Tester_Control::SetSafety(bool UseSafety)
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

Tank_Tester_Control::Tank_Tester_Control(bool UseSafety) :
	m_1(1),m_2(2),m_3(3),m_4(4),
	m_RobotDrive(&m_1,&m_2,&m_3,&m_4),
	//m_RobotDrive(1,2,3,4),  //default Jaguar instantiation
	m_LeftEncoder(3,4),m_RightEncoder(1,2),m_dTime_s(0.0),m_LeftVelocity(0.0),m_LeftVoltage(0.0)
{
	//ResetPos();  may need this later
	SetSafety(UseSafety);
	const double EncoderPulseRate=(1.0/360.0);
	m_LeftEncoder.SetDistancePerPulse(EncoderPulseRate),m_RightEncoder.SetDistancePerPulse(EncoderPulseRate);
	m_LeftEncoder.Start(),m_RightEncoder.Start();
}

Tank_Tester_Control::~Tank_Tester_Control() 
{
	m_LeftEncoder.Stop(),m_RightEncoder.Stop(); 
	m_RobotDrive.SetSafetyEnabled(false);
}

void Tank_Tester_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Tank_Tester_Control::Initialize(const Entity_Properties *props)
{
	const Swerve_Robot_Properties *robot_props=static_cast<const Swerve_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	//This will copy all the props
	m_SwerveRobotProps=robot_props->GetSwerveRobotProps();
	m_LeftEncoder.SetReverseDirection(m_SwerveRobotProps.EncoderReversed_Wheel[Swerve_Robot::eWheel_FL]);
	m_RightEncoder.SetReverseDirection(m_SwerveRobotProps.EncoderReversed_Wheel[Swerve_Robot::eWheel_FR]);
}

double Tank_Tester_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_SwerveRobotProps.MotorToWheelGearRatio * M_PI * m_SwerveRobotProps.WheelDiameter; 
}

void Tank_Tester_Control::Swerve_Drive_Control_TimeChange(double dTime_s)
{
	m_dTime_s=dTime_s;
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
}

void Tank_Tester_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		case Swerve_Robot::eWheel_FR:
			break;
		//case Swerve_Robot::eWheel_RL:
		//case Swerve_Robot::eWheel_RR:
			//m_Encoders[index].ResetPos();
			//break;
		//case Swerve_Robot::eSwivel_FL:
		//case Swerve_Robot::eSwivel_FR:
		//case Swerve_Robot::eSwivel_RL:
		//case Swerve_Robot::eSwivel_RR:
			break;
	}
}

double Tank_Tester_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case Swerve_Robot::eWheel_FL:
		{
			//double LeftRate=m_LeftEncoder.GetRate2(m_dTime_s);
			double LeftRate=m_LeftEncoder.GetRate();
			LeftRate=m_KalFilter_EncodeLeft(LeftRate);
			LeftRate=m_Averager_EncoderLeft.GetAverage(LeftRate);
			LeftRate=IsZero(LeftRate)?0.0:LeftRate;
			m_LeftVelocity=RPS_To_LinearVelocity(LeftRate);
			result=m_LeftVelocity;
		}
			break;
		case Swerve_Robot::eWheel_FR:
		{
			double RightVelocity=0.0;
			//double RightRate=m_RightEncoder.GetRate2(m_dTime_s);
			double RightRate=m_RightEncoder.GetRate();
			RightRate=m_KalFilter_EncodeRight(RightRate);
			RightRate=m_Averager_EncodeRight.GetAverage(RightRate);
			RightRate=IsZero(RightRate)?0.0:RightRate;
			RightVelocity=RPS_To_LinearVelocity(RightRate);
			Dout(m_SwerveRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", m_LeftVelocity,RightVelocity);
			result=RightVelocity;
		}
			break;
		//case Swerve_Robot::eWheel_RL:
		//case Swerve_Robot::eWheel_RR:
			//break;
		//case Swerve_Robot::eSwivel_FL:
		//case Swerve_Robot::eSwivel_FR:
		//case Swerve_Robot::eSwivel_RL:
		//case Swerve_Robot::eSwivel_RR:
			//result=NormalizeRotation2(m_Potentiometers[index-4].GetPotentiometerCurrentPosition());
			//break;
	}
	return result;
}

void Tank_Tester_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	switch (index)
	{
	case Swerve_Robot::eWheel_FL:
		m_LeftVoltage=Voltage;
		break;
	case Swerve_Robot::eWheel_FR:
		if (!m_SwerveRobotProps.ReverseSteering)
		{
			m_RobotDrive.SetLeftRightMotorOutputs(
					(float)(m_LeftVoltage * m_SwerveRobotProps.VoltageScalar),
					(float)(Voltage * m_SwerveRobotProps.VoltageScalar));
		}
		else
		{
			m_RobotDrive.SetLeftRightMotorOutputs(
					(float)(Voltage * m_SwerveRobotProps.VoltageScalar),
					(float)(m_LeftVoltage * m_SwerveRobotProps.VoltageScalar));
		}

		break;
	//case Swerve_Robot::eWheel_RL:
	//case Swerve_Robot::eWheel_RR:
		//break;
	//case Swerve_Robot::eSwivel_FL:
	//case Swerve_Robot::eSwivel_FR:
	//case Swerve_Robot::eSwivel_RL:
	//case Swerve_Robot::eSwivel_RR:
		//break;
	}
}


  /***********************************************************************************************************************************/
 /*														Tank_Nona_Control															*/
/***********************************************************************************************************************************/

//Using the roller wheel and claw 
Tank_Nona_Control::Tank_Nona_Control(bool UseSafety) : Tank_Tester_Control(UseSafety),m_Kicker_Victor(6),m_OnLowGear(3),m_OffLowGear(4)
{
}

Tank_Nona_Control::~Tank_Nona_Control()
{
}

//double Tank_Nona_Control::GetRotaryCurrentPorV(size_t index)
//{
//	return __super::GetRotaryCurrentPorV(index);
//}

void Tank_Nona_Control::Initialize(const Entity_Properties *props)
{
	const Nona_Robot_Properties *robot_props=dynamic_cast<const Nona_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		m_ButterflyProps=*robot_props;  //cache both drive modes
		m_Kicker_Props=robot_props->GetKickerWheelProps().GetRoteryProps();
	}
	__super::Initialize(props);
}

void Tank_Nona_Control::UpdateRotaryVoltage(size_t index,double Voltage)
{
	if (index==Nona_Robot::eWheel_Kicker)
	{
		m_Kicker_Victor.Set((float)(Voltage * m_Kicker_Props.VoltageScalar));
	}
	else
		__super::UpdateRotaryVoltage(index,Voltage);
	
}

void Tank_Nona_Control::CloseSolenoid(size_t index,bool Close)
{
	if (index==Butterfly_Robot::eUseLowGear)
	{
		printf("CloseSolenoid[%d] = %d \n",index,Close);
		m_OnLowGear.Set(Close),m_OffLowGear.Set(!Close);
		
		//keeping code for read but enable for tank robot that has a real gear shifting mechanism 
		#if 0
		Rotary_Properties props=m_ButterflyProps.GetDriveProps();
		const TractionModeProps &traction_props=m_ButterflyProps.GetTractionModeProps();
		props.SetFromShip_Properties(Close?traction_props.ShipProperties.GetShipProps():m_ButterflyProps.GetShipProps());
		#endif
	}
}
