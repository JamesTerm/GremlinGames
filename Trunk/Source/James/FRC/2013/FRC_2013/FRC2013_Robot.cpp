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
#include "Drive/Tank_Robot.h"
#include "Common/Robot_Control_Interface.h"
#include "Common/Rotary_System.h"
#include "Base/Joystick.h"
#include "Base/JoystickBinder.h"
#include "Common/UI_Controller.h"
#include "Common/PIDController.h"
#include "Common/Debug.h"
#include "FRC2013_Robot.h"
#include "UDP_Listener.h"

using namespace Framework::Base;
using namespace std;

namespace Base=Framework::Base;
namespace Scripting=Framework::Scripting;



#undef __DisableEncoderTracking__

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
	Rotary_Velocity_Control("PowerWheels",robot_control,ePowerWheels,eActive),m_pParent(pParent),m_ManualVelocity(0.0),m_IsRunning(false)
{
}

void FRC_2013_Robot::PowerWheels::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); 
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

void FRC_2013_Robot::PowerWheels::TimeChange(double dTime_s)
{
	bool IsTargeting=((m_pParent->m_IsTargeting) && GetEncoderUsage()==eActive);
	if (  IsTargeting )
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested())) 
		{
			//convert linear velocity to angular velocity
			double RPS=m_pParent->m_LinearVelocity / (Pi * GetDimension());
			RPS*=(2.0 * m_pParent->m_PowerErrorCorrection);  //For hooded shoot we'll have to move twice as fast
			SetRequestedVelocity(RPS * Pi2);
			//DOUT5("rps=%f rad=%f",RPS,RPS*Pi2);
		}
		else
			SetRequestedVelocity(0);
	}
	else
	{
		if ((m_IsRunning)||(m_pParent->m_BallConveyorSystem.GetIsFireRequested()))
		{
			//By default this goes from -1 to 1.0 we'll scale this down to work out between 17-35
			//first get the range from 0 - 1
			double positive_range = (m_ManualVelocity * 0.5) + 0.5;
			positive_range=positive_range>0.01?positive_range:0.0;
			const double minRange=GetMinRange();
			const double maxRange=MAX_SPEED;
			const double Scale=(maxRange-minRange) / MAX_SPEED;
			const double Offset=minRange/MAX_SPEED;
			const double Velocity=(positive_range * Scale) + Offset;
			//DOUT5("%f",Velocity);
			size_t DisplayRow=m_pParent->m_RobotProps.GetFRC2013RobotProps().PowerVelocity_DisplayRow;
			if (DisplayRow!=(size_t)-1)
			{
				const double rps=(Velocity * MAX_SPEED) / Pi2;
				Dout(DisplayRow,"%f ,%f",rps,Meters2Feet(rps * Pi * GetDimension()));
			}

			Rotary_Velocity_Control::SetRequestedVelocity_FromNormalized(Velocity);
		}
		else
			Rotary_Velocity_Control::SetRequestedVelocity_FromNormalized(0.0);
	}
	__super::TimeChange(dTime_s);
}

void FRC_2013_Robot::PowerWheels::ResetPos()
{
	m_IsRunning=false;
	__super::ResetPos();
}

  /***********************************************************************************************************************************/
 /*												FRC_2013_Robot::BallConveyorSystem													*/
/***********************************************************************************************************************************/

FRC_2013_Robot::BallConveyorSystem::BallConveyorSystem(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),
	m_FireConveyor("FireConveyor",robot_control,eFireConveyor),
	m_FireDelayTrigger_Time(0.0),m_FireStayOn_Time(0.0),
	m_FireDelayTriggerOn(false),m_FireStayOn(false)
{
	m_ControlSignals.raw=0;
	//This are always open loop as there is no encoder and this is specified by default
}

void FRC_2013_Robot::BallConveyorSystem::Initialize(Base::EventMap& em,const Entity1D_Properties *props)
{
	//These share the same props and fire is scaled from this level
	m_FireConveyor.Initialize(em,props);
}
void FRC_2013_Robot::BallConveyorSystem::ResetPos() 
{
	m_FireConveyor.ResetPos();
	m_ControlSignals.raw=0;
}

void FRC_2013_Robot::BallConveyorSystem::TimeChange(double dTime_s)
{
	const bool FireSensor=m_pParent->m_RobotControl->GetBoolSensorState(eFireConveyor_Sensor);
	const double PowerWheelSpeedDifference=m_pParent->m_PowerWheels.GetRequestedVelocity_Difference();
	const bool PowerWheelReachedTolerance=(m_pParent->m_PowerWheels.GetRequestedVelocity()!=0.0) &&
		(fabs(PowerWheelSpeedDifference)<m_pParent->m_PowerWheels.GetRotary_Properties().PrecisionTolerance);
	//Only fire when the wheel has reached its aiming speed
	bool Fire=(m_ControlSignals.bits.Fire==1) && PowerWheelReachedTolerance;
	//bool Grip=m_ControlSignals.bits.Grip==1;
	bool GripH=m_ControlSignals.bits.GripH==1;
	bool Squirt=m_ControlSignals.bits.Squirt==1;

	if (Fire)
	{
		if (m_FireDelayTriggerOn)
		{
			m_FireDelayTrigger_Time+=dTime_s;
			//printf("Fire delaying =%f\n",m_FireDelayTrigger_Time);
			if (m_FireDelayTrigger_Time>m_pParent->m_RobotProps.GetFRC2013RobotProps().FireTriggerDelay)
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
			if (m_FireStayOn_Time>m_pParent->m_RobotProps.GetFRC2013RobotProps().FireButtonStayOn_Time)
				m_FireStayOn=false;
		}
	}

	//This assumes the motors are in the same orientation: 
	//Note: FireSensor works different for now since the lower middle converyors are removed... we'll need to work out how this will work
	double FireAcceleration= FireSensor | GripH | Squirt | Fire | m_FireStayOn ?
		((Squirt)?m_FireConveyor.GetACCEL():-m_FireConveyor.GetBRAKE()):0.0;
	m_FireConveyor.SetCurrentLinearAcceleration(FireAcceleration);

	m_FireConveyor.AsEntity1D().TimeChange(dTime_s);
}

//This is the manual override, but probably not used if we use spike as it would be wasteful to have a analog control for this
void FRC_2013_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized(double Velocity)
{
	m_FireConveyor.SetRequestedVelocity_FromNormalized(Velocity);
}

void FRC_2013_Robot::BallConveyorSystem::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_FireConveyor.GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		//Ball_SetCurrentVelocity is the manual override
		em->EventValue_Map["Ball_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2013_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"].Subscribe(ehl, *this, &FRC_2013_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"].Subscribe(ehl, *this, &FRC_2013_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_GripH"].Subscribe(ehl, *this, &FRC_2013_Robot::BallConveyorSystem::GripH);
		em->EventOnOff_Map["Ball_Squirt"].Subscribe(ehl, *this, &FRC_2013_Robot::BallConveyorSystem::Squirt);
	}
	else
	{
		em->EventValue_Map["Ball_SetCurrentVelocity"].Remove(*this, &FRC_2013_Robot::BallConveyorSystem::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Ball_Fire"]  .Remove(*this, &FRC_2013_Robot::BallConveyorSystem::Fire);
		em->EventOnOff_Map["Ball_Grip"]  .Remove(*this, &FRC_2013_Robot::BallConveyorSystem::Grip);
		em->EventOnOff_Map["Ball_GripH"]  .Remove(*this, &FRC_2013_Robot::BallConveyorSystem::GripH);
		em->EventOnOff_Map["Ball_Squirt"]  .Remove(*this, &FRC_2013_Robot::BallConveyorSystem::Squirt);
	}
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
		m_PowerWheels(this,robot_control),m_BallConveyorSystem(this,robot_control),
		m_Target(eCenterHighGoal),m_DefensiveKeyPosition(Vec2D(0.0,0.0)),m_UDP_Listener(NULL),
		m_PitchErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),m_AutonPresetIndex(0),
		m_POVSetValve(false),m_IsTargeting(true),m_EnableYawTargeting(false),m_SetClimbGear(false)
{
	m_EnableYawTargeting=true; //for testing until button is implemented (leave on now for servo tests)
	m_UDP_Listener=coodinate_manager_Interface::CreateInstance();
}

FRC_2013_Robot::~FRC_2013_Robot()
{
	coodinate_manager_Interface::DestroyInstance((coodinate_manager_Interface *)m_UDP_Listener);
}

void FRC_2013_Robot::Initialize(Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2013_Robot_Properties *RobotProps=dynamic_cast<const FRC_2013_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)
	m_PitchRamp.Initialize(em,RobotProps?&RobotProps->GetPitchRampProps():NULL);
	m_PowerWheels.Initialize(em,RobotProps?&RobotProps->GetPowerWheelProps():NULL);
	m_BallConveyorSystem.Initialize(em,RobotProps?&RobotProps->GetConveyorProps():NULL);

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
	m_BallConveyorSystem.ResetPos();
}

FRC_2013_Robot::BallConveyorSystem &FRC_2013_Robot::GetBallConveyorSystem()
{
	return m_BallConveyorSystem;
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
	const double c_ViewAngle=43.5;  //Axis M1011 camera
	const double c_HalfViewAngle=c_ViewAngle/2.0;

	//__inline double Get_Ez()
	//{
	//	const double ez=1.0/(tan(DEG_2_RAD(c_ViewAngle)/2.0));
	//	return ez;
	//}

	//doing it this way is faster since it never changes
	const double c_ez=1.0/(tan(DEG_2_RAD(c_ViewAngle)/2.0));

	//For example if the target height is 22.16 feet the distance would be 50, or 10 foot height would be around 22 feet for distance
	//this constant is used to check my pitch math below (typically will be disabled)
	const double c_DistanceCheck=c_TargetBaseHeight*c_ez;

	__inline void GetYawAndDistance(double bx,double by,double &dx,double dy,double &dz)
	{
		dz = (dy * c_ez) / by;
		dx = (bx * dz) / c_ez;
	}

	//This transform is simplified to only works with pitch
	__inline void CameraTransform(double ThetaY,double dx, double dy, double dz, double &ax, double &ay, double &az)
	{
		ax=sin(ThetaY)*dz + cos(ThetaY)*dx;
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
		//If Ypos... is zero no work needs to be done for pitch... also we avoid division by zero too
		//the likelihood of this is rare, but in theory it could make yaw not work for that frame.  
		//Fortunately for us... we'll have error correction because of gravity... so for the game it should be impossible for this to happen except for the rare
		//graze across to get to the final targeting point... point being... the final target point should rest above the target.

		//printf("New coordinates %f , %f\n",listener->GetXpos(),listener->GetYpos());
		const double CurrentPitch=m_RobotControl->GetRotaryCurrentPorV(ePitchRamp);
		double distance,yaw;
		if (VisionConversion::computeDistanceAndYaw(listener->GetXpos(),YOffset,CurrentPitch,yaw,distance))
		{
			 //monitor where it should be against where it actually is
			//printf("p=%.2f a=%.2f\n",m_PitchAngle,CurrentPitch);
			//printf("d=%.2f\n",Meters2Feet(distance));
			//Check math... let's see how the pitch angle measures up to simple offset (it will not factor in the camera transform, but should be close anyhow)
			#undef __DisablePitchDisplay__
			#ifdef __DisablePitchDisplay__
			const double PredictedOffset=tan(CurrentPitch)*VisionConversion::c_DistanceCheck;
			DOUT (4,"p=%.2f y=%.2f test=%.2f error=%.2f",RAD_2_DEG(CurrentPitch),YOffset,PredictedOffset,PredictedOffset-YOffset);
			#endif

			//Now for the final piece... until we actually solve for orientation we'll exclusively just set the ypos to the distance
			//Note: if we were field aware by solving the orientation we could this by placing the final position here, but since we are not (at least for now)
			//we can just adjust for Y and use the POV turning calls for yaw correction
			const Vec2d &Pos_m=GetPos_m();
			SetPosition(Pos_m[0],c_HalfCourtLength-distance);

			if (m_EnableYawTargeting)
			{
				//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
				double value=atan2(-yaw,distance);
				//We set this through the controller so that it goes through the same path and ensures that its in the right mode (just as it is for POV turns)
				m_controller->GetUIController_RW()->Turn_RelativeOffset(value);
			}
		}
		else
			printf("FRC_2013_Robot::TimeChange YOffset=%f\n",YOffset);  //just curious to see how often this would really occur
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
		//m_PitchAngle=atan(sqrt(y2+x2)/x+y/x);
		m_PitchAngle=atan2(y,x);

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
	m_PowerWheels.AsEntity1D().TimeChange(dTime_s);
	m_BallConveyorSystem.TimeChange(dTime_s);
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
	m_SetClimbGear=on;
	SetBypassPosAtt_Update(true);
	m_PitchRamp.SetBypassPos_Update(true);

	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetClimbGear?&m_RobotProps.GetClimbGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);
	m_PitchRamp.SetBypassPos_Update(false);

	m_RobotControl->OpenSolenoid(eUseClimbGear,on);
}

void FRC_2013_Robot::SetClimbGearValue(double Value)
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//printf("\r%f       ",Value);
	if (Value > 0.0)
	{
		if (m_SetClimbGear)
		{
			SetClimbGear(false);
			printf("Now in HighGear\n");
		}
	}
	else
	{
		if (!m_SetClimbGear)
		{
			SetClimbGear(true);
			printf("Now in ClimbGear\n");
		}
	}
}

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
	Framework::Base::EventMap *em=GetEventMap(); 
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
		em->EventOnOff_Map["Robot_Flippers_Solenoid"].Subscribe(ehl,*this, &FRC_2013_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetClimbGear"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGear);
		em->Event_Map["Robot_SetClimbGearOn"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGearOn);
		em->Event_Map["Robot_SetClimbGearOff"].Subscribe(ehl, *this, &FRC_2013_Robot::SetClimbGearOff);
		em->EventValue_Map["Robot_SetClimbGearValue"].Subscribe(ehl,*this, &FRC_2013_Robot::SetClimbGearValue);

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
		em->EventOnOff_Map["Robot_Flippers_Solenoid"]  .Remove(*this, &FRC_2013_Robot::SetFlipperPneumatic);

		em->EventOnOff_Map["Robot_SetClimbGear"]  .Remove(*this, &FRC_2013_Robot::SetClimbGear);
		em->Event_Map["Robot_SetClimbGearOn"]  .Remove(*this, &FRC_2013_Robot::SetClimbGearOn);
		em->Event_Map["Robot_SetClimbGearOff"]  .Remove(*this, &FRC_2013_Robot::SetClimbGearOff);
		em->EventValue_Map["Robot_SetClimbGearValue"].Remove(*this, &FRC_2013_Robot::SetClimbGearValue);

		em->EventValue_Map["Robot_SetPresetPOV"]  .Remove(*this, &FRC_2013_Robot::SetPresetPOV);
		em->EventOnOff_Map["Robot_SetCreepMode"]  .Remove(*this, &FRC_2013_Robot::Robot_SetCreepMode);
	}

	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_PowerWheels.BindAdditionalEventControls(Bind);
	m_BallConveyorSystem.BindAdditionalEventControls(Bind);
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
	m_ConveyorProps(
	"Conveyor",
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
	"Flippers_SetCurrentVelocity","Flippers_SetIntendedPosition","Flippers_SetPotentiometerSafety",
	"Flippers_Advance","Flippers_Retract",
	"Robot_IsTargeting","Robot_SetTargetingOn","Robot_SetTargetingOff","Robot_TurretSetTargetingOff","Robot_SetTargetingValue",
	"Robot_SetClimbGear","Robot_SetClimbGearOn","Robot_SetClimbGearOff","Robot_SetClimbGearValue",
	"Robot_SetPreset1","Robot_SetPreset2","Robot_SetPreset3","Robot_SetPresetPOV",
	"Robot_SetDefensiveKeyValue","Robot_SetDefensiveKeyOn","Robot_SetDefensiveKeyOff",
	"Robot_SetCreepMode","Robot_Flippers_Solenoid"
	//AI Tester events only
#if 1
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
		err = script.GetFieldTable("conveyor");
		if (!err)
		{
			m_ConveyorProps.LoadFromScript(script);
			script.Pop();
		}

		m_ClimbGearProps=*this;  //copy redundant data first
		err = script.GetFieldTable("climb_gear");
		if (!err)
		{
			m_ClimbGearProps.LoadFromScript(script);
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
		m_Robot.GetBallConveyorSystem().Fire(m_IsOn);
	else
		m_Robot.GetBallConveyorSystem().Squirt(m_IsOn);
		
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
	const double PowerWheelSpeedDifference=m_Robot.GetPowerWheels().GetRequestedVelocity_Difference();
	//when we encounter the tolerance dip in speed we are done
	if (fabs(PowerWheelSpeedDifference)>m_Tolerance)
	{
		printf("Ball Deployed\n");
		m_Status=eCompleted;
	}
	return m_Status;
}

  /***********************************************************************************************************************************/
 /*													FRC_2013_Goals::OperateSolenoid													*/
/***********************************************************************************************************************************/

FRC_2013_Goals::OperateSolenoid::OperateSolenoid(FRC_2013_Robot &robot,FRC_2013_Robot::SolenoidDevices SolenoidDevice,bool Open) : m_Robot(robot),
m_SolenoidDevice(SolenoidDevice),m_Terminate(false),m_IsOpen(Open) 
{	
	m_Status=eInactive;
}

FRC_2013_Goals::OperateSolenoid::Goal_Status FRC_2013_Goals::OperateSolenoid::Process(double dTime_s)
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
		case FRC_2013_Robot::eFlipperDown:
			m_Robot.SetFlipperPneumatic(m_IsOpen);
			break;
		case FRC_2013_Robot::eUseClimbGear:
			assert(false);
			break;
	}
	m_Status=eCompleted;
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
	OperateSolenoid *DeployFlipper=NULL;
	Fire *EndSomeFire_On=NULL;
	Goal_Wait *goal_waitEndFire=NULL;
	Fire *EndSomeFire_Off=NULL;
	if (RampIndex != (size_t)-1)
	{
		DeployFlipper=new OperateSolenoid(*Robot,FRC_2013_Robot::eFlipperDown,true);
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
		MainGoal->AddSubgoal(DeployFlipper);
	}
	MainGoal->AddSubgoal(FireOff);
	MainGoal->AddSubgoal(goal_waitforballs);
	if (goal_drive_foward)
		MainGoal->AddSubgoal(goal_drive_foward);
	MainGoal->AddSubgoal(FireOn);
	MainGoal->Activate();
	return MainGoal;
}