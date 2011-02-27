#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "FRC2011_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
const double c_ArmToGearRatio=72.0/28.0;
const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
const double c_PotentiometerToGearRatio=60.0/32.0;
const double c_PotentiometerToArm=c_PotentiometerToGearRatio * c_GearToArmRatio;

//const double c_TestRate=3.0;
//const double c_TestRate=6.0;
const double c_Potentiometer_TestRate=18.0;


  /***************************************************************************************************************/
 /*											Potentiometer_Tester												*/
/***************************************************************************************************************/

Potentiometer_Tester::Potentiometer_Tester() : m_PotentiometerProps(
	"Potentiometer",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	c_Potentiometer_TestRate,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	c_Potentiometer_TestRate,c_Potentiometer_TestRate,
	Ship_1D_Properties::eRobotArm,
	true,	//Using the range
	-c_OptimalAngleDn_r*c_ArmToGearRatio,c_OptimalAngleUp_r*c_ArmToGearRatio
	),Ship_1D("Potentiometer")

{
	Initialize(m_DummyMap,&m_PotentiometerProps);
}
void Potentiometer_Tester::UpdatePotentiometerVoltage(double Voltage)
{
	SetRequestedVelocity(Voltage*m_PotentiometerProps.GetMaxSpeed());
}

double Potentiometer_Tester::GetPotentiometerCurrentPosition()
{
	//TODO this really should be in native potentiometer ratios
	double Pos_m=GetPos_m();
	double height=FRC_2011_Robot::Robot_Arm::AngleToHeight_m(Pos_m);

	//DOUT5("Pot=%f Angle=%f %fft %fin",m_Physics.GetVelocity(),RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);

	return Pos_m;
}

void Potentiometer_Tester::TimeChange()
{
	__super::TimeChange(m_Time_s);
}

  /***************************************************************************************************************/
 /*												Encoder_Simulator												*/
/***************************************************************************************************************/
#define ENCODER_TEST_RATE 1
#if ENCODER_TEST_RATE==0
const double c_Encoder_TestRate=2.916;
const double c_Encoder_MaxAccel=5.0;
#endif

#if ENCODER_TEST_RATE==1
const double c_Encoder_TestRate=2.4;
const double c_Encoder_MaxAccel=4.0;
#endif

#if ENCODER_TEST_RATE==2
const double c_Encoder_TestRate=1.1;
const double c_Encoder_MaxAccel=2.0;
#endif

Encoder_Simulator::Encoder_Simulator(const char EntityName[]) : m_EncoderProps(
	EntityName,
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	c_Encoder_TestRate,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	c_Encoder_MaxAccel,c_Encoder_MaxAccel,
	Ship_1D_Properties::eRobotArm,
	false	//Not using the range
	),Ship_1D(EntityName)
{
	Initialize(m_DummyMap,&m_EncoderProps);
}

void Encoder_Simulator::UpdateEncoderVoltage(double Voltage)
{
	SetRequestedVelocity(Voltage*m_EncoderProps.GetMaxSpeed());
}

double Encoder_Simulator::GetEncoderVelocity()
{
	return m_Physics.GetVelocity();
}

void Encoder_Simulator::TimeChange()
{
	__super::TimeChange(m_Time_s);
}


  /***************************************************************************************************************/
 /*													Encoder_Tester												*/
/***************************************************************************************************************/

Encoder_Tester::Encoder_Tester() : m_LeftEncoder("LeftEncoder"),m_RightEncoder("RightEncoder")
{

}

void Encoder_Tester::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	LeftVelocity=m_LeftEncoder.GetEncoderVelocity();
	RightVelocity=m_RightEncoder.GetEncoderVelocity();
}

void Encoder_Tester::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	m_LeftEncoder.UpdateEncoderVoltage(LeftVoltage);
	m_RightEncoder.UpdateEncoderVoltage(RightVoltage);
}

void Encoder_Tester::SetTimeDelta(double dTime_s)
{
	m_LeftEncoder.SetTimeDelta(dTime_s);
	m_RightEncoder.SetTimeDelta(dTime_s);
}

void Encoder_Tester::TimeChange()
{
	m_LeftEncoder.TimeChange();
	m_RightEncoder.TimeChange();
}
