#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Vehicle_Drive.h"
#include "PIDController.h"
#include "Tank_Robot.h"
#include "Robot_Control_Interface.h"
#include "Rotary_System.h"

using namespace Framework::Base;
using namespace std;

namespace Base=Framework::Base;
namespace Scripting=Framework::Scripting;

  /***********************************************************************************************************************************/
 /*															Rotary_Linear															*/
/***********************************************************************************************************************************/

Rotary_Linear::Rotary_Linear(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex) : 
	Rotary_System(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDController(0.0,0.0,0.0), //This will be overridden in properties
	m_LastPosition(0.0),
	#ifdef __UseScalerPID__
	m_CalibratedScaler(1.0),
	#else
	m_ErrorOffset(0.0),
	#endif
	m_LastTime(0.0),m_MaxSpeedReference(0.0),
	m_UsingPotentiometer(false) //to be safe
	#ifdef __UseScalerPID__
	,  
	m_VoltageOverride(false)
	#endif
{
}

void Rotary_Linear::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if (m_UsingPotentiometer)
		m_LastPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	//This will copy all the props
	m_Rotary_Props=Props->GetRoteryProps();
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);

	m_MaxSpeedReference=Props->GetMaxSpeed();
	m_PIDController.SetInputRange(-m_MaxSpeedReference,m_MaxSpeedReference);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(-m_MaxSpeedReference*tolerance,m_MaxSpeedReference*tolerance);
	m_PIDController.Enable();
	#ifdef __UseScalerPID__
	m_CalibratedScaler=MAX_SPEED;
	#else
	m_ErrorOffset=0.0;
	#endif
	if ((m_Rotary_Props.LoopState==Rotary_Props::eNone)||(m_Rotary_Props.LoopState==Rotary_Props::eOpen))
		SetPotentiometerSafety(true);
	else
		m_UsingPotentiometer=true;
}

void Rotary_Linear::TimeChange(double dTime_s)
{
	const double CurrentVelocity=m_Physics.GetVelocity();
	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	double PotentiometerVelocity=0.0; //increased scope for debugging dump
	
	//Update the position to where the potentiometer says where it actually is
	if ((m_UsingPotentiometer)&&(!GetLockShipToPosition()))
	{
		if (m_LastTime!=0.0)
		{
			double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			PotentiometerVelocity=Displacement/m_LastTime;
			#ifdef __UseScalerPID__
			double LastSpeed=fabs(CurrentVelocity);  //This is last because the time change has not happened yet
			double PotentiometerSpeed=fabs(PotentiometerVelocity);
			double control=0.0;
			//only adjust calibration when both velocities are in the same direction, or in the case where the potentiometer is stopped which will
			//allow the scaler to normalize if it need to start up again.
			if (((CurrentVelocity * PotentiometerVelocity) > 0.0) || IsZero(PotentiometerSpeed) )
			{
				control=-m_PIDController(LastSpeed,PotentiometerSpeed,dTime_s);
				m_CalibratedScaler=MAX_SPEED+control;
			}
			#else
			m_ErrorOffset=m_PIDController(CurrentVelocity,PotentiometerVelocity,dTime_s);
			//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
			m_ErrorOffset=fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;
			#endif

			//DOUT5("pSpeed=%f cal=%f Max=%f",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);
			//printf("\rpSp=%f cal=%f Max=%f                 ",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);

			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
	}
	else
	{
		//If we are manually controlling, we should still update displacement to properly work with limits and maintain where the position really
		//is to seamlessly transfer between manual and auto
		if (m_UsingPotentiometer)
		{
			#ifdef __UseScalerPID__
			m_CalibratedScaler=MAX_SPEED;
			#else
			m_ErrorOffset=0.0;
			#endif
			//double LastSpeed=fabs(m_Physics.GetVelocity());  //This is last because the time change has not happened yet
			double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			PotentiometerVelocity=Displacement/m_LastTime;
			m_PIDController.ResetI();
			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
		else
			m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);  //For ease of debugging the controls (no harm to read)
	}

	m_LastTime=dTime_s;

	__super::TimeChange(dTime_s);
	//Note: CurrentVelocity is retained before the time change (for proper debugging of PID) we use the new velocity here for voltage
	#ifdef __UseScalerPID__
	double Voltage=m_Physics.GetVelocity()/m_CalibratedScaler;
	#else
	double Voltage=(m_Physics.GetVelocity()+m_ErrorOffset)/MAX_SPEED;
	#endif

	//Keep voltage override disabled for simulation to test precision stability
	//if (!m_VoltageOverride)
	if (true)
	{
		//Clamp range, PID (i.e. integral) controls may saturate the amount needed
		if (Voltage>0.0)
		{
			if (Voltage>1.0)
				Voltage=1.0;
		}
		else if (Voltage<0.0)
		{
			if (Voltage<-1.0)
				Voltage=-1.0;
		}
		else
			Voltage=0.0;  //is nan case
	}
	else
	{
		Voltage=0.0;
		m_PIDController.ResetI(m_MaxSpeedReference * -0.99);  //clear error for I for better transition back
	}

	#if 0
	Voltage*=Voltage;  //square them for more give
	//restore the sign
	if (CurrentVelocity<0)
		Voltage=-Voltage;
	#endif

	#ifdef __UseScalerPID__
	#ifdef __DebugLUA__
	if ((m_Rotary_Props.PID_Console_Dump)&&(Voltage!=0.0))
	{
		double PosY=m_LastPosition;
		if (!m_VoltageOverride)
			printf("v=%f y=%f p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler-MAX_SPEED);
		else
			printf("v=%f y=%f VO p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler-MAX_SPEED);
	}
	#endif
	#else
	#ifdef __DebugLUA__
	if ((m_Rotary_Props.PID_Console_Dump)&&(PotentiometerVelocity!=0.0))
	{
		double PosY=m_LastPosition;
		printf("v=%.2f y=%.2f p=%f e=%.2f eo=%.2f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,m_ErrorOffset);
	}
	#endif

	#endif

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);
}

#ifdef __UseScalerPID__
void Rotary_Linear::PosDisplacementCallback(double posDisplacement_m)
{
	m_VoltageOverride=false;
	if ((m_UsingPotentiometer)&&(!GetLockShipToPosition())&&(fabs(posDisplacement_m)<m_Rotary_Props.PrecisionTolerance))
		m_VoltageOverride=true;
}
#endif

void Rotary_Linear::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if ((m_UsingPotentiometer)&&(!GetBypassPos_Update()))
	{
		m_PIDController.Reset();
		m_RobotControl->Reset_Rotary(m_InstanceIndex);
		double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
		Stop();
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
}

void Rotary_Linear::SetPotentiometerSafety(bool DisableFeedback)
{
	//printf("\r%f       ",Value);
	if (DisableFeedback)
	{
		if (m_UsingPotentiometer)
		{
			//first disable it
			m_UsingPotentiometer=false;
			//Now to reset stuff
			printf("Disabling potentiometer for %s\n",GetName().c_str());
			//m_PIDController.Reset();
			ResetPos();
			//This is no longer necessary
			//MAX_SPEED=m_MaxSpeedReference;
			m_LastPosition=0.0;
			#ifdef __UseScalerPID__
			m_CalibratedScaler=MAX_SPEED;
			#else
			m_ErrorOffset=0.0;
			#endif
			m_LastTime=0.0;
			m_UsingRange=false;
		}
	}
	else
	{
		if (!m_UsingPotentiometer)
		{
			m_UsingPotentiometer=true;
			//setup the initial value with the potentiometers value
			printf("Enabling potentiometer for %s\n",GetName().c_str());
			ResetPos();
			m_UsingRange=GetUsingRange_Props();
			#ifdef __UseScalerPID__
			m_CalibratedScaler=MAX_SPEED;
			#else
			m_ErrorOffset=0.0;
			#endif
		}
	}
}
  /***********************************************************************************************************************************/
 /*															Rotary_Angular															*/
/***********************************************************************************************************************************/

Rotary_Angular::Rotary_Angular(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex,EncoderUsage EncoderState) : 
	Rotary_System(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDController(0.0,0.0,0.0), //This will be overridden in properties
	m_CalibratedScaler(1.0),m_ErrorOffset(0.0),
	m_MaxSpeedReference(0.0),m_EncoderVelocity(0.0),m_RequestedVelocity_Difference(0.0),
	m_EncoderState(EncoderState),m_EncoderCachedState(EncoderState)
{
}

void Rotary_Angular::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_EncoderVelocity=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	//This will copy all the props
	m_Rotary_Props=Props->GetRoteryProps();
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);

	const double OutputRange=MAX_SPEED*0.875;  //create a small range
	const double InputRange=20.0;  //create a large enough number that can divide out the voltage and small enough to recover quickly
	m_PIDController.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController.SetOutputRange(-InputRange,OutputRange);
	m_PIDController.Enable();
	m_CalibratedScaler=MAX_SPEED;
	m_ErrorOffset=0.0;

	switch (m_Rotary_Props.LoopState)
	{
	case Rotary_Props::eNone:
		SetEncoderSafety(true);
		break;
	case Rotary_Props::eOpen:
		m_EncoderState=ePassive;
		break;
	case Rotary_Props::eClosed:
		m_EncoderState=eActive;
		break;
	}
}

void Rotary_Angular::TimeChange(double dTime_s)
{
	const double CurrentVelocity=m_Physics.GetVelocity();
	double Encoder_Velocity=0.0;
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
	{
		Encoder_Velocity=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);

		//Unlike linear there is no displacement measurement therefore no need to check GetLockShipToPosition()
		if (m_EncoderState==eActive)
		{
			if (!m_Rotary_Props.UseAggressiveStop)
			{
				double control=0.0;
				//only adjust calibration when both velocities are in the same direction, or in the case where the encoder is stopped which will
				//allow the scaler to normalize if it need to start up again.
				if (((CurrentVelocity * Encoder_Velocity) > 0.0) || IsZero(Encoder_Velocity) )
				{
					control=-m_PIDController(fabs(CurrentVelocity),fabs(Encoder_Velocity),dTime_s);
					m_CalibratedScaler=MAX_SPEED+control;
				}
			}
			else
			{
				m_ErrorOffset=m_PIDController(CurrentVelocity,Encoder_Velocity,dTime_s);
				//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
				m_ErrorOffset=fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;
			}
		}
		else
			m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);  //For ease of debugging the controls (no harm to read)


		m_EncoderVelocity=Encoder_Velocity;
	}
	__super::TimeChange(dTime_s);

	//CurrentVelocity is retained before the time change (for proper debugging of PID) we use the new velocity here for voltage
	//Either error offset or calibrated scaler will be used depending on the aggressive stop property, we need not branch this as
	//they both can be represented in the same equation
	double Voltage=(m_Physics.GetVelocity()+m_ErrorOffset)/m_CalibratedScaler;

	//Keep voltage override disabled for simulation to test precision stability
	//if (!m_VoltageOverride)
	if (true)
	{
		//Clamp range, PID (i.e. integral) controls may saturate the amount needed
		if (Voltage>0.0)
		{
			if (Voltage>1.0)
				Voltage=1.0;
		}
		else if (Voltage<0.0)
		{
			if (Voltage<-1.0)
				Voltage=-1.0;
		}
		else
			Voltage=0.0;  //is nan case
	}
	else
	{
		Voltage=0.0;
		m_PIDController.ResetI(m_MaxSpeedReference * -0.99);  //clear error for I for better transition back
	}

	#if 0
	Voltage*=Voltage;  //square them for more give
	//restore the sign
	if (CurrentVelocity<0)
		Voltage=-Voltage;
	#endif

	#ifdef __DebugLUA__
	if (m_Rotary_Props.PID_Console_Dump && (Encoder_Velocity!=0.0))
		printf("p=%.2f e=%.2f eo=%.2f cs=%.2f\n",CurrentVelocity,Encoder_Velocity,m_ErrorOffset,m_CalibratedScaler-MAX_SPEED);
	#endif

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);

}

bool Rotary_Angular::InjectDisplacement(double DeltaTime_s,double &PositionDisplacement)
{
	bool ret=false;
	const bool UpdateDisplacement=((m_EncoderState==eActive)||(m_EncoderState==ePassive));
	if (UpdateDisplacement)
	{
		double computedVelocity=m_Physics.GetVelocity();
		m_Physics.SetVelocity(m_EncoderVelocity);
		m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement);
		//We must set this back so that the PID can compute the entire error
		m_Physics.SetVelocity(computedVelocity);
		ret=true;
	}
	if (!ret)
		ret=__super::InjectDisplacement(DeltaTime_s,PositionDisplacement);
	return ret;
}

void Rotary_Angular::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
{
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_RequestedVelocity_Difference=VelocityToUse-m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
}

void Rotary_Angular::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first

	m_PIDController.Reset();
	//We may need this if we use Kalman filters
	m_RobotControl->Reset_Rotary(m_InstanceIndex);
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_EncoderVelocity=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	else
		m_EncoderVelocity=0.0;

	//ensure teleop has these set properly
	m_CalibratedScaler=MAX_SPEED;
	m_ErrorOffset=0.0;
	m_RequestedVelocity_Difference=0.0;
}

void Rotary_Angular::SetEncoderSafety(bool DisableFeedback)
{
	//printf("\r%f       ",Value);
	if (DisableFeedback)
	{
		if (m_EncoderState!=eNoEncoder)
		{
			//first disable it
			m_EncoderState=eNoEncoder;
			//Now to reset stuff
			printf("Disabling encoder for %s\n",GetName().c_str());
			//m_PIDController.Reset();
			ResetPos();
			//This is no longer necessary
			//MAX_SPEED=m_MaxSpeedReference;
			m_EncoderVelocity=0.0;
			m_CalibratedScaler=MAX_SPEED;
			m_ErrorOffset=0;
			m_UsingRange=false;
		}
	}
	else
	{
		if (m_EncoderState==eNoEncoder)
		{
			m_EncoderState=m_EncoderCachedState;
			//setup the initial value with the potentiometers value
			printf("Enabling encoder for %s\n",GetName().c_str());
			ResetPos();
			m_UsingRange=GetUsingRange_Props();
			m_CalibratedScaler=MAX_SPEED;
			m_ErrorOffset=0;
		}
	}
}

  /***********************************************************************************************************************************/
 /*														Rotary_Properties															*/
/***********************************************************************************************************************************/

void Rotary_Properties::Init()
{
	Rotary_Props props;
	memset(&props,0,sizeof(Rotary_Props));

	props.VoltageScalar=1.0;
	props.EncoderToRS_Ratio=1.0;
	//Late assign this to override the initial default
	props.PID[0]=1.0; //set PIDs to a safe default of 1,0,0
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.LoopState=Rotary_Props::eNone;  //Always false when control is fully functional
	props.PID_Console_Dump=false;  //Always false unless you want to analyze PID (only one system at a time!)
	props.UseAggressiveStop=false;  //This is only for angular so false is a good default (must be explicit in script otherwise)
	m_RoteryProps=props;
}

void Rotary_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;

	//I shouldn't need this nested field redundancy... just need to be sure all client cases like this
	//err = script.GetFieldTable("rotary_settings");
	//if (!err) 

	{
		//double EncoderToRS_Ratio;
		//double PID[3]; //p,i,d
		//double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
		//int Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
		//bool IsOpen;  //This should always be false once control is fully functional
		//bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)

		script.GetField("voltage_multiply", NULL, NULL, &m_RoteryProps.VoltageScalar);
		script.GetField("encoder_to_wheel_ratio", NULL, NULL, &m_RoteryProps.EncoderToRS_Ratio);
		err = script.GetFieldTable("pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_RoteryProps.PID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_RoteryProps.PID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_RoteryProps.PID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("tolerance", NULL, NULL, &m_RoteryProps.PrecisionTolerance);
		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_RoteryProps.Feedback_DiplayRow=(size_t)fDisplayRow;

		string sTest;
		//I've made it closed so that typing no or NO stands out, but you can use bool as well
		//err = script.GetField("is_closed",&sTest,&bTest,NULL);
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_RoteryProps.LoopState=Rotary_Props::eOpen;
			else if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RoteryProps.LoopState=Rotary_Props::eClosed;
		}
		err = script.GetField("show_pid_dump",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RoteryProps.PID_Console_Dump=true;
		}
		err = script.GetField("use_aggressive_stop",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RoteryProps.UseAggressiveStop=true;
		}

	}
	__super::LoadFromScript(script);
}
