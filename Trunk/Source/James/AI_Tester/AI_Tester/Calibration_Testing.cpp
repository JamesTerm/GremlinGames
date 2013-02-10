#include "stdafx.h"
#include "AI_Tester.h"
#include "Robot_Tester.h"

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
	Ship_1D_Props::eRobotArm,
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
	Ship_1D_Props::eSwivel,
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
	Ship_1D_Props::eRobotArm,
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
	//if (!m_GetEncoderFirstCall) return m_Latency();

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
 /*											EncoderSimulation_Properties										*/
/***************************************************************************************************************/

const double c_OunceInchToNewton=0.00706155183333;

EncoderSimulation_Properties::EncoderSimulation_Properties()
{
	EncoderSimulation_Props props;
	memset(&props,0,sizeof(EncoderSimulation_Props));

	props.Wheel_Mass=1.5;
	props.COF_Efficiency=1.0;
	props.GearReduction=12.4158;
	props.TorqueAppliedOnWheelRadius=0.0508;
	props.DriveWheelRadius=0.0762;
	props.NoMotors=1.0;

	props.motor.FreeSpeed_RPM=5310;
	props.motor.Stall_Torque_NM=343.4 * c_OunceInchToNewton;
	props.motor.Stall_Current_Amp=133.0;
	props.motor.Free_Current_Amp=2.7;

	m_EncoderSimulation_Props=props;
}

void EncoderSimulation_Properties::LoadFromScript(GG_Framework::Logic::Scripting::Script& script)
{
	EncoderSimulation_Props &props=m_EncoderSimulation_Props;
	double test;
	const char* err=NULL;

	err = script.GetField("wheel_mass", NULL, NULL, &test);
	if (!err) props.Wheel_Mass=test;
	err = script.GetField("cof_efficiency", NULL, NULL, &test);
	if (!err) props.COF_Efficiency=test;
	err = script.GetField("gear_reduction", NULL, NULL, &test);
	if (!err) props.GearReduction=test;
	err = script.GetField("torque_on_wheel_radius", NULL, NULL, &test);
	if (!err) props.TorqueAppliedOnWheelRadius=test;
	err = script.GetField("drive_wheel_radius", NULL, NULL, &test);
	if (!err) props.DriveWheelRadius=test;
	err = script.GetField("number_of_motors", NULL, NULL, &test);
	if (!err) props.NoMotors=test;

	err = script.GetField("free_speed_rpm", NULL, NULL, &test);
	if (!err) props.motor.FreeSpeed_RPM=test;
	err = script.GetField("stall_torque", NULL, NULL, &test);
	if (!err) props.motor.Stall_Torque_NM=test;
	err = script.GetField("stall_current_amp", NULL, NULL, &test);
	if (!err) props.motor.Stall_Current_Amp=test;
	err = script.GetField("free_current_amp", NULL, NULL, &test);
	if (!err) props.motor.Free_Current_Amp=test;
}


  /***************************************************************************************************************/
 /*											Drive_Train_Characteristics											*/
/***************************************************************************************************************/

const double c_CIM_Amp_To_Torque_oz=(1.0/(133.0-2.7)) * 343.3;
const double c_CIM_Amp_To_Torque_nm=c_CIM_Amp_To_Torque_oz*c_OunceInchToNewton;
const double c_CIM_Vel_To_Torque_oz=(1.0/(5310/60.0)) * 343.4;
const double c_CIM_Vel_To_Torque_nm=c_CIM_Vel_To_Torque_oz*c_OunceInchToNewton; 
const double c_CIM_Torque_to_Vel_nm=1.0 / c_CIM_Vel_To_Torque_nm;

__inline double Drive_Train_Characteristics::GetAmp_To_Torque_nm(double Amps)
{
	const EncoderSimulation_Props::Motor_Specs &_=m_Props.motor;
	const double c_Amp_To_Torque_nm=(1.0/(_.Stall_Current_Amp-_.Free_Current_Amp)) * _.Stall_Torque_NM;
	return max ((Amps-_.Free_Current_Amp) * c_Amp_To_Torque_nm,0.0);
}
__inline double Drive_Train_Characteristics::GetVel_To_Torque_nm(double Vel_rps)
{
	const EncoderSimulation_Props::Motor_Specs &_=m_Props.motor;
	const double c_Vel_To_Torque_nm=(1.0/(_.FreeSpeed_RPM/60.0)) * _.Stall_Torque_NM;
	return (Vel_rps * c_Vel_To_Torque_nm);
}
__inline double Drive_Train_Characteristics::GetTorque_To_Vel_nm(double Vel_rps)
{
	const EncoderSimulation_Props::Motor_Specs &_=m_Props.motor;
	const double c_Vel_To_Torque_nm=(1.0/(_.FreeSpeed_RPM/60.0)) * _.Stall_Torque_NM;
	const double c_Torque_to_Vel_nm=1.0 / c_Vel_To_Torque_nm;
	return (Vel_rps * c_Torque_to_Vel_nm);
}
//gear reduction (5310/60.0) / (427.68 / 60.0) = 12.415824915824915824915824915825
//TODO compute gear reduction from max speed and pass into props here
Drive_Train_Characteristics::Drive_Train_Characteristics()
{
	EncoderSimulation_Properties default_props;
	m_Props=default_props.GetEncoderSimulationProps();
}
__inline double Drive_Train_Characteristics::GetWheelStallTorque(double Torque)
{
	return Torque * m_Props.GearReduction * m_Props.COF_Efficiency;
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

__inline double Drive_Train_Characteristics::GetWheelRPS_Angular(double AngularVelocity)
{
	return AngularVelocity / (M_PI * 2.0);			
}
__inline double Drive_Train_Characteristics::GetMotorRPS_Angular(double AngularVelocity)
{
	return GetWheelRPS_Angular(AngularVelocity) *  m_Props.GearReduction;
}

__inline double Drive_Train_Characteristics::GetTorqueFromLinearVelocity(double LinearVelocity)
{
	const double MotorTorque=GetVel_To_Torque_nm(GetMotorRPS(LinearVelocity));
	return GetTorqueAtWheel(MotorTorque * 2.0);
}

__inline double Drive_Train_Characteristics::GetWheelTorqueFromVoltage(double Voltage)
{
	const double Amps=fabs(Voltage*133.0);
	const double MotorTorque=GetAmp_To_Torque_nm(Amps);
	const double WheelTorque=GetTorqueAtWheel(MotorTorque * 2.0);
	return (Voltage>0)? WheelTorque : -WheelTorque;  //restore sign
}

__inline double Drive_Train_Characteristics::GetTorqueFromVoltage(double Voltage)
{
	const double Amps=fabs(Voltage*133.0);
	const double MotorTorque=GetAmp_To_Torque_nm(Amps);
	const double WheelTorque=GetWheelStallTorque(MotorTorque * m_Props.NoMotors);
	return (Voltage>0)? WheelTorque : -WheelTorque;  //restore sign
}

__inline double Drive_Train_Characteristics::GetTorqueFromVelocity(double AngularVelocity)
{
	const double MotorTorque=GetVel_To_Torque_nm(GetMotorRPS_Angular(AngularVelocity));
	return GetWheelStallTorque(MotorTorque * m_Props.NoMotors);
}


  /***************************************************************************************************************/
 /*												Encoder_Simulator2												*/
/***************************************************************************************************************/


Encoder_Simulator2::Encoder_Simulator2(const char EntityName[]) : m_Time_s(0.0),m_EncoderScalar(1.0),m_ReverseMultiply(1.0),m_Position(0.0)
{
}

void Encoder_Simulator2::Initialize(const Ship_1D_Properties *props)
{
	const Rotary_Properties *rotary_props=dynamic_cast<const Rotary_Properties *>(props);
	if (rotary_props)
	{
		m_DriveTrain.UpdateProps(rotary_props->GetEncoderSimulationProps());
		m_EncoderScalar=rotary_props->GetRoteryProps().EncoderToRS_Ratio;
	}

	#if 0
	//m_Physics.SetMass(68);  //(about 150 pounds)
	m_Physics.SetMass(50);  //adjust this to match latency we observe
	m_Physics.SetFriction(0.8,0.08);
	#else
	m_Physics.SetMass(m_DriveTrain.GetDriveTrainProps().Wheel_Mass);
	m_Physics.SetFriction(0.8,0.2);
	m_Physics.SetRadiusOfConcentratedMass(m_DriveTrain.GetDriveTrainProps().DriveWheelRadius);
	#endif
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
	#if 0
	double ForceToApply=m_DriveTrain.GetWheelTorqueFromVoltage(Voltage);
	const double ForceAbsorbed=m_DriveTrain.GetWheelTorqueFromVelocity(m_Physics.GetVelocity());
	ForceToApply-=ForceAbsorbed;
	m_Physics.ApplyFractionalForce(ForceToApply,m_Time_s);
	#else
	double TorqueToApply=m_DriveTrain.GetTorqueFromVoltage(Voltage);
	const double TorqueAbsorbed=m_DriveTrain.GetTorqueFromVelocity(m_Physics.GetVelocity());
	TorqueToApply-=TorqueAbsorbed;
	m_Physics.ApplyFractionalTorque(TorqueToApply,m_Time_s,m_DriveTrain.GetDriveTrainProps().TorqueAppliedOnWheelRadius);
	#endif
}

double Encoder_Simulator2::GetEncoderVelocity() const
{
	#if 0
	static size_t i=0;
	double Velocity=m_Physics.GetVelocity();
	if (((i++ % 50)==0)&&(Velocity!=0.0))
		printf("Velocty=%f\n",Velocity *  m_DriveTrain.GetDriveTrainProps().DriveWheelRadius );
	return 0.0;
	#else
	//return m_Physics.GetVelocity();
	return m_Physics.GetVelocity() *  m_DriveTrain.GetDriveTrainProps().DriveWheelRadius * m_ReverseMultiply;
	#endif
}

double Encoder_Simulator2::GetDistance() const
{
	return m_Position;
}

void Encoder_Simulator2::SetReverseDirection(bool reverseDirection)
{
	m_ReverseMultiply= reverseDirection? -1.0 : 1.0;
}

void Encoder_Simulator2::TimeChange()
{
	//m_GetEncoderFirstCall=true;
	const double Ground=0.0;  //ground in radians
	double FrictionForce=m_Physics.GetFrictionalForce(m_Time_s,Ground);
	m_Physics.ApplyFractionalForce(FrictionForce,m_Time_s);  //apply the friction
	double PositionDisplacement;
	m_Physics.TimeChangeUpdate(m_Time_s,PositionDisplacement);
	m_Position+= PositionDisplacement * m_EncoderScalar * m_ReverseMultiply;
}

void Encoder_Simulator2::ResetPos()
{
	m_Physics.ResetVectors();
	m_Position=0;
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
	//Cool way to add friction uneven to simulate 
	//m_RightEncoder.SetFriction(0.8,0.12);
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
