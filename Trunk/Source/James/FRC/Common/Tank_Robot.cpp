#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Vehicle_Drive.h"
#include "PIDController.h"
#include "Tank_Robot.h"

using namespace Framework::Base;
using namespace std;

//namespace Scripting=GG_Framework::Logic::Scripting;
namespace Scripting=Framework::Scripting;


  /***********************************************************************************************************************************/
 /*																Tank_Robot															*/
/***********************************************************************************************************************************/
Tank_Robot::Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool IsAutonomous) : 
	Tank_Drive(EntityName), m_RobotControl(robot_control),
	m_PIDController_Left(0.0,0.0,0.0),	m_PIDController_Right(0.0,0.0,0.0),  //these will be overridden in properties
	#ifdef __UseScalerPID__
	m_UsingEncoders(IsAutonomous),m_IsAutonomous(IsAutonomous),
	m_VoltageOverride(false),
	#else
	m_ErrorOffset_Left(0.0),m_ErrorOffset_Right(0.0),
	m_UsingEncoders(IsAutonomous),m_IsAutonomous(IsAutonomous),
	#endif
	m_UseDeadZoneSkip(true)
{
	//m_UsingEncoders=true; //testing
	#ifdef __UseScalerPID__
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=1.0;
	#endif
}

void Tank_Robot::Initialize(Framework::Base::EventMap& em, const Entity_Properties *props)
{
	__super::Initialize(em,props);
	//TODO construct Arm-Ship1D properties from FRC 2011 Robot properties and pass this into the robot control and arm
	m_RobotControl->Initialize(props);

	const Tank_Robot_Properties *RobotProps=dynamic_cast<const Tank_Robot_Properties *>(props);
	//This will copy all the props
	m_TankRobotProps=RobotProps->GetTankRobotProps();
	m_PIDController_Left.SetPID(m_TankRobotProps.LeftPID[0],m_TankRobotProps.LeftPID[1],m_TankRobotProps.LeftPID[2]);
	m_PIDController_Right.SetPID(m_TankRobotProps.RightPID[0],m_TankRobotProps.RightPID[1],m_TankRobotProps.RightPID[2]);

	const double OutputRange=MAX_SPEED*0.875;  //create a small range
	const double InputRange=20.0;  //create a large enough number that can divide out the voltage and small enough to recover quickly
	m_PIDController_Left.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Left.SetOutputRange(-InputRange,OutputRange);
	m_PIDController_Left.Enable();
	m_PIDController_Right.SetInputRange(-MAX_SPEED,MAX_SPEED);
	m_PIDController_Right.SetOutputRange(-InputRange,OutputRange);
	m_PIDController_Right.Enable();
	#ifdef __UseScalerPID__
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
	#else
	m_ErrorOffset_Left=m_ErrorOffset_Right=0.0;
	#endif
	//This can be dynamically called so we always call it
	SetUseEncoders(!m_TankRobotProps.IsOpen);
}
void Tank_Robot::ResetPos()
{
	__super::ResetPos();
	m_RobotControl->Reset_Encoders();
	m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
	//ensure teleop has these set properly
	#ifdef __UseScalerPID__
	m_CalibratedScaler_Left=m_CalibratedScaler_Right=ENGAGED_MAX_SPEED;
	#else
	m_ErrorOffset_Left=m_ErrorOffset_Right=0.0;
	#endif
	m_UseDeadZoneSkip=true;
}

void Tank_Robot::SetUseEncoders(bool UseEncoders) 
{
	if (!UseEncoders)
	{
		if (m_UsingEncoders)
		{
			//first disable it
			m_UsingEncoders=false;
			//Now to reset stuff
			printf("Disabling encoders for %s\n",GetName().c_str());
			ResetPos();
			m_EncoderGlobalVelocity=Vec2d(0.0,0.0);
		}
	}
	else
	{
		if (!m_UsingEncoders)
		{
			m_UsingEncoders=true;
			//setup the initial value with the potentiometers value
			printf("Enabling encoders for %s\n",GetName().c_str());
			ResetPos();
		}
	}
}

void Tank_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	double Encoder_LeftVelocity,Encoder_RightVelocity;
	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	//Display encoders without applying calibration

	double LeftVelocity=GetLeftVelocity();
	double RightVelocity=GetRightVelocity();

	if (m_UsingEncoders)
	{
		#ifdef __UseScalerPID__
		double control_left=0.0,control_right=0.0;
		//only adjust calibration when both velocities are in the same direction, or in the case where the encoder is stopped which will
		//allow the scaler to normalize if it need to start up again.
		if (((LeftVelocity * Encoder_LeftVelocity) > 0.0) || IsZero(Encoder_LeftVelocity) )
		{
			control_left=-m_PIDController_Left(fabs(LeftVelocity),fabs(Encoder_LeftVelocity),dTime_s);
			m_CalibratedScaler_Left=MAX_SPEED+control_left;
		}
		if (((RightVelocity * Encoder_RightVelocity) > 0.0) || IsZero(Encoder_RightVelocity) )
		{
			control_right=-m_PIDController_Right(fabs(RightVelocity),fabs(Encoder_RightVelocity),dTime_s);
			m_CalibratedScaler_Right=MAX_SPEED+control_right;
		}
		#else
		m_ErrorOffset_Left=m_PIDController_Left(LeftVelocity,Encoder_LeftVelocity,dTime_s);
		m_ErrorOffset_Right=m_PIDController_Right(RightVelocity,Encoder_RightVelocity,dTime_s);
		//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
		m_ErrorOffset_Left=fabs(m_ErrorOffset_Left)>m_TankRobotProps.PrecisionTolerance?m_ErrorOffset_Left:0.0;
		m_ErrorOffset_Right=fabs(m_ErrorOffset_Right)>m_TankRobotProps.PrecisionTolerance?m_ErrorOffset_Right:0.0;
		#endif
		//Adjust the engaged max speed to avoid the PID from overflow lockup
		//ENGAGED_MAX_SPEED=(m_CalibratedScaler_Left+m_CalibratedScaler_Right) / 2.0;
		//DOUT5("p=%f e=%f d=%f cs=%f",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		//printf("\rcl=%f cr=%f, csl=%f csr=%f                ",control_left,control_right,m_CalibratedScaler_Left,m_CalibratedScaler_Right);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,Encoder_LeftVelocity,RightVelocity,Encoder_RightVelocity);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,m_CalibratedScaler_Left,RightVelocity,m_CalibratedScaler_Right);
		//printf("\rp=%f e=%f d=%f cs=%f          ",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		
		#ifdef __UseScalerPID__
		#ifdef __DebugLUA__
		if (m_TankRobotProps.PID_Console_Dump && (RightVelocity!=0.0))
		{
			double PosY=GetPos_m()[1];
			if (!m_VoltageOverride)
				printf("y=%f p=%f e=%f d=%f cs=%f\n",PosY,RightVelocity,Encoder_RightVelocity,fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right-MAX_SPEED);
			else
				printf("y=%f VO p=%f e=%f d=%f cs=%f\n",PosY,RightVelocity,Encoder_RightVelocity,fabs(RightVelocity)-fabs(Encoder_RightVelocity),m_CalibratedScaler_Right-MAX_SPEED);
		}
		#endif
		#else
		#ifdef __DebugLUA__
		if (m_TankRobotProps.PID_Console_Dump && (Encoder_RightVelocity!=0.0))
		{
			double PosY=GetPos_m()[1];
			printf("y=%.2f p=%.2f e=%.2f eo=%.2f p=%.2f e=%.2f eo=%.2f\n",PosY,LeftVelocity,Encoder_LeftVelocity,m_ErrorOffset_Left,RightVelocity,Encoder_RightVelocity,m_ErrorOffset_Right);
		}
		#endif

		#endif
		//For most cases we do not need the dead zone skip
		m_UseDeadZoneSkip=false;

		//For now I'm taking this out... I think it will matter on the ramp to be able to apply slight voltages!
		#ifdef __UseScalerPID__
		//We only use deadzone when we are accelerating in either direction, so first check that both sides are going in the same direction
		//also only apply for lower speeds to avoid choppyness during the cruising phase
		if ((RightVelocity*LeftVelocity > 0.0) && (fabs(Encoder_RightVelocity)<0.5))
		{
			//both sides of velocities are going in the same direction we only need to test one side to determine if it is accelerating
			m_UseDeadZoneSkip=(RightVelocity<0) ? (RightVelocity<Encoder_RightVelocity) :  (RightVelocity>Encoder_RightVelocity); 
		}
		#endif
	}	

	//Update the physics with the actual velocity
	Vec2d LocalVelocity;
	double AngularVelocity;
	InterpolateVelocities(Encoder_LeftVelocity,Encoder_RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
	//TODO add gyro's yaw readings for Angular velocity here
	//Store the value here to be picked up in GetOldVelocity()
	m_EncoderGlobalVelocity=LocalToGlobal(GetAtt_r(),LocalVelocity);
	m_EncoderHeading=AngularVelocity;
	//printf("\rG[0]=%f G[1]=%f        ",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);
	//printf("G[0]=%f G[1]=%f\n",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);

	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Tank_Robot::TimeChange(double dTime_s)
{
	//For the simulated code this must be first so the simulators can have the correct times
	m_RobotControl->Tank_Drive_Control_TimeChange(dTime_s);
	__super::TimeChange(dTime_s);
}

bool Tank_Robot::InjectDisplacement(double DeltaTime_s,Vec2d &PositionDisplacement,double &RotationDisplacement)
{
	bool ret=false;
	const bool UpdateDisplacement=true;
	if (UpdateDisplacement)
	{
		Vec2d computedVelocity=m_Physics.GetLinearVelocity();
		double computedAngularVelocity=m_Physics.GetAngularVelocity();
		m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
		m_Physics.SetAngularVelocity(m_EncoderHeading);
		m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
		//We must set this back so that the PID can compute the entire error
		m_Physics.SetLinearVelocity(computedVelocity);
		m_Physics.SetAngularVelocity(computedAngularVelocity);
		ret=true;
	}
	if (!ret)
		ret=__super::InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	return ret;
}

#ifdef __UseScalerPID__
void Tank_Robot::RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s)
{
	m_VoltageOverride=false;
	if ((m_UsingEncoders)&&(VelocityToUse==0.0)&&(m_rotDisplacement_rad==0.0))
			m_VoltageOverride=true;
}
#endif

//I'm leaving this as a template for derived classes... doing it this way is efficient in that it can use all constants
const double c_rMotorDriveForward_DeadZone=0.0;
const double c_rMotorDriveReverse_DeadZone=0.0;
const double c_lMotorDriveForward_DeadZone=0.0;
const double c_lMotorDriveReverse_DeadZone=0.0;

const double c_rMotorDriveForward_Range=1.0-c_rMotorDriveForward_DeadZone;
const double c_rMotorDriveReverse_Range=1.0-c_rMotorDriveReverse_DeadZone;
const double c_lMotorDriveForward_Range=1.0-c_lMotorDriveForward_DeadZone;
const double c_lMotorDriveReverse_Range=1.0-c_lMotorDriveReverse_DeadZone;

void Tank_Robot::ComputeDeadZone(double &LeftVoltage,double &RightVoltage)
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

void Tank_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	__super::UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	double LeftVelocity=GetLeftVelocity(),RightVelocity=GetRightVelocity();
	double LeftVoltage,RightVoltage;

	#ifdef __UseScalerPID__
	if (m_VoltageOverride)
		LeftVoltage=RightVoltage=0;
	else
	#endif
	{
		{
			#if 0
			double Encoder_LeftVelocity,Encoder_RightVelocity;
			m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
			DOUT5("left=%f %f Right=%f %f",Encoder_LeftVelocity,LeftVelocity,Encoder_RightVelocity,RightVelocity);
			#endif

			#ifdef __UseScalerPID__
			//printf("\r%f %f           ",m_CalibratedScaler_Left,m_CalibratedScaler_Right);
			LeftVoltage=LeftVelocity/m_CalibratedScaler_Left,RightVoltage=RightVelocity/m_CalibratedScaler_Right;
			#else
			LeftVoltage=(LeftVelocity+m_ErrorOffset_Left)/MAX_SPEED,RightVoltage=(RightVelocity+m_ErrorOffset_Right)/MAX_SPEED;
			#endif

			//In teleop always square as it feels right and gives more control to the user

			#ifdef __UseScalerPID__
			//for autonomous (i.e. using encoders) the natural distribution on acceleration will give the best results
			//we can use the m_UseDeadZoneSkip to determine if we are accelerating, more important we must square on
			//deceleration to improve our chance to not overshoot!
			if ((!m_UsingEncoders) || (!m_UseDeadZoneSkip))
			#else
			//Once the new PID system is working the deceleration does not need to worry about overshooting
			if (!m_UsingEncoders)
			#endif
			{
				LeftVoltage*=LeftVoltage,RightVoltage*=RightVoltage;  //square them for more give
				//Clip the voltage as it can become really high values when squaring
				if (LeftVoltage>1.0)
					LeftVoltage=1.0;
				if (RightVoltage>1.0)
					RightVoltage=1.0;
				//restore the sign
				if (LeftVelocity<0)
					LeftVoltage=-LeftVoltage;
				if (RightVelocity<0)
					RightVoltage=-RightVoltage;
			}
		}
		// m_UseDeadZoneSkip,  When true this is ideal for telop, and for acceleration in autonomous as it always starts movement
		// equally on both sides, and avoids stalls.  For deceleration in autonomous, set to false as using the correct 
		// linear distribution of voltage will help avoid over-compensation, especially as it gets closer to stopping
		if (m_UseDeadZoneSkip)
			ComputeDeadZone(LeftVoltage,RightVoltage);
	}

	//if (fabs(RightVoltage)>0.0) printf("RV %f dzk=%d ",RightVoltage,m_UseDeadZoneSkip);
	m_RobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

  /***********************************************************************************************************************************/
 /*													Tank_Robot_Properties															*/
/***********************************************************************************************************************************/

Tank_Robot_Properties::Tank_Robot_Properties()
{
	Tank_Robot_Props props;
	memset(&props,0,sizeof(Tank_Robot_Props));

	//Late assign this to override the initial default
	props.WheelDimensions=Vec2D(0.4953,0.6985); //27.5 x 19.5 where length is in 5 inches in, and width is 3 on each side
	const double c_WheelDiameter=0.1524;  //6 inches
	props.WheelDiameter=c_WheelDiameter;
	props.LeftPID[0]=props.RightPID[0]=1.0; //set PIDs to a safe default of 1,0,0
	props.MotorToWheelGearRatio=1.0;  //most-likely this will be overridden
	props.VoltageScalar=1.0;  //May need to be reversed
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.IsOpen=false;  //Always false when control is fully functional
	props.PID_Console_Dump=false;  //Always false unless you want to analyze PID (only one system at a time!)
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	m_TankRobotProps=props;
}

void Tank_Robot_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	err = script.GetFieldTable("tank_drive");
	if (!err) 
	{
		//Quick snap shot of all the properties
		//Vec2D WheelDimensions;
		//double WheelDiameter;
		//double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
		//double LeftPID[3]; //p,i,d
		//double RightPID[3]; //p,i,d
		//Get the ship dimensions
		err = script.GetFieldTable("wheel_base_dimensions");
		if (!err)
		{
			double length_in, width_in;	
			//If someone is going through the trouble of providing the dimension field I should expect them to provide all the fields!
			err = script.GetField("length_in", NULL, NULL,&length_in);
			ASSERT_MSG(!err, err);
			err = script.GetField("width_in", NULL, NULL,&width_in);
			ASSERT_MSG(!err, err);
			m_TankRobotProps.WheelDimensions=Vec2D(Inches2Meters(width_in),Inches2Meters(length_in));  //x,y  where x=width
			script.Pop();
		}

		double wheel_diameter;
		err=script.GetField("wheel_diameter_in", NULL, NULL, &wheel_diameter);
		if (!err)
			m_TankRobotProps.WheelDiameter=Inches2Meters(wheel_diameter);
		script.GetField("encoder_to_wheel_ratio", NULL, NULL, &m_TankRobotProps.MotorToWheelGearRatio);
		script.GetField("voltage_multiply", NULL, NULL, &m_TankRobotProps.VoltageScalar);
		err = script.GetFieldTable("left_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_TankRobotProps.LeftPID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_TankRobotProps.LeftPID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_TankRobotProps.LeftPID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		err = script.GetFieldTable("right_pid");
		if (!err)
		{
			err = script.GetField("p", NULL, NULL,&m_TankRobotProps.RightPID[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("i", NULL, NULL,&m_TankRobotProps.RightPID[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("d", NULL, NULL,&m_TankRobotProps.RightPID[2]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}
		script.GetField("tolerance", NULL, NULL, &m_TankRobotProps.PrecisionTolerance);

		double fDisplayRow;
		err=script.GetField("ds_display_row", NULL, NULL, &fDisplayRow);
		if (!err)
			m_TankRobotProps.Feedback_DiplayRow=(size_t)fDisplayRow;

		string sTest;
		err = script.GetField("is_closed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='n')||(sTest.c_str()[0]=='N')||(sTest.c_str()[0]=='0'))
				m_TankRobotProps.IsOpen=true;
			else
				m_TankRobotProps.IsOpen=false;
		}
		err = script.GetField("show_pid_dump",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.PID_Console_Dump=true;
		}

		script.Pop(); 
	}
	__super::LoadFromScript(script);
}

