
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
#include "Common/AI_Base_Controller.h"
#include "Drive/Tank_Robot.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "FRC2013_Robot.h"
#include "Common/InOut_Interface.h"
#include "Drive/Tank_Robot_Control.h"
#include "Drive/Servo_Robot_Control.h"
#include "Common/Debug.h"
#include "FRC2013_InOut_Interface.h"

#ifdef __DebugLUA__
#define __ShowLCD__
#endif

#undef __DisableMotorControls__
#undef  __EnablePrintfDumps__
#undef __DisableCompressor__

  /***********************************************************************************************************************************/
 /*													FRC_2013_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2013_Robot_Control::ResetPos()
{
	#ifndef __DisableCompressor__
	//Enable this code if we have a compressor 
	m_Compress.Stop();
	//Allow driver station to control if they want to run the compressor
	if (DriverStation::GetInstance()->GetDigitalIn(8))
	{
		printf("RobotControl reset compressor\n");
		m_Compress.Start();
	}
	#endif
}

enum VictorSlotList
{
	eVictor_NoZeroUsed,
	eVictor_RightMotor1,	//Used in InOut_Interface
	eVictor_RightMotor2,	//Used in InOut_Interface
	eVictor_LeftMotor1,		//Used in InOut_Interface
	eVictor_LeftMotor2,		//Used in InOut_Interface
	eVictor_Turret,
	eVictor_PowerWheel,
	eVictor_Flipper
	//Currently only two more victors available
};
enum RelaySlotList
{
	eRelay_NoZeroUsed,
	eRelay_LowerConveyor,
	eRelay_MiddleConveyor,
	eRelay_FireConveyor,
	eRelay_Compressor=8  //put at the end
};

//No more than 14!
enum DigitalIO_SlotList
{
	eDigitalIO_NoZeroUsed,
	eEncoder_DriveRight_A,
	eEncoder_DriveRight_B,
	eEncoder_DriveLeft_A,
	eEncoder_DriveLeft_B,
	eEncoder_Turret_A,
	eEncoder_Turret_B,
	eEncoder_PowerWheel_A,
	eEncoder_PowerWheel_B,
	eSensor_IntakeConveyor,
	eSensor_MiddleConveyor,
	eSensor_FireConveyor,
	eDigitalOut_BreakDrive_A,  
	eDigitalOut_BreakDrive_B,
	eLimit_Compressor
};

//Note: If any of these are backwards simply switch the on/off order here only!
enum SolenoidSlotList
{
	eSolenoid_NoZeroUsed,
	eSolenoid_UseClimbGear_On,
	eSolenoid_UseClimbGear_Off,
	eSolenoid_FlipperDown,
	eSolenoid_FlipperUp,
	eSolenoid_RampDeployment_On,
	eSolenoid_RampDeployment_Off,
};

//Note: the order of the initialization list must match the way they are in the class declaration, so if the slots need to change, simply
//change them in the enumerations
FRC_2013_Robot_Control::FRC_2013_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),
	#ifdef __UsingTestingKit__
	m_PitchAxis(2),
	#endif
	m_pTankRobotControl(&m_TankRobotControl),
	m_PowerWheel_Victor(eVictor_PowerWheel),
	m_Compress(eLimit_Compressor,eRelay_Compressor),
	m_OnClimbGear(eSolenoid_UseClimbGear_On),m_OffClimbGear(eSolenoid_UseClimbGear_Off),
	m_LowerConveyor_Relay(eRelay_LowerConveyor),m_MiddleConveyor_Relay(eRelay_MiddleConveyor),m_FireConveyor_Relay(eRelay_FireConveyor),
	//Sensors
	m_Turret_Encoder(eEncoder_Turret_A,eEncoder_Turret_B,false,CounterBase::k4X),
	m_PowerWheel_Encoder(eEncoder_PowerWheel_A,eEncoder_PowerWheel_B),
	m_Intake_Limit(eSensor_IntakeConveyor),m_Middle_Limit(eSensor_MiddleConveyor),m_Fire_Limit(eSensor_FireConveyor),
	m_UseBreakDrive_A(eDigitalOut_BreakDrive_A),m_UseBreakDrive_B(eDigitalOut_BreakDrive_B),
	//m_PowerWheelAverager(0.5),
	m_PowerWheel_PriorityAverager(10,0.30)

	//m_Potentiometer(1)
{
	//TODO set the SetDistancePerPulse() for turret
	ResetPos();
	const double EncoderPulseRate=(1.0/360.0);
	m_Turret_Encoder.SetDistancePerPulse(EncoderPulseRate),m_PowerWheel_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_Turret_Encoder.Start(),m_PowerWheel_Encoder.Start();
	m_PowerWheelFilter.Reset();
}

FRC_2013_Robot_Control::~FRC_2013_Robot_Control() 
{
	//m_Compress.Stop();
	m_Turret_Encoder.Stop(),m_PowerWheel_Encoder.Stop();
}

void FRC_2013_Robot_Control::Reset_Rotary(size_t index)
{
	//we probably will not need Kalman filters
	//m_KalFilter_Arm.Reset();
}

void FRC_2013_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __ShowLCD__
	DriverStationLCD * lcd = DriverStationLCD::GetInstance();
	lcd->UpdateLCD();
	#endif
}

void FRC_2013_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
	
	//Note: this will be NULL when Low Gear comes through here!
	const FRC_2013_Robot_Properties *robot_props=dynamic_cast<const FRC_2013_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy
		//Not sure I need to this since I got the copy... we'll see
		//m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
	}
}

//NOTE: for now, never pulse the spike relays, there is too much debate on the damage that will cause
Relay::Value TranslateToRelay(double Voltage)
{
	Relay::Value ret=Relay::kOff;  //*NEVER* want both on!
	const double Threshold=0.08;  //This value is based on dead voltage for arm... feel free to adjust, but keep high enough to avoid noise
	
	if (Voltage>Threshold)
		ret=Relay::kForward;
	else if (Voltage<-Threshold)
		ret=Relay::kReverse;
	return ret;
}

void FRC_2013_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	#ifndef __DisableMotorControls__
	switch (index)
	{
	case FRC_2013_Robot::ePowerWheels:		
		m_PowerWheel_Victor.Set((float)(Voltage *m_RobotProps.GetPowerWheelProps().GetRoteryProps().VoltageScalar));	
		break;
	case FRC_2013_Robot::eFireConveyor:		
		m_FireConveyor_Relay.Set(TranslateToRelay(Voltage * m_RobotProps.GetConveyorProps().GetRoteryProps().VoltageScalar));	break;
	case FRC_2013_Robot::ePitchRamp:
		#ifdef __UsingTestingKit__
		//we can stay in degrees here
		double NewAngle=m_PitchAxis.GetAngle()+(Voltage * m_RobotProps.GetPitchRampProps().GetRoteryProps().VoltageScalar);
		if (NewAngle>Servo::GetMaxAngle())
			NewAngle=Servo::GetMaxAngle();
		else if (NewAngle<Servo::GetMinAngle())
			NewAngle=Servo::GetMinAngle();

		Dout(4,1,"a=%.2f v=%.2f",NewAngle,Voltage);

		m_PitchAxis.SetAngle(NewAngle);
		#else
		#endif
		break;
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
		//	DOUT(4, "ArmVolt=%f ", Voltage);
		//}

		case FRC_2013_Robot::eTurret:
			//m_Turret.Set(Voltage);
			if (m_TurretVoltage!=Voltage)
			{
				printf("TurretVoltage=%f\n",Voltage);
				m_TurretVoltage=Voltage;
			}
			break;
		case FRC_2013_Robot::ePitchRamp:
			//m_PitchRamp.Set(Voltage);   Not sure what this will be... might not be a victor!  could be i2c
			if (m_PitchRampVoltage!=Voltage)
			{
				printf("PitchRamp=%f\n",Voltage);
				m_PitchRampVoltage=Voltage;
			}
			break;
		case FRC_2013_Robot::ePowerWheels:
			//m_PowerWheels.Set(Voltage);
			if (m_PowerWheelVoltage!=Voltage)
			{
				printf("PowerWheels=%f\n",Voltage);
				m_PowerWheelVoltage=Voltage;
			}

			//Example 2... another way to display readings
			//DOUT(4, "RollerVolt=%f ", Voltage);
			break;
		case FRC_2013_Robot::eFlippers:
			if (m_FlipperVoltage!=Voltage)
			{
				printf("Flippers=%f\n",Voltage);
				m_FlipperVoltage=Voltage;
			}
			break;
		case FRC_2013_Robot::eLowerConveyor:
			//m_LowerConveyor.Set(TranslateToRelay(Voltage));  //will be easy to switch to victor
			if (m_LowerConveyorVoltage!=Voltage)
			{
				printf("Lower=%f\n",Voltage);
				m_LowerConveyorVoltage=Voltage;
			}
			break;
		case FRC_2013_Robot::eMiddleConveyor:
			if (m_MiddleConveyorVoltage!=Voltage)
			{
				printf("Middle=%f\n",Voltage);
				m_MiddleConveyorVoltage=Voltage;
			}
			break;
		case FRC_2013_Robot::eFireConveyor:
			if (m_FireConveyorVoltage=Voltage)
			{
				printf("FireConveyor=%f\n",Voltage);
				m_FireConveyorVoltage=Voltage;
			}
			break;
	}
	#endif
	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,1,"p=%.2f",Voltage);
			break;
		case FRC_2013_Robot::ePowerWheels:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,1,"po_v=%.2f",Voltage);
			break;
	}
	#endif
}

bool FRC_2013_Robot_Control::GetBoolSensorState(size_t index)
{
	return false; //remove once we have this working
	bool ret=false;
	switch (index)
	{
	case FRC_2013_Robot::eFireConveyor_Sensor:
		ret= m_Fire_Limit.Get()!=0;
		break;
	default:
		assert (false);
	}
	return ret;
}

double FRC_2013_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			#ifdef __UsingTestingKit__
			//convert the angle into radians and scale it to represent the physical angle
			result= DEG_2_RAD(m_PitchAxis.GetAngle()) * m_RobotProps.GetPitchRampProps().GetRoteryProps().EncoderToRS_Ratio;
			#else
			#endif
			break;
		case FRC_2013_Robot::ePowerWheels:
			#ifndef __DisableMotorControls__
			
			//Here we use the new GetRate2 which should offer better precision
			
			result= m_PowerWheel_Encoder.GetRate();
			//result= m_PowerWheel_Encoder.GetRate2(m_TankRobotControl.Get_dTime_s());
			#if 0
			if (result!=0.0)
				printf("%.2f  %.2f \n",result,m_PowerWheel_PriorityAverager(result));
			#else
			result=m_PowerWheel_PriorityAverager(result);
			#endif
			
			//Quick test of using GetRate() vs. GetRate2()
			#if 0
			if (result>0.0)
				printf("pw1=%.1f pw2=%.1f t=%f\n",m_PowerWheel_Encoder.GetRate(),result,m_TankRobotControl.Get_dTime_s());
			#endif
			
			result= result * m_RobotProps.GetPowerWheelProps().GetRoteryProps().EncoderToRS_Ratio * Pi2;
			
			{
				result=m_PowerWheelFilter(result);
				#if 1
				//double average=m_PowerWheelAverager(result);
				double average=m_PowerWheelAverager.GetAverage(result);
				result=IsZero(average)?0.0:average;
				#else
				result=IsZero(result)?0.0:result;
				#endif
			}
			#else
			//This is temporary code to pacify using a closed loop, remove once we have real implementation
			result= m_PowerWheelVoltage*m_RobotProps.GetPowerWheelProps().GetMaxSpeed();
			#endif
			break;
		case FRC_2013_Robot::eFireConveyor:
			assert(false);  //These should be disabled as there is no encoder for them
			break;
	}
	
	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2013_Robot::ePowerWheels:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,11,"rs=%.2f",result / Pi2);
			break;
	}
	#endif

	return result;
}

void FRC_2013_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2013_Robot::eUseClimbGear:
		printf("UseClimbGear=%d\n",Open);
		m_OnClimbGear.Set(Open),m_OffClimbGear.Set(!Open);
		break;
	}
}
