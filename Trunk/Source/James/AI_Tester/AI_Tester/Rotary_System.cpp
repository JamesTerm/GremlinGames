#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
#include "Calibration_Testing.h"
#include "PIDController.h"
#include "Tank_Robot.h"
#include "Robot_Control_Interface.h"
#include "Rotary_System.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;


  /***********************************************************************************************************************************/
 /*															Rotary_Linear															*/
/***********************************************************************************************************************************/

Rotary_Linear::Rotary_Linear(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex) : 
	Ship_1D(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDController(0.0,0.0,0.0), //This will be overridden in properties
	m_LastPosition(0.0),m_CalibratedScaler(1.0),m_LastTime(0.0),
	m_UsingPotentiometer(false),  //to be safe
	m_VoltageOverride(false)
{
	m_UsingPotentiometer=true;  //for testing on AI simulator (unless I make a control for this)
}

void Rotary_Linear::Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	m_LastPosition=m_RobotControl->GetRotaryCurrentPosition(m_InstanceIndex);
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
	m_CalibratedScaler=MAX_SPEED;
}

void Rotary_Linear::TimeChange(double dTime_s)
{
	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	double PotentiometerVelocity; //increased scope for debugging dump
	
	//Update the position to where the potentiometer says where it actually is
	if (m_UsingPotentiometer)
	{
		if (m_LastTime!=0.0)
		{
			double LastSpeed=fabs(m_Physics.GetVelocity());  //This is last because the time change has not happened yet
			double NewPosition=m_RobotControl->GetRotaryCurrentPosition(m_InstanceIndex);

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			PotentiometerVelocity=Displacement/m_LastTime;
			double PotentiometerSpeed=fabs(PotentiometerVelocity);

			double control=0.0;
			control=-m_PIDController(LastSpeed,PotentiometerSpeed,dTime_s);
			m_CalibratedScaler=MAX_SPEED+control;

			//DOUT5("pSpeed=%f cal=%f Max=%f",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);
			//printf("\rpSp=%f cal=%f Max=%f                 ",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);

			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
		m_LastTime=dTime_s;
	}
	else
	{
		//Test potentiometer readings without applying to current position (disabled by default)
		m_RobotControl->GetRotaryCurrentPosition(m_InstanceIndex);
		//This is only as a sanity fix for manual mode... it should be this already (I'd assert if I could)
		//MAX_SPEED=m_CalibratedScaler=1.0;
	}
	__super::TimeChange(dTime_s);
	double CurrentVelocity=m_Physics.GetVelocity();
	double Voltage=CurrentVelocity/m_CalibratedScaler;

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

	#if 0
	if (Voltage!=0.0)
	{
		double PosY=m_LastPosition;
		if (!m_VoltageOverride)
			printf("v=%f y=%f p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler);
		else
			printf("v=%f y=%f VO p=%f e=%f d=%f cs=%f\n",Voltage,PosY,CurrentVelocity,PotentiometerVelocity,fabs(CurrentVelocity)-fabs(PotentiometerVelocity),m_CalibratedScaler);
	}
	#endif

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);
}

void Rotary_Linear::PosDisplacementCallback(double posDisplacement_m)
{
	m_VoltageOverride=false;
	if ((m_UsingPotentiometer)&&(!GetLockShipToPosition())&&(fabs(posDisplacement_m)<m_Rotary_Props.PrecisionTolerance))
		m_VoltageOverride=true;
}

void Rotary_Linear::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if (m_UsingPotentiometer)
	{
		m_PIDController.Reset();
		m_RobotControl->Reset_Rotary(m_InstanceIndex);
		double NewPosition=m_RobotControl->GetRotaryCurrentPosition(m_InstanceIndex);
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
			printf("Disabling potentiometer\n");
			//m_PIDController.Reset();
			ResetPos();
			//This is no longer necessary
			//MAX_SPEED=m_MaxSpeedReference;
			m_LastPosition=0.0;
			m_CalibratedScaler=MAX_SPEED;
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
			printf("Enabling potentiometer\n");
			ResetPos();
			m_UsingRange=true;
			m_CalibratedScaler=MAX_SPEED;
		}
	}
}

  /***********************************************************************************************************************************/
 /*														Rotary_Properties															*/
/***********************************************************************************************************************************/

Rotary_Properties::Rotary_Properties()
{
	Rotary_Props props;
	memset(&props,0,sizeof(Rotary_Props));

	//Late assign this to override the initial default
	props.PID[0]=1.0; //set PIDs to a safe default of 1,0,0
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	m_RoteryProps=props;
}
