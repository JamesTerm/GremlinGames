#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
#include "PIDController.h"
#include "Calibration_Testing.h"
#include "Tank_Robot.h"
#include "Robot_Control_Interface.h"
#include "Rotary_System.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;


  /***********************************************************************************************************************************/
 /*														Rotary_Position_Control														*/
/***********************************************************************************************************************************/

Rotary_Position_Control::Rotary_Position_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex) : 
	Rotary_System(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDController(0.0,0.0,0.0), //This will be overridden in properties
	m_LastPosition(0.0),m_MatchVelocity(0.0),
	m_ErrorOffset(0.0),
	m_LastTime(0.0),m_PreviousVelocity(0.0),
	m_UsingPotentiometer(false) //to be safe
{
}

void Rotary_Position_Control::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if (m_UsingPotentiometer)
		m_LastPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	//This will copy all the props
	m_Rotary_Props=Props->GetRotaryProps();
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);

	const double MaxSpeedReference=Props->GetMaxSpeed();
	m_PIDController.SetInputRange(-MaxSpeedReference,MaxSpeedReference);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(-MaxSpeedReference*tolerance,MaxSpeedReference*tolerance);
	m_PIDController.Enable();
	m_ErrorOffset=0.0;
	if ((m_Rotary_Props.LoopState==Rotary_Props::eNone)||(m_Rotary_Props.LoopState==Rotary_Props::eOpen))
		SetPotentiometerSafety(true);
	else
		m_UsingPotentiometer=true;
}

void Rotary_Position_Control::TimeChange(double dTime_s)
{
	const double CurrentVelocity=m_Physics.GetVelocity();

	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	if (!m_LastTime) 
	{
		m_LastTime=dTime_s;
		#ifdef AI_TesterCode
		assert(dTime_s!=0.0);
		#endif
	}

	const double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	const double Displacement=NewPosition-m_LastPosition;
	const double PotentiometerVelocity=Displacement/m_LastTime;

	//Update the position to where the potentiometer says where it actually is
	if (m_UsingPotentiometer)
	{
		//For now we'll keep it this way until we have tested it thoroughly... for manual control using the force should help with latency and clear the 'I' if it gets messed up
		//We may however be able to omit the else case as in theory it should all work fine... if we don't use 'I' then it would probably be safe to make the change now
		//We can tune without I if possible to have for manual control
		//  [2/10/2013 James]
		if ((m_PIDController.GetI()==0.0) || (!GetLockShipToPosition()))
		{
			m_ErrorOffset=m_PIDController(CurrentVelocity,PotentiometerVelocity,dTime_s);
			//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
			m_ErrorOffset=fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;
		}
		else
		{
			//If we are manually controlling, we should still update displacement to properly work with limits and maintain where the position really
			//is to seamlessly transfer between manual and auto
			m_ErrorOffset=0.0;
			m_PIDController.ResetI();
		}
		SetPos_m(NewPosition);
	}

	m_LastPosition=NewPosition;
	m_LastTime=dTime_s;

	__super::TimeChange(dTime_s);

	//Note: CurrentVelocity variable is retained before the time change (for proper debugging of PID) we use the new velocity (called Velocity) here for voltage
	const double Velocity=m_Physics.GetVelocity();
	const double Acceleration=(Velocity-m_PreviousVelocity)/dTime_s;

	double Voltage=(Velocity+m_ErrorOffset)/MAX_SPEED;

	bool IsAccel=(Acceleration * Velocity > 0);
	Voltage+=Acceleration*(IsAccel? m_Rotary_Props.InverseMaxAccel : m_Rotary_Props.InverseMaxDecel);

	//Keep track of previous velocity to compute acceleration
	m_PreviousVelocity=Velocity;

	//Apply the polynomial equation to the voltage to linearize the curve
	{
		//Note: equations most-likely will not be symmetrical with the -1 - 0 range so we'll work with the positive range and restore the sign
		double y=fabs(Voltage);
		double *c=m_Rotary_Props.Polynomial;
		double x2=y*y;
		double x3=y*x2;
		double x4=x2*x2;
		y = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*y) + c[0]; 
		Voltage=(Voltage<0)?-y:y;
	}

	if ((IsZero(PotentiometerVelocity)) && IsAccel)
		ComputeDeadZone(Voltage,m_Rotary_Props.Positive_DeadZone,m_Rotary_Props.Negative_DeadZone);

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

	#ifdef __DebugLUA__
	if ((m_Rotary_Props.PID_Console_Dump)&&(PotentiometerVelocity!=0.0))
	{
		double PosY=m_LastPosition;
		//double PosY=RAD_2_DEG(m_LastPosition);
		printf("v=%.2f y=%.2f p=%f e=%.2f eo=%.2f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,m_ErrorOffset);
	}
	#endif

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);
}


void Rotary_Position_Control::ResetPos()
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

void Rotary_Position_Control::SetPotentiometerSafety(bool DisableFeedback)
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
			m_LastPosition=0.0;
			m_ErrorOffset=0.0;
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
			m_ErrorOffset=0.0;
		}
	}
}
  /***********************************************************************************************************************************/
 /*														Rotary_Velocity_Control														*/
/***********************************************************************************************************************************/

Rotary_Velocity_Control::Rotary_Velocity_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex,EncoderUsage EncoderState) : 
	Rotary_System(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDController(0.0,0.0,0.0), //This will be overridden in properties
	m_MatchVelocity(0.0),m_CalibratedScaler(1.0),m_ErrorOffset(0.0),
	m_MaxSpeedReference(0.0),m_EncoderVelocity(0.0),m_RequestedVelocity_Difference(0.0),
	m_EncoderState(EncoderState),m_EncoderCachedState(EncoderState),m_PreviousVelocity(0.0)
{
}

void Rotary_Velocity_Control::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_EncoderVelocity=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	//This will copy all the props
	m_Rotary_Props=Props->GetRotaryProps();
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);

	//Note: for the drive we create a large enough number that can divide out the voltage and small enough to recover quickly,
	//but this turned out to be problematic when using other angular rotary systems... therefore I am going to use the same computation
	//I do for linear, where it allows as slow to max speed as possible.
	//  [3/20/2012 Terminator]
	m_PIDController.SetInputRange(-MAX_SPEED,MAX_SPEED);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(-MAX_SPEED*tolerance,MAX_SPEED*tolerance);
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

void Rotary_Velocity_Control::UpdateRotaryProps(const Rotary_Props &RotaryProps)
{
	m_Rotary_Props=RotaryProps;
	m_CalibratedScaler=MAX_SPEED;
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);
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

void Rotary_Velocity_Control::TimeChange(double dTime_s)
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
	const double Velocity=m_Physics.GetVelocity();
	const double Acceleration=(Velocity-m_PreviousVelocity)/dTime_s;
	//CurrentVelocity is retained before the time change (for proper debugging of PID) we use the new velocity here for voltage
	//Either error offset or calibrated scaler will be used depending on the aggressive stop property, we need not branch this as
	//they both can be represented in the same equation
	double Voltage=(Velocity+m_ErrorOffset)/m_CalibratedScaler;

	bool IsAccel=(Acceleration * Velocity > 0);
	Voltage+=Acceleration*(IsAccel? m_Rotary_Props.InverseMaxAccel : m_Rotary_Props.InverseMaxDecel);

	//Keep track of previous velocity to compute acceleration
	m_PreviousVelocity=Velocity;

	//Apply the polynomial equation to the voltage to linearize the curve
	{
		//Note: equations most-likely will not be symmetrical with the -1 - 0 range so we'll work with the positive range and restore the sign
		double y=fabs(Voltage);
		double *c=m_Rotary_Props.Polynomial;
		double x2=y*y;
		double x3=y*x2;
		double x4=x2*x2;
		y = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*y) + c[0]; 
		Voltage=(Voltage<0)?-y:y;
	}

	if ((IsZero(m_EncoderVelocity)) && IsAccel)
		ComputeDeadZone(Voltage,m_Rotary_Props.Positive_DeadZone,m_Rotary_Props.Negative_DeadZone);

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
	{
		if (m_Rotary_Props.UseAggressiveStop)
			printf("v=%.2f p=%.2f e=%.2f eo=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_ErrorOffset);
		else
		{
			if (m_PIDController.GetI()==0.0)
				printf("v=%.2f p=%.2f e=%.2f eo=%.2f cs=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_ErrorOffset,m_CalibratedScaler/MAX_SPEED);
			else
				printf("v=%.2f p=%.2f e=%.2f i=%.2f cs=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_PIDController.GetTotalError(),m_CalibratedScaler/MAX_SPEED);
		}
	}
	#endif

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);

}

bool Rotary_Velocity_Control::InjectDisplacement(double DeltaTime_s,double &PositionDisplacement)
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

void Rotary_Velocity_Control::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
{
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_RequestedVelocity_Difference=VelocityToUse-m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
}

void Rotary_Velocity_Control::ResetPos()
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

void Rotary_Velocity_Control::SetEncoderSafety(bool DisableFeedback)
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
	props.Polynomial[0]=0.0;
	props.Polynomial[1]=1.0;
	props.Polynomial[2]=0.0;
	props.Polynomial[3]=0.0;
	props.Polynomial[4]=0.0;
	props.InverseMaxAccel=props.InverseMaxAccel=0.0;
	props.Positive_DeadZone=props.Negative_DeadZone=0.0;
	m_RotaryProps=props;
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

		script.GetField("voltage_multiply", NULL, NULL, &m_RotaryProps.VoltageScalar);
		script.GetField("encoder_to_wheel_ratio", NULL, NULL, &m_RotaryProps.EncoderToRS_Ratio);
		err = script.GetFieldTable("pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_RotaryProps.PID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_RotaryProps.PID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_RotaryProps.PID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("tolerance", NULL, NULL, &m_RotaryProps.PrecisionTolerance);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_RotaryProps.Feedback_DiplayRow=(size_t)fDisplayRow;

		string sTest;
		//I've made it closed so that typing no or NO stands out, but you can use bool as well
		//err = script.GetField("is_closed",&sTest,&bTest,NULL);
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_RotaryProps.LoopState=Rotary_Props::eOpen;
			else if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RotaryProps.LoopState=Rotary_Props::eClosed;
		}
		err = script.GetField("show_pid_dump",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RotaryProps.PID_Console_Dump=true;
		}
		err = script.GetField("use_aggressive_stop",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RotaryProps.UseAggressiveStop=true;
		}
		err = script.GetFieldTable("curve_voltage");
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&m_RotaryProps.Polynomial[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&m_RotaryProps.Polynomial[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&m_RotaryProps.Polynomial[2]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&m_RotaryProps.Polynomial[3]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&m_RotaryProps.Polynomial[4]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("inv_max_accel", NULL, NULL, &m_RotaryProps.InverseMaxAccel);
		m_RotaryProps.InverseMaxDecel=m_RotaryProps.InverseMaxAccel;	//set up deceleration to be the same value by default
		script.GetField("inv_max_decel", NULL, NULL, &m_RotaryProps.InverseMaxDecel);

		script.GetField("forward_deadzone", NULL, NULL,&m_RotaryProps.Positive_DeadZone);
		script.GetField("reverse_deadzone", NULL, NULL,&m_RotaryProps.Negative_DeadZone);
		//Ensure the negative settings are negative
		if (m_RotaryProps.Negative_DeadZone>0.0)
			m_RotaryProps.Negative_DeadZone=-m_RotaryProps.Negative_DeadZone;
		//TODO may want to swap forward in reverse settings if the voltage multiply is -1  (I'll want to test this as it happens)

		#ifdef AI_TesterCode
		err = script.GetFieldTable("motor_specs");
		if (!err)
		{
			m_EncoderSimulation.LoadFromScript(script);
			script.Pop();
		}
		#endif
	}
	__super::LoadFromScript(script);
}
