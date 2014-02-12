#include "stdafx.h"
#include "Robot_Tester.h"

#ifdef Robot_TesterCode
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

#else
#include "Common/Debug.h"
#include "FRC2014_Robot.h"
#include "SmartDashboard/SmartDashboard.h"
using namespace Framework::Base;
using namespace std;
#endif


#define __DisableEncoderTracking__
//Enable this to send remote coordinate to network variables to manipulate a shape for tracking
#define __EnableShapeTrackingSimulation__
#if 0
#define UnitMeasure2UI Meters2Feet
#define UI2UnitMeasure Feet2Meters
#else
#define UnitMeasure2UI Meters2Inches
#define UI2UnitMeasure Inches2Meters
#endif

#if 0
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
#endif

  /***********************************************************************************************************************************/
 /*														FRC_2014_Robot::Turret														*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Turret::Turret(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 	m_pParent(parent),m_Velocity(0.0)
{
}


void FRC_2014_Robot::Turret::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["Turret_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Turret::Turret_SetRequestedVelocity);
	}
}

void FRC_2014_Robot::Turret::TimeChange(double dTime_s)
{
	m_Velocity=0.0;

	//Note: GetRequestedVelocity tests for manual override, giving the tolerance a bit more grace for joystick dead-zone
	const bool IsTargeting=m_pParent->IsBallTargeting();
	//const bool IsTargeting=false;
	if (IsTargeting)
	{
			//if (m_AutoDriveState==eAutoDrive_YawOnly)
			{
				//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
				//if (fabs(m_YawAngle)>m_RobotProps.GetFRC2014RobotProps().YawTolerance)
					m_pParent->m_controller->GetUIController_RW()->Turn_RelativeOffset(m_pParent->m_YawAngle,true);
			}
			//else if (m_AutoDriveState==eAutoDrive_FullAuto)
			//{
			//	bool HitWayPoint;
			//	{
			//		const double tolerance=GetRobotProps().GetTankRobotProps().PrecisionTolerance;
			//		const Vec2d &currPos = GetPos_m();
			//		double position_delta=(m_TargetOffset-currPos).length();
			//		HitWayPoint=position_delta<tolerance;
			//	}
			//	if (!HitWayPoint)
			//	{
			//		Vec2d Temp(0,0);
			//		GetController()->DriveToLocation(m_TargetOffset, m_TargetOffset, 1.0, dTime_s,&Temp);
			//	}
			//	else
			//		GetController()->SetShipVelocity(0.0);
			//}

	}
}

void FRC_2014_Robot::Turret::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::PitchRamp														*/
/***********************************************************************************************************************************/
FRC_2014_Robot::PitchRamp::PitchRamp(FRC_2014_Robot *pParent,Rotary_Control_Interface *robot_control) : m_pParent(pParent),m_Velocity(0.0)
{
}


void FRC_2014_Robot::PitchRamp::TimeChange(double dTime_s)
{
	m_Velocity=0.0;
}

void FRC_2014_Robot::PitchRamp::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
	else
	{
		em->EventValue_Map["PitchRamp_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::PitchRamp::Pitch_SetRequestedVelocity);
	}
}

void FRC_2014_Robot::PitchRamp::ResetPos()
{
	m_Velocity=0.0;
}

  /***********************************************************************************************************************************/
 /*														FRC_2014_Robot::Winch														*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Winch::Winch(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("Winch",robot_control,eWinch),m_pParent(parent),m_Advance(false)
{
}


void FRC_2014_Robot::Winch::Advance(bool on)
{
	m_Advance=on;
}

void FRC_2014_Robot::Winch::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	//const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button value
	if (m_Advance)
		SetCurrentLinearAcceleration(Accel);

	__super::TimeChange(dTime_s);
	//Trying to get away from debug outs... however keeping this around for reference to how the gear ratios are used
	//#ifdef Robot_TesterCode
	//const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	//const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
	//double Pos_m=GetPos_m();
	//DOUT4("Arm=%f Angle=%f",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio));
	//#endif

	//const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	//const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
	//SmartDashboard::PutNumber("Catapult_Angle2",RAD_2_DEG(GetPos_m()*c_GearToArmRatio));
}


double FRC_2014_Robot::Winch::PotentiometerRaw_To_Arm_r(double raw) const
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.Catapult_Robot_Props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

void FRC_2014_Robot::Winch::SetChipShot()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Catapult_Robot_Props.ChipShotAngle * props.Catapult_Robot_Props.ArmToGearRatio);
}
void FRC_2014_Robot::Winch::SetGoalShot()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition( props.Catapult_Robot_Props.GoalShotAngle * props.Catapult_Robot_Props.ArmToGearRatio);
}
void FRC_2014_Robot::Winch::Fire_Catapult(bool ReleaseClutch)
{
	m_pParent->m_RobotControl->OpenSolenoid(eReleaseClutch,ReleaseClutch);
	//once released the encoder and position will be zero
	if (ReleaseClutch)
	{
		ResetPos();
		m_pParent->m_RobotControl->Reset_Rotary(eWinch);
	}
}

void FRC_2014_Robot::Winch::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=m_pParent->GetEventMap();
	if (Bind)
	{
		em->EventValue_Map["Winch_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Winch_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::SetPotentiometerSafety);
		
		em->Event_Map["Winch_SetChipShot"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::SetChipShot);
		em->Event_Map["Winch_SetGoalShot"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::SetGoalShot);

		em->EventOnOff_Map["Winch_Advance"].Subscribe(ehl,*this, &FRC_2014_Robot::Winch::Advance);

		em->EventOnOff_Map["Winch_Fire"].Subscribe(ehl, *this, &FRC_2014_Robot::Winch::Fire_Catapult);
	}
	else
	{
		em->EventValue_Map["Winch_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Winch::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["Winch_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::Winch::SetPotentiometerSafety);

		em->Event_Map["Winch_SetChipShot"].Remove(*this, &FRC_2014_Robot::Winch::SetChipShot);
		em->Event_Map["Winch_SetGoalShot"].Remove(*this, &FRC_2014_Robot::Winch::SetGoalShot);

		em->EventOnOff_Map["Winch_Advance"].Remove(*this, &FRC_2014_Robot::Winch::Advance);

		em->EventOnOff_Map["Winch_Fire"]  .Remove(*this, &FRC_2014_Robot::Winch::Fire_Catapult);
	}
}
  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot::Intake_Arm														*/
/***********************************************************************************************************************************/

FRC_2014_Robot::Intake_Arm::Intake_Arm(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control) : 
	Rotary_Position_Control("IntakeArm",robot_control,eIntake_Arm),m_pParent(parent),m_Advance(false),m_Retract(false)
{
}


void FRC_2014_Robot::Intake_Arm::Advance(bool on)
{
	m_Advance=on;
}
void FRC_2014_Robot::Intake_Arm::Retract(bool on)
{
	m_Retract=on;
}

void FRC_2014_Robot::Intake_Arm::TimeChange(double dTime_s)
{
	const double Accel=m_Ship_1D_Props.ACCEL;
	const double Brake=m_Ship_1D_Props.BRAKE;

	//Get in my button values now use xor to only set if one or the other is true (not setting automatically zero's out)
	if (m_Advance ^ m_Retract)
		SetCurrentLinearAcceleration(m_Advance?Accel:-Brake);

	__super::TimeChange(dTime_s);
	//Since we have no potentiometer we can feedback where we think the arm angle is from the entity
	SmartDashboard::PutNumber("IntakeArm_Angle",RAD_2_DEG(GetPos_m()));
}


double FRC_2014_Robot::Intake_Arm::PotentiometerRaw_To_Arm_r(double raw) const
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=props.Intake_Robot_Props.PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}

void FRC_2014_Robot::Intake_Arm::SetStowed()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Stowed_Angle);
}

void FRC_2014_Robot::Intake_Arm::SetDeployed()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Deployed_Angle);
}

void FRC_2014_Robot::Intake_Arm::SetSquirt()
{
	const FRC_2014_Robot_Props &props=m_pParent->GetRobotProps().GetFRC2014RobotProps();
	SetIntendedPosition(props.Intake_Robot_Props.Squirt_Angle);
}


void FRC_2014_Robot::Intake_Arm::BindAdditionalEventControls(bool Bind)
{
	Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["IntakeArm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["IntakeArm_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::SetPotentiometerSafety);
		
		em->Event_Map["IntakeArm_SetStowed"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetStowed);
		em->Event_Map["IntakeArm_SetDeployed"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetDeployed);
		em->Event_Map["IntakeArm_SetSquirt"].Subscribe(ehl, *this, &FRC_2014_Robot::Intake_Arm::SetSquirt);

		em->EventOnOff_Map["IntakeArm_Advance"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::Advance);
		em->EventOnOff_Map["IntakeArm_Retract"].Subscribe(ehl,*this, &FRC_2014_Robot::Intake_Arm::Retract);
	}
	else
	{
		em->EventValue_Map["IntakeArm_SetCurrentVelocity"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetRequestedVelocity_FromNormalized);
		em->EventOnOff_Map["IntakeArm_SetPotentiometerSafety"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetPotentiometerSafety);

		em->Event_Map["IntakeArm_SetStowed"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetStowed);
		em->Event_Map["IntakeArm_SetDeployed"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetDeployed);
		em->Event_Map["IntakeArm_SetSquirt"].Remove(*this, &FRC_2014_Robot::Intake_Arm::SetSquirt);

		em->EventOnOff_Map["IntakeArm_Advance"].Remove(*this, &FRC_2014_Robot::Intake_Arm::Advance);
		em->EventOnOff_Map["IntakeArm_Retract"].Remove(*this, &FRC_2014_Robot::Intake_Arm::Retract);
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
	Tank_Robot(EntityName,robot_control,IsAutonomous), m_RobotControl(robot_control), 
		m_Turret(this,robot_control),m_PitchRamp(this,robot_control),m_Winch(this,robot_control),m_Intake_Arm(this,robot_control),
		m_DefensiveKeyPosition(Vec2D(0.0,0.0)),
		m_YawErrorCorrection(1.0),m_PowerErrorCorrection(1.0),m_DefensiveKeyNormalizedDistance(0.0),m_DefaultPresetIndex(0),
		m_AutonPresetIndex(0),m_YawAngle(0.0),
		m_DisableTurretTargetingValue(false),m_POVSetValve(false),m_SetLowGear(false),m_SetDriverOverride(false),
		m_IsBallTargeting(false)
{
	//ensure the variables are initialized before calling get
	SmartDashboard::PutNumber("X Position",0.0);
	SmartDashboard::PutNumber("Y Position",0.0);
	//Note: The processing vision is setup to use these same variables for both tracking processes (i.e. front and rear camera) we should only need to be tracking one of them at a time
	//We may want to add a prefix window to identify which window they are coming from, but this may not be necessary.
}

void FRC_2014_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	m_RobotControl->Initialize(props);

	const FRC_2014_Robot_Properties *RobotProps=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	m_RobotProps=*RobotProps;  //Copy all the properties (we'll need them for high and low gearing)

	//set to the default key position
	//const FRC_2014_Robot_Props &robot2014props=RobotProps->GetFRC2014RobotProps();
	m_Winch.Initialize(em,RobotProps?&RobotProps->GetWinchProps():NULL);
	m_Intake_Arm.Initialize(em,RobotProps?&RobotProps->GetIntake_ArmProps():NULL);
}
void FRC_2014_Robot::ResetPos()
{
	__super::ResetPos();
	m_Turret.ResetPos();
	m_PitchRamp.ResetPos();
	//TODO this is tacky... will have better low gear method soon
	if (!GetBypassPosAtt_Update())
	{
		m_Winch.ResetPos();
		m_Intake_Arm.ResetPos();
	}
}

namespace VisionConversion
{
	const double c_TargetBaseHeight=Feet2Meters(2.0);
	//Note: for this camera we use square pixels, so we need not worry about pixel aspect ratio
	const double c_X_Image_Res=640.0;		//X Image resolution in pixels, should be 160, 320 or 640
	const double c_Y_Image_Res=480.0;
	const double c_AspectRatio=c_X_Image_Res/c_Y_Image_Res;
	const double c_AspectRatio_recip=c_Y_Image_Res/c_X_Image_Res;
	//const double c_ViewAngle=43.5;  //Axis M1011 camera (in code sample)
	const double c_ViewAngle_x=DEG_2_RAD(45);		//These are the angles I've measured
	const double c_ViewAngle_y=DEG_2_RAD(45);
	const double c_HalfViewAngle_y=c_ViewAngle_y/2.0;

	//doing it this way is faster since it never changes
	const double c_ez_y=(tan(c_ViewAngle_y/2.0));
	const double c_ez_y_recip=1.0/c_ez_y;
	const double c_ez_x=(tan(c_ViewAngle_x/2.0));
	const double c_ez_x_recip=1.0/c_ez_x;

	//For example if the target height is 22.16 feet the distance would be 50, or 10 foot height would be around 22 feet for distance
	//this constant is used to check my pitch math below (typically will be disabled)
	const double c_DistanceCheck=c_TargetBaseHeight*c_ez_y_recip;

	__inline void GetYawAndDistance(double bx,double by,double &dx,double dy,double &dz)
	{
		//Note: the camera angle for x is different than for y... thus for example we have 4:3 aspect ratio
		dz = (dy * c_ez_y_recip) / by;
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

void FRC_2014_Robot::TimeChange(double dTime_s)
{
	//const FRC_2014_Robot_Props &robot_props=m_RobotProps.GetFRC2014RobotProps();

	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	m_Turret.TimeChange(dTime_s);
	m_PitchRamp.TimeChange(dTime_s);
	m_Winch.AsEntity1D().TimeChange(dTime_s);
	m_Intake_Arm.AsEntity1D().TimeChange(dTime_s);

	#ifdef __EnableShapeTrackingSimulation__
	{
		const char * const csz_remote_name="land_reticle";
		Vec2D TargetPos(0.0,0.0);
		Vec2D GlobalPos=TargetPos-GetPos_m();
		Vec2D LocalPos=GlobalToLocal(GetAtt_r(),GlobalPos);
		std::string sBuild=csz_remote_name;
		sBuild+="_x";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(LocalPos[0]));
		sBuild=csz_remote_name;
		sBuild+="_z";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(LocalPos[1]));
		sBuild=csz_remote_name;
		sBuild+="_y";
		SmartDashboard::PutNumber(sBuild,UnitMeasure2UI(0.3048));  //always 1 foot high from center point
	}
	#endif

	//const double  YOffset=-SmartDashboard::GetNumber("Y Position");
	const double XOffset=SmartDashboard::GetNumber("X Position");
	
	using namespace VisionConversion;

	SmartDashboard::PutBoolean("IsBallTargeting",IsBallTargeting());
	if (IsBallTargeting())
	{
		const double CurrentYaw=GetAtt_r();
		//the POV turning call relative offsets adjustments here... the yaw is the opposite side so we apply the negative sign
		#ifndef __UseFileTargetTracking__
		const double SmoothingYaw=0.20; //hard coded for now
		//const double NewYaw=CurrentYaw+atan(yaw/distance);
		const double NewYaw=CurrentYaw+(atan(XOffset * c_AspectRatio_recip * c_ez_x)*SmoothingYaw);
		#else
		//Enable this for playback of file since it cannot really cannot control the pitch
		//const double NewYaw=atan(yaw/distance)-GetAtt_r();
		const double NewYaw=atan(XOffset * c_AspectRatio_recip * c_ez_x)-GetAtt_r();
		#endif

		//Use precision tolerance asset to determine whether to make the change
		//m_YawAngle=(fabs(NewYaw-CurrentYaw)>m_RobotProps.GetTurretProps().GetRotaryProps().PrecisionTolerance)?NewYaw:CurrentYaw;
		const double PrecisionTolerance=DEG_2_RAD(0.5); //TODO put in properties try to keep as low as possible if we need to drive straight
		m_YawAngle=NormalizeRotation2((fabs(NewYaw-CurrentYaw)>PrecisionTolerance)?NewYaw:CurrentYaw);
		//Note: limits will be solved at ship level
		SmartDashboard::PutNumber("Ball Tracking Yaw Angle",RAD_2_DEG(m_YawAngle));
	}

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
	//m_Turret.SetBypassPos_Update(true);
	//m_PitchRamp.SetBypassPos_Update(true);

	//Now for some real magic with the properties!
	__super::Initialize(*GetEventMap(),m_SetLowGear?&m_RobotProps.GetLowGearProps():&m_RobotProps);
	SetBypassPosAtt_Update(false);
	//m_Turret.SetBypassPos_Update(false);
	//m_PitchRamp.SetBypassPos_Update(false);

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

void FRC_2014_Robot::SetDriverOverride(bool on) 
{
	if (m_IsAutonomous) return;  //We don't want to read joystick settings during autonomous
	//I am not yet certain if this if statement is necessary... I'll have to check what all is involved in setting a variable that is already equal
	if (m_SetDriverOverride!=on)
		SmartDashboard::PutBoolean("DriverOverride",on);
	m_SetDriverOverride=on;
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
		em->EventOnOff_Map["Robot_SetDriverOverride"].Subscribe(ehl, *this, &FRC_2014_Robot::SetDriverOverride);
		em->EventOnOff_Map["Robot_BallTargeting"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargeting);
		em->Event_Map["Robot_BallTargeting_On"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargetingOn);
		em->Event_Map["Robot_BallTargeting_Off"].Subscribe(ehl, *this, &FRC_2014_Robot::SetBallTargetingOff);
	}
	else
	{
		em->EventOnOff_Map["Robot_SetLowGear"]  .Remove(*this, &FRC_2014_Robot::SetLowGear);
		em->Event_Map["Robot_SetLowGearOn"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOn);
		em->Event_Map["Robot_SetLowGearOff"]  .Remove(*this, &FRC_2014_Robot::SetLowGearOff);
		em->EventValue_Map["Robot_SetLowGearValue"].Remove(*this, &FRC_2014_Robot::SetLowGearValue);
		em->EventOnOff_Map["Robot_SetDriverOverride"]  .Remove(*this, &FRC_2014_Robot::SetDriverOverride);
		em->EventOnOff_Map["Robot_BallTargeting"]  .Remove(*this, &FRC_2014_Robot::SetBallTargeting);
		em->Event_Map["Robot_BallTargeting_On"]  .Remove(*this, &FRC_2014_Robot::SetBallTargetingOn);
		em->Event_Map["Robot_BallTargeting_Off"]  .Remove(*this, &FRC_2014_Robot::SetBallTargetingOff);
	}

	m_Turret.BindAdditionalEventControls(Bind);
	m_PitchRamp.BindAdditionalEventControls(Bind);
	m_Winch.AsShip1D().BindAdditionalEventControls(Bind);
	m_Intake_Arm.AsShip1D().BindAdditionalEventControls(Bind);
	#ifdef Robot_TesterCode
	m_RobotControl->BindAdditionalEventControls(Bind,GetEventMap(),ehl);
	#endif
	__super::BindAdditionalEventControls(Bind);
}

void FRC_2014_Robot::BindAdditionalUIControls(bool Bind,void *joy, void *key)
{
	m_RobotProps.Get_RobotControls().BindAdditionalUIControls(Bind,joy,key);
	__super::BindAdditionalUIControls(Bind,joy,key);  //call super for more general control assignments
}

void FRC_2014_Robot::UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s)
{
	//Call predecessor (e.g. tank steering) to get some preliminary values
	__super::UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,LockShipHeadingToOrientation,dTime_s);
	if (!m_SetDriverOverride)
	{
		//Note: for now we'll just add the values in... we may wish to consider analyzing the existing direction and use the max, but this would require the joystick
		//values from UI, for now I don't wish to add that complexity as I feel a simple add will suffice
		//Now to add turret and pitch settings
		const double TurretAcceleration=m_Turret.GetCurrentVelocity()*GetHeadingSpeed();
		AngularAcceleration+=TurretAcceleration;
		const double PitchVelocity=m_PitchRamp.GetCurrentVelocity()*GetEngaged_Max_Speed();
		AuxVelocity+=PitchVelocity;
	}
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
		const double c_ArmToGearRatio=72.0/28.0;
		const double c_PotentiometerToArmRatio=36.0/54.0;

		FRC_2014_Robot_Props props;
		//const double KeyDistance=Inches2Meters(144);
		//const double KeyWidth=Inches2Meters(101);
		//const double KeyDepth=Inches2Meters(48);   //not used (yet)
		//const double DefaultY=c_HalfCourtLength-KeyDistance;
		//const double HalfKeyWidth=KeyWidth/2.0;

		props.Catapult_Robot_Props.ArmToGearRatio=c_ArmToGearRatio;
		props.Catapult_Robot_Props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
		//The winch is set up to force the numbers to go up from 0 - 90 where 0 is pointing up
		//This allows gain assist to apply max voltage to its descent
		props.Catapult_Robot_Props.ChipShotAngle=DEG_2_RAD(45.0);
		props.Catapult_Robot_Props.GoalShotAngle=DEG_2_RAD(90.0);

		props.Intake_Robot_Props.ArmToGearRatio=c_ArmToGearRatio;
		props.Intake_Robot_Props.PotentiometerToArmRatio=c_PotentiometerToArmRatio;
		//The intake uses a starting point of 90 to force numbers down from 90 - 0 where zero is pointing straight out
		//This allows the gain assist to apply max force when it goes from deployed to stowed
		props.Intake_Robot_Props.Stowed_Angle=DEG_2_RAD(90.0);
		props.Intake_Robot_Props.Deployed_Angle=DEG_2_RAD(61.0);
		props.Intake_Robot_Props.Squirt_Angle=DEG_2_RAD(90.0);

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
	"Robot_SetLowGear","Robot_SetLowGearOn","Robot_SetLowGearOff","Robot_SetLowGearValue",
	"Robot_SetDriverOverride",
	"Winch_SetChipShot","Winch_SetGoalShot","Winch_SetCurrentVelocity","Winch_Fire","Winch_Advance",
	"IntakeArm_SetCurrentVelocity","IntakeArm_SetStowed","IntakeArm_SetDeployed","IntakeArm_SetSquirt","IntakeArm_Advance","IntakeArm_Retract",
	"Robot_BallTargeting","Robot_BallTargeting_On","Robot_BallTargeting_Off"
};

const char *FRC_2014_Robot_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_FRC_2014_Controls_Events))?g_FRC_2014_Controls_Events[index] : NULL;
}
FRC_2014_Robot_Properties::ControlEvents FRC_2014_Robot_Properties::s_ControlsEvents;

void FRC_2014_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	FRC_2014_Robot_Props &props=m_FRC2014RobotProps;

	const char* err=NULL;
	{
		double version;
		err=script.GetField("version", NULL, NULL, &version);
		if (!err)
			printf ("Version=%.2f\n",version);
	}

	m_ControlAssignmentProps.LoadFromScript(script);
	__super::LoadFromScript(script);
	err = script.GetFieldTable("robot_settings");
	if (!err) 
	{
		err = script.GetFieldTable("catapult");
		if (!err)
		{
			FRC_2014_Robot_Props::Catapult &cat_props=props.Catapult_Robot_Props;
			double fTest;
			err=script.GetField("arm_to_motor", NULL, NULL, &fTest);
			if (!err)
				cat_props.ArmToGearRatio=fTest;
			err=script.GetField("pot_to_arm", NULL, NULL, &fTest);
			if (!err)
				cat_props.PotentiometerToArmRatio=fTest;
			err=script.GetField("chipshot_angle_deg", NULL, NULL, &fTest);
			if (!err)
				cat_props.ChipShotAngle=DEG_2_RAD(fTest);
			err=script.GetField("goalshot_angle_deg", NULL, NULL, &fTest);
			if (!err)
				cat_props.GoalShotAngle=DEG_2_RAD(fTest);
			script.Pop();
		}
		err = script.GetFieldTable("intake");
		if (!err)
		{
			FRC_2014_Robot_Props::Intake &intake_props=props.Intake_Robot_Props;
			double fTest;
			err=script.GetField("arm_to_motor", NULL, NULL, &fTest);
			if (!err)
				intake_props.ArmToGearRatio=fTest;
			err=script.GetField("pot_to_arm", NULL, NULL, &fTest);
			if (!err)
				intake_props.PotentiometerToArmRatio=fTest;
			err=script.GetField("stowed_angle_deg", NULL, NULL, &fTest);
			if (!err)
				intake_props.Stowed_Angle=DEG_2_RAD(fTest);
			err=script.GetField("deployed_angle", NULL, NULL, &fTest);
			if (!err)
				intake_props.Deployed_Angle=DEG_2_RAD(fTest);
			err=script.GetField("squirt_angle", NULL, NULL, &fTest);
			if (!err)
				intake_props.Squirt_Angle=DEG_2_RAD(fTest);
			script.Pop();
		}

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
		err = script.GetFieldTable("winch");
		if (!err)
		{
			m_WinchProps.LoadFromScript(script);
			script.Pop();
		}
		err = script.GetFieldTable("intake_arm");
		if (!err)
		{
			m_Intake_ArmProps.LoadFromScript(script);
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
		case FRC_2014_Robot::eReleaseClutch:
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

#if defined Robot_TesterCode && !defined __TestControlAssignments__
  /***********************************************************************************************************************************/
 /*													FRC_2014_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
		case FRC_2014_Robot::eWinch:
			{
				//	printf("Turret=%f\n",Voltage);
				//DOUT3("Turret Voltage=%f",Voltage);
				const double VoltageToUse=(Voltage>0.0)?Voltage:0.0;
				m_WinchVoltage=VoltageToUse * m_RobotProps.GetWinchProps().GetRotaryProps().VoltageScalar;
				//if (m_WinchVoltage>0.08)
				//	printf("%f\n",m_WinchVoltage);
				m_Winch_Pot.UpdatePotentiometerVoltage(VoltageToUse);
				m_Winch_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
		case FRC_2014_Robot::eIntake_Arm:
			{
				//	printf("Pitch=%f\n",Voltage);
				//DOUT3("Pitch Voltage=%f",Voltage);
				m_IntakeArmVoltage=Voltage * m_RobotProps.GetIntake_ArmProps().GetRotaryProps().VoltageScalar;
				m_IntakeArm_Pot.UpdatePotentiometerVoltage(Voltage);
				m_IntakeArm_Pot.TimeChange();  //have this velocity immediately take effect
			}
			break;
	}
}

FRC_2014_Robot_Control::FRC_2014_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_WinchVoltage(0.0),m_IntakeArmVoltage(0.0),m_PowerWheelVoltage(0.0)
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
		case FRC_2014_Robot::eWinch:
			m_Winch_Pot.ResetPos();
			break;
		case FRC_2014_Robot::eIntake_Arm:
			m_IntakeArm_Pot.ResetPos();
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
		m_Winch_Pot.Initialize(&turret_props);
		m_IntakeArm_Pot.Initialize(&robot_props->GetPitchRampProps());
	}
}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Winch_Pot.SetTimeDelta(dTime_s);
	m_IntakeArm_Pot.SetTimeDelta(dTime_s);
	m_Flippers_Pot.SetTimeDelta(dTime_s);
	m_PowerWheel_Enc.SetTimeDelta(dTime_s);
	m_LowerConveyor_Enc.SetTimeDelta(dTime_s);
	m_MiddleConveyor_Enc.SetTimeDelta(dTime_s);
	m_FireConveyor_Enc.SetTimeDelta(dTime_s);

	//Let's do away with this since we are using the smart dashboard
	////display voltages
	//DOUT(2,"l=%.2f r=%.2f t=%.2f pi=%.2f pw=%.2f lc=%.2f mc=%.2f fc=%.2f\n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage(),
	//	m_WinchVoltage,m_IntakeArmVoltage,m_PowerWheelVoltage,m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage);

	SmartDashboard::PutNumber("WinchVoltage",m_WinchVoltage);
	SmartDashboard::PutNumber("IntakeArmVoltage",m_IntakeArmVoltage);
}


double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const FRC_2014_Robot_Props &props=m_RobotProps.GetFRC2014RobotProps();

	switch (index)
	{
		case FRC_2014_Robot::eWinch:
			{
				const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
				//result=(m_Potentiometer.GetDistance() * m_RobotProps.GetArmProps().GetRotaryProps().EncoderToRS_Ratio) + 0.0;
				//no conversion needed in simulation
				result=(m_Winch_Pot.GetPotentiometerCurrentPosition()) + 0.0;

				//result = m_KalFilter_Arm(result);  //apply the Kalman filter
				SmartDashboard::PutNumber("Catapult_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
				//SmartDashboard::PutNumber("Catapult_Angle",RAD_2_DEG(result*c_GearToArmRatio));
			}
			break;
		case FRC_2014_Robot::eIntake_Arm:
			{
				assert(false);  //no potentiometer 
				const double c_GearToArmRatio=1.0/props.Intake_Robot_Props.ArmToGearRatio;
				result=m_IntakeArm_Pot.GetPotentiometerCurrentPosition();
				SmartDashboard::PutNumber("IntakeArm_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
			}
			break;
	}

	//Let's do away with this and use smart dashboard instead... enable if team doesn't want to use it
	//#ifdef __DebugLUA__
	//switch (index)
	//{
	//	case FRC_2014_Robot::eWinch:
	//		Dout(m_RobotProps.GetTurretProps().GetRotaryProps().Feedback_DiplayRow,14,"d=%.1f",RAD_2_DEG(result));
	//		break;
	//	case FRC_2014_Robot::eIntake_Arm:
	//		Dout(m_RobotProps.GetPitchRampProps().GetRotaryProps().Feedback_DiplayRow,14,"p=%.1f",RAD_2_DEG(result));
	//		break;
	//}
	//#endif

	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2014_Robot::eUseLowGear:
		printf("UseLowGear=%d\n",Open);
		SmartDashboard::PutBoolean("UseHighGear",!Open);
		break;
	case FRC_2014_Robot::eReleaseClutch:
		printf("ReleaseClutch=%d\n",Open);
		SmartDashboard::PutBoolean("ClutchEngaged",!Open);
		break;
	}
}

#else

void FRC_2014_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//This will not be in the wind river... this adds stress to simulate stall on low values
	if ((fabs(Voltage)<0.01) && (Voltage!=0)) Voltage=0.0;

	switch (index)
	{
	case FRC_2014_Robot::eWinch:
		{
			double VoltageToUse=(Voltage>0.0)?Voltage:0.0;
			m_WinchVoltage=VoltageToUse=VoltageToUse * m_RobotProps.GetWinchProps().GetRotaryProps().VoltageScalar;
			Victor_UpdateVoltage(index,VoltageToUse);
			SmartDashboard::PutNumber("WinchVoltage",VoltageToUse);
		}
		break;
	case FRC_2014_Robot::eIntake_Arm:
		{
			Voltage=Voltage * m_RobotProps.GetIntake_ArmProps().GetRotaryProps().VoltageScalar;
			Victor_UpdateVoltage(index,Voltage);
			SmartDashboard::PutNumber("IntakeArmVoltage",Voltage);
		}
		break;
	}
}

FRC_2014_Robot_Control::FRC_2014_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl),m_WinchVoltage(0.0)
{
}

FRC_2014_Robot_Control::~FRC_2014_Robot_Control()
{
	Encoder_Stop(FRC_2014_Robot::eWinch);
}

void FRC_2014_Robot_Control::Reset_Rotary(size_t index)
{
	Encoder_Reset(index);  //This will check for encoder existence implicitly
}

#ifdef __TestControlAssignments__
void FRC_2014_Robot_Control::BindAdditionalEventControls(bool Bind,Base::EventMap *em,IEvent::HandlerList &ehl)
{
}
#endif

void FRC_2014_Robot_Control::Initialize(const Entity_Properties *props)
{
	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);

	const FRC_2014_Robot_Properties *robot_props=dynamic_cast<const FRC_2014_Robot_Properties *>(props);
	assert(robot_props);
	
	m_RobotProps=*robot_props;  //save a copy

	Rotary_Properties turret_props=robot_props->GetTurretProps();
	turret_props.SetUsingRange(false); //TODO why is this here?

	RobotControlCommon_Initialize(robot_props->Get_ControlAssignmentProps());

	//Note: RobotControlCommon_Initialize() must occur before calling any encoder startup code
	const double EncoderPulseRate=(1.0/360.0);
	Encoder_SetDistancePerPulse(FRC_2014_Robot::eWinch,EncoderPulseRate);
	Encoder_Start(FRC_2014_Robot::eWinch);
}

void FRC_2014_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	#ifdef __TestControlAssignments__
	const Rotary_Props &rotary=m_RobotProps.GetWinchProps().GetRotaryProps();
	const double adjustment= m_WinchVoltage*m_RobotProps.GetWinchProps().GetMaxSpeed() * dTime_s * (1.0/rotary.EncoderToRS_Ratio);
	Encoder_TimeChange(FRC_2014_Robot::eWinch,adjustment);
	#endif
}


double FRC_2014_Robot_Control::GetRotaryCurrentPorV(size_t index)
{
	double result=0.0;
	const FRC_2014_Robot_Props &props=m_RobotProps.GetFRC2014RobotProps();

	switch (index)
	{
	case FRC_2014_Robot::eWinch:
		{
			const double c_GearToArmRatio=1.0/props.Catapult_Robot_Props.ArmToGearRatio;
			const double distance=Encoder_GetDistance(index);
			result=(distance * m_RobotProps.GetWinchProps().GetRotaryProps().EncoderToRS_Ratio) + 0.0;

			//result = m_KalFilter_Arm(result);  //apply the Kalman filter
			SmartDashboard::PutNumber("Catapult_Angle",90-RAD_2_DEG(result*c_GearToArmRatio));
			//SmartDashboard::PutNumber("Catapult_Angle",RAD_2_DEG(result));
		}
		break;
	case FRC_2014_Robot::eIntake_Arm:
		assert(false);  //no potentiometer 
		break;
	}

	return result;
}

void FRC_2014_Robot_Control::OpenSolenoid(size_t index,bool Open)
{
	switch (index)
	{
	case FRC_2014_Robot::eUseLowGear:
		//printf("UseLowGear=%d\n",Open);
		SmartDashboard::PutBoolean("UseHighGear",!Open);
		Solenoid_Open(index,Open);
		break;
	case FRC_2014_Robot::eReleaseClutch:
		//printf("ReleaseClutch=%d\n",Open);
		SmartDashboard::PutBoolean("ClutchEngaged",!Open);
		Solenoid_Open(index,Open);
		break;
	}
}

#endif

#ifdef Robot_TesterCode
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
