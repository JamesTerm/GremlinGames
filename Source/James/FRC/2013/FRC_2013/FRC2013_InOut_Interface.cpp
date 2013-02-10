
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
	#ifdef __UsingTestingKit__
	m_LastAxisSetting=32;
	#endif
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
	eVictor_PowerWheel_First,	//The slower wheel
	eVictor_PowerWheel_Second,
	eVictor_Helix,
	eVictor_IntakeMotor,    //The transitional motor (tied to rollers)
	eVictor_Rollers,
	eVictor_IntakeDeployment  //aka flippers 
};
enum RelaySlotList
{
	eRelay_NoZeroUsed,
	eRelay_Compressor=8  //put at the end
};

//No more than 14 cardinal!
enum DigitalIO_SlotList
{
	eDigitalIO_NoZeroUsed,
	eEncoder_DriveRight_A,
	eEncoder_DriveRight_B,
	eEncoder_DriveLeft_A,
	eEncoder_DriveLeft_B,
	eEncoder_PowerWheel_First_A,
	eEncoder_PowerWheel_First_B,
	eEncoder_PowerWheel_Second_A,
	eEncoder_PowerWheel_Second_B,
	eEncoder_IntakeDeployment_A,
	eEncoder_IntakeDeployment_B,
	eLimit_Compressor=14
};

//Note: If any of these are backwards simply switch the on/off order here only!
enum SolenoidSlotList
{
	eSolenoid_NoZeroUsed,
	eSolenoid_EngageDrive_On,
	eSolenoid_EngageDrive_Off,
	eSolenoid_EngageLiftWinch_On,
	eSolenoid_EngageLiftWinch_Off,
	eSolenoid_EngageDropWinch_On,
	eSolenoid_EngageDropWinch_Off,
	eSolenoid_EngageFirePiston_On,
	eSolenoid_EngageFirePiston_Off,
};

//Note: the order of the initialization list must match the way they are in the class declaration, so if the slots need to change, simply
//change them in the enumerations
FRC_2013_Robot_Control::FRC_2013_Robot_Control(bool UseSafety) :
	m_TankRobotControl(UseSafety),
	#ifdef __UsingTestingKit__
	m_PitchAxis(2),m_LastAxisSetting(32),
	#endif
	m_pTankRobotControl(&m_TankRobotControl),
	//Victors--------------------------------
	m_PowerWheel_First_Victor(eVictor_PowerWheel_First),m_PowerWheel_Second_Victor(eVictor_PowerWheel_Second),
	m_Helix_Victor(eVictor_Helix),
	m_IntakeMotor_Victor(eVictor_IntakeMotor),m_Rollers_Victor(eVictor_Rollers),m_IntakeDeployment_Victor(eVictor_IntakeDeployment),
	m_Compress(eLimit_Compressor,eRelay_Compressor),
	//Solenoids------------------------------
	m_EngageDrive(eSolenoid_EngageDrive_On,eSolenoid_EngageDrive_Off),
	m_EngageLiftWinch(eSolenoid_EngageLiftWinch_On,eSolenoid_EngageLiftWinch_Off),
	m_EngageDropWinch(eSolenoid_EngageDropWinch_On,eSolenoid_EngageDropWinch_Off),
	m_EngageFirePiston(eSolenoid_EngageFirePiston_On,eSolenoid_EngageFirePiston_Off),
	//Sensors----------------------------------
	m_IntakeDeployment_Encoder(eEncoder_IntakeDeployment_A,eEncoder_IntakeDeployment_B,false,CounterBase::k4X),
	m_PowerWheel_First_Encoder(eEncoder_PowerWheel_First_A,eEncoder_PowerWheel_First_B),
	m_PowerWheel_Second_Encoder(eEncoder_PowerWheel_Second_A,eEncoder_PowerWheel_Second_B),
	//m_PowerWheelAverager(0.5),
	m_PowerWheel_PriorityAverager(10,0.30)

	//m_Potentiometer(1)
{
	//TODO set the SetDistancePerPulse() for turret
	ResetPos();
	const double EncoderPulseRate=(1.0/360.0);
	m_IntakeDeployment_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_PowerWheel_First_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_PowerWheel_Second_Encoder.SetDistancePerPulse(EncoderPulseRate);
	m_IntakeDeployment_Encoder.Start();
	m_PowerWheel_First_Encoder.Start();
	m_PowerWheel_Second_Encoder.Start();
	m_PowerWheelFilter.Reset();
}

FRC_2013_Robot_Control::~FRC_2013_Robot_Control() 
{
	//m_Compress.Stop();
	m_IntakeDeployment_Encoder.Stop();
	m_PowerWheel_First_Encoder.Stop();
	m_PowerWheel_Second_Encoder.Stop();
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
	case FRC_2013_Robot::ePowerWheelFirstStage:		
		m_PowerWheel_First_Victor.Set((float)(Voltage *m_RobotProps.GetPowerSlowWheelProps().GetRoteryProps().VoltageScalar));	
		break;
	case FRC_2013_Robot::ePowerWheelSecondStage:		
		m_PowerWheel_Second_Victor.Set((float)(Voltage *m_RobotProps.GetPowerWheelProps().GetRoteryProps().VoltageScalar));	
		break;
	case FRC_2013_Robot::eHelix:		
		m_Helix_Victor.Set((float)(Voltage * m_RobotProps.GetHelixProps().GetRoteryProps().VoltageScalar));
		break;
	case FRC_2013_Robot::eRollers:		
		m_Rollers_Victor.Set((float)(Voltage * m_RobotProps.GetRollersProps().GetRoteryProps().VoltageScalar));
		m_IntakeMotor_Victor.Set((float)(Voltage * m_RobotProps.GetRollersProps().GetRoteryProps().VoltageScalar));
		break;
	case FRC_2013_Robot::eIntake_Deployment:		
		m_IntakeDeployment_Victor.Set((float)(Voltage * m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().VoltageScalar));
		break;
	
	case FRC_2013_Robot::ePitchRamp:
		#ifdef __UsingTestingKit__
		//we can stay in degrees here
		double NewAngle=m_LastAxisSetting+(Voltage * m_RobotProps.GetPitchRampProps().GetRoteryProps().VoltageScalar);
		if (NewAngle>Servo::GetMaxAngle())
			NewAngle=Servo::GetMaxAngle();
		else if (NewAngle<Servo::GetMinAngle())
			NewAngle=Servo::GetMinAngle();

		m_LastAxisSetting=NewAngle;
		//Dout(4,1,"a=%.2f v=%.2f",NewAngle,Voltage);

		m_PitchAxis.SetAngle(NewAngle);
		#else
		#endif
		break;
	}
	#endif

	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2013_Robot::ePowerWheelFirstStage:		
			Dout(m_RobotProps.GetPowerSlowWheelProps().GetRoteryProps().Feedback_DiplayRow,1,"p1_v=%.2f",Voltage);
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,1,"p2_v=%.2f",Voltage);
			break;
		case FRC_2013_Robot::eHelix:		
			Dout(m_RobotProps.GetHelixProps().GetRoteryProps().Feedback_DiplayRow,1,"h_v=%.2f",Voltage);
			break;
		case FRC_2013_Robot::eRollers:		
			Dout(m_RobotProps.GetRollersProps().GetRoteryProps().Feedback_DiplayRow,1,"r_v=%.2f",Voltage);
			break;
		case FRC_2013_Robot::eIntake_Deployment:		
			Dout(m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().Feedback_DiplayRow,1,"id_v=%.2f",Voltage);
			break;
		case FRC_2013_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,1,"pr=%.2f",Voltage);
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
	case FRC_2013_Robot::eTest_Sensor:
		//ret= m_Fire_Limit.Get()!=0;
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
			result= DEG_2_RAD(m_PitchAxis.GetAngle() - 32) * m_RobotProps.GetPitchRampProps().GetRoteryProps().EncoderToRS_Ratio;
			//if we are close enough return the finer precision to avoid oscillation
			//if (fabs(result-DEG_2_RAD(m_LastAxisSetting))<DEG_2_RAD(5.0))
				result=DEG_2_RAD(m_LastAxisSetting-32);
			#else
			#endif
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			#ifndef __DisableMotorControls__
			
			//Here we use the new GetRate2 which should offer better precision
			
			result= m_PowerWheel_Second_Encoder.GetRate();
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
		case FRC_2013_Robot::ePowerWheelFirstStage:
			result= m_PowerWheel_First_Encoder.GetRate();
			result= result * m_RobotProps.GetPowerSlowWheelProps().GetRoteryProps().EncoderToRS_Ratio * Pi2;
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			result= m_IntakeDeployment_Encoder.GetRate();
			result= result * m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().EncoderToRS_Ratio * Pi2;
			break;
		case FRC_2013_Robot::eHelix:
			assert(false);  //These should be disabled as there is no encoder for them
			break;
	}
	
	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,14,"pr=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2013_Robot::ePowerWheelFirstStage:
			Dout(m_RobotProps.GetPowerSlowWheelProps().GetRoteryProps().Feedback_DiplayRow,11,"p1=%.2f",result / Pi2);
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,11,"p2=%.2f",result / Pi2);
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			Dout(m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().Feedback_DiplayRow,14,"id=%.1f",RAD_2_DEG(result));
			break;
	}
	#endif

	return result;
}

void FRC_2013_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	const char * const SolenoidState=Open?"Engaged":"Disengaged";
	//Translate the open state into a value for double solenoid
	DoubleSolenoid::Value value=Open ? DoubleSolenoid::kForward : DoubleSolenoid::kReverse;
	switch (index)
	{
	case FRC_2013_Robot::eEngageDriveTrain:
		printf("Drive Train Gear = %s\n",SolenoidState);
		m_EngageDrive.Set(value);
		break;
	case FRC_2013_Robot::eEngageLiftWinch:
		printf("Lift Winch = %s\n",SolenoidState);
		m_EngageLiftWinch.Set(value);
		break;
	case FRC_2013_Robot::eEngageDropWinch:
		printf("Drop Winch = %s\n",SolenoidState);
		m_EngageDropWinch.Set(value);
		break;
	case FRC_2013_Robot::eFirePiston:
		printf("Fire Piston = %s\n",SolenoidState);
		m_EngageFirePiston.Set(value);
		break;
	}
}
