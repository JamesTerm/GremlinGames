#include "stdafx.h"
#include "AI_Tester.h"
#include "Debug.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "CommonUI.h"
	#include "FRC2013_Robot.h"
}

#include "UDP_Listener.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;


#undef __DisableEncoderTracking__
//Perhaps off season we can experiment with being field aware (some code in place for this)
#define __NotFieldAware__
#define __EnablePitchDisplay__
#undef __EnableTargetingDisplay__

//This will make the scale to half with a 0.1 dead zone
static double PositionToVelocity_Tweak(double Value)
{
	const double FilterRange=0.1;
	const double Multiplier=0.5;
	const bool isSquared=true;
	double Temp=fabs(Value); //take out the sign... put it back in the end
	Temp=(Temp>=FilterRange) ? Temp-FilterRange:0.0; 

	Temp=Multiplier*(Temp/(1.0-FilterRange)); //apply scale first then 
	if (isSquared) Temp*=Temp;  //square it if it is squared

	//Now to restore the sign
	Value=(Value<0.0)?-Temp:Temp;
	return Value;
}


  /***********************************************************************************************************************************/
 /*													FRC_2013_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2013_Robot::PitchRamp::PitchRamp(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("PitchRamp",robot_control,ePitchRamp),m_pParent(pParent)
{
}

void FRC_2013_Robot::PitchRamp::SetIntendedPosition_Plus(double Position)
{
	if (GetIsUsingPotentiometer())
	{
		bool IsTargeting=(m_pParent->m_IsTargeting);
		if (!IsTargeting)
		{
			Position=-Position; //flip this around I want the pitch and power to work in the same direction where far away is lower pitch
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (Position * 0.5) + 0.5;
			//positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=GetMaxRange();
			const double Scale=(maxRange-minRange) / maxRange;
			Position=(positive_range * Scale) + minRange;
		}
		//DOUT5("Test=%f",RAD_2_DEG(Position));
		SetIntendedPosition(Position);
	}
	else
		SetRequestedVelocity_FromNormalized(PositionToVelocity_Tweak(Position));   //allow manual use of same control

}

void FRC_2013_Robot::PitchRamp::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && (IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer());
	if (IsTargeting)
	{
		__super::SetIntendedPosition(m_pParent->m_PitchAngle * m_pParent->m_PitchErrorCorrection);
	}
	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2013_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2013_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2013_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2013_Robot::PitchRamp::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2013_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Remove(*this, &FRC_2013_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Remove(*this, &FRC_2013_Robot::PitchRamp::SetPotentiometerSafety);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Robot::PowerWheels														*/
/***********************************************************************************************************************************/

FRC_2013_Robot::PowerWheels::PowerWheels(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	m_pParent(pParent),m_SecondStage("SecondStage",robot_control,ePowerWheelSecondStage,Rotary_Velocity_Control::eActive),
	m_FirstStage("FirstStage",robot_control,ePowerWheelFirstStage,Rotary_Velocity_Control::eActive),m_ManualVelocity(0.0),m_IsRunning(false)
{
}

void FRC_2013_Robot::PowerWheels::Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	m_SecondStage.Initialize(em,props);
	m_FirstStage.Initialize(em,&m_pParent->GetRobotProps().GetPowerSlowWheelProps());
}

void FRC_2013_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_SecondStage.GetEventMap(); 
	if (Bind)
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2013_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Subscribe(ehl,*this, &FRC_2013_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Subscribe(ehl,*this, &FRC_2013_Robot::PowerWheels::SetIsRunning);
	}
	else
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Remove(*this, &FRC_2013_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Remove(*this, &FRC_2013_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Remove(*this, &FRC_2013_Robot::PowerWheels::SetIsRunning);
	}
}

void FRC_2013_Robot::PowerWheels::SetRequestedVelocity_FromNormalized(double Velocity) 
{
	//bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	//This variable is dedicated to non-targeting mode
	m_ManualVelocity=Velocity;
}

void FRC_2013_Robot::PowerWheels::SetEncoderSafety(bool DisableFeedback) 
{
	m_SecondStage.SetEncoderSafety(DisableFeedback);
	m_FirstStage.SetEncoderSafety(DisableFeedback);
}

void FRC_2013_Robot::PowerWheels::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && m_SecondStage.GetEncoderUsage()==Rotary_Velocity_Control::eActive);
	if (  IsTargeting )
	{
		if ((m_IsRunning)||(m_pParent->m_IntakeSystem.GetIsFireRequested())) 
		{
			//convert linear velocity to angular velocity
			double RPS=m_pParent->m_LinearVelocity / (Pi * m_SecondStage.GetDimension());
			RPS*=(2.0 * m_pParent->m_PowerErrorCorrection);  //For hooded shoot we'll have to move twice as fast
			m_SecondStage.SetRequestedVelocity(RPS * Pi2);
			const double FirstStageScalar=0.5;  //TODO properties
			m_FirstStage.SetRequestedVelocity(RPS * FirstStageScalar * Pi2);
			//DOUT5("rps=%f rad=%f",RPS,RPS*Pi2);
		}
		else
		{
			m_SecondStage.SetRequestedVelocity(0);
			m_FirstStage.SetRequestedVelocity(0);
		}
	}
	else
	{
		if ((m_IsRunning)||(m_pParent->m_IntakeSystem.GetIsFireRequested()))
		{
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (m_ManualVelocity * 0.5) + 0.5;
			positive_range=positive_range>0.01?positive_range:0.0;
			const double MaxSpeed=m_SecondStage.GetMaxSpeed();
			const double minRange=m_SecondStage.GetMinRange();
			//Note: this may want to be MaxRange to be consistent
			const double maxRange=m_SecondStage.GetMaxSpeed();
			const double Scale=(maxRange-minRange) / MaxSpeed;
			const double Offset=minRange / MaxSpeed;
			const double Velocity=(positive_range * Scale) + Offset;
			//DOUT5("%f",Velocity);
			size_t DisplayRow=m_pParent->m_RobotProps.GetFRC2013RobotProps().PowerVelocity_DisplayRow;
			if (DisplayRow!=(size_t)-1)
			{
				const double rps=(Velocity * MaxSpeed) / Pi2;
				Dout(DisplayRow,"%f ,%f",rps,Meters2Feet(rps * Pi * m_SecondStage.GetDimension()));
			}

			m_SecondStage.SetRequestedVelocity_FromNormalized(Velocity);
			const double FirstStageScalar=0.5;  //TODO properties
			m_FirstStage.SetRequestedVelocity_FromNormalized(Velocity * FirstStageScalar);
		}
		else
		{
			m_SecondStage.SetRequestedVelocity_FromNormalized(0.0);
			m_FirstStage.SetRequestedVelocity_FromNormalized(0.0);
		}
	}
	m_SecondStage.AsEntity1D().TimeChange(dTime_s);
	m_FirstStage.AsEntity1D().TimeChange(dTime_s);
}

void FRC_2013_Robot::PowerWheels::ResetPos()
{
	m_IsRunning=false;
	m_SecondStage.ResetPos();
	m_FirstStage.ResetPos();
}


  /***********************************************************************************************************************************/
 /*											FRC_2013_Robot::IntakeSystem::Intake_Deployment											*/
/***********************************************************************************************************************************/
FRC_2013_Robot::IntakeSystem::Intake_Deployment::Intake_Deployment(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control) : 
Rotary_Position_Control("Intake_Deployment",robot_control,eIntake_Deployment),m_pParent(pParent),m_Advance(false),m_Retract(false)
{
}

void FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetIntendedPosition(double Position)
{
	//DOUT5("Test=%f",RAD_2_DEG(Position));
	__super::SetIntendedPosition(Position);
}

void FRC_2013_Robot::IntakeSystem::Intake_Deployment::Advance()
{
	SetIntendedPosition(m_pParent->m_RobotProps.GetIntakeDeploymentProps().GetShip_1D_Props().MaxRange);
}
void FRC_2013_Robot::IntakeSystem::Intake_Deployment::Retract()
{
	SetIntendedPosition(m_pParent->m_RobotProps.GetIntakeDeploymentProps().GetShip_1D_Props().MinRange);
}

void FRC_2013_Robot::IntakeSystem::Intake_Deployment::TimeChange(double dTime_s)
{
	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2013_Robot::IntakeSystem::Intake_Deployment::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Intake_Deployment_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Intake_Deployment_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetIntendedPosition);
		em->EventOnOff_Map["Intake_Deployment_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetPotentiometerSafety);
		em->Event_Map["Intake_Deployment_Advance"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::Advance);
		em->Event_Map["Intake_Deployment_Retract"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::Retract);
	}
	else
	{
		em->EventValue_Map["Intake_Deployment_SetCurrentVelocity"].Remove(*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Intake_Deployment_SetIntendedPosition"].Remove(*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetIntendedPosition);
		em->EventOnOff_Map["Intake_Deployment_SetPotentiometerSafety"].Remove(*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::SetPotentiometerSafety);
		em->Event_Map["Intake_Deployment_Advance"].Remove(*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::Advance);
		em->Event_Map["Intake_Deployment_Retract"].Remove(*this, &FRC_2013_Robot::IntakeSystem::Intake_Deployment::Retract);
	}
}

  /***********************************************************************************************************************************/
 /*												FRC_2013_Robot::IntakeSystem														*/
/***********************************************************************************************************************************/

FRC_2013_Robot::IntakeSystem::IntakeSystem(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_Helix("Helix",robot_control,eHelix),m_Rollers("Rollers",robot_control,eRollers),
	m_IntakeDeployment(pParent,robot_control),
	m_FireDelayTrigger_Time(0.0),m_FireStayOn_Time(0.0),
	m_FireDelayTriggerOn(false),m_FireStayOn(false)
{
	m_ControlSignals.raw=0;
	//This are always open loop as there is no encoder and this is specified by default
}

void FRC_2013_Robot::IntakeSystem::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_Helix.Initialize(em,props);
	m_Rollers.Initialize(em,props);  //borrowing helix props as it is simple (like we did for conveyor)
	m_IntakeDeployment.Initialize(em,&m_pParent->GetRobotProps().GetIntakeDeploymentProps());
}
void FRC_2013_Robot::IntakeSystem::ResetPos() 
{
	m_Helix.ResetPos();
	m_Rollers.ResetPos();
	m_IntakeDeployment.ResetPos();
	m_ControlSignals.raw=0;
}

void FRC_2013_Robot::IntakeSystem::TimeChange(double dTime_s)
{
	const FRC_2013_Robot_Properties &properties=m_pParent->m_RobotProps;
	const FRC_2013_Robot_Props &props=properties.GetFRC2013RobotProps();

	//const bool FireSensor=m_pParent->m_RobotControl->GetBoolSensorState(eFireConveyor_Sensor);
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetSecondStageShooter().GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=(m_pParent->m_PowerWheels.GetSecondStageShooter().GetRequestedVelocity()!=0.0) &&
		(fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetSecondStageShooter().GetRotary_Properties().PrecisionTolerance);
	//Only fire when the wheel has reached its aiming speed
	bool Fire=(m_ControlSignals.bits.Fire==1) && PowerWheelReachedTolerance;
	bool Grip=m_ControlSignals.bits.Grip==1;
	//bool GripH=m_ControlSignals.bits.GripH==1;
	bool Squirt=m_ControlSignals.bits.Squirt==1;

	if (Fire)
	{
		if (m_FireDelayTriggerOn)
		{
			m_FireDelayTrigger_Time+=dTime_s;
			//printf("Fire delaying =%f\n",m_FireDelayTrigger_Time);
			if (m_FireDelayTrigger_Time>props.FireTriggerDelay)
				m_FireDelayTriggerOn=false;
		}
	}
	else
	{
		m_FireDelayTriggerOn=true;
		m_FireDelayTrigger_Time=0.0;
	}

	Fire = Fire && !m_FireDelayTriggerOn;

	if (Fire)
	{
		m_FireStayOn=true;
		m_FireStayOn_Time=0.0;
	}
	else
	{
		if (m_FireStayOn)
		{
			m_FireStayOn_Time+=dTime_s;
			//printf("Fire Staying on=%f\n",m_FireStayOn_Time);
			if (m_FireStayOn_Time>props.FireButtonStayOn_Time)
				m_FireStayOn=false;
		}
	}

	//manage fire piston
	if (m_pParent->m_RobotControl->GetIsSolenoidOpen(eFirePiston))
	{
		//manage if when to release
		if ((!Fire)&&(!m_FireStayOn))
			m_pParent->m_RobotControl->CloseSolenoid(eFirePiston);
	}
	else
	{
		//manage when to fire
		if (Fire)
			m_pParent->m_RobotControl->OpenSolenoid(eFirePiston);
	}

	//This assumes the motors are in the same orientation: 
	//Note: we may want to spin helix during fire, but for now... assume operator will load them all in hopper before firing
	const double HelixAcceleration= Grip | Squirt ?
		((Squirt)?m_Helix.GetACCEL():-m_Helix.GetBRAKE()):0.0;
	m_Helix.SetCurrentLinearAcceleration(HelixAcceleration);

	//manage rollers... just like helix except that it is off it it is stowed
	const double Intake_Position=m_pParent->m_RobotControl->GetRotaryCurrentPorV(eIntake_Deployment);
	const bool IsStowed=(fabs(Intake_Position-properties.GetIntakeDeploymentProps().GetShip_1D_Props().MinRange) < properties.GetIntakeDeploymentProps().GetRoteryProps().PrecisionTolerance);
	const double RollerAcceleration= (!IsStowed || Squirt) ? HelixAcceleration : 0.0;
	m_Rollers.SetCurrentLinearAcceleration(RollerAcceleration);

	m_Helix.AsEntity1D().TimeChange(dTime_s);
	m_IntakeDeployment.AsEntity1D().TimeChange(dTime_s);
	m_Rollers.AsEntity1D().TimeChange(dTime_s);
}

//This is the manual override, but probably not used if we use spike as it would be wasteful to have a analog control for this
void FRC_2013_Robot::IntakeSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_Helix.SetRequestedVelocity_FromNormalized(Velocity);
}

void FRC_2013_Robot::IntakeSystem::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_Helix.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//Ball_SetCurrentVelocity is the manual override
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2013_Robot::IntakeSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2013_Robot::IntakeSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2013_Robot::IntakeSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2013_Robot::IntakeSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2013_Robot::IntakeSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2013_Robot::IntakeSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2013_Robot::IntakeSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2013_Robot::IntakeSystem::Squirt);
	}
	m_IntakeDeployment.BindAdditionalEventControls(Bind);
}


  /***********************************************************************************************************************************/
 /*															FRC_2013_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;
const FRC_2013_Robot::Vec2D c_BridgeDimensions=FRC_2013_Robot::Vec2D(Inches2Meters(48),Inches2Meters(88)); //width x length

const FRC_2013_Robot::Vec2D c_TargetBasePosition=FRC_2013_Robot::Vec2D(0.0,c_HalfCourtLength);
const double c_BallShootHeight_inches=55.0;
const double c_TargetBaseHeight= Inches2Meters(98.0 - c_BallShootHeight_inches);
const double c_Target_MidBase_Height= Inches2Meters(61.0 - c_BallShootHeight_inches);
const double c_Target_MiddleHoop_XOffset=Inches2Meters(27+3/8);


FRC_2013_Robot::FRC_2013_Robot(const char EntityName[],FRC_2013_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), m_PitchRamp(this,robot_control),
		m_PowerWheels(this,robot_control),m_IntakeSystem(this,robot_control),
		m_Target(eCenterHighGoal),m_DefensiveKeyPosition(Vec2D(0.0,0.0)),m_UDP_Listener(NULL),
		m_PitchAngle(0.0),
		m_LinearVelocity(0.0),m_HangTime(0.0),  //These may go away
		m_PitchErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),m_AutonPresetIndex(0),
		m_POVSetValve(false),m_IsTargeting(false),m_DriveTargetSelection(eDrive_NoTarget),
		m_SetClimbGear(false),m_SetClimbLeft(false),m_SetClimbRight(false)
{
	//m_IsTargeting=true;
	//m_DriveTargetSelection=eDrive_Goal_Yaw; //for testing until button is implemented
	m_UDP_Listener=coodinate_manager_Interface::CreateInstance();
}

FRC_2013_Robot::~FRC_2013_Robot()
{
	coodinate_manager_Interface::DestroyInstance((coodinate_manager_Interface *)m_UDP_Listener);
}

void FRC_2013_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2013_Robot_Properties *RobotProps=dynamic_cast<const FRC_2013_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
	m_IntakeSystem.Initialize(em,RobotProps?&RobotProps->GetHelixProps():NULL);

	//TODO see if this is still needed
	#if 0
	//set to the default key position
	const FRC_2013_Robot_Props &robot2013props=RobotProps->GetFRC2013RobotProps();
	SetDefaultPosition(robot2013props.PresetPositions[m_DefaultPresetIndex]);
	#endif
}
void FRC_2013_Robot::ResetPos()
{
	//TODO determine if we need to worry about resetting position
	//SetBypassPosAtt_Update(true);
	__super::ResetPos();
	//SetBypassPosAtt_Update(false);

	//This should be false to avoid any conflicts during a reset
	//m_IsTargeting=false;
	m_PitchRamp.ResetPos();
	m_PowerWheels.ResetPos();
	m_IntakeSystem.ResetPos();
	if (!m_SetClimbGear)
	{
		//ensure pneumatics are in drive
		SetClimbState(eClimbState_Drive);
	}
}

FRC_2013_Robot::IntakeSystem &FRC_2013_Robot::GetIntakeSystem()
{
	return m_IntakeSystem;
}

FRC_2013_Robot::PowerWheels &FRC_2013_Robot::GetPowerWheels()
{
	return m_PowerWheels;
}

void FRC_2013_Robot::ApplyErrorCorrection()
{
	const FRC_2013_Robot_Props &robot_props=m_RobotProps.GetFRC2013RobotProps();
	#ifndef __DisableEncoderTracking__
	const Vec2d &Pos_m=GetPos_m();
	#else
	const Vec2d &Pos_m=	robot_props.PresetPositions[m_AutonPresetIndex];
	#endif
	//first determine which quadrant we are in
	//These offsets are offsets added to the array indexes 
	const size_t XOffset=(Pos_m[0]>(robot_props.KeyGrid[1][1])[0]) ? 1 : 0;
	const double YCenterKey=(robot_props.KeyGrid[1][1])[1];
	//TODO figure out y coordinate system offset for 6 rows vs 3
	//The coordinate system is backwards for Y 
	const size_t YOffset=(Pos_m[1] < YCenterKey) ? 1 : 0;
	//Find our normalized targeted coordinates; saturate as needed
	const Vec2D &q00=robot_props.KeyGrid[0+YOffset][0+XOffset];
	const Vec2D &q01=robot_props.KeyGrid[0+YOffset][1+XOffset];
	const double XWidth=q01[0]-q00[0];
	const double xStart=max(Pos_m[0]-q00[0],0.0);
	const double x=min(xStart/XWidth,1.0);
	const Vec2D &q10=robot_props.KeyGrid[1+YOffset][0+XOffset];
	const double YToUse=(c_HalfCourtLength-Pos_m[1]) + (2.0*YCenterKey - c_HalfCourtLength);
	const double YWidth=q10[1]-q00[1];
	const double yStart=max(YToUse-q00[1],0.0);
	const double y=min(yStart/YWidth,1.0);
	//Now to blend.  Top half, bottom half then the halves
	const FRC_2013_Robot_Props::DeliveryCorrectionFields &c00=robot_props.KeyCorrections[0+YOffset][0+XOffset];
	const FRC_2013_Robot_Props::DeliveryCorrectionFields &c01=robot_props.KeyCorrections[0+YOffset][1+XOffset];
	const FRC_2013_Robot_Props::DeliveryCorrectionFields &c10=robot_props.KeyCorrections[1+YOffset][0+XOffset];
	const FRC_2013_Robot_Props::DeliveryCorrectionFields &c11=robot_props.KeyCorrections[1+YOffset][1+XOffset];

	const double pc_TopHalf=    (x * c01.PowerCorrection) + ((1.0-x)*c00.PowerCorrection);
	const double pc_BottomHalf= (x * c11.PowerCorrection) + ((1.0-x)*c10.PowerCorrection);
	const double pc = (y * pc_BottomHalf) + ((1.0-y) * pc_TopHalf);

	const double yc_TopHalf=    (x * c01.PitchCorrection) + ((1.0-x)*c00.PitchCorrection);
	const double yc_BottomHalf= (x * c11.PitchCorrection) + ((1.0-x)*c10.PitchCorrection);
	const double yc = (y * yc_TopHalf) + ((1.0-y) * yc_BottomHalf);

	//Now to apply correction... for now we'll apply to the easiest pieces possible and change if needed
	m_PitchErrorCorrection=yc;
	m_PowerErrorCorrection=pc;
	//DOUT(5,"pc=%.2f yc=%.2f x=%.2f y=%.2f xo=%d yo=%d",pc,yc,x,y,XOffset,YOffset);
	//We can use the error grid cells directly by simply positioning the robot at the right place
	size_t HackedIndex;
	switch (m_Target)
	{
	case eLeftGoal:
	case eRightGoal:
		m_PowerErrorCorrection=robot_props.Autonomous_Props.TwoShotScaler;
		break;
	default:
		HackedIndex=0;
		break;
	}

}

namespace VisionConversion
{
	const double c_X_Image_Res=640.0;		//X Image resolution in pixels, should be 160, 320 or 640
	//const double c_ViewAngle=43.5;  //Axis M1011 camera (in code sample)
	const double c_ViewAngle_x=47;		//These are the angles I've measured
	const double c_ViewAngle_y=40;
	const double c_HalfViewAngle_y=c_ViewAngle_y/2.0;

	//doing it this way is faster since it never changes
	const double c_ez_y=1.0/(tan(DEG_2_RAD(c_ViewAngle_y)/2.0));
	const double c_ez_x=(tan(DEG_2_RAD(c_ViewAngle_x)/2.0));

	//For example if the target height is 22.16 feet the distance would be 50, or 10 foot height would be around 22 feet for distance
	//this constant is used to check my pitch math below (typically will be disabled)
	const double c_DistanceCheck=c_TargetBaseHeight*c_ez_y;

	__inline void GetYawAndDistance(double bx,double by,double &dx,double dy,double &dz)
	{
		//Note: the camera angle for x is different than for y... thus for example we have 4:3 aspect ratio
		dz = (dy * c_ez_y) / by;
		dx = (bx * dz) * c_ez_x;
	}

	//This transform is simplified to only works with pitch
	__inline void CameraTransform(double ThetaY,double dx, double dy, double dz, double &ax, double &ay, double &az)
	{
		//assert(ThetaY<PI);
		//ax=(dz*sin(ThetaY) + dx) / cos(ThetaY);
		ax=dx;  //I suspect that I may be interpreting the equation wrong... it seems to go in the wrong direction, but may need to retest 
		ay=dy;
		az=cos(ThetaY)*dz - sin(ThetaY)*dx;
	}

	__inline bool computeDistanceAndYaw (double Ax1,double Ay1,double currentPitch,double &ax1,double &az1) 
	{
		if (IsZero(Ay1)) return false;  //avoid division by zero
		//Now to input the aiming system for the d (x,y,z) equations prior to camera transformation
		const double dy = c_TargetBaseHeight;
		double dx,dz;
		GetYawAndDistance(Ax1,Ay1,dx,dy,dz);
		double ay1;
		CameraTransform(currentPitch,dx,dy,dz,ax1,ay1,az1);
		//TODO see if we want kalman
		//printf("\r x=%.2f y=%.2f dx=%.2f dz=%.2f       ",m_Dx(Ax1),m_Dz(Ay1),m_Ax(dx),m_Az(dz));
		//printf("\r dx=%.2f dz=%.2f ax=%.2f az=%.2f       ",m_Dx(dx),m_Dz(dz),m_Ax(ax1),m_Az(az1));

		//printf("x=%.2f y=%.2f dx=%.2f dz=%.2f ax=%.2f az=%.2f\n",Ax1,Ay1,dx,dz,ax1,az1);

		return true;
		//return c_X_Image_Res * c_TargetBaseHeight / (height * 12 * 2 * tan(DEG_2_RAD(c_ViewAngle)));
	}

}



void FRC_2013_Robot::TimeChange(double dTime_s)
{
	coodinate_manager_Interface *listener=(coodinate_manager_Interface *)m_UDP_Listener;
	listener->TimeChange(dTime_s);

	//Leave the macro enable for ease of disabling the corrections (in case it goes horribly wrong) :)
	#if 1
	if (listener->IsUpdated())
	{
		//TODO see if we want a positive Y for up... for now we can convert it here
		const double  YOffset=-listener->GetYpos();
		const double XOffset=listener->GetXpos();
		//If Ypos... is zero no work needs to be done for pitch... also we avoid division by zero too
		//the likelihood of this is rare, but in theory it could make yaw not work for that frame.  
		//Fortunately for us... we'll have error correction because of gravity... so for the game it should be impossible for this to happen except for the rare
		//graze across to get to the final targeting point... point being... the final target point should rest above the target.

		//printf("New coordinates %f , %f\n",listener->GetXpos(),listener->GetYpos());
		const double CurrentPitch=m_RobotControl->GetRotaryCurrentPorV(ePitchRamp);
		double distance,yaw;
		if ((YOffset!=0)&&(VisionConversion::computeDistanceAndYaw(listener->GetXpos(),YOffset,CurrentPitch,yaw,distance)))
		{
			 //monitor where it should be against where it actually is
			//printf("p=%.2f a=%.2f\n",m_PitchAngle,CurrentPitch);
			//printf("d=%.2f\n",Meters2Feet(distance));
			//Check math... let's see how the pitch angle measures up to simple offset (it will not factor in the camera transform, but should be close anyhow)
			#ifdef __EnableTargetingDisplay__
			#if 0
			const double PredictedOffset=tan(m_PitchAngle)*VisionConversion::c_DistanceCheck;
			Dout (4,"p%.2f y%.2f t%.2f e%.2f",RAD_2_DEG(CurrentPitch),YOffset,PredictedOffset,PredictedOffset-YOffset);
			#endif
			#if 0
			const double PredictedOffset=sin(atan(yaw/distance))*VisionConversion::c_DistanceCheck;
			Dout (4,"y%.2f x%.2f t%.2f e%.2f",RAD_2_DEG(CurrentPitch),XOffset,PredictedOffset,PredictedOffset-XOffset);
			//Dout (4,"x=%.2f yaw=%.2f",XOffset,yaw);
			#endif
			#endif

			#ifndef __NotFieldAware__
			//Now for the final piece... until we actually solve for orientation we'll exclusively just set the ypos to the distance
			//Note: if we were field aware by solving the orientation we could this by placing the final position here, but since we are not (at least for now)
			//we can just adjust for Y and use the POV turning calls for yaw correction
			const Vec2d &Pos_m=GetPos_m();
			SetPosition(Pos_m[0],c_HalfCourtLength-distance);
			#else
			//printf("\rD=%.2f      ",distance);
			//m_PitchAngle=CurrentPitch+atan(m_TargetHeight/distance);
			#if 1
			const double NewPitch=CurrentPitch+atan(YOffset/VisionConversion::c_DistanceCheck);
			#else
			//Enable this for playback of file since it cannot really cannot control the pitch
			const double NewPitch=atan(m_TargetHeight/distance);
			#endif

			//Use precision tolerance asset to determine whether to make the change
			m_PitchAngle=(fabs(NewPitch-CurrentPitch)>m_RobotProps.GetPitchRampProps().GetRoteryProps().PrecisionTolerance)?NewPitch:CurrentPitch;

			//ensure we do not have some crazy computation of pitch
			if (m_PitchAngle>DEG_2_RAD(80))
				m_PitchAngle=DEG_2_RAD(80);
			else if (m_PitchAngle<0)
				m_PitchAngle=0;
			#endif

			if (m_DriveTargetSelection==eDrive_Goal_Yaw)
			{
				//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
				double value=atan(yaw/distance);
				if (fabs(value)>m_RobotProps.GetFRC2013RobotProps().YawTolerance)
				{
					//We set this through the controller so that it goes through the same path and ensures that its in the right mode (just as it is for POV turns)
					m_controller->GetUIController_RW()->Turn_RelativeOffset(value);
				}
			}
		}
		//else
			//printf("FRC_2013_Robot::TimeChange YOffset=%f\n",YOffset);  //just curious to see how often this would really occur
	}
	#endif

	const FRC_2013_Robot_Props &robot_props=m_RobotProps.GetFRC2013RobotProps();
	#ifndef __DisableEncoderTracking__
	const Vec2d &Pos_m=GetPos_m();
	//Got to make this fit within 20 chars :(
	Dout(robot_props.Coordinates_DiplayRow,"%.2f %.2f %.1f",Meters2Feet(Pos_m[0]),
		Meters2Feet(Pos_m[1]),RAD_2_DEG(GetAtt_r()));
	#else
	const Vec2d &Pos_m=	robot_props.PresetPositions[m_AutonPresetIndex];
	{	//Even though this is disabled... still want it to read correctly for encoder reading and calibration
		const Vec2d &Pos_temp=GetPos_m();
		Dout(robot_props.Coordinates_DiplayRow,"%.2f %.2f %.1f",Meters2Feet(Pos_temp[0]),
			Meters2Feet(Pos_temp[1]),RAD_2_DEG(GetAtt_r()));
	}
	#endif

	switch (m_Target)
	{
		case eCenterHighGoal:
			m_TargetOffset=c_TargetBasePosition;  //2d top view x,y of the target
			m_TargetHeight=c_TargetBaseHeight;    //1d z height (front view) of the target
			break;
		case eLeftGoal:
			m_TargetOffset=Vec2D(-c_Target_MiddleHoop_XOffset,c_HalfCourtLength);
			m_TargetHeight=c_Target_MidBase_Height;
			break;
		case eRightGoal:
			m_TargetOffset=Vec2D(c_Target_MiddleHoop_XOffset,c_HalfCourtLength);
			m_TargetHeight=c_Target_MidBase_Height;
			break;
		case eDefensiveKey:
			m_TargetOffset=m_DefensiveKeyPosition;
			m_TargetHeight=0;
			break;
	}

	const double x=Vec2D(Pos_m-m_TargetOffset).length();

	//TODO clean this up... mostly obsolete
	//
	//Now to compute my pitch, power, and hang time
	{
		//TODO factor in rotation if it is significant
		const double y=m_TargetHeight;
		const double y2=y*y;
		const double x2=x*x;
		const double g=9.80665;
		//These equations come from here http://www.lightingsciences.ca/pdf/BNEWSEM2.PDF

		#ifndef __NotFieldAware__
		m_PitchAngle=atan2(y,x);
		#endif

		//Be sure G is in the same units as x and y!  (all in meters in code)
		//	V=sqrt(G(sqrt(y^2+x^2)+y))
		//	This is equation 7 solving v
		m_LinearVelocity=sqrt(g*(sqrt(y2+x2)+y));

		ApplyErrorCorrection();

		//ta=(sin(theta)*v)/g   //This is equation 2 solving for t1
		//tb=(x-ta*cos(theta)*v)/(cos(theta)*v)   //this is equation 3 solving for t2
		//	hang time= ta+tb 
		double ta,tb;
		ta=(sin(m_PitchAngle)*m_LinearVelocity)/g;
		tb=(x-ta*cos(m_PitchAngle)*m_LinearVelocity)/(cos(m_PitchAngle)*m_LinearVelocity);
		m_HangTime = ta+tb;
		{
			DOUT(5,"d=%f p=%f v=%f ht=%f",Meters2Feet(x) ,RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity),m_HangTime);
			Dout(robot_props.TargetVars_DisplayRow,"%.2f %.1f",RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity));
		}
	}
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_PitchRamp.AsEntity1D().TimeChange(dTime_s);
	m_PowerWheels.TimeChange(dTime_s);
	m_IntakeSystem.TimeChange(dTime_s);
}

const double c_rMotorDriveForward_DeadZone=0.02;
const double c_rMotorDriveReverse_DeadZone=0.02;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.02;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void FRC_2013_Robot::ComputeDeadZone(double &LeftVoltage,double &RightVoltage)
{
	//Eliminate the deadzone
	if (LeftVoltage>0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveForward_Range) + c_lMotorDriveForward_DeadZone;
	else if (LeftVoltage < 0.0)
		LeftVoltage=(LeftVoltage * c_lMotorDriveReverse_Range) - c_lMotorDriveReverse_DeadZone;

	if (RightVoltage>0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveForward_Range) + c_rMotorDriveForward_DeadZone;
	else if (RightVoltage < 0.0)
		RightVoltage=(RightVoltage * c_rMotorDriveReverse_Range) - c_rMotorDriveReverse_DeadZone;
}

const FRC_2013_Robot_Properties &FRC_2013_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

void FRC_2013_Robot::SetTargetingValue(double Value)
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//TODO determine final scaler factor for the pitch (may want to make this a property)
	//printf("\r%f       ",Value);
	if (Value > -0.98)
	{
		if (m_IsTargeting)
		{
			SetTargeting(false);
			printf("Disabling Targeting\n");
		}
	}
	else
	{
		if (!m_IsTargeting)
		{
			SetTargeting(true);
			printf("Enabling Targeting\n");
		}
	}
}

void FRC_2013_Robot::SetClimbGear(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	bool ClimbAgain=false;
	if (on)  //check to see if we are done with previous climb
	{
		const Goal *goal=GetGoal();
		if ((!goal) || (goal->GetStatus()==Goal::eCompleted) || (goal->GetStatus()==Goal::eInactive))
			ClimbAgain=true;
	}
	if ((!m_SetClimbGear)||(ClimbAgain))
	{
		if (on)
		{
			printf("Now in ClimbGear\n");
			m_SetClimbGear=true;

			Goal *oldgoal=ClearGoal();
			if (oldgoal)
				delete oldgoal;

			Goal *goal=NULL;
			goal=FRC_2013_Goals::Climb(this);
			if (goal)
				goal->Activate(); //now with the goal(s) loaded activate it
			SetGoal(goal);

			//Now to turn on auto pilot (disabling controls)
			GetController()->GetUIController_RW()->SetAutoPilot(true);
		}
	}
	else
	{
		if (!on)
		{
			printf("Now in DriveGear\n");
			//Ship first then tank  (so PID settings have correct ship props)
			UpdateShipProperties(m_RobotProps.GetShipProps());
			UpdateTankProps(m_RobotProps.GetTankRobotProps());

			m_SetClimbGear=false;
			Goal *oldgoal=ClearGoal();
			if (oldgoal)
				delete oldgoal;
			SetGoal(NULL);
			//Note: may want this to be a goal that waits... it will probably be ok though
			GetController()->GetUIController_RW()->Stop();  //make sure nothing is moving!
			//Now to turn off auto pilot (regain controls)
			//Note: for the robot this acts like a kill switch, so we'll want to leave controls disabled
			//for the simulation this will restore it back to drive (since it doesn't matter what state the pneumatics are in
			#ifdef AI_TesterCode
			GetController()->GetUIController_RW()->SetAutoPilot(false);
			#endif
		}
	}
}

void FRC_2013_Robot::SetClimbGear_LeftButton(bool on)
{
	m_SetClimbLeft=on;
	if (m_SetClimbLeft&&m_SetClimbRight)
		SetClimbGear(true);
}
void FRC_2013_Robot::SetClimbGear_RightButton(bool on)
{
	m_SetClimbRight=on;
	if (m_SetClimbLeft&&m_SetClimbRight)
		SetClimbGear(true);
}


void FRC_2013_Robot::SetClimbState(ClimbState climb_state)
{
	//Note: the order of each of these will disengage others before engage... as a fall back precaution, but the client code
	//really needs to set state to neutral with a wait time before going into the next state so that there is never a time when
	//multiple gears are engaged at any moment
	switch (climb_state)
	{
	case eClimbState_Neutral:
		m_RobotControl->CloseSolenoid(eEngageDriveTrain);
		m_RobotControl->CloseSolenoid(eEngageLiftWinch);
		m_RobotControl->CloseSolenoid(eEngageDropWinch);
		break;
	case eClimbState_Drive:
		m_RobotControl->CloseSolenoid(eEngageLiftWinch);
		m_RobotControl->CloseSolenoid(eEngageDropWinch);
		m_RobotControl->OpenSolenoid(eEngageDriveTrain);
		//Ship first then tank  (so PID settings have correct ship props)
		UpdateShipProperties(m_RobotProps.GetShipProps());
		UpdateTankProps(m_RobotProps.GetTankRobotProps());
		break;
	case eClimbState_RaiseLift:
		m_RobotControl->CloseSolenoid(eEngageDriveTrain);
		m_RobotControl->CloseSolenoid(eEngageDropWinch);
		m_RobotControl->OpenSolenoid(eEngageLiftWinch);
		//Ship first then tank  (so PID settings have correct ship props)
		UpdateShipProperties(m_RobotProps.GetClimbGearLiftProps().GetShipProps());
		UpdateTankProps(m_RobotProps.GetClimbGearLiftProps().GetTankRobotProps());
		break;
	case eClimbState_DropLift:
		m_RobotControl->OpenSolenoid(eEngageDropWinch);
		//Ship first then tank  (so PID settings have correct ship props)
		UpdateShipProperties(m_RobotProps.GetClimbGearDropProps().GetShipProps());
		UpdateTankProps(m_RobotProps.GetClimbGearDropProps().GetTankRobotProps());
		break;
	case eClimbState_DropLift2:
		m_RobotControl->CloseSolenoid(eEngageDriveTrain);
		m_RobotControl->CloseSolenoid(eEngageLiftWinch);
		break;
	}
}

bool FRC_2013_Robot::IsStopped() const
{
	double EncoderLeftVelocity,EncoderRightVelocity;
	m_RobotControl->GetLeftRightVelocity(EncoderLeftVelocity,EncoderRightVelocity);
	//It was very tempting to add these, but they could cancel each other out in a perfect turn case
	return (IsZero(EncoderLeftVelocity)&&IsZero(EncoderRightVelocity));
}

//Keep around as a fall back template if the targeting is not working

//void FRC_2013_Robot::SetPresetPosition(size_t index,bool IgnoreOrientation)
//{
//	Vec2D position=m_RobotProps.GetFRC2013RobotProps().PresetPositions[index];
//	SetPosition(position[0],position[1]);
//
//	if (!IgnoreOrientation)
//	{	
//		//with no turret assume its always in the zero position
//		SetAttitude(0.0);
//	}
//}

//void FRC_2013_Robot::Set_Auton_PresetPosition(size_t index)
//{
//	m_AutonPresetIndex=index;
//	m_IsTargeting=true;  //This is just in case the pitch is in wrong position or if it is missing
//	SetPresetPosition(index,true);
//	SetAttitude(Pi);
//}

void FRC_2013_Robot::SetTarget(Targets target)
{
	m_Target=target;
}

void FRC_2013_Robot::SetDefensiveKeyOn()
{
	//We'll simply plot the coordinates of the key based on position and orientation of the turret
	//This is really a scale of 0 - 1 multiplied against 40 feet, but simplified to 0 - 2 * 20
	const double Distance=Feet2Meters((m_DefensiveKeyNormalizedDistance + 1.0) * 20.0);
	double Direction=0.0;
	double Y=(sin(Direction+PI_2) * Distance) + GetPos_m()[1];
	double X=(cos(-Direction+PI_2) * Distance) + GetPos_m()[0];
	printf("Direction=%f Distance=%f x=%f y=%f\n",RAD_2_DEG(Direction),Meters2Feet(Distance),Meters2Feet(X),Meters2Feet(Y));
	m_DefensiveKeyPosition=Vec2D(X,Y);
	m_Target=eDefensiveKey;
}


void FRC_2013_Robot::SetPresetPOV (double value)
{
	//We put the typical case first (save the amount of branching)
	if (value!=-1)
	{
		if (!m_POVSetValve)
		{
			m_POVSetValve=true;
			//so breaking down the index
			//0 = up
			//1 = up right
			//2 = right
			//3 = down right
			//4 = down
			//5 = down left
			//6 = left
			//7 = left up
			//size_t index=(size_t)(value/45.0);
			//switch (index)
			//{
			//	case 0:	SetPresetPosition(0);	break;
			//	case 2: SetPresetPosition(2);	break;
			//	case 6: SetPresetPosition(1);	break;
			//}
		}
	}
	else 
		m_POVSetValve=false;
}

void FRC_2013_Robot::Robot_SetCreepMode(bool on) 
{
	//TODO this may be omitted
	//SetUseEncoders(on,false);  //High gear can use them
}

void FRC_2013_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_SetTargeting"].Subscribe(ehl, *this, &FRC_2013_Robot::SetTargeting);
		em->Event_Map["Robot_SetTargetingOn"].Subscribe(ehl, *this, &FRC_2013_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"].Subscribe(ehl, *this, &FRC_2013_Robot::SetTargetingOff);
		//em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Subscribe(ehl,*this, &FRC_2013_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Subscribe(ehl,*this, &FRC_2013_Robot::SetTargetingValue);
		em->EventValue_Map["Robot_SetDefensiveKeyValue"].Subscribe(ehl,*this, &FRC_2013_Robot::SetDefensiveKeyPosition);
		em->Event_Map["Robot_SetDefensiveKeyOn"].Subscribe(ehl, *this, &FRC_2013_Robot::SetDefensiveKeyOn);
		em->Event_Map["Robot_SetDefensiveKeyOff"].Subscribe(ehl, *this, &FRC_2013_Robot::SetDefensiveKeyOff);
		//em->EventOnOff_Map["Robot_Flippers_Solenoid"].Subscribe(ehl,*this, &FRC_2013_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetClimbGear"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGear);
		em->Event_Map["Robot_SetClimbGearOn"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGearOn);
		em->Event_Map["Robot_SetClimbGearOff"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGearOff);
		em->EventOnOff_Map["Robot_SetClimbGear_LeftButton"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGear_LeftButton);
		em->EventOnOff_Map["Robot_SetClimbGear_RightButton"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGear_RightButton);
		
		em->EventValue_Map["Robot_SetPresetPOV"].Subscribe(ehl, *this, &FRC_2013_Robot::SetPresetPOV);
		em->EventOnOff_Map["Robot_SetCreepMode"].Subscribe(ehl, *this, &FRC_2013_Robot::Robot_SetCreepMode);
	}
	else
	{
		em->EventOnOff_Map["Robot_SetTargeting"]  .Remove(*this, &FRC_2013_Robot::SetTargeting);
		em->Event_Map["Robot_SetTargetingOn"]  .Remove(*this, &FRC_2013_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"]  .Remove(*this, &FRC_2013_Robot::SetTargetingOff);
		//em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Remove(*this, &FRC_2013_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Remove(*this, &FRC_2013_Robot::SetTargetingValue);
		em->EventValue_Map["Robot_SetDefensiveKeyValue"].Remove(*this, &FRC_2013_Robot::SetDefensiveKeyPosition);
		em->Event_Map["Robot_SetDefensiveKeyOn"]  .Remove(*this, &FRC_2013_Robot::SetDefensiveKeyOn);
		em->Event_Map["Robot_SetDefensiveKeyOff"]  .Remove(*this, &FRC_2013_Robot::SetDefensiveKeyOff);
		//em->EventOnOff_Map["Robot_Flippers_Solenoid"]  .Remove(*this, &FRC_2013_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetClimbGear"]  .Remove(*this, &FRC_2013_Robot::SetClimbGear);
		em->Event_Map["Robot_SetClimbGearOn"]  .Remove(*this, &FRC_2013_Robot::SetClimbGearOn);
		em->Event_Map["Robot_SetClimbGearOff"]  .Remove(*this, &FRC_2013_Robot::SetClimbGearOff);
		em->EventOnOff_Map["Robot_SetClimbGear_LeftButton"]  .Remove(*this, &FRC_2013_Robot::SetClimbGear_LeftButton);
		em->EventOnOff_Map["Robot_SetClimbGear_RightButton"]  .Remove(*this, &FRC_2013_Robot::SetClimbGear_RightButton);

		em->EventValue_Map["Robot_SetPresetPOV"]  .Remove(*this, &FRC_2013_Robot::SetPresetPOV);
		em->EventOnOff_Map["Robot_SetCreepMode"]  .Remove(*this, &FRC_2013_Robot::Robot_SetCreepMode);
	}

	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_PowerWheels.BindAdditionalEventControls(Bind);
	m_IntakeSystem.BindAdditionalEventControls(Bind);
	#ifdef AI_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
}

void FRC_2013_Robot::BindAdditionalUIControls(bool Bind,void *joy)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy);
	__super::BindAdditionalUIControls(Bind,joy);  //call super for more general control assignments
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2013_Robot_Properties::FRC_2013_Robot_Properties()  : 
	m_PitchRampProps(
	"Pitch",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eRobotArm,
	true,	//Using the range
	DEG_2_RAD(45-3),DEG_2_RAD(70+3) //add padding for quick response time (as close to limits will slow it down)
	),
	m_PowerWheelProps(
	"PowerWheels",
	2.0,    //Mass
	Inches2Meters(6),   //Dimension  (needed to convert linear to angular velocity)
	(5000.0/60.0) * Pi2,   //Max Speed (This is clocked at 5000 rpm) 
	60.0,60.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	60.0,60.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false,28.0 * Pi2,0.0,	//No limit ever!  (but we are using the min range as a way to set minimum speed)
	true //This is angular
	),
	m_PowerSlowWheelProps(
	"PowerWheels",
	2.0,    //Mass
	Inches2Meters(6),   //Dimension  (needed to convert linear to angular velocity)
	(5000.0/60.0) * Pi2,   //Max Speed (This is clocked at 5000 rpm) 
	60.0,60.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	60.0,60.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false,28.0 * Pi2,0.0,	//No limit ever!  (but we are using the min range as a way to set minimum speed)
	true //This is angular
	),
	m_HelixProps(
	"Helix",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Props::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	),
	m_IntakeDeploymentProps(
	"IntakeDeployment",
	2.0,    //Mass
	Inches2Meters(12),   //Dimension  (this should be correct)
	1.4 * Pi2,   //Max Speed  (Parker gave this one, should be good)
	10.0,10.0, //ACCEL, BRAKE  (should be relatively quick)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eRobotArm,
	true,	//Using the range
	0,PI_2 //solve in script
	),

	m_RobotControls(&s_ControlsEvents)
{
	{
		FRC_2013_Robot_Props props;
		const double KeyDistance=Inches2Meters(144);
		const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		const double DefaultY=c_HalfCourtLength-KeyDistance;
		const double HalfKeyWidth=KeyWidth/2.0;
		props.PresetPositions[0]=Vec2D(0.0,DefaultY);
		props.PresetPositions[1]=Vec2D(-HalfKeyWidth,DefaultY);
		props.PresetPositions[2]=Vec2D(HalfKeyWidth,DefaultY);
		props.FireTriggerDelay=0.100;  //e.g. 10 iterations of good tolerance
		props.FireButtonStayOn_Time=0.100; //100 ms
		props.Coordinates_DiplayRow=(size_t)-1;
		props.TargetVars_DisplayRow=(size_t)-1;
		props.PowerVelocity_DisplayRow=(size_t)-1;
		props.YawTolerance=0.001; //give a good high precision for default

		for (size_t row=0;row<6;row++)
		{
			for (size_t column=0;column<3;column++)
			{
				Vec2D &cell=props.KeyGrid[row][column];
				const double spread=Feet2Meters(7.0);
				const double x=spread * ((double)column-1.0);
				const double y=(spread * ((double)row-1.0)) + DefaultY;
				cell=Vec2D(x,y);
				props.KeyCorrections[row][column].PowerCorrection=1.0;
				props.KeyCorrections[row][column].PitchCorrection=1.0;
			}
		}

		FRC_2013_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		auton.MoveForward=0.0;
		auton.TwoShotScaler=1.0;
		auton.RampLeft_ErrorCorrection_Offset=
		auton.RampRight_ErrorCorrection_Offset=
		auton.RampCenter_ErrorCorrection_Offset=Vec2D(0.0,0.0);
		auton.XLeftArc=auton.XRightArc=1.9;
		FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_1=auton.FirstBall_Wait;
		ball_1.InitialWait=4.0;
		ball_1.TimeOutWait=-1.0;
		ball_1.ToleranceThreshold=0.0;
		FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_2=auton.SecondBall_Wait;
		ball_2.InitialWait=4.0;
		ball_2.TimeOutWait=-1.0;
		ball_2.ToleranceThreshold=0.0;
		FRC_2013_Robot_Props::Climb_Properties &climb_props=props.Climb_Props;
		climb_props.LiftDistance=1.0;
		climb_props.DropDistance=-1.0;
		m_FRC2013RobotProps=props;
	}
	{
		Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

		//Late assign this to override the initial default
		//Was originally 0.4953 19.5 width for 2011
		//Now is 0.517652 20.38 according to Parker  (not too worried about the length)
		props.WheelDimensions=Vec2D(0.517652,0.6985); //27.5 x 20.38
		props.WheelDiameter=c_WheelDiameter;
		props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
		props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
		m_TankRobotProps=props;
	}
	{
		Rotary_Props props=m_PitchRampProps.RoteryProps(); //start with super class settings
		props.PrecisionTolerance=0.001; //we need high precision
		m_PitchRampProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PowerWheelProps.RoteryProps(); //start with super class settings
		props.PrecisionTolerance=0.1; //we need decent precision (this will depend on ramp up time too)
		m_PowerWheelProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PowerSlowWheelProps.RoteryProps(); //start with super class settings
		props.PrecisionTolerance=0.1; //we need decent precision (this will depend on ramp up time too)
		m_PowerSlowWheelProps.RoteryProps()=props;
	}
}

const char *ProcessVec2D(FRC_2013_Robot_Props &m_FRC2013RobotProps,Scripting::Script& script,Vec2d &Dest)
{
	const char *err;
	typedef FRC_2013_Robot_Properties::Vec2D Vec2D;
	double length, width;	
	//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
	err = script.GetField("y", NULL, NULL,&length);
	if (err)
	{
		err = script.GetField("y_ft", NULL, NULL,&length);
		if (!err)
			length=Feet2Meters(length);
		else
		{
			err = script.GetField("y_in", NULL, NULL,&length);
			if (!err)
				length=Inches2Meters(length);
		}

	}
	ASSERT_MSG(!err, err);
	err = script.GetField("x", NULL, NULL,&width);
	if (err)
	{
		err = script.GetField("x_ft", NULL, NULL,&width);
		if (!err)
			width=Feet2Meters(width);
		else
		{
			err = script.GetField("x_in", NULL, NULL,&width);
			if (!err)
				width=Inches2Meters(width);
		}
	}
	ASSERT_MSG(!err, err);
	Dest=Vec2D(width,length);  //x,y  where x=width
	script.Pop();
	return err;
}

const char *ProcessKey(FRC_2013_Robot_Props &m_FRC2013RobotProps,Scripting::Script& script,size_t index)
{
	const char *err;
	typedef FRC_2013_Robot_Properties::Vec2D Vec2D;
	Vec2D PresetPosition;
	err=ProcessVec2D(m_FRC2013RobotProps,script,PresetPosition);
	ASSERT_MSG(!err, err);
	PresetPosition[1]=c_HalfCourtLength-PresetPosition[1];
	m_FRC2013RobotProps.PresetPositions[index]=PresetPosition;  //x,y  where x=width
	return err;
}

const char *ProcessKeyCorrection(FRC_2013_Robot_Props &m_FRC2013RobotProps,Scripting::Script& script,size_t row,size_t column)
{
	const char* err=NULL;
	char CellName[4];
	CellName[0]='c';
	CellName[1]='1'+row;
	CellName[2]='1'+column;
	CellName[3]=0;
	err = script.GetFieldTable(CellName);

	err = script.GetField("p", NULL, NULL,&m_FRC2013RobotProps.KeyCorrections[row][column].PowerCorrection);
	err = script.GetField("y", NULL, NULL,&m_FRC2013RobotProps.KeyCorrections[row][column].PitchCorrection);

	script.Pop();
	return err;
}

//declared as global to avoid allocation on stack each iteration
const char * const g_FRC_2013_Controls_Events[] = 
{
	"Turret_SetCurrentVelocity","Turret_SetIntendedPosition","Turret_SetPotentiometerSafety",
	"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
	"PowerWheels_SetCurrentVelocity","PowerWheels_SetEncoderSafety","PowerWheels_IsRunning",
	"Ball_SetCurrentVelocity","Ball_Fire","Ball_Squirt","Ball_Grip","Ball_GripL","Ball_GripM","Ball_GripH",
	"Intake_Deployment_SetCurrentVelocity","Intake_Deployment_SetIntendedPosition","Intake_Deployment_SetPotentiometerSafety",
	"Intake_Deployment_Advance","Intake_Deployment_Retract",
	"Robot_IsTargeting","Robot_SetTargetingOn","Robot_SetTargetingOff","Robot_TurretSetTargetingOff","Robot_SetTargetingValue",
	"Robot_SetClimbGear","Robot_SetClimbGearOn","Robot_SetClimbGearOff",
	"Robot_SetClimbGear_LeftButton","Robot_SetClimbGear_RightButton",
	"Robot_SetPreset1","Robot_SetPreset2","Robot_SetPreset3","Robot_SetPresetPOV",
	"Robot_SetDefensiveKeyValue","Robot_SetDefensiveKeyOn","Robot_SetDefensiveKeyOff"
	//AI Tester events only
	#ifdef AI_TesterCode
	,"Ball_SlowWheel"
	#endif
};

const char *FRC_2013_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2013_Controls_Events))?g_FRC_2013_Controls_Events[index] : NULL;
}
FRC_2013_Robot_Properties::ControlEvents FRC_2013_Robot_Properties::s_ControlsEvents;

void FRC_2013_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		double version;
		err=script.GetField("version", NULL, NULL, &version);
		if (!err)
			printf ("Version=%.2f\n",version);
	}

	__super::LoadFromScript(script);
	err = script.GetFieldTable("robot_settings");
	if (!err) 
	{
		err = script.GetFieldTable("pitch");
		if (!err)
		{
			m_PitchRampProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("power");
		if (!err)
		{
			m_PowerWheelProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("power_first_stage");
		if (!err)
		{
			m_PowerSlowWheelProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("helix");
		if (!err)
		{
			m_HelixProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("intake_deployment");
		if (!err)
		{
			m_IntakeDeploymentProps.LoadFromScript(script);
			script.Pop();
		}

		m_ClimbGearLiftProps=*this;  //copy redundant data first
		err = script.GetFieldTable("climb_gear_lift");
		if (!err)
		{
			m_ClimbGearLiftProps.LoadFromScript(script);
			script.Pop();
		}

		m_ClimbGearDropProps=*this;  //copy redundant data first
		err = script.GetFieldTable("climb_gear_drop");
		if (!err)
		{
			m_ClimbGearDropProps.LoadFromScript(script);
			script.Pop();
		}
		
		err = script.GetFieldTable("key_1");
		if (!err) ProcessKey(m_FRC2013RobotProps,script,0);

		err = script.GetFieldTable("key_2");
		if (!err) ProcessKey(m_FRC2013RobotProps,script,1);

		err = script.GetFieldTable("key_3");
		if (!err) ProcessKey(m_FRC2013RobotProps,script,2);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2013RobotProps.Coordinates_DiplayRow=(size_t)fDisplayRow;
		err=script.GetField("ds_target_vars_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2013RobotProps.TargetVars_DisplayRow=(size_t)fDisplayRow;

		err=script.GetField("ds_power_velocity_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2013RobotProps.PowerVelocity_DisplayRow=(size_t)fDisplayRow;

		script.GetField("yaw_tolerance", NULL, NULL, &m_FRC2013RobotProps.YawTolerance);

		script.GetField("fire_trigger_delay", NULL, NULL, &m_FRC2013RobotProps.FireTriggerDelay);
		script.GetField("fire_stay_on_time", NULL, NULL, &m_FRC2013RobotProps.FireButtonStayOn_Time);

		err = script.GetFieldTable("grid_corrections");
		if (!err)
		{
			for (size_t row=0;row<6;row++)
			{
				for (size_t column=0;column<3;column++)
				{
					err=ProcessKeyCorrection(m_FRC2013RobotProps,script,row,column);
					assert(!err);
				}
			}
			script.Pop();
		}
		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct FRC_2013_Robot_Props::Autonomous_Properties &auton=m_FRC2013RobotProps.Autonomous_Props;
			{
				double length;
				err = script.GetField("move_forward_ft", NULL, NULL,&length);
				if (!err)
					auton.MoveForward=Feet2Meters(length);
			}
			err = script.GetField("two_shot_scaler", NULL, NULL,&auton.TwoShotScaler);

			err = script.GetFieldTable("ramp_left");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2013RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampLeft_ErrorCorrection_Offset=OffsetPosition;
			}
			err = script.GetFieldTable("ramp_right");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2013RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampRight_ErrorCorrection_Offset=OffsetPosition;
			}
			err = script.GetFieldTable("ramp_center");
			if (!err)
			{
				Vec2D OffsetPosition;
				err=ProcessVec2D(m_FRC2013RobotProps,script,OffsetPosition);
				ASSERT_MSG(!err, err);
				auton.RampCenter_ErrorCorrection_Offset=OffsetPosition;
			}
			{
				const char * const fieldTable[]=
				{
					"ball_1","ball_2"
				};
				//You just gotta love pointers to do this!  ;)
				FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info *ballTable[]=
				{
					&auton.FirstBall_Wait,&auton.SecondBall_Wait
				};
				for (size_t i=0;i<2;i++)
				{
					err = script.GetFieldTable(fieldTable[i]);
					if (!err)
					{
						FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball=*ballTable[i];
						err=script.GetField("initial_wait", NULL, NULL, &ball.InitialWait);
						ASSERT_MSG(!err, err);
						err=script.GetField("timeout_wait", NULL, NULL, &ball.TimeOutWait);
						ASSERT_MSG(!err, err);
						err=script.GetField("tolerance", NULL, NULL, &ball.ToleranceThreshold);
						ASSERT_MSG(!err, err);
						script.Pop();
					}
				}
			}

			script.GetField("x_left_arc", NULL, NULL, &auton.XLeftArc);
			script.GetField("x_right_arc", NULL, NULL, &auton.XRightArc);
			script.Pop();
		}

		err = script.GetFieldTable("climb");
		if (!err)
		{
			struct FRC_2013_Robot_Props::Climb_Properties &climb=m_FRC2013RobotProps.Climb_Props;
			{
				double length;
				err = script.GetField("lift_ft", NULL, NULL,&length);
				if (!err)
					climb.LiftDistance=Feet2Meters(length);
				err = script.GetField("drop_ft", NULL, NULL,&length);
				if (!err)
					climb.DropDistance=Feet2Meters(length);
			}
			script.Pop();
		}

		//This is the main robot settings pop
		script.Pop();
	}
	err = script.GetFieldTable("controls");
	if (!err)
	{
		m_RobotControls.LoadFromScript(script);
		script.Pop();
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2013_Goals::Fire														*/
/***********************************************************************************************************************************/

FRC_2013_Goals::Fire::Fire(FRC_2013_Robot &robot,bool On, bool DoSquirt) : m_Robot(robot),m_Terminate(false),m_IsOn(On),m_DoSquirt(DoSquirt)
{
	m_Status=eInactive;
}
FRC_2013_Goals::Fire::Goal_Status FRC_2013_Goals::Fire::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	if (!m_DoSquirt)
		m_Robot.GetIntakeSystem().Fire(m_IsOn);
	else
		m_Robot.GetIntakeSystem().Squirt(m_IsOn);
		
	m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Goals::WaitForBall														*/
/***********************************************************************************************************************************/

FRC_2013_Goals::WaitForBall::WaitForBall(FRC_2013_Robot &robot,double Tolerance) :  m_Robot(robot),m_Tolerance(Tolerance),m_Terminate(false)
{
	m_Status=eInactive;
}
Goal::Goal_Status FRC_2013_Goals::WaitForBall::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	//Keep simple... if we have noisy artifacts that yield false positives then we may need to consider blend and / or Kalman
	const double PowerWheelSpeedDifference=m_Robot.GetPowerWheels().GetSecondStageShooter().GetRequestedVelocity_Difference();
	//when we encounter the tolerance dip in speed we are done
	if (fabs(PowerWheelSpeedDifference)>m_Tolerance)
	{
		printf("Ball Deployed\n");
		m_Status=eCompleted;
	}
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Goals::ChangeClimbState												*/
/***********************************************************************************************************************************/

FRC_2013_Goals::ChangeClimbState::ChangeClimbState(FRC_2013_Robot &robot,FRC_2013_Robot::ClimbState climb_state,double TimeToTakeEffect_s) : m_Robot(robot),
m_ClimbState(climb_state),m_TimeToWait(TimeToTakeEffect_s),m_Terminate(false)
{	
	m_Status=eInactive;
	m_TimeAccrued=0.0;
}

void FRC_2013_Goals::ChangeClimbState::Activate() 
{
	m_Robot.SetClimbState(m_ClimbState);
	m_Status=eActive;
}

FRC_2013_Goals::ChangeClimbState::Goal_Status FRC_2013_Goals::ChangeClimbState::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	m_TimeAccrued+=dTime_s;
	if (m_TimeAccrued>m_TimeToWait)
		m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Goals::ResetPosition													*/
/***********************************************************************************************************************************/

FRC_2013_Goals::ResetPosition::ResetPosition(FRC_2013_Robot &robot): m_Robot(robot),m_Timer(0.0),m_TimeStopped(-1.0) 
{
	m_Status=eInactive;
}

void FRC_2013_Goals::ResetPosition::Activate() 
{
	m_Status=eActive;
	m_Robot.GetController()->GetUIController_RW()->Stop();  //ensure the robot is stopped
}

FRC_2013_Goals::ChangeClimbState::Goal_Status FRC_2013_Goals::ResetPosition::Process(double dTime_s) 
{
	ActivateIfInactive();
	if (m_Timer<0.500)
	{
		if (!m_Robot.IsStopped()) 
			m_TimeStopped=-1.0;
		else //It is stopped this iteration
		{
			//Check first run case... reset to zero
			if (m_TimeStopped==-1.0)
				m_TimeStopped=0.0;
			m_TimeStopped+=dTime_s;  //count how much time its been at zero
		}
		//If we've been stopped long enough... reset position and complete goal
		if (m_TimeStopped>0.100)
		{
			//before resetting the position... ensure there is no movement
			m_Robot.ResetPos();
			m_Status=eCompleted;
		}
	}
	else
	{
		m_Status=eFailed;
		printf("  ***ResetPosition::Process time exceeded- sending failed status\n");
		//For robot code... do not assert here let code run its course perhaps a kill switch or a reattempt could happen
		//simulation code should fix problem right away if this happens
		#ifdef AI_TesterCode
		assert(false);
		#endif
	}
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*															FRC_2013_Goals															*/
/***********************************************************************************************************************************/

Goal *FRC_2013_Goals::Get_ShootBalls(FRC_2013_Robot *Robot,bool DoSquirt)
{
	//Goal_Wait *goal_waitforturret=new Goal_Wait(1.0); //wait for turret
	Goal_Wait *goal_waitforballs1=new Goal_Wait(8.0); //wait for balls
	Fire *FireOn=new Fire(*Robot,true,DoSquirt);
	Goal_Wait *goal_waitforballs2=new Goal_Wait(7.0); //wait for balls
	Fire *FireOff=new Fire(*Robot,false,DoSquirt);
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs2);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->AddSubgoal(goal_waitforballs1);
	//MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

Goal *FRC_2013_Goals::Climb(FRC_2013_Robot *Robot)
{
	const FRC_2013_Robot_Props &props=Robot->GetRobotProps().GetFRC2013RobotProps();
	const FRC_2013_Robot_Props::Climb_Properties &climb_props=props.Climb_Props;
	// reset the coordinates to use way points.  This will also ensure there is no movement
	ResetPosition *goal_reset_1=new ResetPosition(*Robot);
	ChangeClimbState *goal_decouple_drive_1=new ChangeClimbState(*Robot,FRC_2013_Robot::eClimbState_Neutral);		   //de-couple the drive via pneumatic 1
	ChangeClimbState *goal_couple_elevator_UP=new ChangeClimbState(*Robot,FRC_2013_Robot::eClimbState_RaiseLift);  //couple the elevator UP winch via pneumatic 2
	//Construct a way point
	WayPoint wp;
	wp.Position[0]=0.0;
	wp.Position[1]=climb_props.LiftDistance;
	wp.Power=1.0;
	Goal_Ship_MoveToPosition *goal_spool_lift_winch=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);  //run the drive motors a very specific distance 

	//This also takes care of ... engage your control loop 'brake mode'.  By ensuring that there is no movement before resetting the position
	ResetPosition *goal_reset_2=new ResetPosition(*Robot);

	//de-couple elevator UP winch, and engage elevator DOWN winch
	ChangeClimbState *goal_couple_elevator_DOWN=new ChangeClimbState(*Robot,FRC_2013_Robot::eClimbState_DropLift);
	ChangeClimbState *goal_couple_elevator_DOWN_releaseLiftWinch=new ChangeClimbState(*Robot,FRC_2013_Robot::eClimbState_DropLift2);

	wp.Position[1]=climb_props.DropDistance;
	Goal_Ship_MoveToPosition *goal_spool_drop_winch=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);  //run the drive motors a very specific distance 

	//engage the VEX motor on each drive side to LOCK the gearboxes (solves no power hanging).
	//For now this is just a backup plan that would need a rotary system

	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	//MainGoal->AddSubgoal(goal_JamGear);
	MainGoal->AddSubgoal(goal_spool_drop_winch);
	MainGoal->AddSubgoal(goal_couple_elevator_DOWN_releaseLiftWinch);
	MainGoal->AddSubgoal(goal_couple_elevator_DOWN);
	MainGoal->AddSubgoal(goal_reset_2);
	MainGoal->AddSubgoal(goal_spool_lift_winch);
	MainGoal->AddSubgoal(goal_couple_elevator_UP);
	MainGoal->AddSubgoal(goal_decouple_drive_1);
	MainGoal->AddSubgoal(goal_reset_1);
	return MainGoal;
}

Goal *FRC_2013_Goals::Get_FRC2013_Autonomous(FRC_2013_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex)
{
	const FRC_2013_Robot_Props::Autonomous_Properties &auton=Robot->GetRobotProps().GetFRC2013RobotProps().Autonomous_Props;
	//Robot->Set_Auton_PresetPosition(KeyIndex);
	Robot->SetTarget((FRC_2013_Robot::Targets)TargetIndex);
	Fire *FireOn=new Fire(*Robot,true);
	#if 0
	Goal_Wait *goal_waitforballs=new Goal_Wait(auton.FirstBall_Wait.InitialWait); //wait for balls
	#else
	Goal_Ship_MoveToPosition *goal_drive_foward=NULL;
	if (auton.MoveForward!=0.0)
	{
		const Vec2d start_pos=Robot->GetRobotProps().GetFRC2013RobotProps().PresetPositions[KeyIndex];
		WayPoint wp;
		wp.Position[0]=start_pos[0];
		wp.Position[1]=start_pos[1]+auton.MoveForward;
		wp.Power=1.0;
		goal_drive_foward=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,true,true);
	}

	Generic_CompositeGoal *goal_waitforballs= new Generic_CompositeGoal;
	{
		const FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_1=auton.FirstBall_Wait;
		const FRC_2013_Robot_Props::Autonomous_Properties::WaitForBall_Info &ball_2=auton.SecondBall_Wait;

		Generic_CompositeGoal *Ball_1_Composite= new Generic_CompositeGoal;
		if (ball_1.ToleranceThreshold!=0.0)
		{
			//Create the wait for ball goal
			WaitForBall *Ball_1_Wait=new WaitForBall(*Robot,ball_1.ToleranceThreshold);
			//determine if we have a timeout
			if (ball_1.TimeOutWait==-1.0)
				Ball_1_Composite->AddSubgoal(Ball_1_Wait);	
			else
			{
				MultitaskGoal *WaitWithTimeout1=new MultitaskGoal(false);
				WaitWithTimeout1->AddGoal(Ball_1_Wait);
				WaitWithTimeout1->AddGoal(new Goal_Wait(ball_1.TimeOutWait));
				Ball_1_Composite->AddSubgoal(WaitWithTimeout1);
			}
		}
		Ball_1_Composite->AddSubgoal(new Goal_Wait(ball_1.InitialWait));
		Ball_1_Composite->Activate(); //ready to go


		Generic_CompositeGoal *Ball_2_Composite= new Generic_CompositeGoal;
		if (ball_2.ToleranceThreshold!=0.0)
		{
			//Create the wait for ball goal
			WaitForBall *Ball_2_Wait=new WaitForBall(*Robot,ball_2.ToleranceThreshold);
			//determine if we have a timeout
			if (ball_2.TimeOutWait==-1.0)
				Ball_2_Composite->AddSubgoal(Ball_2_Wait);	
			else
			{
				MultitaskGoal *WaitWithTimeout2=new MultitaskGoal(false);
				WaitWithTimeout2->AddGoal(Ball_2_Wait);
				WaitWithTimeout2->AddGoal(new Goal_Wait(ball_2.TimeOutWait));
				Ball_2_Composite->AddSubgoal(WaitWithTimeout2);
			}
		}
		Ball_2_Composite->AddSubgoal(new Goal_Wait(ball_2.InitialWait));
		Ball_2_Composite->Activate(); //ready to go

		//put in backwards
		goal_waitforballs->AddSubgoal(Ball_2_Composite);
		goal_waitforballs->AddSubgoal(Ball_1_Composite);
		goal_waitforballs->Activate(); //ready to go
	}
	#endif
	Fire *FireOff=new Fire(*Robot,false);

	Goal_Ship_MoveToPosition *goal_drive_1=NULL;
	Goal_Ship_MoveToPosition *goal_drive_2=NULL;
	//OperateSolenoid *DeployFlipper=NULL;
	Fire *EndSomeFire_On=NULL;
	Goal_Wait *goal_waitEndFire=NULL;
	Fire *EndSomeFire_Off=NULL;
	if (RampIndex != (size_t)-1)
	{
		//DeployFlipper=new OperateSolenoid(*Robot,FRC_2013_Robot::eFlipperDown,true);
		const double YPad=Inches2Meters(5); //establish our Y being 5 inches from the ramp
		double Y = (c_BridgeDimensions[1] / 2.0) + YPad;
		double X;
		double X_Tweak;
		switch (RampIndex)
		{
			case 0: 
				X=0,X_Tweak=0; 
				X+=auton.RampCenter_ErrorCorrection_Offset[0];
				Y+=auton.RampCenter_ErrorCorrection_Offset[1];
				break;
			case 1: 
				X=-(c_HalfCourtWidth-(c_BridgeDimensions[0]/2.0)),X_Tweak=-(c_HalfCourtWidth+auton.XLeftArc); 
				X+=auton.RampLeft_ErrorCorrection_Offset[0];
				Y+=auton.RampLeft_ErrorCorrection_Offset[1];
				break;
			case 2: 
				X= (c_HalfCourtWidth-(c_BridgeDimensions[0]/2.0)),X_Tweak= (c_HalfCourtWidth+auton.XRightArc); 
				X+=auton.RampRight_ErrorCorrection_Offset[0];
				Y+=auton.RampRight_ErrorCorrection_Offset[1];
				break;
		}
		WayPoint wp;
		wp.Position[0]=X;
		wp.Position[1]=Y;
		wp.Power=1.0;
		goal_drive_2=new Goal_Ship_MoveToPosition(Robot->GetController(),wp);
		wp.Position[1]= (Robot->GetPos_m()[1] + Y) / 2.0;  //mid point on the Y so it can straighten out
		wp.Position[0]=  X_Tweak;
		goal_drive_1=new Goal_Ship_MoveToPosition(Robot->GetController(),wp,false,false,0.01); //don't stop on this one
		//Since turret is disabled for targeting only fire if we are in the middle key
		if (RampIndex==0)
		{
			EndSomeFire_On=new Fire(*Robot,true);
			goal_waitEndFire=new Goal_Wait(8.0); //wait for balls
			EndSomeFire_Off=new Fire(*Robot,false);
		}
	}
	//Inserted in reverse since this is LIFO stack list
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	if (goal_drive_1)
	{
		if (RampIndex==0)
		{
			MainGoal->AddSubgoal(EndSomeFire_Off);
			MainGoal->AddSubgoal(goal_waitEndFire);
		}
		MainGoal->AddSubgoal(goal_drive_2);
		if (RampIndex==0)
			MainGoal->AddSubgoal(EndSomeFire_On);
		MainGoal->AddSubgoal(goal_drive_1);
		//MainGoal->AddSubgoal(DeployFlipper);
	}
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs);
	if (goal_drive_foward)
		MainGoal->AddSubgoal(goal_drive_foward);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->Activate();
	return MainGoal;
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2013_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_PitchRampVoltage=Voltage;
				m_Pitch_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Pitch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			if (m_SlowWheel) Voltage=0.0;
			m_PowerWheelVoltage=Voltage;
			m_PowerWheel_Enc.UpdateEncoderVoltage(Voltage);
			m_PowerWheel_Enc.TimeChange();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2013_Robot::ePowerWheelFirstStage:
			if (m_SlowWheel) Voltage=0.0;
			m_PowerSlowWheelVoltage=Voltage;
			m_PowerSlowWheel_Enc.UpdateEncoderVoltage(Voltage);
			m_PowerSlowWheel_Enc.TimeChange();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2013_Robot::eHelix:
			m_HelixVoltage=Voltage;
			m_Helix_Enc.UpdateEncoderVoltage(Voltage);
			m_Helix_Enc.TimeChange();
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			{
				//	printf("IntakeDeployment=%f\n",Voltage);
				//DOUT3("IntakeDeployment Voltage=%f",Voltage);
				m_IntakeDeploymentVoltage=Voltage;
				m_IntakeDeployment_Pot.UpdatePotentiometerVoltage(Voltage);
				m_IntakeDeployment_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2013_Robot::eRollers:
			m_RollersVoltage=Voltage;
			m_Rollers_Enc.UpdateEncoderVoltage(Voltage);
			m_Rollers_Enc.TimeChange();
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
	case FRC_2013_Robot::ePitchRamp:
		Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,1,"p=%.2f",Voltage);
		break;
	case FRC_2013_Robot::ePowerWheelSecondStage:
		//Only display second stage
		Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,1,"po_v=%.2f",Voltage);
		break;
	case FRC_2013_Robot::eIntake_Deployment:
		Dout(m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().Feedback_DiplayRow,1,"f=%.2f",Voltage);
		break;
	}
	#endif
}

#ifdef __TestXAxisServoDump__
void FRC_2013_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) 
{
	LeftVelocity=m_LastLeftVelocity,RightVelocity=m_LastRightVelocity;
	//Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f", LeftVelocity,RightVelocity);
}

void FRC_2013_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) 
{
	//For now leave this disabled... should not need to script this
	Dout(2, "l=%.1f r=%.1f", LeftVoltage,RightVoltage);

	//first interpolate the angular velocity
	const Tank_Robot_Props &props=m_RobotProps.GetTankRobotProps();
	const double D=props.WheelDimensions.length();
	//Here we go it is finally working I just needed to take out the last division
	const Vec2d &WheelDimensions=props.WheelDimensions;
	//L is the vehicle’s wheelbase
	const double L=WheelDimensions[1];
	//W is the vehicle’s track width
	const double W=WheelDimensions[0];
	const double skid=cos(atan2(W,L));
	const double MaxSpeed=m_RobotProps.GetShipProps().MAX_SPEED;
	const double omega = ((LeftVoltage*MaxSpeed*skid) + (RightVoltage*MaxSpeed*-skid)) * 0.5;

	double AngularVelocity=(omega / (Pi * D)) * Pi2;
	if (props.ReverseSteering)
		AngularVelocity*=-1.0;

	double NewAngle=m_LastYawAxisSetting+(RAD_2_DEG(AngularVelocity * m_dTime_s) * props.MotorToWheelGearRatio);
	//double NewAngle=m_LastYawAxisSetting+RAD_2_DEG(AngularVelocity * m_dTime_s);
	if (NewAngle>170)
		NewAngle=170;
	else if (NewAngle<0)
		NewAngle=0;

	//Ensure the angle deltas of angular velocity are calibrated to servo's angles
	m_LastYawAxisSetting=NewAngle;
	Dout(4, "a=%.2f av=%.2f",m_LastYawAxisSetting,AngularVelocity);
	//if (!IsZero(AngularVelocity))
	//	printf("a=%.2f av=%.2f\n",m_LastYawAxisSetting,AngularVelocity);

	//m_YawControl.SetAngle(m_LastYawAxisSetting);

	const double inv_skid=1.0/cos(atan2(W,L));
	double RCW=AngularVelocity;
	double RPS=RCW / Pi2;
	RCW=RPS * (Pi * D) * inv_skid;  //D is the turning diameter

	m_LastLeftVelocity = + RCW;
	m_LastRightVelocity = - RCW;
}
#endif

bool FRC_2013_Robot_Control::GetBoolSensorState(size_t index)
{
	bool ret;
	switch (index)
	{
	case FRC_2013_Robot::eTest_Sensor:
		ret=m_FireSensor;
		break;
	default:
		assert (false);
	}
	return ret;
}

FRC_2013_Robot_Control::FRC_2013_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_PowerWheelVoltage(0.0),m_PowerSlowWheelVoltage(0.0),m_dTime_s(0.0),
	m_FireSensor(false),m_SlowWheel(false),m_FirePiston(false)
{
	#ifdef __TestXAxisServoDump__
	m_LastYawAxisSetting=m_LastLeftVelocity=m_LastRightVelocity=0.0;
	#endif
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
	#if 0
	Dout(1,"");
	Dout(2,"");
	Dout(3,"");
	Dout(4,"");
	Dout(5,"");
	#endif
}

void FRC_2013_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			m_Pitch_Pot.ResetPos();
			//We may want this for more accurate simulation
			//m_Pitch_Pot.SetPos_m((m_Pitch_Pot.GetMinRange()+m_Pitch_Pot.GetMaxRange()) / 2.0);
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			m_PowerWheel_Enc.ResetPos();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2013_Robot::ePowerWheelFirstStage:
			m_PowerSlowWheel_Enc.ResetPos();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2013_Robot::eHelix:
			m_Helix_Enc.ResetPos();
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			m_IntakeDeployment_Pot.ResetPos();
			break;
		case FRC_2013_Robot::eRollers:
			m_Rollers_Enc.ResetPos();
			break;
	}
}

//This is only for AI Tester
void FRC_2013_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
	if (Bind)
	{
		em->EventOnOff_Map["Ball_FireSensor"].Subscribe(ehl, *this, &FRC_2013_Robot_Control::TriggerFire);
		em->EventOnOff_Map["Ball_SlowWheel"].Subscribe(ehl, *this, &FRC_2013_Robot_Control::SlowWheel);
	}
	else
	{
		em->EventOnOff_Map["Ball_FireSensor"]  .Remove(*this, &FRC_2013_Robot_Control::TriggerFire);
		em->EventOnOff_Map["Ball_SlowWheel"]  .Remove(*this, &FRC_2013_Robot_Control::SlowWheel);
	}
}

void FRC_2013_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2013_Robot_Properties *robot_props=dynamic_cast<const FRC_2013_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		m_Pitch_Pot.Initialize(&robot_props->GetPitchRampProps());
		m_PowerWheel_Enc.Initialize(&robot_props->GetPowerWheelProps());
		m_PowerSlowWheel_Enc.Initialize(&robot_props->GetPowerWheelProps());
		m_Helix_Enc.Initialize(&robot_props->GetHelixProps());
		m_IntakeDeployment_Pot.Initialize(&robot_props->GetIntakeDeploymentProps());
		m_Rollers_Enc.Initialize(&robot_props->GetHelixProps());  //borrow helix... these are like conveyor
	}
}

void FRC_2013_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Pitch_Pot.SetTimeDelta(dTime_s);
	m_PowerWheel_Enc.SetTimeDelta(dTime_s);
	m_PowerSlowWheel_Enc.SetTimeDelta(dTime_s);
	m_Helix_Enc.SetTimeDelta(dTime_s);
	m_IntakeDeployment_Pot.SetTimeDelta(dTime_s);
	m_Rollers_Enc.SetTimeDelta(dTime_s);
	//display voltages
	DOUT(2,"l=%.2f r=%.2f pi=%.2f pw=%.2f hx=%.2f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
		m_PitchRampVoltage,m_PowerWheelVoltage,m_HelixVoltage);
	m_dTime_s=dTime_s;
}


double FRC_2013_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:

			result=m_Pitch_Pot.GetPotentiometerCurrentPosition();
			#ifdef __EnablePitchDisplay__
			//DOUT (4,"pitch=%.2f ",RAD_2_DEG(result));
			DOUT (4,"pitch=%.2f intake=%.2f",RAD_2_DEG(result),RAD_2_DEG(m_IntakeDeployment_Pot.GetPotentiometerCurrentPosition()));
			#endif
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			result=m_PowerWheel_Enc.GetEncoderVelocity();
			break;
		case FRC_2013_Robot::ePowerWheelFirstStage:
			result=m_PowerSlowWheel_Enc.GetEncoderVelocity();
			break;
		case FRC_2013_Robot::eHelix:
			result=m_Helix_Enc.GetEncoderVelocity();
			//DOUT5 ("vel=%f",result);
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			result=m_IntakeDeployment_Pot.GetPotentiometerCurrentPosition();
			break;
		case FRC_2013_Robot::eRollers:
			result=m_Rollers_Enc.GetEncoderVelocity();
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2013_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2013_Robot::ePowerWheelSecondStage:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,11,"rs=%.2f",result / Pi2);
			break;
		case FRC_2013_Robot::eIntake_Deployment:
			Dout(m_RobotProps.GetIntakeDeploymentProps().GetRoteryProps().Feedback_DiplayRow,14,"f=%.1f",RAD_2_DEG(result));
			break;
	}
	#endif

	return result;
}

void FRC_2013_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	const char * const SolenoidState=Open?"Engaged":"Disengaged";
	switch (index)
	{
	case FRC_2013_Robot::eEngageDriveTrain:
		printf("Drive Train Gear = %s\n",SolenoidState);
		break;
	case FRC_2013_Robot::eEngageLiftWinch:
		printf("Lift Winch = %s\n",SolenoidState);
		break;
	case FRC_2013_Robot::eEngageDropWinch:
		printf("Drop Winch = %s\n",SolenoidState);
		break;
	case FRC_2013_Robot::eFirePiston:
		printf("Fire Piston = %s\n",SolenoidState);
		m_FirePiston=Open;
		break;
	}
}

bool FRC_2013_Robot_Control::GetIsSolenoidOpen(size_t index) const
{
	bool ret=false;
	switch (index)
	{
	case FRC_2013_Robot::eEngageDriveTrain:
		break;
	case FRC_2013_Robot::eEngageLiftWinch:
		break;
	case FRC_2013_Robot::eEngageDropWinch:
		break;
	case FRC_2013_Robot::eFirePiston:
		ret=m_FirePiston;
		break;
	}
	return ret;
}


  /***************************************************************************************************************/
 /*											FRC_2013_Power_Wheel_UI												*/
/***************************************************************************************************************/

void FRC_2013_Power_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.6,-2.0);
	Myprops.m_Color=osg::Vec4(1.0,0.0,0.5,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
	m_PowerWheelMaxSpeed=m_RobotControl->GetRobotProps().GetPowerWheelProps().GetMaxSpeed();
}

void FRC_2013_Power_Wheel_UI::TimeChange(double dTime_s)
{
	FRC_2013_Control_Interface *pw_access=m_RobotControl;
	double NormalizedVelocity=pw_access->GetRotaryCurrentPorV(FRC_2013_Robot::ePowerWheelSecondStage) / m_PowerWheelMaxSpeed;
	//NormalizedVelocity-=0.2;
	//if (NormalizedVelocity<0.0)
	//	NormalizedVelocity=0.0;

	//Scale down the rotation to something easy to gauge in UI
	AddRotation((NormalizedVelocity * 18) * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2013_Power_Wheel_UI												*/
/***************************************************************************************************************/

void FRC_2013_Power_Slow_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(-0.6,-2.0);
	Myprops.m_Color=osg::Vec4(1.0,1.0,0.0,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
	m_PowerWheelMaxSpeed=m_RobotControl->GetRobotProps().GetPowerWheelProps().GetMaxSpeed();
}

void FRC_2013_Power_Slow_Wheel_UI::TimeChange(double dTime_s)
{
	FRC_2013_Control_Interface *pw_access=m_RobotControl;
	double NormalizedVelocity=pw_access->GetRotaryCurrentPorV(FRC_2013_Robot::ePowerWheelFirstStage) / m_PowerWheelMaxSpeed;

	//Scale down the rotation to something easy to gauge in UI
	AddRotation((NormalizedVelocity * 18) * dTime_s);
}

  /***************************************************************************************************************/
 /*												FRC_2013_Rollers_UI												*/
/***************************************************************************************************************/

void FRC_2013_Rollers_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,2.0);
	Myprops.m_Color=osg::Vec4(0.75,0.75,0.75,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
}

void FRC_2013_Rollers_UI::TimeChange(double dTime_s)
{
	FRC_2013_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2013_Robot::eRollers);
	AddRotation(Velocity* 0.5 * dTime_s);
	double y_offset=pw_access->GetRotaryCurrentPorV(FRC_2013_Robot::eIntake_Deployment);
	//normalize
	const Ship_1D_Props &props=m_RobotControl->GetRobotProps().GetIntakeDeploymentProps().GetShip_1D_Props();
	y_offset-=props.MinRange;
	y_offset/=fabs(props.MaxRange-props.MinRange);
	//printf("\rTest=%.2f     ",y_offset);
	UpdatePosition(0.0,2.0+y_offset);
}


  /***************************************************************************************************************/
 /*											FRC_2013_Fire_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2013_Fire_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,1.0);
	Myprops.m_Color=osg::Vec4(0.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2013_Fire_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2013_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2013_Robot::eHelix);
	AddRotation(Velocity* 0.5 * dTime_s);
	double y_offset=pw_access->GetIsSolenoidOpen(FRC_2013_Robot::eFirePiston)==true?-1.8:1.0;
	UpdatePosition(0.0,y_offset);
}

  /***************************************************************************************************************/
 /*												FRC_2013_Robot_UI												*/
/***************************************************************************************************************/

FRC_2013_Robot_UI::FRC_2013_Robot_UI(const char EntityName[]) : FRC_2013_Robot(EntityName,this),FRC_2013_Robot_Control(),
		m_TankUI(this),m_PowerWheelUI(this),m_PowerSlowWheelUI(this),m_Helix(this),m_Rollers(this)
{
}

void FRC_2013_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
	m_PowerWheelUI.TimeChange(dTime_s);
	m_PowerSlowWheelUI.TimeChange(dTime_s);
	m_Helix.TimeChange(dTime_s);
	m_Rollers.TimeChange(dTime_s);
}
void FRC_2013_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
	m_PowerWheelUI.Initialize(em);
	m_PowerSlowWheelUI.Initialize(em);
	m_Helix.Initialize(em);
	m_Rollers.Initialize(em);
}

void FRC_2013_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
	m_PowerWheelUI.UI_Init(parent);
	m_PowerSlowWheelUI.UI_Init(parent);
	m_Helix.UI_Init(parent);
	m_Rollers.UI_Init(parent);
}
void FRC_2013_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
	m_PowerWheelUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_PowerSlowWheelUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_Helix.update(nv,draw,parent_pos,-GetAtt_r());
	m_Rollers.update(nv,draw,parent_pos,-GetAtt_r());
}
void FRC_2013_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
	m_PowerWheelUI.Text_SizeToUse(SizeToUse);
	m_PowerSlowWheelUI.Text_SizeToUse(SizeToUse);
	m_Helix.Text_SizeToUse(SizeToUse);
	m_Rollers.Text_SizeToUse(SizeToUse);
}
void FRC_2013_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
	m_PowerWheelUI.UpdateScene(geode,AddOrRemove);
	m_PowerSlowWheelUI.UpdateScene(geode,AddOrRemove);
	m_Helix.UpdateScene(geode,AddOrRemove);
	m_Rollers.UpdateScene(geode,AddOrRemove);
}
