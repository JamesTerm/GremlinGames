#define __DisablePotentiometerCalibration__
const bool c_UsingArmLimits=false;

#include "Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "Base/Vec2d.h"
#include "Base/Misc.h"
#include "Base/Event.h"
#include "Base/EventMap.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship_1D.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Robot_Tank.h"
#include "FRC2011_Robot.h"

using namespace Framework::Base;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
const double c_ArmLength_m=1.8288;  //6 feet
const double c_ArmToGearRatio=72.0/28.0;
const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
const double c_PotentiometerToGearRatio=60.0/32.0;
const double c_PotentiometerToArm=c_PotentiometerToGearRatio * c_GearToArmRatio;
const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;
  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Ship_1D(EntityName),m_RobotControl(robot_control)
{
}

double FRC_2011_Robot::Robot_Arm::AngleToHeight_m(double Angle_r)
{
	return (sin(Angle_r*c_GearToArmRatio)*c_ArmLength_m)+c_GearHeightOffset;
}
double FRC_2011_Robot::Robot_Arm::Arm_AngleToHeight_m(double Angle_r)
{
	return (sin(Angle_r)*c_ArmLength_m)+c_GearHeightOffset;
}

double FRC_2011_Robot::Robot_Arm::HeightToAngle_r(double Height_m)
{
	return asin((Height_m-c_GearHeightOffset)/c_ArmLength_m) * c_ArmToGearRatio;
}

double FRC_2011_Robot::Robot_Arm::PotentiometerRaw_To_Arm_r(double raw)
{
	const int RawRangeHalf=512;
	double ret=((raw / RawRangeHalf)-1.0) * DEG_2_RAD(270.0/2.0);  //normalize and use a 270 degree scalar (in radians)
	ret*=c_PotentiometerToArm;  //convert to arm's gear ratio
	return ret;
}


void FRC_2011_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	//Update the position to where the potentiometer says where it actually is
	#ifndef __DisablePotentiometerCalibration__
	SetPos_m(m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio);
	#endif
	//Temp testing potentiometer readings without applying to current position
	//m_RobotControl->GetArmCurrentPosition();
	__super::TimeChange(dTime_s);
	m_RobotControl->UpdateArmVelocity(m_Physics.GetVelocity());
	//double Pos_m=GetPos_m();
	//double height=AngleToHeight_m(Pos_m);
	//DOUT4("Arm=%f Angle=%f %fft %fin",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
}

void FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized(double Velocity)
{
	//we must have flood control so that other controls may work (the joystick will call this on every time slice!)
	if (Velocity!=m_LastNormalizedVelocity)
	{
		//scale the velocity to the max speed's magnitude
		double VelocityScaled=Velocity*GetMaxSpeed();
		SetRequestedVelocity(VelocityScaled);
		m_LastNormalizedVelocity=Velocity;
	}
}

void FRC_2011_Robot::Robot_Arm::SetPos0feet()
{
	SetIntendedPosition( HeightToAngle_r(0.0) );
}
void FRC_2011_Robot::Robot_Arm::SetPos3feet()
{
	SetIntendedPosition( HeightToAngle_r(0.9144) );
}
void FRC_2011_Robot::Robot_Arm::SetPos6feet()
{
	SetIntendedPosition( HeightToAngle_r(1.8288) );
}
void FRC_2011_Robot::Robot_Arm::SetPos9feet()
{
	SetIntendedPosition( HeightToAngle_r(2.7432) );
}
void FRC_2011_Robot::Robot_Arm::CloseClaw(bool Close)
{
	m_RobotControl->CloseClaw(Close);
}

void FRC_2011_Robot::Robot_Arm::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->Event_Map["Arm_SetPos0feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
		em->EventOnOff_Map["Arm_Claw"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::CloseClaw);

	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->Event_Map["Arm_SetPos0feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
		em->EventOnOff_Map["Arm_Claw"]  .Remove(*this, &FRC_2011_Robot::Robot_Arm::CloseClaw);
	}
}

  /***********************************************************************************************************************************/
 /*															FRC_2011_Robot															*/
/***********************************************************************************************************************************/
FRC_2011_Robot::FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders) : 
	Robot_Tank(EntityName), m_RobotControl(robot_control), m_Arm(EntityName,robot_control),m_UsingEncoders(UseEncoders)
{
}

void FRC_2011_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2011_Robot_Properties *RobotProps=static_cast<const FRC_2011_Robot_Properties *>(props);
	const Ship_1D_Properties *ArmProps=RobotProps?&RobotProps->GetArmProps():NULL;
	m_Arm.Initialize(em,ArmProps);
}
void FRC_2011_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
}

void FRC_2011_Robot::TimeChange(double dTime_s)
{
	if (m_UsingEncoders)
	{
		double LeftVelocity,RightVelocity;
		m_RobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);
		Vec2d LocalVelocity;
		double AngularVelocity;
		InterpolateVelocities(LeftVelocity,RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
		GetPhysics().SetLinearVelocity(LocalToGlobal(GetAtt_r(),LocalVelocity));
		GetPhysics().SetAngularVelocity(AngularVelocity);
	}

	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
}

double FRC_2011_Robot::RPS_To_LinearVelocity(double RPS)
{
	return RPS * c_MotorToWheelGearRatio * M_PI * c_WheelDiameter; 
}

void FRC_2011_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	m_RobotControl->UpdateLeftRightVelocity(GetLeftVelocity(),GetRightVelocity());
}

void FRC_2011_Robot::OpenDeploymentDoor(bool Open)
{
	m_RobotControl->OpenDeploymentDoor(Open);
}
void FRC_2011_Robot::ReleaseLazySusan(bool Release)
{
	m_RobotControl->ReleaseLazySusan(Release);
}

void FRC_2011_Robot::BindAdditionalEventControls(bool Bind)
{
	Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventOnOff_Map["Robot_OpenDoor"].Subscribe(ehl, *this, &FRC_2011_Robot::OpenDeploymentDoor);
		em->EventOnOff_Map["Robot_ReleaseLazySusan"].Subscribe(ehl, *this, &FRC_2011_Robot::ReleaseLazySusan);
	}
	else
	{
		em->EventOnOff_Map["Robot_OpenDoor"]  .Remove(*this, &FRC_2011_Robot::OpenDeploymentDoor);
		em->EventOnOff_Map["Robot_ReleaseLazySusan"]  .Remove(*this, &FRC_2011_Robot::ReleaseLazySusan);
	}

	Ship_1D &ArmShip_Access=m_Arm;
	ArmShip_Access.BindAdditionalEventControls(Bind);
}


  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot_Properties														*/
/***********************************************************************************************************************************/

FRC_2011_Robot_Properties::FRC_2011_Robot_Properties() : m_ArmProps(
	"Arm",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0, //Max Acceleration Forward/Reverse  find the balance between being quick enough without jarring the tube out of its grip
	Ship_1D_Properties::eRobotArm,
	c_UsingArmLimits,	//Using the range
	-c_OptimalAngleDn_r*c_ArmToGearRatio,c_OptimalAngleUp_r*c_ArmToGearRatio
	)
{
}
