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


  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Ship_1D(EntityName),m_RobotControl(robot_control)
{
}
void FRC_2011_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	//TODO add method to read height here
	__super::TimeChange(dTime_s);
	m_RobotControl->UpdateArmVelocity(m_Physics.GetLinearVelocity());
	double Pos_m=GetPos_m();
	//DOUT4("Arm=%f Pos=%fm %fft %fin",m_Physics.GetLinearVelocity(),Pos_m,Pos_m*3.2808399,Pos_m*39.3700787);
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
	SetIntendedPosition(0.0);
}
void FRC_2011_Robot::Robot_Arm::SetPos3feet()
{
	SetIntendedPosition(0.9144);
}
void FRC_2011_Robot::Robot_Arm::SetPos6feet()
{
	SetIntendedPosition(1.8288);
}
void FRC_2011_Robot::Robot_Arm::SetPos9feet()
{
	SetIntendedPosition(2.7432);
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

	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->Event_Map["Arm_SetPos0feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
	}

}

  /***********************************************************************************************************************************/
 /*															FRC_2011_Robot															*/
/***********************************************************************************************************************************/

FRC_2011_Robot::FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Robot_Tank(EntityName), m_RobotControl(robot_control), m_Arm(EntityName,robot_control)
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
	__super::TimeChange(dTime_s);
	Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	arm_entity.TimeChange(dTime_s);
}

void FRC_2011_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	m_RobotControl->UpdateLeftRightVelocity(GetLeftVelocity(),GetRightVelocity());
}

void FRC_2011_Robot::BindAdditionalEventControls(bool Bind)
{
	//Nothing to bind here since the base has everything we need, but other components will need their bindings
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
	2.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	2.0,2.0, //Max Acceleration Forward/Reverse  find the balance between being quick enough without jarring the tube out of its grip
	Ship_1D_Properties::eRobotArm,
	true,	//Using the range
	0.0,9.0 //0 - 9
	)
{
}
