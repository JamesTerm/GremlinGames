#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "PIDController.h"
	#include "FRC2011_Robot.h"
}

const bool c_UsingArmLimits=true;

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double c_OptimalAngleUp_r=DEG_2_RAD(70.0);
const double c_OptimalAngleDn_r=DEG_2_RAD(50.0);
const double c_ArmLength_m=1.8288;  //6 feet
const double c_ArmToGearRatio=72.0/28.0;
const double c_GearToArmRatio=1.0/c_ArmToGearRatio;
//const double c_PotentiometerToGearRatio=60.0/32.0;
//const double c_PotentiometerToArmRatio=c_PotentiometerToGearRatio * c_GearToArmRatio;
const double c_PotentiometerToArmRatio=36.0/54.0;
const double c_PotentiometerToGearRatio=c_PotentiometerToArmRatio * c_ArmToGearRatio;
const double c_PotentiometerMaxRotation=DEG_2_RAD(270.0);
const double c_GearHeightOffset=1.397;  //55 inches
const double c_WheelDiameter=0.1524;  //6 inches
const double c_MotorToWheelGearRatio=12.0/36.0;

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot::Robot_Arm														*/
/***********************************************************************************************************************************/

FRC_2011_Robot::Robot_Arm::Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control) : 
	Ship_1D(EntityName),m_RobotControl(robot_control),
	//m_PIDController(0.5,1.0,0.0),
	//m_PIDController(1.0,0.5,0.0),
	m_PIDController(1.0,0.0,0.0),
	m_LastPosition(0.0),m_CalibratedScaler(1.0),m_LastTime(0.0),
	m_UsingPotentiometer(false)  //to be safe
{
}

void FRC_2011_Robot::Robot_Arm::Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	m_LastPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
	__super::Initialize(em,props);
	const Ship_1D_Properties *ship=dynamic_cast<const Ship_1D_Properties *>(props);
	assert(ship);
	m_MaxSpeedReference=ship->GetMaxSpeed();
	m_PIDController.SetInputRange(-m_MaxSpeedReference,m_MaxSpeedReference);
	m_PIDController.SetOutputRange(-m_MaxSpeedReference,m_MaxSpeedReference);
	m_PIDController.Enable();
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
	ret*=c_PotentiometerToArmRatio;  //convert to arm's gear ratio
	return ret;
}


void FRC_2011_Robot::Robot_Arm::TimeChange(double dTime_s)
{
	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated

	//Update the position to where the potentiometer says where it actually is
	//if (m_UsingPotentiometer)
	if (true)
	{
		if (m_LastTime!=0.0)
		{
			double LastSpeed=fabs(m_Physics.GetVelocity());  //This is last because the time change has not happened yet
			double NewPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;

			//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
			double Displacement=NewPosition-m_LastPosition;
			double PotentiometerVelocity=Displacement/m_LastTime;
			double PotentiometerSpeed=fabs(PotentiometerVelocity);
			//Give some tolerance to help keep readings stable
			if (fabs(PotentiometerSpeed-LastSpeed)<0.5)
				PotentiometerSpeed=LastSpeed;
			#if 0
			m_CalibratedScaler=!IsZero(PotentiometerSpeed)?PotentiometerSpeed/LastSpeed:
				m_CalibratedScaler>0.25?m_CalibratedScaler:1.0;  //Hack: be careful not to use a value to close to zero as a scaler otherwise it could deadlock
			#else
			double control=-m_PIDController(LastSpeed,PotentiometerSpeed,dTime_s);
			m_CalibratedScaler=1.0+control;
			//if (m_CalibratedScaler<0.25)  //Hack, safety check to avoid negative values
			//	m_CalibratedScaler=1.0;
			#endif
			MAX_SPEED=m_MaxSpeedReference*m_CalibratedScaler;

			//TODO investigate this against just setting the position
			//update the velocity to the potentiometer's velocity (if we are locking to a position)
			if (!GetLockShipToPosition())
				m_Physics.SetVelocity(PotentiometerVelocity);
			//DOUT5("pSpeed=%f cal=%f Max=%f",PotentiometerSpeed,m_CalibratedScaler,MAX_SPEED);
			SetPos_m(NewPosition);
			m_LastPosition=NewPosition;
		}
		m_LastTime=dTime_s;
	}
	else
	{
		//Test potentiometer readings without applying to current position (disabled by default)
		m_RobotControl->GetArmCurrentPosition();
	}
	__super::TimeChange(dTime_s);
	double CurrentVelocity=m_Physics.GetVelocity();
	double Voltage=CurrentVelocity/MAX_SPEED;
	m_RobotControl->UpdateArmVoltage(Voltage);
	//Show current height (only in AI Tester)
	#if 1
	double Pos_m=GetPos_m();
	double height=AngleToHeight_m(Pos_m);
	DOUT4("Arm=%f Angle=%f %fft %fin",CurrentVelocity,RAD_2_DEG(Pos_m*c_GearToArmRatio),height*3.2808399,height*39.3700787);
	#endif
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

void FRC_2011_Robot::Robot_Arm::ResetPos()
{
	__super::ResetPos();  //Let the super do it stuff first
	if (m_UsingPotentiometer)
	{
		m_PIDController.Reset();
		m_RobotControl->Reset_Arm();
		double NewPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
		Stop();
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
}

void FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety(double Value)
{
	//printf("\r%f       ",Value);
	if (Value < -0.8)
	{
		if (m_UsingPotentiometer)
		{
			//first disable it
			m_UsingPotentiometer=false;
			//Now to reset stuff
			printf("Disabling potentiometer\n");
			//m_PIDController.Reset();
			ResetPos();
			MAX_SPEED=m_MaxSpeedReference;
			m_LastPosition=0.0;
			m_CalibratedScaler=1.0;
			m_LastTime=0.0;
			m_UsingRange=false;
		}
	}
	else
	{
		if (!m_UsingPotentiometer)
		{
			m_UsingPotentiometer=true;
			//setup the initial value with the potentiometers value
			printf("Enabling potentiometer\n");
			ResetPos();
			m_UsingRange=true;
		}
	}
}

double ArmHeightToBack(double value)
{
	const double Vertical=PI/2.0*c_ArmToGearRatio;
	return Vertical + (Vertical-value);
}

void FRC_2011_Robot::Robot_Arm::SetPos0feet()
{
	SetIntendedPosition( HeightToAngle_r(0.0) );
}
void FRC_2011_Robot::Robot_Arm::SetPos3feet()
{
	//Not used, but kept for reference
	//SetIntendedPosition(ArmHeightToBack( HeightToAngle_r(1.143)) );
	SetIntendedPosition(HeightToAngle_r(0.9144));
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
	GG_Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Arm_SetPotentiometerSafety"].Subscribe(ehl,*this, &FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety);
		
		em->Event_Map["Arm_SetPos0feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos0feet);
		em->Event_Map["Arm_SetPos3feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos3feet);
		em->Event_Map["Arm_SetPos6feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos6feet);
		em->Event_Map["Arm_SetPos9feet"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::SetPos9feet);
		em->EventOnOff_Map["Arm_Claw"].Subscribe(ehl, *this, &FRC_2011_Robot::Robot_Arm::CloseClaw);

	}
	else
	{
		em->EventValue_Map["Arm_SetCurrentVelocity"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetRequestedVelocity_FromNormalized);
		em->EventValue_Map["Arm_SetPotentiometerSafety"].Remove(*this, &FRC_2011_Robot::Robot_Arm::SetPotentiometerSafety);
		
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
	Robot_Tank(EntityName), m_RobotControl(robot_control), m_Arm(EntityName,robot_control),m_PIDController_Left(1.0,1.0,0.25),
	m_PIDController_Right(1.0,1.0,0.25),m_UsingEncoders(UseEncoders)
{
	//m_UsingEncoders=true;  //Testing
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=1.0;
}

void FRC_2011_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2011_Robot_Properties *RobotProps=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
	const Ship_1D_Properties *ArmProps=RobotProps?&RobotProps->GetArmProps():NULL;
	m_Arm.Initialize(em,ArmProps);
	m_PIDController_Left.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Left.SetOutputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Left.Enable();
	m_PIDController_Right.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Right.SetOutputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Right.Enable();
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
}
void FRC_2011_Robot::ResetPos()
{
	__super::ResetPos();
	m_Arm.ResetPos();
	m_RobotControl->Reset_Encoders();
	m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
}

void FRC_2011_Robot::TimeChange(double dTime_s)
{
	m_RobotControl->TimeChange(dTime_s);  //This must be first so the simulators can have the correct times
	if (m_UsingEncoders)
	{
		double Encoder_LeftVelocity,Encoder_RightVelocity;
		m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);

		double LeftVelocity=GetLeftVelocity();
		double RightVelocity=GetRightVelocity();

		double control_left=0.0,control_right=0.0;
		//only adjust calibration when both velocities are in the same direction
		if ((LeftVelocity * Encoder_LeftVelocity) > 0.0)
		{
			control_left=m_PIDController_Left(fabs(LeftVelocity),fabs(Encoder_LeftVelocity),dTime_s);
			m_CalibratedScaler_Left=MAX_SPEED * (1.0 + (-control_left));
		}
		if ((RightVelocity * Encoder_RightVelocity) > 0.0)
		{
			control_right=m_PIDController_Right(fabs(RightVelocity),fabs(Encoder_RightVelocity),dTime_s);
			m_CalibratedScaler_Right=MAX_SPEED * (1.0 + (-control_right));
		}

		//Adjust the engaged max speed to avoid the PID from overflow lockup
		ENGAGED_MAX_SPEED=(m_CalibratedScaler_Left+m_CalibratedScaler_Right) / 2.0;
		//DOUT5("cl=%f cr=%f, csl=%f csr=%f",control_left,control_right,m_CalibratedScaler_Left,m_CalibratedScaler_Right);
		//printf("\rcl=%f cr=%f, csl=%f csr=%f                ",control_left,control_right,m_CalibratedScaler_Left,m_CalibratedScaler_Right);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,Encoder_LeftVelocity,RightVelocity,Encoder_RightVelocity);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,m_CalibratedScaler_Left,RightVelocity,m_CalibratedScaler_Right);
		
		//Update the physics with the actual velocity
		Vec2d LocalVelocity;
		double AngularVelocity;
		InterpolateVelocities(Encoder_LeftVelocity,Encoder_RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
		//TODO add gyro's yaw readings for Angular velocity here
		Vec2d GlobalVelocity=LocalToGlobal(GetAtt_r(),LocalVelocity);
		//printf("\rG[0]=%f G[1]=%f        ",GlobalVelocity[0],GlobalVelocity[1]);
		m_Physics.SetLinearVelocity(GlobalVelocity);
	}
	else
	{
		//Display encoders without applying calibration
		double Encoder_LeftVelocity,Encoder_RightVelocity;
		m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
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
	m_RobotControl->UpdateLeftRightVoltage(GetLeftVelocity()/m_CalibratedScaler_Left,GetRightVelocity()/m_CalibratedScaler_Right);
}

void FRC_2011_Robot::CloseDeploymentDoor(bool Close)
{
	m_RobotControl->CloseDeploymentDoor(Close);
}
//void FRC_2011_Robot::ReleaseLazySusan(bool Release)
//{
//	m_RobotControl->ReleaseLazySusan(Release);
//}

void FRC_2011_Robot::BindAdditionalEventControls(bool Bind)
{
	Entity2D::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
	if (Bind)
	{
		em->EventOnOff_Map["Robot_CloseDoor"].Subscribe(ehl, *this, &FRC_2011_Robot::CloseDeploymentDoor);
		//em->EventOnOff_Map["Robot_ReleaseLazySusan"].Subscribe(ehl, *this, &FRC_2011_Robot::ReleaseLazySusan);
	}
	else
	{
		em->EventOnOff_Map["Robot_CloseDoor"]  .Remove(*this, &FRC_2011_Robot::CloseDeploymentDoor);
		//em->EventOnOff_Map["Robot_ReleaseLazySusan"]  .Remove(*this, &FRC_2011_Robot::ReleaseLazySusan);
	}

	Ship_1D &ArmShip_Access=m_Arm;
	ArmShip_Access.BindAdditionalEventControls(Bind);
}

  /***********************************************************************************************************************************/
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

void Robot_Control::Reset_Arm()
{
	m_KalFilter_Arm.Reset();
}

void Robot_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Robot_Control::Initialize(const Entity_Properties *props)
{
	const FRC_2011_Robot_Properties *robot_props=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
	assert(robot_props);
	m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();
	m_ArmMaxSpeed=robot_props->GetArmProps().GetMaxSpeed();
}

void Robot_Control::TimeChange(double dTime_s)
{
	m_Potentiometer.SetTimeDelta(dTime_s);
	m_Encoders.SetTimeDelta(dTime_s);
}

void Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	m_Encoders.GetLeftRightVelocity(LeftVelocity,RightVelocity);
}

void Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	double LeftVoltageToUse=min(LeftVoltage,1.0);
	double RightVoltageToUse=min(RightVoltage,1.0);
	//DOUT2("left=%f right=%f \n",LeftVelocity,RightVelocity);
	DOUT2("left=%f right=%f \n",LeftVoltageToUse,RightVoltageToUse);
	m_Encoders.UpdateLeftRightVoltage(LeftVoltageToUse,RightVoltageToUse);
	m_Encoders.TimeChange();   //have this velocity immediately take effect
}

//const double c_Arm_DeadZone=0.150;  //was 0.085 for out off
const double c_Arm_DeadZone=0.085;   //This has better results
const double c_Arm_Range=1.0-c_Arm_DeadZone;

void Robot_Control::UpdateArmVoltage(double Voltage)
{

	//TODO determine why the deadzone code has adverse results
	//Eliminate the deadzone
	//Voltage=(Voltage * c_Arm_Range) + ((Voltage>0.0) ? c_Arm_DeadZone : -c_Arm_DeadZone); 

	//This prevents the motor from over heating when it is close enough to its destination
	//for the AI simulation, this simulated it not being able to use lower precision to correct
	if (fabs(Voltage)<=c_Arm_DeadZone)
		Voltage=0.0;
	//else
	//	printf("Arm=%f\n",Voltage);

	//float VoltageToUse=min((float)Voltage,1.0f);
	//DOUT5("Arm=%f",Voltage);
	m_Potentiometer.UpdatePotentiometerVoltage(Voltage);
	m_Potentiometer.TimeChange();  //have this velocity immediately take effect
}

double Robot_Control::GetArmCurrentPosition()
{
	double result=m_Potentiometer.GetPotentiometerCurrentPosition()*c_PotentiometerToArmRatio;
	//result = m_KalFilter_Arm(result);  //apply the Kalman filter
	return result;
}

void Robot_Control::CloseClaw(bool Close)
{
	DebugOutput("CloseClaw=%d\n",Close);
}

void Robot_Control::CloseDeploymentDoor(bool Close)
{
	DebugOutput("CloseDeploymentDoor=%d\n",Close);
}

//void Robot_Control::ReleaseLazySusan(bool Release)
//{
//	DebugOutput("ReleaseLazySusan=%d\n",Release);
//}

  /***********************************************************************************************************************************/
 /*													FRC_2011_Robot_Properties														*/
/***********************************************************************************************************************************/

FRC_2011_Robot_Properties::FRC_2011_Robot_Properties() : m_ArmProps(
	"Arm",
	2.0,    //Mass
	0.0,   //Dimension  (this really does not matter for this, there is currently no functionality for this property, although it could impact limits)
	18.0,   //Max Speed
	1.0,1.0, //ACCEL, BRAKE  (These can be ignored)
	24.0,24.0, //Max Acceleration Forward/Reverse  find the balance between being quick enough without jarring the tube out of its grip
	Ship_1D_Properties::eRobotArm,
	c_UsingArmLimits,	//Using the range
	-c_OptimalAngleDn_r*c_ArmToGearRatio,c_OptimalAngleUp_r*c_ArmToGearRatio
	)
{
}
