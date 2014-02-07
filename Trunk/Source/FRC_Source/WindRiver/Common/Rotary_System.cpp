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
#include "Poly.h"
#include "Robot_Control_Interface.h"
#include "Rotary_System.h"
#include "SmartDashboard/SmartDashboard.h"


#ifdef Robot_TesterCode
using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;
#else
using namespace Framework::Base;
using namespace std;
#endif
  /***********************************************************************************************************************************/
 /*															Rotary_System															*/
/***********************************************************************************************************************************/

void Rotary_System::InitNetworkProperties(const Rotary_Props &props,bool AddArmAssist)
{
	switch (props.LoopState)
	{
		case Rotary_Props::eOpen:
			SmartDashboard::PutString("Loop State","open");
			break;
		case Rotary_Props::eClosed:
			SmartDashboard::PutString("Loop State","closed");
			break;
		case Rotary_Props::eNone:
			SmartDashboard::PutString("Loop State","none");
			break;
		case Rotary_Props::eClosed_ManualAssist:
			SmartDashboard::PutString("Loop State","closed2");
			break;
	}

	SmartDashboard::PutNumber("Tolerance",props.PrecisionTolerance);

	if (!AddArmAssist)
	{
		SmartDashboard::PutNumber("velocity P",props.PID[0]);
		SmartDashboard::PutNumber("velocity I",props.PID[1]);
		SmartDashboard::PutNumber("velocity D",props.PID[2]);
		
		SmartDashboard::PutNumber("gain accel",props.InverseMaxAccel);
		SmartDashboard::PutNumber("gain decel",props.InverseMaxDecel);
	}
	else
	{
		const Rotary_Props::Rotary_Arm_GainAssist_Props &arm=props.ArmGainAssist; 

		SmartDashboard::PutNumber("Up P",arm.PID_Up[0]);
		SmartDashboard::PutNumber("Up I",arm.PID_Up[1]);
		SmartDashboard::PutNumber("Up D",arm.PID_Up[2]);

		SmartDashboard::PutBoolean("Use Up PID Only",arm.UsePID_Up_Only);
		if (!arm.UsePID_Up_Only)
		{
			SmartDashboard::PutNumber("Down P",arm.PID_Down[0]);
			SmartDashboard::PutNumber("Down I",arm.PID_Down[1]);
			SmartDashboard::PutNumber("Down D",arm.PID_Down[2]);
		}
		
		SmartDashboard::PutNumber("gain accel up",arm.InverseMaxAccel_Up);
		SmartDashboard::PutNumber("gain decel up",arm.InverseMaxDecel_Up);
		SmartDashboard::PutNumber("gain accel down",arm.InverseMaxAccel_Down);
		SmartDashboard::PutNumber("gain decel down",arm.InverseMaxDecel_Down);

		SmartDashboard::PutNumber("gravity gain voltage",arm.SlowVelocityVoltage);
		SmartDashboard::PutNumber("gravity gain velocity",arm.SlowVelocity);
		SmartDashboard::PutNumber("Tolerance Count",arm.ToleranceConsecutiveCount);
		SmartDashboard::PutNumber("Lag Predict Up",arm.VelocityPredictUp);
		SmartDashboard::PutNumber("Lag Predict Down",arm.VelocityPredictDown);

		SmartDashboard::PutNumber("pulse burst time",arm.PulseBurstTimeMs);
		SmartDashboard::PutNumber("pulse burst range",arm.PulseBurstRange);
	}
}

void Rotary_System::NetworkEditProperties(Rotary_Props &props,bool AddArmAssist)
{
	string strValue=SmartDashboard::GetString("Loop State");
	if (strcmp(strValue.c_str(),"open")==0) props.LoopState=Rotary_Props::eOpen;
	else if (strcmp(strValue.c_str(),"closed")==0) props.LoopState=Rotary_Props::eClosed;
	else if (strcmp(strValue.c_str(),"closed2")==0) props.LoopState=Rotary_Props::eClosed_ManualAssist;
	else if (strcmp(strValue.c_str(),"none")==0) props.LoopState=Rotary_Props::eNone;

	props.PrecisionTolerance=SmartDashboard::GetNumber("Tolerance");

	if (!AddArmAssist)
	{
		props.PID[0]=SmartDashboard::GetNumber("velocity P");
		props.PID[1]=SmartDashboard::GetNumber("velocity I");
		props.PID[2]=SmartDashboard::GetNumber("velocity D");

		props.InverseMaxAccel=SmartDashboard::GetNumber("gain accel");
		props.InverseMaxDecel=SmartDashboard::GetNumber("gain decel");
	}
	else
	{
		Rotary_Props::Rotary_Arm_GainAssist_Props &arm=props.ArmGainAssist; 
		arm.PID_Up[0]=SmartDashboard::GetNumber("Up P");
		arm.PID_Up[1]=SmartDashboard::GetNumber("Up I");
		arm.PID_Up[2]=SmartDashboard::GetNumber("Up D");

		bool OldCheckUpOnly=arm.UsePID_Up_Only;
		arm.UsePID_Up_Only=SmartDashboard::GetBoolean("Use Up PID Only");
		//Check for new case to populate
		if (!arm.UsePID_Up_Only && OldCheckUpOnly)
		{
			SmartDashboard::PutNumber("Down P",arm.PID_Down[0]);
			SmartDashboard::PutNumber("Down I",arm.PID_Down[1]);
			SmartDashboard::PutNumber("Down D",arm.PID_Down[2]);
		}

		if (!arm.UsePID_Up_Only)
		{
			arm.PID_Down[0]=SmartDashboard::GetNumber("Down P");
			arm.PID_Down[1]=SmartDashboard::GetNumber("Down I");
			arm.PID_Down[2]=SmartDashboard::GetNumber("Down D");
		}
		
		arm.InverseMaxAccel_Up=SmartDashboard::GetNumber("gain accel up");
		arm.InverseMaxDecel_Up=SmartDashboard::GetNumber("gain decel up");
		arm.InverseMaxAccel_Down=SmartDashboard::GetNumber("gain accel down");
		arm.InverseMaxDecel_Down=SmartDashboard::GetNumber("gain decel down");

		arm.SlowVelocityVoltage=SmartDashboard::GetNumber("gravity gain voltage");
		arm.SlowVelocity=SmartDashboard::GetNumber("gravity gain velocity");
		arm.ToleranceConsecutiveCount=SmartDashboard::GetNumber("Tolerance Count");
		arm.VelocityPredictUp=SmartDashboard::GetNumber("Lag Predict Up");
		arm.VelocityPredictDown=SmartDashboard::GetNumber("Lag Predict Down");
		arm.PulseBurstTimeMs=SmartDashboard::GetNumber("pulse burst time");
		arm.PulseBurstRange=SmartDashboard::GetNumber("pulse burst range");
	}
}

  /***********************************************************************************************************************************/
 /*														Rotary_Position_Control														*/
/***********************************************************************************************************************************/

Rotary_Position_Control::Rotary_Position_Control(const char EntityName[],Rotary_Control_Interface *robot_control,size_t InstanceIndex) : 
	Rotary_System(EntityName),m_RobotControl(robot_control),m_InstanceIndex(InstanceIndex),
	m_PIDControllerUp(0.0,0.0,0.0), //This will be overridden in properties
	m_PIDControllerDown(0.0,0.0,0.0),
	m_LastPosition(0.0),m_MatchVelocity(0.0),
	m_ErrorOffset(0.0),
	m_LastTime(0.0),m_PreviousVelocity(0.0),
	m_BurstIntensity(0.0),m_CurrentBurstTime(0.0),
	m_PotentiometerState(eNoPot), //to be safe
	m_ToleranceCounter(0)
{
}

void Rotary_Position_Control::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if (m_PotentiometerState!=eNoPot)
		m_LastPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	m_VoltagePoly.Initialize(&Props->GetRotaryProps().Voltage_Terms);
	//This will copy all the props
	m_Rotary_Props=Props->GetRotaryProps();
	m_PIDControllerUp.SetPID(m_Rotary_Props.ArmGainAssist.PID_Up[0],m_Rotary_Props.ArmGainAssist.PID_Up[1],m_Rotary_Props.ArmGainAssist.PID_Up[2]);
	m_PIDControllerDown.SetPID(m_Rotary_Props.ArmGainAssist.PID_Down[0],m_Rotary_Props.ArmGainAssist.PID_Down[1],m_Rotary_Props.ArmGainAssist.PID_Down[2]);

	const double MaxSpeedReference=Props->GetMaxSpeed();
	m_PIDControllerUp.SetInputRange(-MaxSpeedReference,MaxSpeedReference);
	m_PIDControllerDown.SetInputRange(-MaxSpeedReference,MaxSpeedReference);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDControllerUp.SetOutputRange(-MaxSpeedReference*tolerance,MaxSpeedReference*tolerance);
	m_PIDControllerDown.SetOutputRange(-MaxSpeedReference*tolerance,MaxSpeedReference*tolerance);
	//The idea here is that the arm may rest at a stop point that needs consistent voltage to keep steady
	if (m_Rotary_Props.ArmGainAssist.SlowVelocityVoltage!=0.0)
	{
		m_PIDControllerUp.SetAutoResetI(false);
		m_PIDControllerDown.SetAutoResetI(false);
	}
	m_PIDControllerUp.Enable();
	m_PIDControllerDown.Enable();
	m_ErrorOffset=0.0;
	switch (m_Rotary_Props.LoopState)
	{
	case Rotary_Props::eNone:
		SetPotentiometerSafety(true);
		break;
	case Rotary_Props::eOpen:
		m_PotentiometerState=ePassive;
		break;
	case Rotary_Props::eClosed:
	case Rotary_Props::eClosed_ManualAssist:
		m_PotentiometerState=eActive;
		break;
	}
	//It is assumed that this property is constant throughout the whole session
	if (m_Rotary_Props.PID_Console_Dump)
		InitNetworkProperties(m_Rotary_Props,true);
}

void Rotary_Position_Control::TimeChange(double dTime_s)
{
	//TODO we'll probably want velocity PID for turret no load type... we'll need to test to see
	const bool TuneVelocity=false;
	const double CurrentVelocity=m_Physics.GetVelocity();
	const Rotary_Props::Rotary_Arm_GainAssist_Props &arm= m_Rotary_Props.ArmGainAssist;
	const bool NeedGainAssistForUp=((arm.SlowVelocityVoltage!=0.0)&&(CurrentVelocity>0.0));

	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated
	if (!m_LastTime) 
	{
		m_LastTime=dTime_s;
		#ifdef Robot_TesterCode
		assert(dTime_s!=0.0);
		#endif
	}

	const double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	const double Displacement=NewPosition-m_LastPosition;
	const double PotentiometerVelocity=Displacement/m_LastTime;

	double BurstIntensity=0.0;

	//Update the position to where the potentiometer says where it actually is
	if (m_PotentiometerState==eActive)
	{
		if ((!GetLockShipToPosition()) || (m_Rotary_Props.LoopState==Rotary_Props::eClosed_ManualAssist))
		{
			if (TuneVelocity)
			{
				if (m_PIDControllerUp.GetI()==0.0)
				{
					m_ErrorOffset=m_PIDControllerUp(CurrentVelocity,PotentiometerVelocity,dTime_s);
					const double Acceleration=(CurrentVelocity-m_PreviousVelocity)/dTime_s;
					const bool Decel=(Acceleration * CurrentVelocity < 0);
					//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
					//Note: that it is important to bias towards deceleration this can help satisfy both requirements of avoiding oscillation as well
					//As well as avoiding a potential overshoot when trying stop at a precise distance
					m_ErrorOffset=Decel || fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;
				}
			}
			else
			{
				const double PredictedPositionUp=NewPosition + (PotentiometerVelocity * arm.VelocityPredictUp);
				//PID will correct for position... this may need to use I to compensate for latency
				if ((arm.UsePID_Up_Only)||(GetPos_m()>PredictedPositionUp))
				{
					m_PIDControllerDown.ResetI();
					m_ErrorOffset=m_PIDControllerUp(GetPos_m(),PredictedPositionUp,dTime_s);
				}
				else
				{
					const double PredictedPosition=NewPosition + (PotentiometerVelocity * arm.VelocityPredictDown);
					m_PIDControllerUp.ResetI();
					m_ErrorOffset=m_PIDControllerDown(GetPos_m(),PredictedPosition,dTime_s);
				}

				//unlike for velocity all error offset values are taken... two PIDs and I should help stabilize oscillation
				if ((arm.PulseBurstTimeMs>0.0)&&(fabs(m_ErrorOffset)<arm.PulseBurstRange)&&(fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance))
				{
					//we are in the pulse burst zone... now to manage the burst intensity
					m_CurrentBurstTime+=dTime_s;  //increment the pulse burst time with this new time slice
					//The off-time should really match the latency time where we can sample the change interval and get a reading
					//This could be its own property if necessary
					const double Off_Time=(GetPos_m()>PredictedPositionUp)?arm.VelocityPredictUp:arm.VelocityPredictDown;
					//We use the negative sign bit to indicate it was turned off... or zero
					if (m_BurstIntensity<=0.0)
					{	//Burst has been off... is it time to turn it back on
						if (m_CurrentBurstTime>=Off_Time)
						{
							//Turn on the pulse... the intensity here is computed by the overlap
							const double overlap=m_CurrentBurstTime-Off_Time;
							if (overlap<dTime_s)
								BurstIntensity=overlap/dTime_s;
							else
							{
								BurstIntensity=1.0;
								//This shouldn't happen often... probably shouldn't matter much... but keep this for diagnostic testing
								printf("test burst begin... overlap=%.2f vs delta slice=%.2f\n",overlap,dTime_s);
							}
							m_CurrentBurstTime=0.0;  //reset timer
						}
					}
					else
					{  //Burst has been on... is it time to turn it back off
						if (m_CurrentBurstTime>=arm.PulseBurstTimeMs)
						{
							//Turn on the pulse... the intensity here is computed by the overlap
							const double overlap=m_CurrentBurstTime-arm.PulseBurstTimeMs;
							//We use the negative sign bit to indicate it was turned off... or zero
							if (overlap<dTime_s)
								BurstIntensity=-(overlap/dTime_s);
							else
							{
								BurstIntensity=0.0;
								//This shouldn't happen often... probably shouldn't matter much... but keep this for diagnostic testing
								printf("test burst end... overlap=%.2f vs delta slice=%.2f\n",overlap,dTime_s);
							}
							m_CurrentBurstTime=0.0;  //reset timer
						}
						else
							BurstIntensity=1.0;  //still on under time... so keep it on full
					}
				}
				//When in eClosed_ManualAssist check the locked position to end weak voltage when it is within tolerance
				//setpoint will turn locked position on once the tolerance is achieved with its count
				if (GetLockShipToPosition())
					m_ErrorOffset=fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;

			}
		}
		else
		{
			//If we are manually controlling, we should still update displacement to properly work with limits and maintain where the position really
			//is to seamlessly transfer between manual and auto
			m_ErrorOffset=0.0;
			m_PIDControllerUp.ResetI();
			m_PIDControllerDown.ResetI();
		}
		//We do not want to alter position if we are using position control PID
		if ((TuneVelocity) || IsZero(NewPosition) || NeedGainAssistForUp)
			SetPos_m(NewPosition);
	}

	else if (m_PotentiometerState==ePassive)
	{
		//ensure the positions are calibrated when we are not moving
		if (IsZero(NewPosition) || NeedGainAssistForUp)
				SetPos_m(NewPosition);  //this will help min and max limits work properly even though we do not have PID
	}

	//if we are heading for an intended position and we graze on it... turn off the corrections
	if  (!GetLockShipToPosition())
	{
		
		if (fabs(NewPosition-m_IntendedPosition)<m_Rotary_Props.PrecisionTolerance)
			m_ToleranceCounter++;
		else
			m_ToleranceCounter=0;

		if (m_ToleranceCounter >= arm.ToleranceConsecutiveCount)
			SetCurrentLinearAcceleration(0.0);  //lock ship to position
	}

	m_LastPosition=NewPosition;
	m_LastTime=dTime_s;

	__super::TimeChange(dTime_s);

	//Note: CurrentVelocity variable is retained before the time change (for proper debugging of PID) we use the new velocity (called Velocity) here for voltage
	const double Velocity=m_Physics.GetVelocity();
	const double Acceleration=(Velocity-m_PreviousVelocity)/dTime_s;

	double Voltage=(Velocity+m_ErrorOffset)/m_MaxSpeed;

	bool IsAccel=(Acceleration * Velocity > 0);
	if (Velocity>0)
		Voltage+=Acceleration*(IsAccel? arm.InverseMaxAccel_Up : arm.InverseMaxDecel_Up);
	else
		Voltage+=Acceleration*(IsAccel? arm.InverseMaxAccel_Down : arm.InverseMaxDecel_Down);

	//See if we are using the arm gain assist (only when going up)
	if ((NeedGainAssistForUp) || ((arm.SlowVelocityVoltage!=0.0) && m_ErrorOffset>0.0))
	{
		//first start out by allowing the max amount to correspond to the angle of the arm... this assumes the arm zero degrees is parallel to the ground
		//90 is straight up... should work for angles below zero (e.g. hiking viking)... angles greater than 90 will be negative which is also correct
		const double MaxVoltage=cos(NewPosition * arm.GainAssistAngleScalar) * arm.SlowVelocityVoltage;
		double BlendStrength=0.0;
		const double SlowVelocity=arm.SlowVelocity;
		//Now to compute blend strength... a simple linear distribution of how much slower it is from the slow velocity
		if (MaxVoltage>0.0)
		{
			if (PotentiometerVelocity<SlowVelocity)
				BlendStrength= (SlowVelocity-PotentiometerVelocity) / SlowVelocity;
		}
		else
		{
			if (PotentiometerVelocity>-SlowVelocity)
				BlendStrength= (SlowVelocity-fabs(PotentiometerVelocity)) / SlowVelocity;
		}
		Voltage+= MaxVoltage * BlendStrength;
	}

	//apply additional pulse burst as needed
	m_BurstIntensity=BurstIntensity;
	//The intensity we use is the same as full speed with the gain assist... we may want to have own properties for these if they do not work well
	const double PulseBurst=fabs(m_BurstIntensity) * ((m_ErrorOffset>0.0)?m_MaxSpeed * 1.0 * arm.InverseMaxAccel_Up : (-m_MaxSpeed) * 1.0 * arm.InverseMaxAccel_Down);
	Voltage+= PulseBurst;

	//if (PulseBurst!=0.0)
	//	printf("pb=%.2f\n",PulseBurst);
	#if 0
	if ((arm.PulseBurstTimeMs>0.0)&&(fabs(m_ErrorOffset)<arm.PulseBurstRange)&&(fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance))
	{
		if (BurstIntensity==0)
			printf("\rOff %.2f       ",m_CurrentBurstTime);
		else
			printf("\n+On=%.2f  pb=%.2f from bi=%.2f\n",m_CurrentBurstTime,PulseBurst,m_BurstIntensity);
	}
	#endif

	//Keep track of previous velocity to compute acceleration
	m_PreviousVelocity=Velocity;

	#if 0
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
	#else
	Voltage=m_VoltagePoly(Voltage);
	#endif

	if (IsZero(PotentiometerVelocity) && (CurrentVelocity==0.0))
	{
		//avoid dead zone... if we are accelerating set the dead zone to the minim value... all else zero out
		if (IsAccel)
			ComputeDeadZone(Voltage,m_Rotary_Props.Positive_DeadZone,m_Rotary_Props.Negative_DeadZone);
		else
			ComputeDeadZone(Voltage,m_Rotary_Props.Positive_DeadZone,m_Rotary_Props.Negative_DeadZone,true);
	}

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

	if (m_Rotary_Props.PID_Console_Dump)
	{
		NetworkEditProperties(m_Rotary_Props,true);
		m_PIDControllerUp.SetPID(m_Rotary_Props.ArmGainAssist.PID_Up[0],m_Rotary_Props.ArmGainAssist.PID_Up[1],m_Rotary_Props.ArmGainAssist.PID_Up[2]);
		m_PIDControllerDown.SetPID(m_Rotary_Props.ArmGainAssist.PID_Down[0],m_Rotary_Props.ArmGainAssist.PID_Down[1],m_Rotary_Props.ArmGainAssist.PID_Down[2]);
		switch (m_Rotary_Props.LoopState)
		{
		case Rotary_Props::eNone:
			SetPotentiometerSafety(true);
			break;
		case Rotary_Props::eOpen:
			m_PotentiometerState=ePassive;
			break;
		case Rotary_Props::eClosed:
		case Rotary_Props::eClosed_ManualAssist:
			m_PotentiometerState=eActive;
			break;
		}

		#ifdef __DebugLUA__
		const double PosY=m_LastPosition * arm.GainAssistAngleScalar; //The scalar makes position more readable
		const double PredictedPosY=GetPos_m()  * arm.GainAssistAngleScalar;
		if ((fabs(PotentiometerVelocity)>0.03)||(CurrentVelocity!=0.0)||(Voltage!=0.0))
		{
			//double PosY=RAD_2_DEG(m_LastPosition * arm.GainAssistAngleScalar);
			printf("v=%.2f y=%.2f py=%.2f p=%.2f e=%.2f eo=%.2f\n",Voltage,PosY,PredictedPosY,CurrentVelocity,PotentiometerVelocity,m_ErrorOffset);
		}
		//We may want a way to pick these separately 
		#if 1
		SmartDashboard::PutNumber("voltage",Voltage);
		SmartDashboard::PutNumber("actual y",PosY);
		SmartDashboard::PutNumber("desired y",PredictedPosY);
		SmartDashboard::PutNumber("desired velocity",CurrentVelocity);
		SmartDashboard::PutNumber("actual velocity",PotentiometerVelocity);
		SmartDashboard::PutNumber("pid error offset",m_ErrorOffset);
		#endif
		#endif
	}

	m_RobotControl->UpdateRotaryVoltage(m_InstanceIndex,Voltage);
}


void Rotary_Position_Control::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	//We may need this if we use Kalman filters
	m_RobotControl->Reset_Rotary(m_InstanceIndex);
	if ((m_PotentiometerState!=eNoPot)&&(!GetBypassPos_Update()))
	{
		m_PIDControllerUp.Reset();
		m_PIDControllerDown.Reset();
		m_RobotControl->Reset_Rotary(m_InstanceIndex);
		double NewPosition=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
		Stop();
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
	m_ToleranceCounter=0;
}

void Rotary_Position_Control::SetPotentiometerSafety(bool DisableFeedback)
{
	//printf("\r%f       ",Value);
	if (DisableFeedback)
	{
		if (m_PotentiometerState!=eNoPot)
		{
			//first disable it
			m_PotentiometerState=eNoPot;
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
		if (m_PotentiometerState==eNoPot)
		{
			switch (m_Rotary_Props.LoopState)
			{
			case Rotary_Props::eNone:
				m_PotentiometerState=eNoPot;
				//This should not happen but added for completeness
				printf("Rotary_Velocity_Control::SetEncoderSafety %s set to no potentiometer\n",GetName().c_str());
				break;
			case Rotary_Props::eOpen:
				m_PotentiometerState=ePassive;
				break;
			case Rotary_Props::eClosed:
			case Rotary_Props::eClosed_ManualAssist:
				m_PotentiometerState=eActive;
				break;
			}

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
	m_EncoderState(EncoderState),m_PreviousVelocity(0.0)
{
}

void Rotary_Velocity_Control::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	if ((m_EncoderState==eActive)||(m_EncoderState==ePassive))
		m_EncoderVelocity=m_RobotControl->GetRotaryCurrentPorV(m_InstanceIndex);
	__super::Initialize(em,props);
	const Rotary_Properties *Props=dynamic_cast<const Rotary_Properties *>(props);
	assert(Props);
	m_VoltagePoly.Initialize(&Props->GetRotaryProps().Voltage_Terms);
	//This will copy all the props
	m_Rotary_Props=Props->GetRotaryProps();
	m_PIDController.SetPID(m_Rotary_Props.PID[0],m_Rotary_Props.PID[1],m_Rotary_Props.PID[2]);

	//Note: for the drive we create a large enough number that can divide out the voltage and small enough to recover quickly,
	//but this turned out to be problematic when using other angular rotary systems... therefore I am going to use the same computation
	//I do for linear, where it allows as slow to max speed as possible.
	//  [3/20/2012 Terminator]
	m_PIDController.SetInputRange(m_MaxSpeed_Reverse,m_MaxSpeed_Forward);
	double tolerance=0.99; //we must be less than one (on the positive range) to avoid lockup
	m_PIDController.SetOutputRange(m_MaxSpeed_Reverse*tolerance,m_MaxSpeed_Forward*tolerance);
	m_PIDController.Enable();
	m_CalibratedScaler=m_MaxSpeed;
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
	default:
		assert(false);
	}
	//It is assumed that this property is constant throughout the whole session
	if (m_Rotary_Props.PID_Console_Dump)
		InitNetworkProperties(m_Rotary_Props);
}

void Rotary_Velocity_Control::UpdateRotaryProps(const Rotary_Props &RotaryProps)
{
	m_Rotary_Props=RotaryProps;
	m_CalibratedScaler=m_MaxSpeed;
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
	default:
		assert(false);
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
					m_CalibratedScaler=m_MaxSpeed+control;
				}
			}
			else
			{
				m_ErrorOffset=m_PIDController(CurrentVelocity,Encoder_Velocity,dTime_s);
				const double Acceleration=(CurrentVelocity-m_PreviousVelocity)/dTime_s;
				const bool Decel=(Acceleration * CurrentVelocity < 0);
				//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
				//Note: that it is important to bias towards deceleration this can help satisfy both requirements of avoiding oscillation as well
				//As well as avoiding a potential overshoot when trying stop at a precise distance
				m_ErrorOffset=Decel || fabs(m_ErrorOffset)>m_Rotary_Props.PrecisionTolerance?m_ErrorOffset:0.0;
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
	#if 0
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
	#else
	Voltage=m_VoltagePoly(Voltage);
	#endif

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

	if (m_Rotary_Props.PID_Console_Dump)
	{
		NetworkEditProperties(m_Rotary_Props);
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
		default:
			assert(false);
		}

		#ifdef __DebugLUA__
		if (Encoder_Velocity!=0.0)
		{
			if (m_Rotary_Props.UseAggressiveStop)
				printf("v=%.2f p=%.2f e=%.2f eo=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_ErrorOffset);
			else
			{
				if (m_PIDController.GetI()==0.0)
					printf("v=%.2f p=%.2f e=%.2f eo=%.2f cs=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_ErrorOffset,m_CalibratedScaler/m_MaxSpeed);
				else
					printf("v=%.2f p=%.2f e=%.2f i=%.2f cs=%.2f\n",Voltage,CurrentVelocity,Encoder_Velocity,m_PIDController.GetTotalError(),m_CalibratedScaler/m_MaxSpeed);
			}
		}
		#endif
	}

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
	m_CalibratedScaler=m_MaxSpeed;
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
			//m_MaxSpeed=m_MaxSpeedReference;
			m_EncoderVelocity=0.0;
			m_CalibratedScaler=m_MaxSpeed;
			m_ErrorOffset=0;
			m_UsingRange=false;
		}
	}
	else
	{
		if (m_EncoderState==eNoEncoder)
		{
			switch (m_Rotary_Props.LoopState)
			{
			case Rotary_Props::eNone:
				m_EncoderState=eNoEncoder;
				//This should not happen but added for completeness
				printf("Rotary_Velocity_Control::SetEncoderSafety %s set to no encoder\n",GetName().c_str());
				break;
			case Rotary_Props::eOpen:
				m_EncoderState=ePassive;
				break;
			case Rotary_Props::eClosed:
				m_EncoderState=eActive;
				break;
			default:
				assert(false);
			}
			//setup the initial value with the potentiometers value
			printf("Enabling encoder for %s\n",GetName().c_str());
			ResetPos();
			m_UsingRange=GetUsingRange_Props();
			m_CalibratedScaler=m_MaxSpeed;
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
	props.PotentiometerOffset=0.0;
	//Late assign this to override the initial default
	props.PID[0]=1.0; //set PIDs to a safe default of 1,0,0
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.LoopState=Rotary_Props::eNone;  //Always false when control is fully functional
	props.PID_Console_Dump=false;  //Always false unless you want to analyze PID (only one system at a time!)
	props.UseAggressiveStop=false;  //This is only for angular so false is a good default (must be explicit in script otherwise)
	#if 0
	props.Polynomial[0]=0.0;
	props.Polynomial[1]=1.0;
	props.Polynomial[2]=0.0;
	props.Polynomial[3]=0.0;
	props.Polynomial[4]=0.0;
	#else
	props.Voltage_Terms.Init();
	#endif
	props.InverseMaxAccel=props.InverseMaxAccel=0.0;
	props.Positive_DeadZone=props.Negative_DeadZone=0.0;
	Rotary_Props::Rotary_Arm_GainAssist_Props &arm=props.ArmGainAssist; 
	arm.SlowVelocity=arm.SlowVelocityVoltage=0.0;
	arm.GainAssistAngleScalar=1.0;
	arm.InverseMaxAccel_Down=arm.InverseMaxAccel_Up=arm.InverseMaxDecel_Down=arm.InverseMaxDecel_Up=0.0;
	for (size_t i=0;i<3;i++)
		arm.PID_Down[i]=arm.PID_Up[i]=0.0;
	arm.ToleranceConsecutiveCount=1;
	arm.VelocityPredictUp=arm.VelocityPredictDown=0.0;
	arm.UsePID_Up_Only=false;
	arm.PulseBurstRange=0.0;
	arm.PulseBurstTimeMs=0.0;
	m_RotaryProps=props;
}

void Rotary_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	Rotary_Props::Rotary_Arm_GainAssist_Props &arm=m_RotaryProps.ArmGainAssist;
	string sTest;
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
		script.GetField("pot_offset", NULL, NULL, &m_RotaryProps.PotentiometerOffset);
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
			for (size_t i=0;i<3;i++)
				m_RotaryProps.ArmGainAssist.PID_Up[i]=arm.PID_Down[i]=m_RotaryProps.PID[i];
		}
		err=script.GetField("use_pid_up_only", &sTest, NULL, NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				arm.UsePID_Up_Only=false;
			else if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				arm.UsePID_Up_Only=true;
		}

		err = script.GetFieldTable("pid_up");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&arm.PID_Up[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&arm.PID_Up[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&arm.PID_Up[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

		err = script.GetFieldTable("pid_down");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&arm.PID_Down[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&arm.PID_Down[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&arm.PID_Down[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

		script.GetField("tolerance", NULL, NULL, &m_RotaryProps.PrecisionTolerance);
		double fValue;
		err=script.GetField("tolerance_count", NULL, NULL, &fValue);
		if (!err)
			arm.ToleranceConsecutiveCount=(size_t)fValue;

		err=script.GetField("ds_display_row", NULL, NULL, &fValue);
		if (!err)
			m_RotaryProps.Feedback_DiplayRow=(size_t)fValue;

		//I've made it closed so that typing no or NO stands out, but you can use bool as well
		//err = script.GetField("is_closed",&sTest,&bTest,NULL);
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_RotaryProps.LoopState=Rotary_Props::eOpen;
			else if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RotaryProps.LoopState=Rotary_Props::eClosed;
			else if (sTest.c_str()[0]=='2')		//support multiple ways to have this mode
				m_RotaryProps.LoopState=Rotary_Props::eClosed_ManualAssist;
		}
		err = script.GetField("is_closed2",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_RotaryProps.LoopState=Rotary_Props::eOpen;
			else if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_RotaryProps.LoopState=Rotary_Props::eClosed_ManualAssist;
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

		m_RotaryProps.Voltage_Terms.LoadFromScript(script,"curve_voltage");

		script.GetField("inv_max_accel", NULL, NULL, &m_RotaryProps.InverseMaxAccel);
		m_RotaryProps.InverseMaxDecel=m_RotaryProps.InverseMaxAccel;	//set up deceleration to be the same value by default
		arm.InverseMaxAccel_Up=arm.InverseMaxAccel_Down=arm.InverseMaxDecel_Up=arm.InverseMaxDecel_Down=m_RotaryProps.InverseMaxAccel;
		err=script.GetField("inv_max_decel", NULL, NULL, &m_RotaryProps.InverseMaxDecel);
		if (!err)
			arm.InverseMaxDecel_Up=arm.InverseMaxDecel_Down=m_RotaryProps.InverseMaxAccel;
		script.GetField("inv_max_accel_up", NULL, NULL, &arm.InverseMaxAccel_Up);
		script.GetField("inv_max_accel_down", NULL, NULL, &arm.InverseMaxAccel_Down);
		script.GetField("inv_max_decel_up", NULL, NULL, &arm.InverseMaxDecel_Up);
		script.GetField("inv_max_decel_down", NULL, NULL, &arm.InverseMaxDecel_Down);

		script.GetField("forward_deadzone", NULL, NULL,&m_RotaryProps.Positive_DeadZone);
		script.GetField("reverse_deadzone", NULL, NULL,&m_RotaryProps.Negative_DeadZone);
		//Ensure the negative settings are negative
		if (m_RotaryProps.Negative_DeadZone>0.0)
			m_RotaryProps.Negative_DeadZone=-m_RotaryProps.Negative_DeadZone;
		//TODO may want to swap forward in reverse settings if the voltage multiply is -1  (I'll want to test this as it happens)

		script.GetField("slow_velocity_voltage", NULL, NULL,&arm.SlowVelocityVoltage);
		script.GetField("slow_velocity", NULL, NULL,&arm.SlowVelocity);
		script.GetField("slow_angle_scalar", NULL, NULL, &arm.GainAssistAngleScalar);
		script.GetField("predict_up",NULL,NULL,&arm.VelocityPredictUp);
		script.GetField("predict_down",NULL,NULL,&arm.VelocityPredictDown);
		script.GetField("pulse_burst_range",NULL,NULL,&arm.PulseBurstRange);
		script.GetField("pulse_burst_time",NULL,NULL,&arm.PulseBurstTimeMs);

		#ifdef Robot_TesterCode
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