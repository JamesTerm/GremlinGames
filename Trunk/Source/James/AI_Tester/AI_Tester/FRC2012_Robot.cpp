#include "stdafx.h"
#include "AI_Tester.h"
#include "Debug.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
	#include "FRC2012_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

namespace Base=GG_Framework::Base;
namespace Scripting=GG_Framework::Logic::Scripting;

  /***********************************************************************************************************************************/
 /*														FRC_2012_Robot::Turret														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Turret::Turret(FRC_2012_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Linear("Turret",robot_control,eTurret),m_pParent(parent),m_Velocity(0.0)
{
}

void FRC_2012_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Turret::Turret_SetRequestedVelocity);
		em->EventOnOff_Map["Turret_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Turret::SetPotentiometerSafety);
	}
}

void FRC_2012_Robot::Turret::TimeChange(double dTime_s)
{
	SetRequestedVelocity_FromNormalized(m_Velocity);
	m_Velocity=0.0;

	if ((!m_pParent->m_DisableTurretTargetingValue) && (m_pParent->m_IsTargeting)&&(IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer())
	{
		Vec2D Target=m_pParent->m_TargetOffset;
		Target-=m_pParent->GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);
		AngleToUse-=m_pParent->GetAtt_r();
		SetIntendedPosition(NormalizeRotation2(AngleToUse));
		//TODO factor in velocity once we have our ball velocity (to solve for time)
	}
	__super::TimeChange(dTime_s);
}

//This will make the scale to half with a 0.1 dead zone
double PositionToVelocity_Tweak(double Value)
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
 /*													FRC_2012_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::PitchRamp::PitchRamp(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Linear("PitchRamp",robot_control,ePitchRamp),m_pParent(pParent)
{
}

void FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus(double Position)
{
	if (GetIsUsingPotentiometer())
	{
		bool IsTargeting=(m_pParent->m_IsTargeting);
		if (!IsTargeting)
		{
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

void FRC_2012_Robot::PitchRamp::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && (IsZero(GetRequestedVelocity())) && GetIsUsingPotentiometer());
	if (IsTargeting)
	{
		__super::SetIntendedPosition(m_pParent->m_PitchAngle);
	}
	__super::TimeChange(dTime_s);
}

void FRC_2012_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["PitchRamp_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetIntendedPosition_Plus);
		em->EventOnOff_Map["PitchRamp_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::PitchRamp::SetPotentiometerSafety);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::PowerWheels														*/
/***********************************************************************************************************************************/

FRC_2012_Robot::PowerWheels::PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
	Rotary_Angular("PowerWheels",robot_control,ePowerWheels,eActive),m_pParent(pParent),m_IsRunning(false)
{
}

void FRC_2012_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Subscribe(ehl,*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
	else
	{
		em->EventValue_Map["PowerWheels_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["PowerWheels_SetEncoderSafety"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetEncoderSafety);
		em->EventOnOff_Map["PowerWheels_IsRunning"].Remove(*this, &FRC_2012_Robot::PowerWheels::SetIsRunning);
	}
}

void FRC_2012_Robot::PowerWheels::SetRequestedVelocity_FromNormalized(double Velocity) 
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	if (!IsTargeting)
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested()))
		{
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (Velocity * 0.5) + 0.5;
			positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=5.0 * Pi2;  //TODO determine slowest speed to use
			const double maxRange=MAX_SPEED;
			const double Scale=(maxRange-minRange) / MAX_SPEED;
			const double Offset=minRange/MAX_SPEED;
			Velocity=(positive_range * Scale) + Offset;
			//DOUT5("%f",Velocity);
			__super::SetRequestedVelocity_FromNormalized(Velocity);
		}
		else
			__super::SetRequestedVelocity_FromNormalized(0.0);
	}
}

void FRC_2012_Robot::PowerWheels::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	if (  IsTargeting )
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested())) 
		{
			//convert linear velocity to angular velocity
			double RPS=m_pParent->m_LinearVelocity / (Pi * GetDimension());
			RPS*=2.0;  //For hooded shoot we'll have to move twice as fast
			SetRequestedVelocity(RPS * Pi2);
			//DOUT5("v=%f rps=%f rad=%f",m_pParent->m_LinearVelocity * 3.2808399,RPS,RPS*Pi2);
		}
		else
			SetRequestedVelocity(0);
	}
	__super::TimeChange(dTime_s);
}

  /***********************************************************************************************************************************/
 /*												FRC_2012_Robot::BallConveyorSystem													*/
/***********************************************************************************************************************************/

FRC_2012_Robot::BallConveyorSystem::BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_LowerConveyor("LowerConveyor",robot_control,eLowerConveyor),m_MiddleConveyor("MiddleConveyor",robot_control,eMiddleConveyor),
	m_FireConveyor("FireConveyor",robot_control,eFireConveyor),
		m_Grip(false),m_Squirt(false),m_Fire(false)
{
	//This are always open loop as there is no encoder and this is specified by default
}

void FRC_2012_Robot::BallConveyorSystem::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_LowerConveyor.Initialize(em,props);
	m_MiddleConveyor.Initialize(em,props);
	m_FireConveyor.Initialize(em,props);
}

void FRC_2012_Robot::BallConveyorSystem::TimeChange(double dTime_s)
{
	const bool LowerSensor=m_pParent->m_RobotControl->GetBoolSensorState(eLowerConveyor_Sensor);
	const bool MiddleSensor=m_pParent->m_RobotControl->GetBoolSensorState(eMiddleConveyor_Sensor);
	const bool FireSensor=m_pParent->m_RobotControl->GetBoolSensorState(eFireConveyor_Sensor);
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetRotary_Properties().PrecisionTolerance;
	//Only fire when the wheel has reached its aiming speed
	bool Fire=m_Fire && PowerWheelReachedTolerance;

	//This assumes the motors are in the same orientation: 
	double LowerAcceleration=((m_Grip & (!LowerSensor)) || (LowerSensor & (!MiddleSensor))) | m_Squirt | Fire ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_LowerConveyor.SetCurrentLinearAcceleration(LowerAcceleration);

	double MiddleAcceleration= ((LowerSensor & (!MiddleSensor)) || (MiddleSensor & (!FireSensor))) |  m_Squirt | Fire  ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_MiddleConveyor.SetCurrentLinearAcceleration(MiddleAcceleration);

	double FireAcceleration= (MiddleSensor & (!FireSensor)) | m_Squirt | Fire  ?
		((m_Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_FireConveyor.SetCurrentLinearAcceleration(FireAcceleration);

	m_LowerConveyor.AsEntity1D().TimeChange(dTime_s);
	m_MiddleConveyor.AsEntity1D().TimeChange(dTime_s);
	m_FireConveyor.AsEntity1D().TimeChange(dTime_s);
}

//This is the manual override, but probably not used if we use spike as it would be wasteful to have a analog control for this
void FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_LowerConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_MiddleConveyor.SetRequestedVelocity_FromNormalized(Velocity);
	m_FireConveyor.SetRequestedVelocity_FromNormalized(Velocity);
}

void FRC_2012_Robot::BallConveyorSystem::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_MiddleConveyor.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//Ball_SetCurrentVelocity is the manual override
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot::Flippers														*/
/***********************************************************************************************************************************/
FRC_2012_Robot::Flippers::Flippers(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control) : 
Rotary_Linear("Flippers",robot_control,eFlippers),m_pParent(pParent),m_Advance(false),m_Retract(false)
{
}

void FRC_2012_Robot::Flippers::SetIntendedPosition(double Position)
{
	//DOUT5("Test=%f",RAD_2_DEG(Position));
	__super::SetIntendedPosition(Position);
}

void FRC_2012_Robot::Flippers::TimeChange(double dTime_s)
{
	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Advance ^ m_Retract)
		SetCurrentLinearAcceleration(m_Advance?ACCEL:-BRAKE);

	__super::TimeChange(dTime_s);
}

void FRC_2012_Robot::Flippers::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Flippers_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Flippers_SetIntendedPosition"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetIntendedPosition);
		em->EventOnOff_Map["Flippers_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::SetPotentiometerSafety);
		em->EventOnOff_Map["Flippers_Advance"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::Advance);
		em->EventOnOff_Map["Flippers_Retract"].Subscribe(ehl,*this, &FRC_2012_Robot::Flippers::Retract);
	}
	else
	{
		em->EventValue_Map["Flippers_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::Flippers::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Flippers_SetIntendedPosition"].Remove(*this, &FRC_2012_Robot::Flippers::SetIntendedPosition);
		em->EventOnOff_Map["Flippers_SetPotentiometerSafety"].Remove(*this, &FRC_2012_Robot::Flippers::SetPotentiometerSafety);
		em->EventOnOff_Map["Flippers_Advance"].Remove(*this, &FRC_2012_Robot::Flippers::Advance);
		em->EventOnOff_Map["Flippers_Retract"].Remove(*this, &FRC_2012_Robot::Flippers::Retract);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/

const double c_CourtLength=Feet2Meters(54);
const double c_CourtWidth=Feet2Meters(27);
const double c_HalfCourtLength=c_CourtLength/2.0;
const double c_HalfCourtWidth=c_CourtWidth/2.0;

const FRC_2012_Robot::Vec2D c_TargetBasePosition=FRC_2012_Robot::Vec2D(0.0,c_HalfCourtLength);
const double c_BallShootHeight_inches=55.0;
const double c_TargetBaseHeight= Inches2Meters(98.0 - c_BallShootHeight_inches);
//http://www.sciencedaily.com/releases/2011/03/110310151224.htm
//The results show the optimal aim points make a "V" shape near the top center of the backboard's "square," which is actually a 
//24-inch by 18-inch rectangle which surrounds the rim
const FRC_2012_Robot::Vec2D c_BankShot_Box=FRC_2012_Robot::Vec2D(Inches2Meters(24),Inches2Meters(18)); //width x length
//http://esciencenews.com/articles/2011/03/10/the.physics.bank.shots
const double c_BankShot_BackboardY_Offset=Inches2Meters(3.327);
const double c_BankShot_V_Angle=0.64267086025537;
const double c_BankShot_V_Distance=Inches2Meters(166164/8299); //about 20.02 inches
const double c_BankShot_V_MiddlePointSaturationDistance=Inches2Meters(1066219/331960);  //about 3.211
const double c_BankShot_V_Hieght_plus_SatPoint=Inches2Meters(12.86);
const double c_BankShot_V_SatHieght=Inches2Meters(2.57111110379327);
const double c_BankShot_Initial_V_Hieght=Inches2Meters(12.86)-c_BankShot_V_SatHieght; //about 10.288 inches to the point of the V

FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,size_t DefaultPresetIndex,bool IsAutonomous) : 
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), m_Turret(this,robot_control),m_PitchRamp(this,robot_control),
		m_PowerWheels(this,robot_control),m_BallConveyorSystem(this,robot_control),m_Flippers(this,robot_control),m_DefaultPresetIndex(DefaultPresetIndex),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_IsTargeting(true),m_SetLowGear(false)
{
}

void FRC_2012_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)
	m_Turret.Initialize(em,RobotProps?&RobotProps->GetTurretProps():NULL);
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
	m_BallConveyorSystem.Initialize(em,RobotProps?&RobotProps->GetConveyorProps():NULL);
	m_Flippers.Initialize(em,RobotProps?&RobotProps->GetFlipperProps():NULL);

	//set to the default key position
	const FRC_2012_Robot_Props &robot2012props=RobotProps->GetFRC2012RobotProps();
	SetDefaultPosition(robot2012props.PresetPositions[m_DefaultPresetIndex]);
}
void FRC_2012_Robot::ResetPos()
{
	__super::ResetPos();
	//This only matters if we do not have an axis assigned to targeting; otherwise it will be overridden
	m_IsTargeting=true;
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	m_PowerWheels.ResetPos();
	m_BallConveyorSystem.ResetPos();
	m_Flippers.ResetPos();
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	m_TargetOffset=c_TargetBasePosition;  //2d top view x,y of the target
	m_TargetHeight=c_TargetBaseHeight;    //1d z height (front view) of the target
	const FRC_2012_Robot_Props &robot_props=m_RobotProps.GetFRC2012RobotProps();
	Vec2d Pos_m=GetPos_m();
	//Got to make this fit within 20 chars :(
	Dout(robot_props.Coordinates_DiplayRow,"%.2f %.2f %.1f",Meters2Feet(Pos_m[0]),
		Meters2Feet(Pos_m[1]),RAD_2_DEG(GetAtt_r()));
	const double x=Vec2D(Pos_m-m_TargetOffset).length();
	{
		const bool DoBankShot= (x < Feet2Meters(16));  //if we are less than 16 feet away
		if (DoBankShot)
		{
			m_TargetOffset[1]+=c_BankShot_BackboardY_Offset;  //The point extends beyond the backboard
			const double XOffsetRatio=min (fabs(Pos_m[0]/c_HalfCourtWidth),1.0);  //restore sign in the end
			//Use this ratio to travel along the V distance... linear distribution should be adequate
			const double VOffset=XOffsetRatio*c_BankShot_V_Distance;
			//generate x / y from our VOffset
			double YawOffset=sin(c_BankShot_V_Angle) * VOffset;
			double PitchOffset=cos(c_BankShot_V_Angle) * VOffset;
			if (PitchOffset<c_BankShot_V_SatHieght)
				PitchOffset=c_BankShot_V_SatHieght;
			if (Pos_m[0]<0.0)
				YawOffset=-YawOffset;
			//DOUT(5,"v=%f x=%f y=%f",Meters2Inches(VOffset),Meters2Inches(YawOffset),Meters2Inches(PitchOffset) );
			m_TargetOffset[0]+=YawOffset;
			m_TargetHeight+=(c_BankShot_Initial_V_Hieght + PitchOffset);
			//DOUT(5,"x=%f y=%f",Meters2Inches(m_TargetOffset[0]),Meters2Inches(m_TargetHeight) );
		}
		else
			m_TargetOffset[1]-=Inches2Meters(9+6);  //hoop diameter 18... half that is 9 plus the 6 inch extension out

	}

	//TODO tweak adjustments based off my position in the field here
	//
	//Now to compute my pitch, power, and hang time
	{
		//TODO factor in rotation if it is significant
		const double y=m_TargetHeight;
		const double y2=y*y;
		const double x2=x*x;
		const double g=9.80665;
		//These equations come from here http://www.lightingsciences.ca/pdf/BNEWSEM2.PDF

		//Where y = height displacement (or goal - player)
		//	[theta=atan(sqrt(y^2+x^2)/x+y/x)]
		//This is equation 8 solving theta
		m_PitchAngle=atan(sqrt(y2+x2)/x+y/x);

		//Be sure G is in the same units as x and y!  (all in meters in code)
		//	V=sqrt(G(sqrt(y^2+x^2)+y))
		//	This is equation 7 solving v
		m_LinearVelocity=sqrt(g*(sqrt(y2+x2)+y));

		//ta=(sin(theta)*v)/g   //This is equation 2 solving for t1
		//tb=(x-ta*cos(theta)*v)/(cos(theta)*v)   //this is equation 3 solving for t2
		//	hang time= ta+tb 
		double ta,tb;
		ta=(sin(m_PitchAngle)*m_LinearVelocity)/g;
		tb=(x-ta*cos(m_PitchAngle)*m_LinearVelocity)/(cos(m_PitchAngle)*m_LinearVelocity);
		m_HangTime = ta+tb;
		{
			DOUT(5,"d=%f p=%f v=%f ht=%f",Meters2Feet(x) ,RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity),m_HangTime);
			double RPS=m_LinearVelocity / (Pi * m_PowerWheels.GetDimension());
			RPS*=2.0;  //For hooded shoot we'll have to move twice as fast
			Dout(robot_props.TargetVars_DisplayRow,"%.2f %.2f %.1f",RAD_2_DEG(m_Turret.GetPos_m()) ,RAD_2_DEG(m_PitchAngle),RPS);
		}
	}
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.AsEntity1D().TimeChange(dTime_s);
	m_PitchRamp.AsEntity1D().TimeChange(dTime_s);
	m_PowerWheels.AsEntity1D().TimeChange(dTime_s);
	m_BallConveyorSystem.TimeChange(dTime_s);
	m_Flippers.AsEntity1D().TimeChange(dTime_s);
}

const double c_rMotorDriveForward_DeadZone=0.02;
const double c_rMotorDriveReverse_DeadZone=0.02;
const double c_lMotorDriveForward_DeadZone=0.02;
const double c_lMotorDriveReverse_DeadZone=0.02;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void FRC_2012_Robot::ComputeDeadZone(double &LeftVoltage,double &RightVoltage)
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

void FRC_2012_Robot::SetTargetingValue(double Value)
{
	//TODO determine final scaler factor for the pitch (may want to make this a property)
	//printf("\r%f       ",Value);
	if (Value > -0.98)
	{
		if (m_IsTargeting)
		{
			m_IsTargeting=false;
			printf("Disabling Targeting\n");
		}
	}
	else
	{
		if (!m_IsTargeting)
		{
			m_IsTargeting=true;
			printf("Enabling Targeting\n");
		}
	}
}

void FRC_2012_Robot::SetLowGear(bool on) 
{
	m_SetLowGear=on;
	SetBypassPosAtt_Update(true);
	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetLowGear?&m_RobotProps.GetLowGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);

	m_RobotControl->OpenSolenoid(eUseLowGear,on);
}

void FRC_2012_Robot::SetLowGearValue(double Value)
{
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

void FRC_2012_Robot::SetPresetPosition(size_t index)
{
	Vec2D position=m_RobotProps.GetFRC2012RobotProps().PresetPositions[index];
	SetPosition(position[0],position[1]);

	Vec2D Target=m_TargetOffset;
	Target-=GetPos_m();
	const double Angle=atan2(Target[1],Target[0]);
	double AngleToUse=-(Angle-PI_2);

	double TurretPos=NormalizeRotation2(AngleToUse)-m_Turret.GetPos_m();
	SetAttitude(TurretPos);
}

void FRC_2012_Robot::SetPresetPOV (double value)
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
			size_t index=(size_t)(value/45.0);
			switch (index)
			{
				case 0:	SetPresetPosition(0);	break;
				case 2: SetPresetPosition(2);	break;
				case 6: SetPresetPosition(1);	break;
			}
		}
	}
	else 
		m_POVSetValve=false;
}

void FRC_2012_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D::EventMap *em=GetEventMap(); 
	if (Bind)
	{
		em->EventOnOff_Map["Robot_IsTargeting"].Subscribe(ehl, *this, &FRC_2012_Robot::IsTargeting);
		em->Event_Map["Robot_SetTargetingOn"].Subscribe(ehl, *this, &FRC_2012_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"].Subscribe(ehl, *this, &FRC_2012_Robot::SetTargetingOff);
		em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Subscribe(ehl,*this, &FRC_2012_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Subscribe(ehl,*this, &FRC_2012_Robot::SetTargetingValue);

		em->EventOnOff_Map["Robot_SetLowGear"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"].Subscribe(ehl, *this, &FRC_2012_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Subscribe(ehl,*this, &FRC_2012_Robot::SetLowGearValue);

		em->Event_Map["Robot_SetPreset1"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset1);
		em->Event_Map["Robot_SetPreset2"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset2);
		em->Event_Map["Robot_SetPreset3"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPreset3);
		em->EventValue_Map["Robot_SetPresetPOV"].Subscribe(ehl, *this, &FRC_2012_Robot::SetPresetPOV);
	}
	else
	{
		em->EventOnOff_Map["Robot_IsTargeting"]  .Remove(*this, &FRC_2012_Robot::IsTargeting);
		em->Event_Map["Robot_SetTargetingOn"]  .Remove(*this, &FRC_2012_Robot::SetTargetingOn);
		em->Event_Map["Robot_SetTargetingOff"]  .Remove(*this, &FRC_2012_Robot::SetTargetingOff);
		em->EventOnOff_Map["Robot_TurretSetTargetingOff"].Remove(*this, &FRC_2012_Robot::SetTurretTargetingOff);
		em->EventValue_Map["Robot_SetTargetingValue"].Remove(*this, &FRC_2012_Robot::SetTargetingValue);

		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2012_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2012_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2012_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2012_Robot::SetLowGearValue);

		em->Event_Map["Robot_SetPreset1"]  .Remove(*this, &FRC_2012_Robot::SetPreset1);
		em->Event_Map["Robot_SetPreset2"]  .Remove(*this, &FRC_2012_Robot::SetPreset2);
		em->Event_Map["Robot_SetPreset3"]  .Remove(*this, &FRC_2012_Robot::SetPreset3);
		em->EventValue_Map["Robot_SetPresetPOV"]  .Remove(*this, &FRC_2012_Robot::SetPresetPOV);
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_PowerWheels.BindAdditionalEventControls(Bind);
	m_BallConveyorSystem.BindAdditionalEventControls(Bind);
	m_Flippers.BindAdditionalEventControls(Bind);
	//Note: This is only for AITester
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
}


  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=Inches2Meters(6);
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2012_Robot_Properties::FRC_2012_Robot_Properties()  : m_TurretProps(
	"Turret",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eSwivel,
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
	Ship_1D_Properties::eRobotArm,
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
	Ship_1D_Properties::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	),
	m_ConveyorProps(
	"Conveyor",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	//RS-550 motor with 64:1 BaneBots transmission, so this is spec at 19300 rpm free, and 17250 peak efficiency
	//17250 / 64 = 287.5 = rps of motor / 64 reduction = 4.492 rps * 2pi = 28.22524
	28,   //Max Speed (rounded as we need not have precision)
	112.0,112.0, //ACCEL, BRAKE  (These work with the buttons, give max acceleration)
	112.0,112.0, //Max Acceleration Forward/Reverse  these can be real fast about a quarter of a second
	Ship_1D_Properties::eSimpleMotor,
	false,0.0,0.0,	//No limit ever!
	true //This is angular
	),
	m_FlipperProps(
	"Flippers",
	2.0,    //Mass
	Inches2Meters(12),   //Dimension  (this should be correct)
	1.4 * Pi2,   //Max Speed  (Parker gave this one, should be good)
	10.0,10.0, //ACCEL, BRAKE  (should be relatively quick)
	10.0,10.0, //Max Acceleration Forward/Reverse 
	Ship_1D_Properties::eRobotArm,
	true,	//Using the range
	-PI_2,PI_2 //TODO
	)
{
	{
		FRC_2012_Robot_Props props;
		const double KeyDistance=Inches2Meters(144);
		const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		const double DefaultY=c_HalfCourtLength-KeyDistance;
		const double HalfKeyWidth=KeyWidth/2.0;
		props.PresetPositions[0]=Vec2D(0.0,DefaultY);
		props.PresetPositions[1]=Vec2D(-HalfKeyWidth,DefaultY);
		props.PresetPositions[2]=Vec2D(HalfKeyWidth,DefaultY);
		props.Coordinates_DiplayRow=(size_t)-1;
		props.TargetVars_DisplayRow=(size_t)-1;
		m_FRC2012RobotProps=props;
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
		Rotary_Props props=m_TurretProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_TurretProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PitchRampProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.001; //we need high precision
		m_PitchRampProps.RoteryProps()=props;
	}
	{
		Rotary_Props props=m_PowerWheelProps.RoteryProps(); //start with super class settings
		props.PID[0]=1.0;
		props.PrecisionTolerance=0.1; //we need decent precision (this will depend on ramp up time too)
		m_PowerWheelProps.RoteryProps()=props;
	}
}

const char *ProcessKey(FRC_2012_Robot_Props &m_FRC2012RobotProps,Scripting::Script& script,size_t index)
{
	const char *err;
	typedef FRC_2012_Robot_Properties::Vec2D Vec2D;
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
	m_FRC2012RobotProps.PresetPositions[index]=Vec2D(width,c_HalfCourtLength-length);  //x,y  where x=width
	script.Pop();
	return err;
}

void FRC_2012_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
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
		err = script.GetFieldTable("power");
		if (!err)
		{
			m_PowerWheelProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("conveyor");
		if (!err)
		{
			m_ConveyorProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("flippers");
		if (!err)
		{
			m_FlipperProps.LoadFromScript(script);
			script.Pop();
		}

		m_LowGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("low_gear");
		if (!err)
		{
			m_LowGearProps.LoadFromScript(script);
			script.Pop();
		}

		
		err = script.GetFieldTable("key_1");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,0);

		err = script.GetFieldTable("key_2");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,1);

		err = script.GetFieldTable("key_3");
		if (!err) ProcessKey(m_FRC2012RobotProps,script,2);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.Coordinates_DiplayRow=(size_t)fDisplayRow;
		err=script.GetField("ds_target_vars_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.TargetVars_DisplayRow=(size_t)fDisplayRow;

		script.Pop();
	}
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) return;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			{
				//	printf("Turret=%f\n",Voltage);
				//DOUT3("Turret Voltage=%f",Voltage);
				m_TurretVoltage=Voltage;
				m_Turret_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Turret_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePitchRamp:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_PitchRampVoltage=Voltage;
				m_Pitch_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Pitch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::eFlippers:
			{
				//	printf("Flippers=%f\n",Voltage);
				//DOUT3("Flippers Voltage=%f",Voltage);
				m_FlipperVoltage=Voltage;
				m_Flippers_Pot.UpdatePotentiometerVoltage(Voltage);
				m_Flippers_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2012_Robot::ePowerWheels:
			m_PowerWheelVoltage=Voltage;
			m_PowerWheel_Enc.UpdateEncoderVoltage(Voltage);
			m_PowerWheel_Enc.TimeChange();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2012_Robot::eLowerConveyor:
			m_LowerConveyorVoltage=Voltage;
			m_LowerConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_LowerConveyor_Enc.TimeChange();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			m_MiddleConveyorVoltage=Voltage;
			m_MiddleConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_MiddleConveyor_Enc.TimeChange();
			break;
		case FRC_2012_Robot::eFireConveyor:
			m_FireConveyorVoltage=Voltage;
			m_FireConveyor_Enc.UpdateEncoderVoltage(Voltage);
			m_FireConveyor_Enc.TimeChange();
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
	case FRC_2012_Robot::eTurret:
		Dout(m_RobotProps.GetTurretProps().GetRoteryProps().Feedback_DiplayRow,1,"tu_v=%.2f",Voltage);
		break;
	case FRC_2012_Robot::ePitchRamp:
		Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,1,"pi_v=%.2f",Voltage);
		break;
	case FRC_2012_Robot::ePowerWheels:
		Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,1,"po_v=%.2f",Voltage);
		break;
	case FRC_2012_Robot::eFlippers:
		Dout(m_RobotProps.GetFlipperProps().GetRoteryProps().Feedback_DiplayRow,1,"fl_v=%.2f",Voltage);
		break;
	}
	#endif
}

bool FRC_2012_Robot_Control::GetBoolSensorState(size_t index)
{
	bool ret;
	switch (index)
	{
	case FRC_2012_Robot::eLowerConveyor_Sensor:
		ret=m_LowerSensor;
		break;
	case FRC_2012_Robot::eMiddleConveyor_Sensor:
		ret=m_MiddleSensor;
		break;
	case FRC_2012_Robot::eFireConveyor_Sensor:
		ret=m_FireSensor;
		break;
	default:
		assert (false);
	}
	return ret;
}

FRC_2012_Robot_Control::FRC_2012_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_TurretVoltage(0.0),m_PowerWheelVoltage(0.0),
	m_LowerSensor(false),m_MiddleSensor(false),m_FireSensor(false)
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

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			m_Turret_Pot.ResetPos();
			break;
		case FRC_2012_Robot::ePitchRamp:
			m_Pitch_Pot.ResetPos();
			//We may want this for more accurate simulation
			//m_Pitch_Pot.SetPos_m((m_Pitch_Pot.GetMinRange()+m_Pitch_Pot.GetMaxRange()) / 2.0);
			break;
		case FRC_2012_Robot::eFlippers:
			m_Flippers_Pot.ResetPos();
			break;
		case FRC_2012_Robot::ePowerWheels:
			m_PowerWheel_Enc.ResetPos();
			//DOUT3("Arm Voltage=%f",Voltage);
			break;
		case FRC_2012_Robot::eLowerConveyor:
			m_LowerConveyor_Enc.ResetPos();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			m_MiddleConveyor_Enc.ResetPos();
			break;
		case FRC_2012_Robot::eFireConveyor:
			m_FireConveyor_Enc.ResetPos();
			break;
	}
}

//This is only for AI Tester
void FRC_2012_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
	if (Bind)
	{
		em->EventOnOff_Map["Ball_LowerSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerLower);
		em->EventOnOff_Map["Ball_MiddleSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerMiddle);
		em->EventOnOff_Map["Ball_FireSensor"].Subscribe(ehl, *this, &FRC_2012_Robot_Control::TriggerFire);
	}
	else
	{
		em->EventOnOff_Map["Ball_LowerSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerLower);
		em->EventOnOff_Map["Ball_MiddleSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerMiddle);
		em->EventOnOff_Map["Ball_FireSensor"]  .Remove(*this, &FRC_2012_Robot_Control::TriggerFire);
	}
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	if (robot_props)
	{
		m_RobotProps=*robot_props;  //save a copy

		m_Turret_Pot.Initialize(&robot_props->GetTurretProps());
		m_Pitch_Pot.Initialize(&robot_props->GetPitchRampProps());
		m_Flippers_Pot.Initialize(&robot_props->GetFlipperProps());
		m_PowerWheel_Enc.Initialize(&robot_props->GetPowerWheelProps());
		m_LowerConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
		m_MiddleConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
		m_FireConveyor_Enc.Initialize(&robot_props->GetConveyorProps());
	}
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
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


double FRC_2012_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;

	switch (index)
	{
		case FRC_2012_Robot::eTurret:
		
			result=m_Turret_Pot.GetPotentiometerCurrentPosition();
			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			break;
		case FRC_2012_Robot::ePitchRamp:

			result=m_Pitch_Pot.GetPotentiometerCurrentPosition();
			DOUT (4,"pitch=%f flippers=%f",RAD_2_DEG(result),RAD_2_DEG(m_Flippers_Pot.GetPotentiometerCurrentPosition()));
			break;
		case FRC_2012_Robot::eFlippers:
			result=m_Flippers_Pot.GetPotentiometerCurrentPosition();
			break;
		case FRC_2012_Robot::ePowerWheels:
			result=m_PowerWheel_Enc.GetEncoderVelocity();
			break;
		case FRC_2012_Robot::eLowerConveyor:
			result=m_LowerConveyor_Enc.GetEncoderVelocity();
			break;
		case FRC_2012_Robot::eMiddleConveyor:
			result=m_MiddleConveyor_Enc.GetEncoderVelocity();
			//DOUT4 ("vel=%f",result);
			break;
		case FRC_2012_Robot::eFireConveyor:
			result=m_FireConveyor_Enc.GetEncoderVelocity();
			//DOUT5 ("vel=%f",result);
			break;
	}

	#ifdef __DebugLUA__
	switch (index)
	{
		case FRC_2012_Robot::eTurret:
			Dout(m_RobotProps.GetTurretProps().GetRoteryProps().Feedback_DiplayRow,11," d=%.2f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePitchRamp:
			Dout(m_RobotProps.GetPitchRampProps().GetRoteryProps().Feedback_DiplayRow,11," d=%.2f",RAD_2_DEG(result));
			break;
		case FRC_2012_Robot::ePowerWheels:
			Dout(m_RobotProps.GetPowerWheelProps().GetRoteryProps().Feedback_DiplayRow,11,"rs=%.2f",result / Pi2);
			break;
		case FRC_2012_Robot::eFlippers:
			Dout(m_RobotProps.GetFlipperProps().GetRoteryProps().Feedback_DiplayRow,11," d=%.2f",RAD_2_DEG(result));
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
		//m_UseLowGear=Open;
		break;
	case FRC_2012_Robot::eRampDeployment:
		printf("RampDeployment=%d\n",Open);
		//m_RampDeployment=Open;
		break;
	}
}
  /***********************************************************************************************************************************/
 /*														FRC_2012_Turret_UI															*/
/***********************************************************************************************************************************/

void FRC_2012_Turret_UI::Initialize(Entity2D::EventMap& em, const Turret_Properties *props)
{
	if (props)
		m_props=*props;
	else
		m_props.YOffset=3.0;
}
void FRC_2012_Turret_UI::UI_Init(Actor_Text *parent)
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Turret= new osgText::Text;
	m_Turret->setColor(osg::Vec4(0.0,1.0,0.0,1.0));
	m_Turret->setCharacterSize(m_UIParent->GetFontSize());
	m_Turret->setFontResolution(10,10);
	m_Turret->setPosition(position);
	m_Turret->setAlignment(osgText::Text::CENTER_CENTER);
	m_Turret->setText(L"\\/\n|\n( )");
	m_Turret->setUpdateCallback(m_UIParent);
}
void FRC_2012_Turret_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	FRC_2012_Control_Interface *turret_access=m_RobotControl;
	double Swivel=(-turret_access->GetRotaryCurrentPorV(FRC_2012_Robot::eTurret));
	double HeadingToUse=Heading+Swivel;
	const double FS=m_UIParent->GetFontSize();

	const double TurretLength=1.0;
	Vec2d TurretOffset(0,TurretLength);
	TurretOffset=GlobalToLocal(Swivel ,TurretOffset);
	TurretOffset[1]+=(m_props.YOffset-TurretLength);

	const Vec2d TurretLocalOffset=GlobalToLocal(Heading ,TurretOffset);
	const osg::Vec3 TurretPos (parent_pos[0]+( TurretLocalOffset[0]*FS),parent_pos[1]+( TurretLocalOffset[1]*FS),parent_pos[2]);

	//const char *TeamName=m_UIParent->GetEntityProperties_Interface()->GetTeamName();
	//if (strcmp(TeamName,"red")==0)
	//	m_Turret->setColor(osg::Vec4(1.0f,0.0f,0.5f,1.0f));  //This is almost magenta (easier to see)
	//else if (strcmp(TeamName,"blue")==0)
	//	m_Turret->setColor(osg::Vec4(0.0f,0.5f,1.0f,1.0f));  //This is almost cyan (easier to see too)

	if (m_Turret.valid())
	{
		m_Turret->setPosition(TurretPos);
		m_Turret->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}

}
void FRC_2012_Turret_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Turret.valid()) m_Turret->setCharacterSize(SizeToUse);
}
void FRC_2012_Turret_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Turret.valid()) geode->addDrawable(m_Turret);
	else
		if (m_Turret.valid()) geode->removeDrawable(m_Turret);
}
  /***************************************************************************************************************/
 /*												Side_Wheel_UI													*/
/***************************************************************************************************************/

void Side_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	if (props)
		m_props=*props;
	else
	{
		m_props.m_Offset=Vec2d(0,0);
		m_props.m_Color=osg::Vec4(1.0,0.0,0.5,1.0);
		m_props.m_TextDisplay=L"|";
	}
}

void Side_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Wheel= new osgText::Text;
	m_Wheel->setColor(m_props.m_Color);
	m_Wheel->setCharacterSize(m_UIParent->GetFontSize());
	m_Wheel->setFontResolution(10,10);
	m_Wheel->setPosition(position);
	m_Wheel->setAlignment(osgText::Text::CENTER_CENTER);
	m_Wheel->setText(m_props.m_TextDisplay);
	m_Wheel->setUpdateCallback(m_UIParent);
}

void Side_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Wheel.valid()) geode->addDrawable(m_Wheel);
	else
		if (m_Wheel.valid()) geode->removeDrawable(m_Wheel);
}

void Side_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	double HeadingToUse=Heading+m_Rotation;
	const double FS=m_UIParent->GetFontSize();
	const Vec2d WheelOffset(m_props.m_Offset[0],m_props.m_Offset[1]);
	const Vec2d WheelLocalOffset=GlobalToLocal(Heading,WheelOffset);
	const osg::Vec3 WheelPos (parent_pos[0]+( WheelLocalOffset[0]*FS),parent_pos[1]+( WheelLocalOffset[1]*FS),parent_pos[2]);

	if (m_Wheel.valid())
	{
		m_Wheel->setPosition(WheelPos);
		m_Wheel->setRotation(FromLW_Rot_Radians(HeadingToUse,0.0,0.0));
	}
}

void Side_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Wheel.valid()) m_Wheel->setCharacterSize(SizeToUse);
}

void Side_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

  /***************************************************************************************************************/
 /*											FRC_2012_Power_Wheel_UI												*/
/***************************************************************************************************************/

void FRC_2012_Power_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,2.0);
	Myprops.m_Color=osg::Vec4(1.0,0.0,0.5,1.0);
	Myprops.m_TextDisplay=L"|";

	__super::Initialize(em,&Myprops);
	m_PowerWheelMaxSpeed=m_RobotControl->GetRobotProps().GetPowerWheelProps().GetMaxSpeed();
}

void FRC_2012_Power_Wheel_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double NormalizedVelocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::ePowerWheels) / m_PowerWheelMaxSpeed;
	//NormalizedVelocity-=0.2;
	//if (NormalizedVelocity<0.0)
	//	NormalizedVelocity=0.0;

	//Scale down the rotation to something easy to gauge in UI
	AddRotation((NormalizedVelocity * 18) * dTime_s);
}
  /***************************************************************************************************************/
 /*											FRC_2012_Lower_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Lower_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.0,-1.5);
	Myprops.m_Color=osg::Vec4(0.5,1.0,1.0,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Lower_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eLowerConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Middle_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Middle_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.30,-0.5);
	Myprops.m_Color=osg::Vec4(1.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Middle_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eMiddleConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*											FRC_2012_Fire_Conveyor_UI											*/
/***************************************************************************************************************/

void FRC_2012_Fire_Conveyor_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	Wheel_Properties Myprops;
	Myprops.m_Offset=Vec2d(0.60,0.5);
	Myprops.m_Color=osg::Vec4(0.0,1.0,0.5,1.0);
	Myprops.m_TextDisplay=L"-";

	__super::Initialize(em,&Myprops);
}

void FRC_2012_Fire_Conveyor_UI::TimeChange(double dTime_s)
{
	FRC_2012_Control_Interface *pw_access=m_RobotControl;
	double Velocity=pw_access->GetRotaryCurrentPorV(FRC_2012_Robot::eFireConveyor);
	AddRotation(Velocity* 0.5 * dTime_s);
}

  /***************************************************************************************************************/
 /*												FRC_2012_Robot_UI												*/
/***************************************************************************************************************/

FRC_2012_Robot_UI::FRC_2012_Robot_UI(const char EntityName[]) : FRC_2012_Robot(EntityName,this),FRC_2012_Robot_Control(),
		m_TankUI(this),m_TurretUI(this),m_PowerWheelUI(this),m_LowerConveyor(this),m_MiddleConveyor(this),m_FireConveyor(this)
{
}

void FRC_2012_Robot_UI::TimeChange(double dTime_s) 
{
	__super::TimeChange(dTime_s);
	m_TankUI.TimeChange(dTime_s);
	m_PowerWheelUI.TimeChange(dTime_s);
	m_LowerConveyor.TimeChange(dTime_s);
	m_MiddleConveyor.TimeChange(dTime_s);
	m_FireConveyor.TimeChange(dTime_s);
}
void FRC_2012_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_TankUI.Initialize(em,props);
	m_TurretUI.Initialize(em);
	m_PowerWheelUI.Initialize(em);
	m_LowerConveyor.Initialize(em);
	m_MiddleConveyor.Initialize(em);
	m_FireConveyor.Initialize(em);
}

void FRC_2012_Robot_UI::UI_Init(Actor_Text *parent) 
{
	m_TankUI.UI_Init(parent);
	m_TurretUI.UI_Init(parent);
	m_PowerWheelUI.UI_Init(parent);
	m_LowerConveyor.UI_Init(parent);
	m_MiddleConveyor.UI_Init(parent);
	m_FireConveyor.UI_Init(parent);
}
void FRC_2012_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
{
	m_TankUI.custom_update(nv,draw,parent_pos);
	m_TurretUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_PowerWheelUI.update(nv,draw,parent_pos,-GetAtt_r());
	m_LowerConveyor.update(nv,draw,parent_pos,-GetAtt_r());
	m_MiddleConveyor.update(nv,draw,parent_pos,-GetAtt_r());
	m_FireConveyor.update(nv,draw,parent_pos,-GetAtt_r());
}
void FRC_2012_Robot_UI::Text_SizeToUse(double SizeToUse) 
{
	m_TankUI.Text_SizeToUse(SizeToUse);
	m_TurretUI.Text_SizeToUse(SizeToUse);
	m_PowerWheelUI.Text_SizeToUse(SizeToUse);
	m_LowerConveyor.Text_SizeToUse(SizeToUse);
	m_MiddleConveyor.Text_SizeToUse(SizeToUse);
	m_FireConveyor.Text_SizeToUse(SizeToUse);
}
void FRC_2012_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove) 
{
	m_TankUI.UpdateScene(geode,AddOrRemove);
	m_TurretUI.UpdateScene(geode,AddOrRemove);
	m_PowerWheelUI.UpdateScene(geode,AddOrRemove);
	m_LowerConveyor.UpdateScene(geode,AddOrRemove);
	m_MiddleConveyor.UpdateScene(geode,AddOrRemove);
	m_FireConveyor.UpdateScene(geode,AddOrRemove);
}
