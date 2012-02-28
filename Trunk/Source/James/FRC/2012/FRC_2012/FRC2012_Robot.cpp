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
#include "Common/AI_Base_Controller.h"
#include "Common/Vehicle_Drive.h"
#include "Common/PIDController.h"
#include "Common/Tank_Robot.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "Common/Debug.h"
#include "FRC2012_Robot.h"

using namespace Framework::Base;
using namespace std;

namespace Base=Framework::Base;
namespace Scripting=Framework::Scripting;

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
		SetIntendedPosition(NormalizeRotation2(AngleToUse) * m_pParent->m_YawErrorCorrection);
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
			size_t DisplayRow=m_pParent->m_RobotProps.GetFRC2012RobotProps().PowerVelocity_DisplayRow;
			if (DisplayRow!=(size_t)-1)
			{
				const double rps=(Velocity * MAX_SPEED) / Pi2;
				Dout(DisplayRow,"%f ,%f",rps,Meters2Feet(rps * Pi * GetDimension()));
			}

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
	m_FireConveyor("FireConveyor",robot_control,eFireConveyor)
{
	m_ControlSignals.raw=0;
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
	bool Fire=(m_ControlSignals.bits.Fire==1) && PowerWheelReachedTolerance;
	bool Grip=m_ControlSignals.bits.Grip==1;
	bool GripL=m_ControlSignals.bits.GripL==1;
	bool GripM=m_ControlSignals.bits.GripM==1;
	bool GripH=m_ControlSignals.bits.GripH==1;
	bool Squirt=m_ControlSignals.bits.Squirt==1;

	//This assumes the motors are in the same orientation: 
	double LowerAcceleration=((Grip & (!LowerSensor)) || (LowerSensor & (!MiddleSensor))) | GripL | Squirt | Fire ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_LowerConveyor.SetCurrentLinearAcceleration(LowerAcceleration);

	double MiddleAcceleration= ((LowerSensor & (!MiddleSensor)) || (MiddleSensor & (!FireSensor))) | GripM | Squirt | Fire  ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
	m_MiddleConveyor.SetCurrentLinearAcceleration(MiddleAcceleration);

	double FireAcceleration= (MiddleSensor & (!FireSensor)) | GripH | Squirt | Fire  ?
		((Squirt)?m_MiddleConveyor.GetACCEL():-m_MiddleConveyor.GetBRAKE()):0.0;
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
		em->EventOnOff_Map["Ball_GripL"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripL);
		em->EventOnOff_Map["Ball_GripM"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripM);
		em->EventOnOff_Map["Ball_GripH"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::GripH);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2012_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2012_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_GripL"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripL);
		em->EventOnOff_Map["Ball_GripM"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripM);
		em->EventOnOff_Map["Ball_GripH"]  .Remove(*this, &FRC_2012_Robot::BallConveyorSystem::GripH);
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
		m_PowerWheels(this,robot_control),m_BallConveyorSystem(this,robot_control),m_Flippers(this,robot_control),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefaultPresetIndex(DefaultPresetIndex),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_IsTargeting(true),m_SetLowGear(false)
{
}

void FRC_2012_Robot::Initialize(Base::EventMap& em, const Entity_Properties *props)
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

FRC_2012_Robot::BallConveyorSystem &FRC_2012_Robot::GetBallConveyorSystem()
{
	return m_BallConveyorSystem;
}

void FRC_2012_Robot::ApplyErrorCorrection()
{
	const FRC_2012_Robot_Props &robot_props=m_RobotProps.GetFRC2012RobotProps();
	const Vec2d &Pos_m=GetPos_m();
	//first determine which quadrant we are in
	//These offsets are offsets added to the array indexes 
	const size_t XOffset=(Pos_m[0]>(robot_props.KeyGrid[1][1])[0]) ? 1 : 0;
	const double YCenterKey=(robot_props.KeyGrid[1][1])[1];
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
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c00=robot_props.KeyCorrections[0+YOffset][0+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c01=robot_props.KeyCorrections[0+YOffset][1+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c10=robot_props.KeyCorrections[1+YOffset][0+XOffset];
	const FRC_2012_Robot_Props::DeliveryCorrectionFields &c11=robot_props.KeyCorrections[1+YOffset][1+XOffset];

	const double pc_TopHalf=    (x * c01.PowerCorrection) + ((1.0-x)*c00.PowerCorrection);
	const double pc_BottomHalf= (x * c11.PowerCorrection) + ((1.0-x)*c10.PowerCorrection);
	const double pc = (y * pc_BottomHalf) + ((1.0-y) * pc_TopHalf);

	const double yc_TopHalf=    (x * c01.YawCorrection) + ((1.0-x)*c00.YawCorrection);
	const double yc_BottomHalf= (x * c11.YawCorrection) + ((1.0-x)*c10.YawCorrection);
	const double yc = (y * yc_TopHalf) + ((1.0-y) * yc_BottomHalf);

	//Now to apply correction... for now we'll apply to the easiest pieces possible and change if needed
	m_YawErrorCorrection=pc;
	m_PowerErrorCorrection=yc;
	//DOUT(5,"pc=%f yc=%f x=%f y=%f",pc,yc,x,y);
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	m_TargetOffset=c_TargetBasePosition;  //2d top view x,y of the target
	m_TargetHeight=c_TargetBaseHeight;    //1d z height (front view) of the target
	const FRC_2012_Robot_Props &robot_props=m_RobotProps.GetFRC2012RobotProps();
	const Vec2d &Pos_m=GetPos_m();
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
			Dout(robot_props.TargetVars_DisplayRow,"%.2f %.2f %.1f",RAD_2_DEG(m_Turret.GetPos_m()) ,RAD_2_DEG(m_PitchAngle),Meters2Feet(m_LinearVelocity));
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

void FRC_2012_Robot::SetPresetPosition(size_t index,bool IgnoreOrientation)
{
	Vec2D position=m_RobotProps.GetFRC2012RobotProps().PresetPositions[index];
	SetPosition(position[0],position[1]);

	if (!IgnoreOrientation)
	{	
		Vec2D Target=m_TargetOffset;
		Target-=GetPos_m();
		const double Angle=atan2(Target[1],Target[0]);
		double AngleToUse=-(Angle-PI_2);

		double TurretPos=NormalizeRotation2(AngleToUse)-m_Turret.GetPos_m();
		SetAttitude(TurretPos);
	}
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
	Framework::Base::EventMap *em=GetEventMap(); 
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
}

void FRC_2012_Robot::BindAdditionalUIControls(bool Bind,void *joy)
{
	Framework::UI::JoyStick_Binder *p_joy=(Framework::UI::JoyStick_Binder *)joy;
	const FRC_2012_Robot_Properties::Controls_List &robot_controls=m_RobotProps.Get_RobotControls();
	for (size_t i=0;i<robot_controls.size();i++)
	{
		const FRC_2012_Robot_Properties::Control_Props &control=robot_controls[i];

		for (size_t j=0;j<control.EventList.size();j++)
		{
			const UI_Controller::Controller_Element_Properties &element=control.EventList[j];
			switch (element.Type)
			{
			case UI_Controller::Controller_Element_Properties::eJoystickAnalog:
				{
					const UI_Controller::Controller_Element_Properties::ElementTypeSpecific::AnalogSpecifics_rw &analog=element.Specifics.Analog;
					p_joy->AddJoy_Analog_Default(analog.JoyAxis,element.Event.c_str(),analog.IsFlipped,analog.Multiplier,
						analog.FilterRange,analog.IsSquared,control.Controller.c_str());
				}
				break;
			case UI_Controller::Controller_Element_Properties::eJoystickButton:
				{
					const UI_Controller::Controller_Element_Properties::ElementTypeSpecific::ButtonSpecifics_rw &button=element.Specifics.Button;
					p_joy->AddJoy_Button_Default(button.WhichButton,element.Event.c_str(),button.useOnOff,button.dbl_click,control.Controller.c_str());
				}
				break;
			}
		}
	}
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
		props.PowerVelocity_DisplayRow=(size_t)-1;

		for (size_t row=0;row<3;row++)
		{
			for (size_t column=0;column<3;column++)
			{
				Vec2D &cell=props.KeyGrid[row][column];
				const double spread=Feet2Meters(7.0);
				const double x=spread * ((double)column-1.0);
				const double y=(spread * ((double)row-1.0)) + DefaultY;
				cell=Vec2D(x,y);
				props.KeyCorrections[row][column].PowerCorrection=1.0;
				props.KeyCorrections[row][column].YawCorrection=1.0;
			}
		}

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

const char *ProcessKeyCorrection(FRC_2012_Robot_Props &m_FRC2012RobotProps,Scripting::Script& script,size_t row,size_t column)
{
	const char* err=NULL;
	char CellName[4];
	CellName[0]='c';
	CellName[1]='1'+row;
	CellName[2]='1'+column;
	CellName[3]=0;
	err = script.GetFieldTable(CellName);

	err = script.GetField("p", NULL, NULL,&m_FRC2012RobotProps.KeyCorrections[row][column].PowerCorrection);
	err = script.GetField("x", NULL, NULL,&m_FRC2012RobotProps.KeyCorrections[row][column].YawCorrection);

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

		err=script.GetField("ds_power_velocity_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_FRC2012RobotProps.PowerVelocity_DisplayRow=(size_t)fDisplayRow;

		err = script.GetFieldTable("grid_corrections");
		if (!err)
		{
			for (size_t row=0;row<3;row++)
			{
				for (size_t column=0;column<3;column++)
				{
					err=ProcessKeyCorrection(m_FRC2012RobotProps,script,row,column);
					assert(!err);
				}
			}
			script.Pop();
		}

		err = script.GetFieldTable("controls");
		if (!err)
		{
			const char * const Events[] = 
			{
				"Joystick_SetCurrentSpeed_2","Analog_Turn",
				"Turret_SetCurrentVelocity","Turret_SetPotentiometerSafety",
				"PitchRamp_SetCurrentVelocity","PitchRamp_SetIntendedPosition","PitchRamp_SetPotentiometerSafety",
				"PowerWheels_SetCurrentVelocity","PowerWheels_SetEncoderSafety","PowerWheels_IsRunning",
				"Ball_SetCurrentVelocity","Ball_Fire","Ball_Squirt","Ball_Grip","Ball_GripL","Ball_GripM","Ball_GripH",
				"Flippers_SetCurrentVelocity","Flippers_SetIntendedPosition","Flippers_SetPotentiometerSafety",
				"Flippers_Advance","Flippers_Retract",
				"Robot_IsTargeting","Robot_SetTargetingOn","Robot_SetTargetingOff","Robot_TurretSetTargetingOff","Robot_SetTargetingValue",
				"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue",
				"Robot_SetPreset1","Robot_SetPreset2","Robot_SetPreset3","Robot_SetPresetPOV",
			};

			//TODO we may use actual product names here, but this will be fine for wind river build
			const char * const Controls[] =
			{
				"Joystick_1","Joystick_2","Joystick_3"
			};

			for (size_t i=0;i<_countof(Controls);i++)
			{
				err = script.GetFieldTable(Controls[i]);
				if (!err)
				{
					Control_Props control;
					control.Controller=Controls[i];
					for (size_t j=0;j<_countof(Events);j++)
					{
						UI_Controller::Controller_Element_Properties element;
						err=UI_Controller::ExtractControllerElementProperties(element,Events[j],script);
						if (!err)
							control.EventList.push_back(element);
					}
					m_RobotControls.push_back(control);
					script.Pop();
				}
			}
		}
		script.Pop();
	}
}


  /***********************************************************************************************************************************/
 /*														FRC_2012_Goals::Fire														*/
/***********************************************************************************************************************************/

FRC_2012_Goals::Fire::Fire(FRC_2012_Robot &robot,bool On) : m_Robot(robot),m_Terminate(false),m_IsOn(On)
{
	m_Status=eInactive;
}
FRC_2012_Goals::Fire::Goal_Status FRC_2012_Goals::Fire::Process(double dTime_s)
{
	if (m_Terminate)
	{
		if (m_Status==eActive)
			m_Status=eFailed;
		return m_Status;
	}
	ActivateIfInactive();
	m_Robot.GetBallConveyorSystem().Fire(m_IsOn);
	m_Status=eCompleted;
	return m_Status;
}


  /***********************************************************************************************************************************/
 /*															FRC_2012_Goals															*/
/***********************************************************************************************************************************/

Goal *FRC_2012_Goals::Get_ShootBalls(FRC_2012_Robot *Robot)
{
	Goal_Wait *goal_waitforturret=new Goal_Wait(1.0); //wait for turret
	Fire *FireOn=new Fire(*Robot,true);
	Goal_Wait *goal_waitforballs=new Goal_Wait(4.0); //wait for balls
	Fire *FireOff=new Fire(*Robot,false);
	Goal_NotifyWhenComplete *MainGoal=new Goal_NotifyWhenComplete(*Robot->GetEventMap(),"Complete");
	//Inserted in reverse since this is LIFO stack list
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->AddSubgoal(goal_waitforturret);
	return MainGoal;
}

Goal *FRC_2012_Goals::Get_ShootBalls_WithPreset(FRC_2012_Robot *Robot,size_t KeyIndex)
{
	Robot->SetPresetPosition(KeyIndex,true);
	return Get_ShootBalls(Robot);
}

//TODO Nuke this class upon successful testing of the LUA controls

  /***********************************************************************************************************************************/
 /*														FRC_2012_UI_Controller														*/
/***********************************************************************************************************************************/
#undef __2011Joysticks__
#undef __2012Joystick_with_LogitechOperator__
#undef __2012ActualControls__

#undef __AirFlo__
#undef __UsingXTerminator__

FRC_2012_UI_Controller::FRC_2012_UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller) : UI_Controller(joy,base_controller)
{
	using namespace Framework::UI;
	#if 0
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Joystick_SetCurrentSpeed_2",true,1.0,0.1,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",false,1.0,0.1,true,"Joystick_1");
	#endif

	#ifdef __2011Joysticks__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Robot_SetLowGearValue",true,1.0,0.0,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Turret_SetCurrentVelocity",false,1.0,0.1,true,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"PitchRamp_SetCurrentVelocity",false,1.0,0.1,true,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"PowerWheels_SetCurrentVelocity",true,1.0,0.0,false,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Robot_SetTargetingValue",true,1.000,0.0,false,"Joystick_2");
	
	joy.AddJoy_Button_Default( 0,"Ball_Fire",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Ball_Grip",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 6,"Ball_Squirt",true,false,"Joystick_2");
	#if 0
	joy.AddJoy_Button_Default( 7,"Robot_SetTargetingOn",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 8,"Robot_SetTargetingOff",false,false,"Joystick_2");
	#endif
	joy.AddJoy_Button_Default( 10,"PowerWheels_IsRunning",true,false,"Joystick_2");
	#endif
	
	#ifdef __2012Joystick_with_LogitechOperator__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Robot_SetLowGearValue",true,1.0,0.0,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Turret_SetCurrentVelocity",true,1.0,0.0,false,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"PowerWheels_SetCurrentVelocity",true,1.000,0.0,false,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"PitchRamp_SetIntendedPosition",true,1.142000,0.0,false,"Joystick_2");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Robot_SetTargetingValue",true,1.142000,0.0,false,"Joystick_2");
			
	joy.AddJoy_Button_Default( 1,"Ball_Grip",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 0,"Ball_Squirt",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Ball_Fire",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 3,"PowerWheels_IsRunning",true,false,"Joystick_2");
	#endif
	#ifdef __2012ActualControls__
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Robot_SetLowGearValue",true,1.0,0.0,false,"Joystick_1");
	joy.AddJoy_Button_Default( 0,"Flippers_Retract",true,false,"Joystick_1");
	joy.AddJoy_Button_Default( 3,"Flippers_Advance",true,false,"Joystick_1");
	
	//scaled down to 0.5 to allow fine tuning and a good top acceleration speed (may change with the lua script tweaks)
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Turret_SetCurrentVelocity",true,0.5,0.1,false,"Joystick_2");
	joy.AddJoy_Button_Default( 1,"Ball_Grip",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 0,"Ball_Squirt",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 3,"Ball_Fire",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 2,"PowerWheels_IsRunning",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 5,"Robot_TurretSetTargetingOff",true,false,"Joystick_2");
	joy.AddJoy_Button_Default( 4,"Robot_SetPreset1",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 8,"Robot_SetPreset2",false,false,"Joystick_2");
	joy.AddJoy_Button_Default( 9,"Robot_SetPreset3",false,false,"Joystick_2");
	
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"PitchRamp_SetIntendedPosition",true,1.142000,0.0,false,"Joystick_3");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Robot_SetTargetingValue",true,1.142000,0.0,false,"Joystick_3");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"PowerWheels_SetCurrentVelocity",true,1.000,0.0,false,"Joystick_3");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Turret_SetCurrentVelocity",true,0.5,0.1,true,"Joystick_3");
	
	//TODO test to ensure up is even and down is odd
	joy.AddJoy_Button_Default( 1,"Ball_Grip",true,false,"Joystick_3");
	joy.AddJoy_Button_Default( 0,"Ball_Squirt",true,false,"Joystick_3");
	joy.AddJoy_Button_Default( 5,"Ball_Fire",true,false,"Joystick_3");
	joy.AddJoy_Button_Default( 3,"PowerWheels_IsRunning",true,false,"Joystick_3");
	#endif
}
