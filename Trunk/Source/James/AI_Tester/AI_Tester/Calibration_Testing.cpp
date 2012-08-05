#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
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
 /*											Potentiometer_Tester2												*/
/***************************************************************************************************************/

Potentiometer_Tester2::Potentiometer_Tester2() : m_PotentiometerProps(
	"Potentiometer2",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	10.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	10.0,10.0,
	Ship_1D_Properties::eSwivel,
	true,	//Using the range
	DEG_2_RAD(-180.0),DEG_2_RAD(180.0)
	),Ship_1D("Potentiometer2")

{
	m_Bypass=false;
}

void Potentiometer_Tester2::Initialize(const Ship_1D_Properties *props)
{
	if (props)
		m_PotentiometerProps=*props;
	__super::Initialize(m_DummyMap,&m_PotentiometerProps);
}

void Potentiometer_Tester2::UpdatePotentiometerVoltage(double Voltage)
{
	//Voltage=GetTweakedVoltage(Voltage);
	if (!m_Bypass)
		SetRequestedVelocity(Voltage*m_PotentiometerProps.GetMaxSpeed());
	else
		SetRequestedVelocity(0.0);
}

double Potentiometer_Tester2::GetPotentiometerCurrentPosition()
{
	//Note this is in native potentiometer ratios
	double Pos_m=GetPos_m();
	return Pos_m;
}

void Potentiometer_Tester2::TimeChange()
{
	__super::TimeChange(m_Time_s);
}

  /***************************************************************************************************************/
 /*												Encoder_Simulator												*/
/***************************************************************************************************************/
#define ENCODER_TEST_RATE 1
#if ENCODER_TEST_RATE==0
const double c_Encoder_TestRate=16.0;
const double c_Encoder_MaxAccel=120.0;
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

Encoder_Simulator::Encoder_Simulator(const char EntityName[]) : m_Time_s(0.0),m_EncoderProps(
	EntityName,
	68.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	c_Encoder_TestRate,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	c_Encoder_MaxAccel,c_Encoder_MaxAccel,
	Ship_1D_Properties::eRobotArm,
	false	//Not using the range
	),Ship_1D(EntityName),m_Latency(0.300),
	m_EncoderScalar(1.0),m_GetEncoderFirstCall(false)
{
}

void Encoder_Simulator::Initialize(const Ship_1D_Properties *props)
{
	if (props)
		m_EncoderProps=*props;
	__super::Initialize(m_DummyMap,&m_EncoderProps);
}

void Encoder_Simulator::UpdateEncoderVoltage(double Voltage)
{
	//Here is some stress to emulate a bad curved victor
	#if 0
	//This is how it would be if the motor was set to non-coast
	double VoltageMag=pow(fabs(Voltage),0.5);
	Voltage= (Voltage>0.0)?	VoltageMag:-VoltageMag;
	#endif
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

const double Polynomial[5]=
{
	//0.0,1.0   ,0.0    ,0.0   ,0.0
	  0.0,2.4878,-2.2091,0.7134,0.0
};

double Encoder_Simulator::GetEncoderVelocity()
{
	#if 0
	return m_Physics.GetVelocity() * m_EncoderScalar;
	#else
	if (!m_GetEncoderFirstCall) return m_Latency();

	double Voltage = m_Physics.GetVelocity() / m_EncoderProps.GetMaxSpeed();
	double Direction=Voltage<0 ? -1.0 : 1.0;
	Voltage=fabs(Voltage); //make positive
	//Apply the victor curve
	//Apply the polynomial equation to the voltage to linearize the curve
	{
		const double *c=Polynomial;
		double x2=Voltage*Voltage;
		double x3=Voltage*x2;
		double x4=x2*x2;
		Voltage = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*Voltage) + c[0]; 
		Voltage *= Direction;
	}
	double ret=Voltage * m_EncoderProps.GetMaxSpeed() * m_EncoderScalar;
	//ret=m_Latency(ret,m_Time_s);
	m_GetEncoderFirstCall=false; //weed out the repeat calls
	return ret;
	#endif
}

void Encoder_Simulator::SetReverseDirection(bool reverseDirection)
{
	m_EncoderScalar= reverseDirection? -1.0 : 1.0;
}

void Encoder_Simulator::TimeChange()
{
	m_GetEncoderFirstCall=true;
	__super::TimeChange(m_Time_s);
}

  /***************************************************************************************************************/
 /*												Encoder_Simulator2												*/
/***************************************************************************************************************/
const double c_OunceInchToNewton=0.00706155183333;
const double c_CIM_Amp_To_Torque_oz=(1.0/(133.0-2.7)) * 343.3;
const double c_CIM_Amp_To_Torque_nm=c_CIM_Amp_To_Torque_oz*c_OunceInchToNewton;
const double c_CIM_Vel_To_Torque_oz=(1.0/(5310/60.0)) * 343.4;
const double c_CIM_Vel_To_Torque_nm=c_CIM_Vel_To_Torque_oz*c_OunceInchToNewton; 
const double c_CIM_Torque_to_Vel_nm=1.0 / c_CIM_Vel_To_Torque_nm;

__inline double Drive_Train_Characteristics::GetAmp_To_Torque_nm(double Amps)
{
	return max ((Amps-2.7) * c_CIM_Amp_To_Torque_nm,0.0);
}
__inline double Drive_Train_Characteristics::GetVel_To_Torque_nm(double Vel_rps)
{
	return (Vel_rps * c_CIM_Vel_To_Torque_nm);
}
__inline double Drive_Train_Characteristics::GetTorque_To_Vel_nm(double Vel_rps)
{
	return (Vel_rps * c_CIM_Torque_to_Vel_nm);
}
//gear reduction (5310/60.0) / (427.68 / 60.0) = 12.415824915824915824915824915825
//TODO compute gear reduction from max speed and pass into props here
Drive_Train_Characteristics::Drive_Train_Characteristics() : m_Props(1.0,12.4158,0.0762,2.0) 
{
}
__inline double Drive_Train_Characteristics::GetWheelStallTorque(double Torque)
{
	return Torque * m_Props.GearReduction * m_Props.DriveTrain_Efficiency;
}
__inline double Drive_Train_Characteristics::GetTorqueAtWheel(double Torque)
{
	return (GetWheelStallTorque(Torque) / m_Props.DriveWheelRadius);
}
__inline double Drive_Train_Characteristics::GetWheelRPS(double LinearVelocity)
{
	return LinearVelocity / (M_PI * 2.0 * m_Props.DriveWheelRadius);			
}
__inline double Drive_Train_Characteristics::GetMotorRPS(double LinearVelocity)
{
	return GetWheelRPS(LinearVelocity) *  m_Props.GearReduction;
}
__inline double Drive_Train_Characteristics::GetTorqueFromLinearVelocity(double LinearVelocity)
{
	const double MotorTorque=GetVel_To_Torque_nm(GetMotorRPS(LinearVelocity));
	return GetTorqueAtWheel(MotorTorque * 2.0);
}

__inline double Drive_Train_Characteristics::GetTorqueFromVoltage(double Voltage)
{
	const double Amps=fabs(Voltage*133.0);
	const double MotorTorque=GetAmp_To_Torque_nm(Amps);
	const double WheelTorque=GetTorqueAtWheel(MotorTorque * 2.0);
	return (Voltage>0)? WheelTorque : -WheelTorque;  //restore sign
}


Encoder_Simulator2::Encoder_Simulator2(const char EntityName[]) : m_Time_s(0.0),m_EncoderProps(
	EntityName,
	68.0,    //Mass (150 pounds)
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	c_Encoder_TestRate,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	c_Encoder_MaxAccel,c_Encoder_MaxAccel,
	Ship_1D_Properties::eRobotArm,
	false	//Not using the range
	),
	m_EncoderScalar(1.0)
{
}

void Encoder_Simulator2::Initialize(const Ship_1D_Properties *props)
{
	if (props)
		m_EncoderProps=*props;
		//m_Physics.SetMass(68);  //(about 150 pounds)
	m_Physics.SetMass(20);
	m_Physics.SetFriction(0.8,0.2);
}

void Encoder_Simulator2::UpdateEncoderVoltage(double Voltage)
{
	double Direction=Voltage<0 ? -1.0 : 1.0;
	Voltage=fabs(Voltage); //make positive
	//Apply the victor curve
	//Apply the polynomial equation to the voltage to linearize the curve
	{
		const double *c=Polynomial;
		double x2=Voltage*Voltage;
		double x3=Voltage*x2;
		double x4=x2*x2;
		Voltage = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*Voltage) + c[0]; 
		Voltage *= Direction;
	}
	//From this point it is a percentage (hopefully linear distribution after applying the curve) of the max force to apply... This can be
	//computed from stall torque ratings of motor with various gear reductions and so forth
	//on JVN's spread sheet torque at wheel is (WST / DWR) * 2  (for nm)  (Wheel stall torque / Drive Wheel Radius * 2 sides)
	//where stall torque is ST / GearReduction * DriveTrain efficiency
	//This is all computed in the drive train
	double ForceToApply=m_DriveTrain.GetTorqueFromVoltage(Voltage);
	const double ForceAbsorbed=m_DriveTrain.GetTorqueFromLinearVelocity(m_Physics.GetVelocity());
	ForceToApply-=ForceAbsorbed;
	m_Physics.ApplyFractionalForce(ForceToApply,m_Time_s);
}

double Encoder_Simulator2::GetEncoderVelocity() const
{
	return m_Physics.GetVelocity();
}

void Encoder_Simulator2::SetReverseDirection(bool reverseDirection)
{
	m_EncoderScalar= reverseDirection? -1.0 : 1.0;
}

void Encoder_Simulator2::TimeChange()
{
	//m_GetEncoderFirstCall=true;
	const double Ground=0.0;  //ground in radians
	double FrictionForce=m_Physics.GetFrictionalForce(m_Time_s,Ground);
	m_Physics.ApplyFractionalForce(FrictionForce,m_Time_s);  //apply the friction
}

  /***************************************************************************************************************/
 /*													Encoder_Tester												*/
/***************************************************************************************************************/

Encoder_Tester::Encoder_Tester() : m_LeftEncoder("LeftEncoder"),m_RightEncoder("RightEncoder")
{
	m_LeftEncoder.Initialize(NULL);
	m_RightEncoder.Initialize(NULL);
}

void Encoder_Tester::Initialize(const Ship_1D_Properties *props)
{
	m_LeftEncoder.Initialize(props);
	m_RightEncoder.Initialize(props);
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
