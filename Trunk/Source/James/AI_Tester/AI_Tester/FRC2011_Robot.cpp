#include "stdafx.h"
#include "AI_Tester.h"
namespace AI_Tester
{
	#include "Calibration_Testing.h"
	#include "FRC2011_Robot.h"
}

#undef __DisablePotentiometerCalibration__
const bool c_UsingArmLimits=false;

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
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
	Ship_1D(EntityName),m_RobotControl(robot_control),m_LastPosition(0.0),m_CalibratedScaler(1.0),m_LastTime(0.0)
{
}

void FRC_2011_Robot::Robot_Arm::Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props)
{
	m_LastPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
	__super::Initialize(em,props);
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
	//Note: the order has to be in this order where it grabs the potentiometer position first and then performs the time change and finally updates the
	//new arm velocity.  Doing it this way avoids oscillating if the potentiometer and gear have been calibrated

	//Update the position to where the potentiometer says where it actually is
	#ifndef __DisablePotentiometerCalibration__
	if (m_LastTime!=0.0)
	{
		double LastSpeed=fabs(m_Physics.GetVelocity());  //This is last because the time change has not happened yet
		double NewPosition=m_RobotControl->GetArmCurrentPosition()*c_ArmToGearRatio;
		//The order here is as such where if the potentiometer's distance is greater (in either direction), we'll multiply by a value less than one
		double PotentiometerDistance=fabs(NewPosition-m_LastPosition);
		double PotentiometerSpeed=PotentiometerDistance/m_LastTime;
		m_CalibratedScaler=PotentiometerSpeed!=0.0?LastSpeed/PotentiometerSpeed:1.0;
		//double Discrepancy=PotentiometerSpeed-LastSpeed;
		//DOUT5("pSpeed=%f cal=%f Disc=%f",PotentiometerSpeed,m_CalibratedScaler,Discrepancy);
		SetPos_m(NewPosition);
		m_LastPosition=NewPosition;
	}
	m_LastTime=dTime_s;
	#else
	//Temp testing potentiometer readings without applying to current position
	m_RobotControl->GetArmCurrentPosition();
	#endif
	__super::TimeChange(dTime_s);
	double CurrentVelocity=m_Physics.GetVelocity();
	m_RobotControl->UpdateArmVelocity(CurrentVelocity*m_CalibratedScaler);
	//Show current height (only in AI Tester)
	#if 0
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
	GG_Framework::Base::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
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
	//m_UsingEncoders=true;  //Testing
	m_CalibratedScaler=1.0;
}

void FRC_2011_Robot::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const FRC_2011_Robot_Properties *RobotProps=dynamic_cast<const FRC_2011_Robot_Properties *>(props);
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
	m_RobotControl->TimeChange(dTime_s);  //This must be first so the simulators can have the correct times
	if (m_UsingEncoders)
	{
		Vec2d LocalVelocity;
		double AngularVelocity;
		double Encoder_LeftVelocity,Encoder_RightVelocity;
		m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
		
		InterpolateVelocities(Encoder_LeftVelocity,Encoder_RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
		//The order here is as such where if the encoder's distance is greater (in either direction), we'll multiply by a value less than one
		double EncoderSpeed=LocalVelocity.length();
		double EntitySpeed=m_Physics.GetLinearVelocity().length();
		//When the distance is close enough to zero use the scaled value as before
		m_CalibratedScaler=!IsZero(EntitySpeed)?EncoderSpeed/EntitySpeed:
			m_CalibratedScaler>0.25?m_CalibratedScaler:1.0;  //Hack: be careful not to use a value to close to zero as a scaler otherwise it could deadlock
		ENGAGED_MAX_SPEED=MAX_SPEED*m_CalibratedScaler;
		//DOUT4("scaler=%f Eng=%f",m_CalibratedScaler,ENGAGED_MAX_SPEED);

		#if 1
		GetPhysics().SetLinearVelocity(LocalToGlobal(GetAtt_r(),LocalVelocity));
		GetPhysics().SetAngularVelocity(AngularVelocity);
		#else
		DOUT4("Left=%f Right=%f",LeftVelocity,RightVelocity);
		#endif
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
	m_RobotControl->UpdateLeftRightVoltage(GetLeftVelocity()/ENGAGED_MAX_SPEED,GetRightVelocity()/ENGAGED_MAX_SPEED);
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
	Entity2D::EventMap *em=GetEventMap(); //grrr had to explicitly specify which EventMap
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
 /*															Robot_Control															*/
/***********************************************************************************************************************************/

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
void Robot_Control::UpdateArmVelocity(double Velocity)
{
	//TODO we should submit potentiometer ratio, but for now lets factor out this complexity
	float VelocityToUse=(float)(Velocity/m_ArmMaxSpeed);
	//DOUT4("Arm=%f",Velocity/m_ArmMaxSpeed);
	m_Potentiometer.UpdatePotentiometerVelocity(VelocityToUse);
	m_Potentiometer.TimeChange();  //have this velocity immediately take effect
}

double Robot_Control::GetArmCurrentPosition()
{
	return m_Potentiometer.GetPotentiometerCurrentPosition()*c_GearToArmRatio;
}

void Robot_Control::CloseClaw(bool Close)
{
	DebugOutput("CloseClaw=%d\n",Close);
}

void Robot_Control::OpenDeploymentDoor(bool Open)
{
	DebugOutput("OpenDeploymentDoor=%d\n",Open);
}

void Robot_Control::ReleaseLazySusan(bool Release)
{
	DebugOutput("ReleaseLazySusan=%d\n",Release);
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
