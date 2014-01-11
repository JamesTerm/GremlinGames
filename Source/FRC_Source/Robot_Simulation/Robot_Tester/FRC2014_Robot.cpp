#include "stdafx.h"
#include "Robot_Tester.h"
namespace Robot_Tester
{
	#include "Tank_Robot_UI.h"
	#include "CommonUI.h"
	#include "FRC2014_Robot.h"
}

using namespace Robot_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;


#define __DisableTurretTargeting__
#define __DisableEncoderTracking__

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
 /*														FRC_2014_Robot::Turret														*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Turret::Turret(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("Turret",robot_control,eTurret),m_pParent(parent),m_Velocity(0.0),m_LastIntendedPosition(0.0)
{
}

void FRC_2014_Robot::Turret::SetIntendedPosition_Plus(double Position)
{
	if (GetPotUsage()!=Rotary_Position_Control::eNoPot)
	{
		if (((fabs(m_LastIntendedPosition-Position)<0.01)) || (!(IsZero(GetRequestedVelocity()))) )
			return;
		{
			m_LastIntendedPosition=Position; //grab it before all the conversions
			Position=-Position; 
			//By default this goes from -1 to 1.0 
			//first get the range from 0 - 1
			double positive_range = (Position * 0.5) + 0.5;
			//positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=GetMaxRange();
			const double Scale=(maxRange-minRange);
			Position=(positive_range * Scale) + minRange;
			//DOUT5("Test=%f",RAD_2_DEG(Position));
			SetIntendedPosition(Position);
		}
	}
	else
		Turret_SetRequestedVelocity(PositionToVelocity_Tweak(Position));   //allow manual use of same control
}

void FRC_2014_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventValue_Map["Turret_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2014_Robot::Turret::SetIntendedPosition_Plus);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventValue_Map["Turret_SetIntendedPosition"].Remove(*this, &FRC_2014_Robot::Turret::SetIntendedPosition_Plus);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::Turret::SetPotentiometerSafety);
	}
}

void FRC_2014_Robot::Turret::TimeChange(double dTime_s)
{
	SetRequestedVelocity_FromNormalized(m_Velocity);
	m_Velocity=0.0;

	#ifndef __DisableTurretTargeting__
	if ((!m_pParent->m_DisableTurretTargetingValue) && (m_pParent->m_IsTargeting)&&(IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer())
	{
		Vec2D Target=m_pParent->m_TargetOffset;
		Target-=m_pParent->GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);
		AngleToUse-=m_pParent->GetAtt_r();
		SetIntendedPosition(NormalizeRotation2(AngleToUse) * m_pParent->m_YawErrorCorrection);
		//TODO factor in velocity once we have our ball velocity (to solve for time)
	}
	#endif

	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2014_Robot::Turret::ResetPos()
{
	__super::ResetPos();
	SetPos_m(-Pi);  //It starts out backwards
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2014_Robot::PitchRamp::PitchRamp(FRC_2014_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("PitchRamp",robot_control,ePitchRamp),m_pParent(pParent)
{
}

void FRC_2014_Robot::PitchRamp::SetIntendedPosition_Plus(double Position)
{
	if (GetPotUsage()!=Rotary_Position_Control::eNoPot)
	{
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

void FRC_2014_Robot::PitchRamp::TimeChange(double dTime_s)
{
	__super::SetIntendedPosition(m_pParent->m_PitchAngle);
	__super::TimeChange(dTime_s);
	#ifdef __DebugLUA__
	Dout(m_pParent->m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,7,"p%.1f",RAD_2_DEG(GetPos_m()));
	#endif
}

void FRC_2014_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2014_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::PitchRamp::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Remove(*this, &FRC_2014_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::PitchRamp::SetPotentiometerSafety);
	}
}


  /***********************************************************************************************************************************/
 /*															FRC_2014_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

FRC_2014_Robot::FRC_2014_Robot(const char EntityName[],FRC_2014_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), m_Turret(this,robot_control),m_PitchRamp(this,robot_control),
		m_DefensiveKeyPosition(Vec2D(0.0,0.0)),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),m_AutonPresetIndex(0),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_SetLowGear(false)
{
}

void FRC_2014_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2014_Robot_Properties *RobotProps=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);

	//set to the default key position
	const FRC_2014_Robot_Props &robot2014props=RobotProps->GetFRC2014RobotProps();
}
void FRC_2014_Robot::ResetPos()
{
	//We cannot reset position between auton and telop
	SetBypassPosAtt_Update(true);
	__super::ResetPos();
	SetBypassPosAtt_Update(false);

	//This should be false to avoid any conflicts during a reset
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
}

void FRC_2014_Robot::TimeChange(double dTime_s)
{
	const FRC_2014_Robot_Props &robot_props=m_RobotProps.GetFRC2014RobotProps();

	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.AsEntity1D().TimeChange(dTime_s);
	m_PitchRamp.AsEntity1D().TimeChange(dTime_s);
}

const FRC_2014_Robot_Properties &FRC_2014_Robot::GetRobotProps() const
{
	return m_RobotProps;
}

void FRC_2014_Robot::SetLowGear(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	m_SetLowGear=on;
	SetBypassPosAtt_Update(true);
	m_Turret.SetBypassPos_Update(true);
	m_PitchRamp.SetBypassPos_Update(true);

	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetLowGear?&m_RobotProps.GetLowGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);
	m_Turret.SetBypassPos_Update(false);
	m_PitchRamp.SetBypassPos_Update(false);

	m_RobotControl->OpenSolenoid(eUseLowGear,on);
}

void FRC_2014_Robot::SetLowGearValue(double Value)
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//printf("\r%f       ",Value);
	if (Value > 0.0)
	{
		if (m_SetLowGear)
		{
			SetLowGear(false);
			printf("Now in HighGear\n");
		}
	}
	else
	{
		if (!m_SetLowGear)
		{
			SetLowGear(true);
			printf("Now in LowGear\n");
		}
	}
}

void FRC_2014_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D_Kind::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_SetLowGear"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"].Subscribe(ehl, *this, &FRC_2014_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Subscribe(ehl,*this, &FRC_2014_Robot::SetLowGearValue);
	}
	else
	{
		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2014_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2014_Robot::SetLowGearValue);
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
}

void FRC_2014_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2014_Robot_Properties::FRC_2014_Robot_Properties()  : m_TurretProps(
	"Turret",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Props::eSwivel,
	true,	//Using the range
	-Pi,Pi
	),
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
	m_RobotControls(&s_ControlsEvents)
{
	{
		FRC_2014_Robot_Props props;
		const double KeyDistance=Inches2Meters(144);
		const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		const double DefaultY=c_HalfCourtLength-KeyDistance;
		const double HalfKeyWidth=KeyWidth/2.0;

		FRC_2014_Robot_Props::Autonomous_Properties &auton=props.Autonomous_Props;
		auton.MoveForward=0.0;
		m_FRC2014RobotProps=props;
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
		Rotary_Props props=m_TurretProps.RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_TurretProps.RotaryProps()=props;
	}
	{
		Rotary_Props props=m_PitchRampProps.RotaryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_PitchRampProps.RotaryProps()=props;
	}
}

const char *ProcessVec2D(FRC_2014_Robot_Props &m_FRC2014RobotProps,Scripting::Script& script,Vec2d &Dest)
{
	const char *err;
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


//declared as global to avoid allocation on stack each iteration
const char * const g_FRC_2014_Controls_Events[] = 
{
	"Turret_SetCurrentVelocity","Turret_SetIntendedPosition","Turret_SetPotentiometerSafety",
	"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
	"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue"
};

const char *FRC_2014_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2014_Controls_Events))?g_FRC_2014_Controls_Events[index] : NULL;
}
FRC_2014_Robot_Properties::ControlEvents FRC_2014_Robot_Properties::s_ControlsEvents;

void FRC_2014_Robot_Properties::LoadFromScript(Scripting::Script& script)
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
		err = script.GetFieldTable("turret");
		if (!err)
		{
			m_TurretProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("pitch");
		if (!err)
		{
			m_PitchRampProps.LoadFromScript(script);
			script.Pop();
		}

		m_LowGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("low_gear");
		if (!err)
		{
			m_LowGearProps.LoadFromScript(script);
			script.Pop();
		}

		err = script.GetFieldTable("auton");
		if (!err)
		{
			struct FRC_2014_Robot_Props::Autonomous_Properties &auton=m_FRC2014RobotProps.Autonomous_Props;
			{
				double length;
				err = script.GetField("move_forward_ft", NULL, NULL,&length);
				if (!err)
					auton.MoveForward=Feet2Meters(length);
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
 /*														FRC_2014_Goals::Fire														*/
/***********************************************************************************************************************************/

FRC_2014_Goals::Fire::Fire(FRC_2014_Robot &robot,bool On, bool DoSquirt) : m_Robot(robot),m_Terminate(false),m_IsOn(On),m_DoSquirt(DoSquirt)
{
	m_Status=eInactive;
}
FRC_2014_Goals::Fire::Goal_Status FRC_2014_Goals::Fire::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
		
	m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Goals::WaitForBall														*/
/***********************************************************************************************************************************/

FRC_2014_Goals::WaitForBall::WaitForBall(FRC_2014_Robot &robot,double Tolerance) :  m_Robot(robot),m_Tolerance(Tolerance),m_Terminate(false)
{
	m_Status=eInactive;
}
Goal::Goal_Status FRC_2014_Goals::WaitForBall::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	//TODO
	if (true)
	{
		printf("Ball Deployed\n");
		m_Status=eCompleted;
	}
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Goals::OperateSolenoid													*/
/***********************************************************************************************************************************/

FRC_2014_Goals::OperateSolenoid::OperateSolenoid(FRC_2014_Robot &robot,FRC_2014_Robot::SolenoidDevices SolenoidDevice,bool Open) : m_Robot(robot),
m_SolenoidDevice(SolenoidDevice),m_Terminate(false),m_IsOpen(Open) 
{	
	m_Status=eInactive;
}

FRC_2014_Goals::OperateSolenoid::Goal_Status FRC_2014_Goals::OperateSolenoid::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	switch (m_SolenoidDevice)
	{
		case FRC_2014_Robot::eUseLowGear:
			assert(false);
			break;
	}
	m_Status=eCompleted;
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*															FRC_2014_Goals															*/
/***********************************************************************************************************************************/

Goal *FRC_2014_Goals::Get_ShootBalls(FRC_2014_Robot *Robot,bool DoSquirt)
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

Goal *FRC_2014_Goals::Get_ShootBalls_WithPreset(FRC_2014_Robot *Robot,size_t KeyIndex)
{
	return Get_ShootBalls(Robot);
}

Goal *FRC_2014_Goals::Get_FRC2014_Autonomous(FRC_2014_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex)
{
	//TODO
	return NULL;
}

#ifdef Robot_TesterCode
  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
		case FRC_2014_Robot::eTurret:
			{
				//	printf("Turret=%f\n",Voltage);
				//DOUT3("Turret Voltage=%f",Voltage);
				m_TurretVoltage=Voltage;
				m_Turret_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Turret_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2014_Robot::ePitchRamp:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_PitchRampVoltage=Voltage;
				m_Pitch_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Pitch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
	case FRC_2014_Robot::eTurret:
		Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,1,"t=%.2f",Voltage);
		break;
	case FRC_2014_Robot::ePitchRamp:
		Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,1,"p=%.2f",Voltage);
		break;
	}
	#endif
}

FRC_2014_Robot_Control::FRC_2014_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_TurretVoltage(0.0),m_PowerWheelVoltage(0.0)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
	#if 0
	Dout(1,"");
	Dout(2,"");
	Dout(3,"");
	Dout(4,"");
	Dout(5,"");
	#endif
}

void FRC_2014_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case FRC_2014_Robot::eTurret:
			m_Turret_Pot.ResetPos();
			break;
		case FRC_2014_Robot::ePitchRamp:
			m_Pitch_Pot.ResetPos();
			//We may want this for more accurate simulation
			//m_Pitch_Pot.SetPos_m((m_Pitch_Pot.GetMinRange()+m_Pitch_Pot.GetMaxRange()) / 2.0);
			break;
	}
}

//This is only for Robot Tester
void FRC_2014_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}

void FRC_2014_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2014_Robot_Properties *robot_props=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		Rotary_Properties turret_props=robot_props->GetTurretProps();
		//turret_props.SetMinRange(0);
		//turret_props.SetMaxRange(Pi2);
		turret_props.SetUsingRange(false);
		m_Turret_Pot.Initialize(&turret_props);
		m_Pitch_Pot.Initialize(&robot_props->GetPitchRampProps());
	}
}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Turret_Pot.SetTimeDelta(dTime_s);
	m_Pitch_Pot.SetTimeDelta(dTime_s);
	m_Flippers_Pot.SetTimeDelta(dTime_s);
	m_PowerWheel_Enc.SetTimeDelta(dTime_s);
	m_LowerConveyor_Enc.SetTimeDelta(dTime_s);
	m_MiddleConveyor_Enc.SetTimeDelta(dTime_s);
	m_FireConveyor_Enc.SetTimeDelta(dTime_s);
	//display voltages
	DOUT(2,"l=%.2f r=%.2f t=%.2f pi=%.2f pw=%.2f lc=%.2f mc=%.2f fc=%.2f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
		m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage,m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage);
}


double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2014_Robot::eTurret:
		
			result=NormalizeRotation2(m_Turret_Pot.GetPotentiometerCurrentPosition() - Pi);
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			break;
		case FRC_2014_Robot::ePitchRamp:

			result=m_Pitch_Pot.GetPotentiometerCurrentPosition();
			DOUT (4,"pitch=%f flippers=%f",RAD_2_DEG(result),RAD_2_DEG(m_Flippers_Pot.GetPotentiometerCurrentPosition()));
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2014_Robot::eTurret:
			Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,14,"d=%.1f",RAD_2_DEG(result));
			break;
		case FRC_2014_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
			break;
	}
	#endif

	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2014_Robot::eUseLowGear:
		printf("UseLowGear=%d\n",Open);
		//m_UseLowGear=Open;
		break;
	}
}

  /***************************************************************************************************************/
 /*												FRC_2014_Robot_UI												*/
/***************************************************************************************************************/

FRC_2014_Robot_UI::FRC_2014_Robot_UI(const char EntityName[]) : FRC_2014_Robot(EntityName,this),FRC_2014_Robot_Control(),
		m_TankUI(this)
{
}

void FRC_2014_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
}
void FRC_2014_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
}

void FRC_2014_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
}
void FRC_2014_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
}
void FRC_2014_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
}
void FRC_2014_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
}

#endif