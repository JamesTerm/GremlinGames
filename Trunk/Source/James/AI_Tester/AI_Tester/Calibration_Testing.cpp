#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "FRC2011_Robot.h"
	#include "FRC2012_Robot.h"
}

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
const double c_ArmToGearRatio=72.0/28.0;
const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
//const double c_PotentiometerToGearRatio=60.0/32.0;
//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
const double c_PotentiometerToArmRatio=36.0/54.0;
const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
const double c_ArmToPotentiometerRatio=1.0/c_PotentiometerToArmRatio;
const double c_GearToPotentiometer=1.0/c_PotentiometerToGearRatio;
//const double c_TestRate=3.0;
//const double c_TestRate=6.0;
//const double c_Potentiometer_TestRate=18.0;
const double c_Potentiometer_TestRate=24.0;
//const double c_Potentiometer_TestRate=48.0;


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
	DEG_2_RAD(-135.0),DEG_2_RAD(135.0)
	),Ship_1D("Potentiometer")

{
	m_Bypass=false;
	Initialize(m_DummyMap,&m_PotentiometerProps);
}

const double c_DeadZone=0.085;
const double c_MidRangeZone=.150;
const double c_MidDistance=c_MidRangeZone-c_DeadZone;
const double c_rMidDistance=1.0/c_MidDistance;
const double c_EndDistance=1.0-c_MidRangeZone;
const double c_rEndDistance=1.0/c_EndDistance;

double GetTweakedVoltage(double Voltage)
{
	double VoltMag=fabs(Voltage);
	double sign=Voltage / VoltMag;

	//simulate stresses on the robot
	if (VoltMag>0.0)
	{
		if (VoltMag<c_DeadZone)
		{
			DebugOutput("Buzz %f\n",Voltage);   //simulate no movement but hearing the motor
			VoltMag=0.0;
		}
		else if (VoltMag<c_MidRangeZone)
			VoltMag=(VoltMag-c_DeadZone) * c_rMidDistance *0.5;
		else
			VoltMag=((VoltMag-c_MidRangeZone) * c_rEndDistance * 0.5) + 0.5;

		Voltage=VoltMag*sign;
	}
	return Voltage;
}

void Potentiometer_Tester::UpdatePotentiometerVoltage(double Voltage)
{
	Voltage=GetTweakedVoltage(Voltage);
	if (!m_Bypass)
		SetRequestedVelocity(Voltage*c_GearToPotentiometer*m_PotentiometerProps.GetMaxSpeed());
	else
		SetRequestedVelocity(0.0);
}

double Potentiometer_Tester::GetPotentiometerCurrentPosition()
{
	//Note this is in native potentiometer ratios
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
const double c_Encoder_TestRate=4.0;
const double c_Encoder_MaxAccel=10.0;
#endif

#if ENCODER_TEST_RATE==1
const double c_Encoder_TestRate=2.916;
const double c_Encoder_MaxAccel=60.0;
#endif

#if ENCODER_TEST_RATE==2
const double c_Encoder_TestRate=2.4;
const double c_Encoder_MaxAccel=4.0;
#endif

#if ENCODER_TEST_RATE==3
const double c_Encoder_TestRate=1.1;
const double c_Encoder_MaxAccel=2.0;
#endif

Encoder_Simulator::Encoder_Simulator(const char EntityName[]) : m_EncoderProps(
	EntityName,
	68.0,    //Mass
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
	double VoltageMag=pow(fabs(Voltage),0.5);
	Voltage= (Voltage>0.0)?	VoltageMag:-VoltageMag;
	//This is how it would be if the motor was set to non-coast
	SetRequestedVelocity(Voltage*m_EncoderProps.GetMaxSpeed());
	#if 0
	//For coast it is more like applying force
	double Velocity=m_Physics.GetVelocity();
	double MaxSpeed=m_EncoderProps.GetMaxSpeed();
	double Accel=Voltage*5000.0*MaxSpeed * m_Time_s;  //determine current acceleration by applying the ratio with a scalar  
	double filter=1.0 - (fabs(Velocity) / MaxSpeed);  //as there is more speed there is less torque
	//double friction=((Velocity>0.04)?-0.025 : (Velocity<-0.04)?+0.025 : 0.0 )* rTime ;  //There is a constant amount of friction opposite to current velocity
	double friction=((Velocity>0.04)?-250 : (Velocity<-0.04)?+250 : 0.0 )* m_Time_s;  //There is a constant amount of friction opposite to current velocity
	SetCurrentLinearAcceleration((Accel * filter * filter) + friction); //square the filter (matches read out better)
	#endif
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
	//LeftVoltage=GetTweakedVoltage(LeftVoltage);
	//RightVoltage=GetTweakedVoltage(RightVoltage);
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
