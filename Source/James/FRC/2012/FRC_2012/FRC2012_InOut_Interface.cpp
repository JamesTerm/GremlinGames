
#include "WPILib.h"

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Base/Script.h"
#include "Common/Entity_Properties.h"
#include "Common/Physics_1D.h"
#include "Common/Physics_2D.h"
#include "Common/Entity2D.h"
#include "Common/Goal.h"
#include "Common/Ship_1D.h"
#include "Common/Ship.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/AI_Base_Controller.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2012_Robot.h"
#include "FRC2012_Camera.h"
#include "Common/InOut_Interface.h"
#include "Common/Debug.h"
#include "FRC2012_InOut_Interface.h"

#ifdef __DebugLUA__
#define __ShowLCD__
#endif

#undef __DisableMotorControls__
#define  __EnablePrintfDumps__

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::ResetPos()
{
	//Enable this code if we have a compressor 
	//m_Compress.Stop();
	////Allow driver station to control if they want to run the compressor
	//if (DriverStation::GetInstance()->GetDigitalIn(7))
	//{
	//	printf("RobotControl reset compressor\n");
	//	m_Compress.Start();
	//}
}

//TODO this year instead of hard coding numbers we may want to do an enum for each system for example
//Note: We should not need to populate the secondary variable m_Compress(5,2)... this will work differently this year as that number
//refers to an instance of the same module type, which we will not have.
enum VictorSlotList
{
	eVictor_NoZeroUsed,
	eVictor_RightMotor1,	//Used in InOut_Interface
	eVictor_RightMotor2,	//Used in InOut_Interface
	eVictor_LeftMotor1,		//Used in InOut_Interface
	eVictor_LeftMotor2,		//Used in InOut_Interface
	eVictor_Turret,
	eVictor_PowerWheel,
	eVictor_PitchWheel,
	eVictor_Flipper
	//Currently only two more victors available
};
enum RelaySlotList
{
	eRelay_NoZeroUsed,
	eRelay_LowerConveyor,
	eRelay_MiddleConveyor,
	eRelay_FireConveyor
};
enum SensorSlotList
{
	eSensor_LowerConveyor,
	eSensor_MiddleConveyor,
	eSensor_FireConveyor
};

//Note: If any of these are backwards simply switch the on/off order here only!
enum SolenoidSlotList
{
	eSolenoid_NoZeroUsed,
	eSolenoid_UseLowGear_On,
	eSolenoid_UseLowGear_Off,
	eSolenoid_RampDeployment_On,
	eSolenoid_RampDeployment_Off,
};

//Note: the order of the initialization list must match the way they are in the class declaration, so if the slots need to change, simply
//change them in the enumerations
FRC_2012_Robot_Control::FRC_2012_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),m_pTankRobotControl(&m_TankRobotControl),
	m_Turret_Victor(eVictor_Turret),m_PowerWheel_Victor(eVictor_PowerWheel),m_PitchRamp_Victor(eVictor_PitchWheel),m_Flipper_Victor(eVictor_Flipper),
	m_OnLowGear(eSolenoid_UseLowGear_On),m_OffLowGear(eSolenoid_UseLowGear_Off),
	m_OnRampDeployment(eSolenoid_RampDeployment_On),m_OffRampDeployment(eSolenoid_RampDeployment_Off) //,
	//m_Compress(5,2),
	//m_Potentiometer(1)
{
	ResetPos();
}

FRC_2012_Robot_Control::~FRC_2012_Robot_Control() 
{
	//m_Compress.Stop();
}

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	//we probably will not need Kalman filters
	//m_KalFilter_Arm.Reset();
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
	#ifndef __DisableCamera__
	m_Camera.CameraProcessing_TimeChange(dTime_s);
	#endif
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
	
	//Note: this will be NULL when Low Gear comes through here!
	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy
		//Not sure I need to this since I got the copy... we'll see
		//m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	}
}

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	#ifndef __DisableMotorControls__
	switch (index)
	{
	case FRC_2012_Robot::eTurret:		m_Turret_Victor.Set((float)Voltage);		break;
	case FRC_2012_Robot::ePitchRamp:	m_PitchRamp_Victor.Set((float)Voltage);		break;
	case FRC_2012_Robot::ePowerWheels:	m_PowerWheel_Victor.Set((float)Voltage);	break;
	//case FRC_2012_Robot::eLowerConveyor:
	//	break;
	//case FRC_2012_Robot::eMiddleConveyor:
	//	break;
	//case FRC_2012_Robot::eFireConveyor:
	//	break;
	}
	#endif

	#ifdef __EnablePrintfDumps__
	switch (index)
	{
		//Example... if we want to show readings

		//case FRC_2011_Robot::eArm:
		//{
		//	#if 0
		//	float ToUse=DriverStation::GetInstance()->GetAnalogIn(1) - 1.0;
		//	m_ArmMotor.Set(ToUse);
		//	return;
		//	#endif
		//	
		//	//Note: client code needs to check the levels are correct!
		//	m_ArmMotor.Set(Voltage);  //always the same velocity for both!
		//	#ifdef __ShowPotentiometerReadings__
		//	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
		//	lcd->PrintfLine(DriverStationLCD::kUser_Line4, "ArmVolt=%f ", Voltage);
		//	#endif
		//}

		case FRC_2012_Robot::eTurret:
			//m_Turret.Set(Voltage);
			if (m_TurretVoltage!=Voltage)
			{
				printf("TurretVoltage=%f\n",Voltage);
				m_TurretVoltage=Voltage;
			}
			break;
		case FRC_2012_Robot::ePitchRamp:
			//m_PitchRamp.Set(Voltage);   Not sure what this will be... might not be a victor!  could be i2c
			if (m_PitchRampVoltage!=Voltage)
			{
				printf("PitchRamp=%f\n",Voltage);
				m_PitchRampVoltage=Voltage;
			}
			break;
		case FRC_2012_Robot::ePowerWheels:
			//m_PowerWheels.Set(Voltage);
			if (m_PowerWheelVoltage!=Voltage)
			{
				printf("PowerWheels=%f\n",Voltage);
				m_PowerWheelVoltage=Voltage;
			}

			//Example 2... another way to display readings
			//#ifdef __ShowRollerReadings__
			//DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			//lcd->PrintfLine(DriverStationLCD::kUser_Line4, "RollerVolt=%f ", Voltage);
			//#endif

			break;
		case FRC_2012_Robot::eLowerConveyor:
			//m_LowerConveyor.Set(TranslateToRelay(Voltage));  //will be easy to switch to victor
			if (m_LowerConveyorVoltage!=Voltage)
			{
				printf("Lower=%f\n",Voltage);
				m_LowerConveyorVoltage=Voltage;
			}
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			if (m_MiddleConveyorVoltage!=Voltage)
			{
				printf("Middle=%f\n",Voltage);
				m_MiddleConveyorVoltage=Voltage;
			}
			break;
		case FRC_2012_Robot::eFireConveyor:
			if (m_FireConveyorVoltage=Voltage)
			{
				printf("FireConveyor=%f\n",Voltage);
				m_FireConveyorVoltage=Voltage;
			}
			break;
	}
	#endif
}

bool FRC_2012_Robot_Control::GetBoolSensorState(size_t index)
{
	bool ret=false;
	switch (index)
	{
	case FRC_2012_Robot::eLowerConveyor_Sensor:
		//ret=GetDigitalIn();... TODO I believe its a digital in with a get that returns a value
		break;
	case FRC_2012_Robot::eMiddleConveyor_Sensor:
		break;
	case FRC_2012_Robot::eFireConveyor_Sensor:
		break;
	default:
		assert (false);
	}
	return ret;
}

double FRC_2012_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			//This is like a potentiometer analog in... I've included how I did it last year below, but it may not work the same way
			//I never applied kalman filter, but we can if we need to... I found it to have some strange quirks

			//double raw_value = (double)m_Potentiometer.GetAverageValue();
			////raw_value = m_KalFilter_Arm(raw_value);  //apply the Kalman filter
			////Note the value is inverted with the negative operator
			//double ret=-FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(raw_value);
			////I may keep these on as they should be useful feedback
			//#ifdef __ShowPotentiometerReadings__
			//DriverStationLCD * lcd = DriverStationLCD::GetInstance();
			//double height=FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(ret);
			////lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %.1fft %.1fin", RAD_2_DEG(ret),height*3.2808399,height*39.3700787);
			//lcd->PrintfLine(DriverStationLCD::kUser_Line3, "%.1f %f %.1fft ", RAD_2_DEG(ret),height,height*3.2808399);
			////lcd->PrintfLine(DriverStationLCD::kUser_Line3, "1: Pot=%.1f ", raw_value);
			//#endif
			break;
		case FRC_2012_Robot::ePitchRamp:
			//Not sure what kind of sensor this will be yet
			break;
		case FRC_2012_Robot::ePowerWheels:
			//result=m_PowerWheel.GetRate();
			//This is temporary code to pacify using a closed loop, remove once we have real implementation
			result= m_PowerWheelVoltage*m_RobotProps.GetPowerWheelProps().GetMaxSpeed();
			break;
		case FRC_2012_Robot::eLowerConveyor:
		case FRC_2012_Robot::eMiddleConveyor:
		case FRC_2012_Robot::eFireConveyor:
			assert(false);  //These should be disabled as there is no encoder for them
			break;
	}
	
	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			Dout(m_RobotProps.GetTurretProps().GetRoteryProps().Feedback_DiplayRow,"turret=%f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,"pitch=%f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePowerWheels:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,"power=%f",result / Pi2);
			break;
	}
	#endif

	return result;
}

void FRC_2012_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2012_Robot::eUseLowGear:
		printf("UseLowGear=%d\n",Open);
		m_OnLowGear.Set(Open),m_OffLowGear.Set(!Open);
		break;
	case FRC_2012_Robot::eRampDeployment:
		printf("RampDeployment=%d\n",Open);
		m_OnRampDeployment.Set(Open),m_OnRampDeployment.Set(!Open);
		break;
	}
}
