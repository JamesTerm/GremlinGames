#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "FRC2011_Robot.h"
	#include "Swerve_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);  //We may limit swerve to this
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

  /***********************************************************************************************************************************/
 /*															FRC_2011_Robot															*/
/***********************************************************************************************************************************/
Swerve_Robot::Swerve_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders) : 
	Swerve_Drive(EntityName), m_RobotControl(robot_control), 
	m_UsingEncoders(UseEncoders) //,m_VoltageOverride(false),m_UseDeadZoneSkip(true)
{
	//m_UsingEncoders=true; //testing
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=1.0;
}

void Swerve_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	//const FRC_2011_Robot_Properties *RobotProps=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
	//m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	//m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);

	//const double OutputRange=MAX_SPEED*0.875;  //create a small range
	//const double InputRange=20.0;  //create a large enough number that can divide out the voltage and small enough to recover quickly
	//m_PIDController_Left.SetInputRange(-MAX_SPEED,MAX_SPEED);
	//m_PIDController_Left.SetOutputRange(-InputRange,OutputRange);
	//m_PIDController_Left.Enable();
	//m_PIDController_Right.SetInputRange(-MAX_SPEED,MAX_SPEED);
	//m_PIDController_Right.SetOutputRange(-InputRange,OutputRange);
	//m_PIDController_Right.Enable();
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
}
void Swerve_Robot::ResetPos()
{
	__super::ResetPos();
	//m_Arm.ResetPos();
	//m_Claw.ResetPos();
	m_RobotControl->Reset_Encoders();
	//m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
	//ensure teleop has these set properly
	//m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
	//m_UseDeadZoneSkip=true;
}

void Swerve_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->TimeChange(dTime_s);
	//TODO add encoder support here
	//{
	//	//Display encoders without applying calibration
	//	double Encoder_LeftVelocity,Encoder_RightVelocity;
	//	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	//}
	__super::TimeChange(dTime_s);
}

bool Swerve_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	bool ret=false;
	//if (m_UsingEncoders)
	//{
	//	Vec2d computedVelocity=m_Physics.GetLinearVelocity();
	//	//double computedAngularVelocity=m_Physics.GetAngularVelocity();
	//	m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
	//	//m_Physics.SetAngularVelocity(m_EncoderHeading);
	//	m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	//	//We must set this back so that the PID can compute the entire error
	//	m_Physics.SetLinearVelocity(computedVelocity);
	//	//m_Physics.SetAngularVelocity(computedAngularVelocity);
	//	ret=true;
	//}
	return ret;
}

double Swerve_Robot::RPS_To_LinearVelocity(double RPS)
{
	return RPS * c_MotorToWheelGearRatio * M_PI * c_WheelDiameter; 
}

//void Swerve_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
//{
//	m_VoltageOverride=false;
//	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
//			m_VoltageOverride=true;
//}

void Swerve_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	//double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	//m_RobotControl->UpdateLeftRightVoltage(RightVoltage,LeftVoltage);
}

