#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "FRC2012_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;


  /***********************************************************************************************************************************/
 /*															FRC_2012_Robot															*/
/***********************************************************************************************************************************/
FRC_2012_Robot::FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders) : 
	Tank_Robot(EntityName,robot_control,UseEncoders), m_RobotControl(robot_control)
{
}

void FRC_2012_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2012_Robot_Properties *RobotProps=dynamic_cast<const FRC_2012_Robot_Properties *>(props);
	//m_Arm.Initialize(em,RobotProps?&RobotProps->GetArmProps():NULL);
	//m_Claw.Initialize(em,RobotProps?&RobotProps->GetClawProps():NULL);
}
void FRC_2012_Robot::ResetPos()
{
	__super::ResetPos();
	//m_Arm.ResetPos();
	//m_Claw.ResetPos();
}

void FRC_2012_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Robot_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
	//Entity1D &arm_entity=m_Arm;  //This gets around keeping time change protected in derived classes
	//arm_entity.TimeChange(dTime_s);
	//Entity1D &claw_entity=m_Claw;  //This gets around keeping time change protected in derived classes
	//claw_entity.TimeChange(dTime_s);
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


void FRC_2012_Robot::BindAdditionalEventControls(bool Bind)
{
	//Entity2D::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	//if (Bind)
	//	em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2012_Robot::CloseDeploymentDoor);
	//else
	//	em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2012_Robot::CloseDeploymentDoor);

	//Ship_1D &ArmShip_Access=m_Arm;
	//ArmShip_Access.BindAdditionalEventControls(Bind);
	//Ship_1D &ClawShip_Access=m_Claw;
	//ClawShip_Access.BindAdditionalEventControls(Bind);
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Control															*/
/***********************************************************************************************************************************/

void FRC_2012_Robot_Control::UpdateVoltage(size_t index,double Voltage)
{
	//switch (index)
	//{
	//	case FRC_2012_Robot::eArm:
	//	{
	//		//	printf("Arm=%f\n",Voltage);
	//		//DOUT3("Arm Voltage=%f",Voltage);
	//		m_ArmVoltage=Voltage;
	//		//Note: I have to reverse the voltage again since the wires are currently crossed on the robot
	//		m_Potentiometer.UpdatePotentiometerVoltage(-Voltage);
	//		m_Potentiometer.TimeChange();  //have this velocity immediately take effect
	//	}
	//		break;
	//	case FRC_2012_Robot::eRollers:
	//		m_RollerVoltage=Voltage;
	//		//DOUT3("Arm Voltage=%f",Voltage);
	//		break;
	//}
}

FRC_2012_Robot_Control::FRC_2012_Robot_Control() : m_pTankRobotControl(&m_TankRobotControl)
{
	m_TankRobotControl.SetDisplayVoltage(false); //disable display there so we can do it here
}

void FRC_2012_Robot_Control::Reset_Rotary(size_t index)
{
	m_KalFilter_Arm.Reset();
}

void FRC_2012_Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2012_Robot_Properties *robot_props=dynamic_cast<const FRC_2012_Robot_Properties *>(props);

	assert(robot_props);

	Tank_Drive_Control_Interface *tank_interface=m_pTankRobotControl;
	tank_interface->Initialize(props);
}

void FRC_2012_Robot_Control::Robot_Control_TimeChange(double dTime_s)
{
	m_Potentiometer.SetTimeDelta(dTime_s);
	//display voltages
	DOUT2("l=%f r=%f \n",m_TankRobotControl.GetLeftVoltage(),m_TankRobotControl.GetRightVoltage());
}

//void Robot_Control::UpdateVoltage(size_t index,double Voltage)
//{
//}

double FRC_2012_Robot_Control::GetRotaryCurrentPosition(size_t index)
{
	return 0.0;
}

  /***********************************************************************************************************************************/
 /*													FRC_2012_Robot_Properties														*/
/***********************************************************************************************************************************/

const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

FRC_2012_Robot_Properties::FRC_2012_Robot_Properties() 
{
	Tank_Robot_Props props=m_TankRobotProps; //start with super class settings

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
	props.WheelDiameter=c_WheelDiameter;
	props.LeftPID[1]=props.RightPID[1]=1.0; //set the I's to one... so it should be 1,1,0
	props.MotorToWheelGearRatio=c_MotorToWheelGearRatio;
	m_TankRobotProps=props;
}
