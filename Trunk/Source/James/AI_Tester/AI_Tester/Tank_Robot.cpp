#include "stdafx.h"
#include "AI_Tester.h"
#include "Debug.h"
namespace AI_Tester
{
	#include "PIDController.h"
	#include "Calibration_Testing.h"
	#include "Tank_Robot.h"
	#include "Robot_Control_Interface.h"
	#include "Rotary_System.h"
}

#ifdef AI_TesterCode
using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi2=M_PI*2.0;
#else
using namespace Framework::Base;
using namespace std;
#endif

  /***********************************************************************************************************************************/
 /*																Tank_Robot															*/
/***********************************************************************************************************************************/
Tank_Robot::Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool IsAutonomous) :
	Ship_Tester(EntityName), 
	m_IsAutonomous(IsAutonomous),m_RobotControl(robot_control),m_VehicleDrive(NULL),
	m_PIDController_Left(0.0,0.0,0.0),	m_PIDController_Right(0.0,0.0,0.0),  //these will be overridden in properties
	m_ErrorOffset_Left(0.0),m_ErrorOffset_Right(0.0),
	m_UsingEncoders(IsAutonomous),
	m_Heading(0.0), m_HeadingUpdateTimer(0.0),
	m_PreviousLeftVelocity(0.0),m_PreviousRightVelocity(0.0)
{
	m_Physics.SetHeadingToUse(&m_Heading);  //We manage the heading
	//m_UsingEncoders=true; //testing
}

void Tank_Robot::DestroyDrive() 
{
	delete m_VehicleDrive;
	m_VehicleDrive=NULL;
}

Tank_Robot::~Tank_Robot()
{
	DestroyDrive();
}

void Tank_Robot::Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props)
{
	m_VehicleDrive=CreateDrive();
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
	m_ErrorOffset_Left=m_ErrorOffset_Right=0.0;
	//This can be dynamically called so we always call it
	SetUseEncoders(!m_TankRobotProps.IsOpen);
	m_TankSteering.SetStraightDeadZone_Tolerance(RobotProps->GetTankRobotProps().TankSteering_Tolerance);
}
void Tank_Robot::Reset(bool ResetPosition)
{
	//This is here in case it is needed typically this is not needed here as other code will call ResetPos() explicitly
	if (ResetPosition)
	{
		ResetPos();
		m_Heading=0.0;
	}
	m_RobotControl->Reset_Encoders();
	m_PIDController_Left.Reset(),m_PIDController_Right.Reset();
	//ensure teleop has these set properly
	m_ErrorOffset_Left=m_ErrorOffset_Right=0.0;
	m_PreviousLeftVelocity=m_PreviousRightVelocity=0.0;
}

void Tank_Robot::SetUseEncoders(bool UseEncoders,bool ResetPosition) 
{
	if (!UseEncoders)
	{
		if (m_UsingEncoders)
		{
			//first disable it
			m_UsingEncoders=false;
			printf("Disabling encoders for %s\n",GetName().c_str());
			//Now to reset stuff
			Reset(ResetPosition);
			m_EncoderGlobalVelocity=Vec2d(0.0,0.0);
		}
	}
	else
	{
		if (!m_UsingEncoders)
		{
			m_UsingEncoders=true;
			printf("Enabling encoders for %s\n",GetName().c_str());
			//setup the initial value with the encoders value
			Reset(ResetPosition);
		}
	}
}

void Tank_Robot::SetIsAutonomous(bool IsAutonomous)
{
	m_IsAutonomous=IsAutonomous;  //this is important (to disable joystick controls etc)
	//We only explicitly turn them on... not off (that will be configured else where)
	if (IsAutonomous)
		SetUseEncoders(true);
}

void Tank_Robot::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	double Encoder_LeftVelocity,Encoder_RightVelocity;
	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	m_Encoder_LeftVelocity=Encoder_LeftVelocity,m_Encoder_RightVelocity=Encoder_RightVelocity;  //cache for later

	//Note: the capping for the velocities interpreted, this can occur easily when going full speed while starting to turn, the ship's velocity
	//does not count against the velocity if it is turning, and what ends up happening is that we cap off max voltage for one side and the rate of turn
	//may get compromised... this will all be resolved in the injection displacement, and should be fine.  However, we need to cap the speeds here to
	//eliminate getting error, as there really is no error for this case... and when it straightens back out there will not be an overwhelming amount
	//of error to manage.  This was noticeable even using just PD for powerful sudden adjustment in the turn back caused it to over compensate slightly
	//(looked like a critical dampening recovery).
	//  [7/27/2012 JamesK]

	const double LeftVelocity=min(max(m_VehicleDrive->GetLeftVelocity(),-ENGAGED_MAX_SPEED),ENGAGED_MAX_SPEED);
	const double RightVelocity=min(max(m_VehicleDrive->GetRightVelocity(),-ENGAGED_MAX_SPEED),ENGAGED_MAX_SPEED);

	if (m_UsingEncoders)
	{
		m_ErrorOffset_Left=m_PIDController_Left(LeftVelocity,Encoder_LeftVelocity,dTime_s);
		m_ErrorOffset_Right=m_PIDController_Right(RightVelocity,Encoder_RightVelocity,dTime_s);

		const double LeftAcceleration=(LeftVelocity-m_PreviousLeftVelocity)/dTime_s;
		const double RightAcceleration=(RightVelocity-m_PreviousRightVelocity)/dTime_s;
		const bool LeftDecel=(LeftAcceleration * LeftVelocity <= 0);
		const bool RightDecel=(RightAcceleration * RightVelocity <= 0);

		//normalize errors... these will not be reflected for I so it is safe to normalize here to avoid introducing oscillation from P
		//Note: that it is important to bias towards deceleration this can help satisfy both requirements of avoiding oscillation as well
		//As well as avoiding a potential overshoot when trying stop at a precise distance
		m_ErrorOffset_Left=LeftDecel || fabs(m_ErrorOffset_Left)>m_TankRobotProps.PrecisionTolerance?m_ErrorOffset_Left:0.0;
		m_ErrorOffset_Right=RightDecel || fabs(m_ErrorOffset_Right)>m_TankRobotProps.PrecisionTolerance?m_ErrorOffset_Right:0.0;

		//Adjust the engaged max speed to avoid the PID from overflow lockup
		//ENGAGED_MAX_SPEED=(m_CalibratedScaler_Left+m_CalibratedScaler_Right) / 2.0;
		//DOUT5("p=%f e=%f d=%f cs=%f",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		//printf("\rcl=%f cr=%f, csl=%f csr=%f                ",control_left,control_right,m_CalibratedScaler_Left,m_CalibratedScaler_Right);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,Encoder_LeftVelocity,RightVelocity,Encoder_RightVelocity);
		//printf("\rl=%f,%f r=%f,%f       ",LeftVelocity,m_CalibratedScaler_Left,RightVelocity,m_CalibratedScaler_Right);
		//printf("\rp=%f e=%f d=%f cs=%f          ",RightVelocity,Encoder_RightVelocity,RightVelocity-Encoder_RightVelocity,m_CalibratedScaler_Right);
		
		#ifdef __DebugLUA__
		if (m_TankRobotProps.PID_Console_Dump &&  ((Encoder_LeftVelocity!=0.0)||(Encoder_RightVelocity!=0.0)))
		{
			double PosY=GetPos_m()[1];
			printf("y=%.2f p=%.2f e=%.2f eo=%.2f p=%.2f e=%.2f eo=%.2f\n",PosY,LeftVelocity,Encoder_LeftVelocity,m_ErrorOffset_Left,RightVelocity,Encoder_RightVelocity,m_ErrorOffset_Right);
		}
		#endif
	}	
	else
	{
		#ifdef __DebugLUA__
		if (m_TankRobotProps.PID_Console_Dump && ((Encoder_LeftVelocity!=0.0)||(Encoder_RightVelocity!=0.0)))
		{
			double PosY=GetPos_m()[1];
			printf("y=%.2f p=%.2f e=%.2f eo=%.2f p=%.2f e=%.2f eo=%.2f\n",PosY,LeftVelocity,Encoder_LeftVelocity,m_ErrorOffset_Left,RightVelocity,Encoder_RightVelocity,m_ErrorOffset_Right);
		}
		#endif
	}
	
	//Update the physics with the actual velocity
	Vec2d LocalVelocity;
	double AngularVelocity;
	m_VehicleDrive->InterpolateVelocities(Encoder_LeftVelocity,Encoder_RightVelocity,LocalVelocity,AngularVelocity,dTime_s);
	//TODO add gyro's yaw readings for Angular velocity here
	//Store the value here to be picked up in GetOldVelocity()
	m_EncoderGlobalVelocity=LocalToGlobal(GetAtt_r(),LocalVelocity);
	m_EncoderAngularVelocity=AngularVelocity;
	//printf("\rG[0]=%f G[1]=%f        ",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);
	//printf("G[0]=%f G[1]=%f\n",m_EncoderGlobalVelocity[0],m_EncoderGlobalVelocity[1]);

	m_VehicleDrive->InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
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
	//Note: for now there is no passive setting, which would be great for open loop driving while maintaining the position as it was for rebound rumble
	//Instead we can keep the logic simple and only apply displacement if we are using the encoders... this way the simulations of the open loop (lesser stress)
	//will work properly without adding this extra complexity
	//  [8/27/2012 Terminator]
	if (m_UsingEncoders)
	{
		Vec2d computedVelocity=m_Physics.GetLinearVelocity();
		double computedAngularVelocity=m_Physics.GetAngularVelocity();
		m_Physics.SetLinearVelocity(m_EncoderGlobalVelocity);
		m_Physics.SetAngularVelocity(m_EncoderAngularVelocity);
		m_Physics.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
		const double &HeadingLatency=m_TankRobotProps.HeadingLatency;
		if (HeadingLatency!=0.0)
		{	//manage the heading update... 
			m_HeadingUpdateTimer+=DeltaTime_s;
			if (m_HeadingUpdateTimer>HeadingLatency)
			{
				m_HeadingUpdateTimer-=HeadingLatency;
				//This should never happen unless we had a huge delta time (e.g. breakpoint)
				if (m_HeadingUpdateTimer>HeadingLatency)
				{
					m_HeadingUpdateTimer=0.0; //Just reset... it is not critical to have exact interval
					printf("Warning: m_HeadingUpdateTimer>m_TankRobotProps.InputLatency\n");
				}
				//Sync up with our entities heading on this latency interval
				m_Heading=GetAtt_r();
			}
			else
			{
				//Use our original angular velocity for the heading update to avoid having to make corrections
				m_Heading+=computedAngularVelocity*DeltaTime_s;
			}
		}
		else
		{
			m_Heading+=RotationDisplacement;  // else always pull heading from the injected displacement (always in sync with entity)
		}
		//We must set this back so that the PID can compute the entire error
		m_Physics.SetLinearVelocity(computedVelocity);
		m_Physics.SetAngularVelocity(computedAngularVelocity);
		ret=true;
	}
	else
		m_Heading=GetAtt_r();
	if (!ret)
		ret=m_VehicleDrive->InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	return ret;
}

void Tank_Robot::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	m_VehicleDrive->UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	double LeftVelocity=m_VehicleDrive->GetLeftVelocity(),RightVelocity=m_VehicleDrive->GetRightVelocity();
	double LeftVoltage,RightVoltage;

	
		
	#if 0
	double Encoder_LeftVelocity,Encoder_RightVelocity;
	m_RobotControl->GetLeftRightVelocity(Encoder_LeftVelocity,Encoder_RightVelocity);
	DOUT5("left=%f %f Right=%f %f",Encoder_LeftVelocity,LeftVelocity,Encoder_RightVelocity,RightVelocity);
	#endif

	//This shows the simpler computation without the force
	#if 0
	LeftVoltage=(LeftVelocity+m_ErrorOffset_Left)/ (MAX_SPEED + m_TankRobotProps.LeftMaxSpeedOffset);
	RightVoltage=(RightVelocity+m_ErrorOffset_Right)/ (MAX_SPEED + m_TankRobotProps.RightMaxSpeedOffset);
	#else
	//compute acceleration
	const double LeftAcceleration=(LeftVelocity-m_PreviousLeftVelocity)/dTime_s;
	const double RightAcceleration=(RightVelocity-m_PreviousRightVelocity)/dTime_s;
	//Since the mass is not dynamic (like it would be for an arm) we'll absorb the acceleration the final scalar
	//This should be fine for speed control type of rotary systems
	//compute force from the computed mass this is the sum of all moment and weight
	//const double ComputedMass=1.0;
	//const double LeftForce=LeftAcceleration*ComputedMass;
	//const double RightForce=RightAcceleration*ComputedMass;

	//DOUT5("%f %f",LeftAcceleration,RightAcceleration);
	LeftVoltage=(LeftVelocity+m_ErrorOffset_Left)/ (MAX_SPEED + m_TankRobotProps.LeftMaxSpeedOffset);
	RightVoltage=(RightVelocity+m_ErrorOffset_Right)/ (MAX_SPEED + m_TankRobotProps.RightMaxSpeedOffset);
	//Note: we accelerate when both the acceleration and velocity are both going in the same direction so we can multiply them together to determine this
	const bool LeftAccel=(LeftAcceleration * LeftVelocity > 0);
	LeftVoltage+=LeftAcceleration*(LeftAccel? m_TankRobotProps.InverseMaxAccel_Left : m_TankRobotProps.InverseMaxDecel_Left);
	const bool RightAccel=(RightAcceleration * RightVelocity > 0);
	RightVoltage+=RightAcceleration*(RightAccel ? m_TankRobotProps.InverseMaxAccel_Right : m_TankRobotProps.InverseMaxDecel_Right);

	//Keep track of previous velocity to compute acceleration
	m_PreviousLeftVelocity=LeftVelocity,m_PreviousRightVelocity=RightVelocity;
	#endif

	//Apply the polynomial equation to the voltage to linearize the curve
	//Note: equations most-likely will not be symmetrical with the -1 - 0 range so we'll work with the positive range and restore the sign
	{
		double Voltage=fabs(LeftVoltage);
		double *c=m_TankRobotProps.Polynomial;
		double x2=Voltage*Voltage;
		double x3=Voltage*x2;
		double x4=x2*x2;
		Voltage = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*Voltage) + c[0]; 
		Voltage=min(Voltage,1.0); //Clip the voltage as it can become really high values when applying equation
		LeftVoltage=(LeftVoltage<0)?-Voltage:Voltage; //restore sign

		Voltage=fabs(RightVoltage);
		x2=Voltage*Voltage;
		x3=Voltage*x2;
		x4=x2*x2;
		Voltage = (c[4]*x4) + (c[3]*x3) + (c[2]*x2) + (c[1]*Voltage) + c[0]; 
		Voltage=min(Voltage,1.0); //Clip the voltage as it can become really high values when applying equation
		RightVoltage=(RightVoltage<0)?-Voltage:Voltage; //restore sign
	}
	
	{  //Dead zone management
		//The dead zone is only used when accelerating and the encoder reads no movement it does not skew the rest of the values like
		//a typical dead zone adjustment since all other values are assumed to be calibrated in the polynomial equation above tested against
		//a steady state of many points.  The dead zone is something that should found empirically and is easy to determine in a PID dump
		//when using no dead zone when looking at the very beginning of acceleration

		//left side
		if ((IsZero(m_Encoder_LeftVelocity)) && LeftAccel)
			ComputeDeadZone(LeftVoltage,m_TankRobotProps.Positive_DeadZone_Left,m_TankRobotProps.Negative_DeadZone_Left);
		//right side
		if ((IsZero(m_Encoder_RightVelocity)) && RightAccel)
			ComputeDeadZone(RightVoltage,m_TankRobotProps.Positive_DeadZone_Right,m_TankRobotProps.Negative_DeadZone_Right);
	}

	//if (fabs(RightVoltage)>0.0) printf("RV %f dzk=%d ",RightVoltage,m_UseDeadZoneSkip);

	#ifdef __DebugLUA__
	if (m_TankRobotProps.PID_Console_Dump && ((LeftVoltage!=0.0)||(RightVoltage!=0.0)))
		printf("v=%.2f v=%.2f ",LeftVoltage,RightVoltage);
	#endif

	m_RobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);
}

void Tank_Robot::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	m_VehicleDrive->ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	//We are not going to use these interpolated values in the control (it would corrupt it)... however we can monitor them here, or choose to
	//view them here as needed
	Vec2D force;
	double torque;
	InterpolateThrusterChanges(force,torque,dTime_s);
	__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
}

void Tank_Robot::ResetPos()
{
	m_VehicleDrive->ResetPos();
	__super::ResetPos();
}

void Tank_Robot::SetAttitude(double radians)
{
	m_Heading=radians;
	__super::SetAttitude(radians);
}

void Tank_Robot::UpdateTankProps(const Tank_Robot_Props &TankProps)
{
	//This is very similar to Initialize() but only for things we are interested in changing safely dynamically
	m_TankRobotProps=TankProps;
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
	m_ErrorOffset_Left=m_ErrorOffset_Right=0.0;
	//This can be dynamically called so we always call it
	SetUseEncoders(!m_TankRobotProps.IsOpen);
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
	props.HeadingLatency=0.0;
	props.MotorToWheelGearRatio=1.0;  //most-likely this will be overridden
	props.VoltageScalar=1.0;  //May need to be reversed
	props.Feedback_DiplayRow=(size_t)-1;  //Only assigned to a row during calibration of feedback sensor
	props.IsOpen=true;  //Always true by default until control is fully functional
	props.PID_Console_Dump=false;  //Always false unless you want to analyze PID (only one system at a time!)
	props.PrecisionTolerance=0.01;  //It is really hard to say what the default should be
	props.LeftMaxSpeedOffset=props.RightMaxSpeedOffset=0.0;
	props.ReverseSteering=false;
	props.Polynomial[0]=0.0;
	props.Polynomial[1]=1.0;
	props.Polynomial[2]=0.0;
	props.Polynomial[3]=0.0;
	props.Polynomial[4]=0.0;
	props.LeftEncoderReversed=false;
	props.RightEncoderReversed=false;
	props.DriveTo_ForceDegradeScalar=Vec2d(1.0,1.0);
	props.TankSteering_Tolerance=0.05;
	props.InverseMaxAccel_Left=props.InverseMaxDecel_Left=0.0;
	props.InverseMaxAccel_Right=props.InverseMaxDecel_Right=0.0;
	props.Positive_DeadZone_Left=props.Positive_DeadZone_Right=0.0;
	props.Negative_DeadZone_Left=props.Negative_DeadZone_Right=0.0;
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
		err = script.GetField("heading_latency", NULL, NULL, &m_TankRobotProps.HeadingLatency);
		script.GetField("left_max_offset", NULL, NULL, &m_TankRobotProps.LeftMaxSpeedOffset);
		script.GetField("right_max_offset", NULL, NULL, &m_TankRobotProps.RightMaxSpeedOffset);
		script.GetField("drive_to_scale", NULL, NULL, &m_TankRobotProps.DriveTo_ForceDegradeScalar[1]);
		script.GetField("strafe_to_scale", NULL, NULL, &m_TankRobotProps.DriveTo_ForceDegradeScalar[0]);

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
		err = script.GetField("reverse_steering",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.ReverseSteering=true;
		}
		err = script.GetField("left_encoder_reversed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.LeftEncoderReversed=true;
		}
		err = script.GetField("right_encoder_reversed",&sTest,NULL,NULL);
		if (!err)
		{
			if ((sTest.c_str()[0]=='y')||(sTest.c_str()[0]=='Y')||(sTest.c_str()[0]=='1'))
				m_TankRobotProps.RightEncoderReversed=true;
		}

		err = script.GetFieldTable("curve_voltage");
		if (!err)
		{
			err = script.GetField("c", NULL, NULL,&m_TankRobotProps.Polynomial[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t1", NULL, NULL,&m_TankRobotProps.Polynomial[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t2", NULL, NULL,&m_TankRobotProps.Polynomial[2]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t3", NULL, NULL,&m_TankRobotProps.Polynomial[3]);
			ASSERT_MSG(!err, err);
			err = script.GetField("t4", NULL, NULL,&m_TankRobotProps.Polynomial[4]);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

		script.GetField("inv_max_accel", NULL, NULL, &m_TankRobotProps.InverseMaxAccel_Left);
		m_TankRobotProps.InverseMaxAccel_Right=m_TankRobotProps.InverseMaxAccel_Left;
		//set up deceleration to be the same value by default
		m_TankRobotProps.InverseMaxDecel_Left=m_TankRobotProps.InverseMaxDecel_Right=m_TankRobotProps.InverseMaxAccel_Left;  
		err = script.GetField("inv_max_decel", NULL, NULL, &m_TankRobotProps.InverseMaxDecel_Left);
		if (!err)
			m_TankRobotProps.InverseMaxDecel_Right=m_TankRobotProps.InverseMaxDecel_Left;

		//and now for the specific cases
		script.GetField("inv_max_accel_left", NULL, NULL, &m_TankRobotProps.InverseMaxAccel_Left);
		script.GetField("inv_max_accel_right", NULL, NULL, &m_TankRobotProps.InverseMaxAccel_Right);
		script.GetField("inv_max_decel_left", NULL, NULL, &m_TankRobotProps.InverseMaxDecel_Left);
		script.GetField("inv_max_decel_right", NULL, NULL, &m_TankRobotProps.InverseMaxDecel_Right);

		script.GetField("forward_deadzone_left", NULL, NULL,&m_TankRobotProps.Positive_DeadZone_Left);
		script.GetField("forward_deadzone_right", NULL, NULL,&m_TankRobotProps.Positive_DeadZone_Right);
		script.GetField("reverse_deadzone_left", NULL, NULL,&m_TankRobotProps.Negative_DeadZone_Left);
		script.GetField("reverse_deadzone_right", NULL, NULL,&m_TankRobotProps.Negative_DeadZone_Right);
		//Ensure the negative settings are negative
		if (m_TankRobotProps.Negative_DeadZone_Left>0.0)
			m_TankRobotProps.Negative_DeadZone_Left=-m_TankRobotProps.Negative_DeadZone_Left;
		if (m_TankRobotProps.Negative_DeadZone_Right>0.0)
			m_TankRobotProps.Negative_DeadZone_Right=-m_TankRobotProps.Negative_DeadZone_Right;
		//TODO may want to swap forward in reverse settings if the voltage multiply is -1  (I'll want to test this as it happens)

		#ifdef AI_TesterCode
		err = script.GetFieldTable("motor_specs");
		if (!err)
		{
			m_EncoderSimulation.LoadFromScript(script);
			script.Pop();
		}
		#endif

		script.Pop(); 
	}
	err = script.GetFieldTable("controls");
	if (!err)
	{
		script.GetField("tank_steering_tolerance", NULL, NULL,&m_TankRobotProps.TankSteering_Tolerance);
		script.Pop();
	}

	__super::LoadFromScript(script);
}

#ifdef AI_TesterCode

  /***********************************************************************************************************************************/
 /*														Tank_Robot_Control															*/
/***********************************************************************************************************************************/

Tank_Robot_Control::Tank_Robot_Control() : m_LeftVoltage(0.0),m_RightVoltage(0.0),m_DisplayVoltage(true)
{
}

void Tank_Robot_Control::Reset_Encoders()
{
	m_KalFilter_EncodeLeft.Reset(),m_KalFilter_EncodeRight.Reset();	
}

void Tank_Robot_Control::Initialize(const Entity_Properties *props)
{
	const Tank_Robot_Properties *robot_props=dynamic_cast<const Tank_Robot_Properties *>(props);

	//For now robot_props can be NULL since the swerve robot is borrowing it
	if (robot_props)
	{
		assert(robot_props);
		m_RobotMaxSpeed=robot_props->GetEngagedMaxSpeed();

		//This will copy all the props
		m_TankRobotProps=robot_props->GetTankRobotProps();
		//We'll try to construct the props to match our properties
		//Note: for max accel it needs to be powerful enough to handle curve equations
		Rotary_Properties props("TankEncoder",2.0,0.0,m_RobotMaxSpeed,1.0,1.0,robot_props->GetMaxAccelForward() * 3.0,robot_props->GetMaxAccelReverse() * 3.0);
		props.RotaryProps().EncoderToRS_Ratio=m_TankRobotProps.MotorToWheelGearRatio;
		#ifdef AI_TesterCode
		props.EncoderSimulationProps()=robot_props->GetEncoderSimulationProps();
		#endif
		m_Encoders.Initialize(&props);

		#if 1
		//Now to set the encoders reverse state
		m_Encoders.SetLeftRightReverseDirectionEncoder(m_TankRobotProps.LeftEncoderReversed,m_TankRobotProps.RightEncoderReversed);
		#else
		//Testing a side imbalance
		m_Encoders.SetLeftRightScalar(1.0,0.94);
		#endif
	}
}

void Tank_Robot_Control::Tank_Drive_Control_TimeChange(double dTime_s)
{
	m_Encoders.SetTimeDelta(dTime_s);
	if (m_DisplayVoltage)
	{
		//display voltages
		DOUT2("l=%f r=%f\n",m_LeftVoltage,m_RightVoltage);
	}
}

double Tank_Robot_Control::RPS_To_LinearVelocity(double RPS)
{
	return RPS * m_TankRobotProps.MotorToWheelGearRatio * M_PI * m_TankRobotProps.WheelDiameter; 
}

void Tank_Robot_Control::GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)
{
	m_Encoders.GetLeftRightVelocity(LeftVelocity,RightVelocity);
	Dout(m_TankRobotProps.Feedback_DiplayRow,"l=%.1f r=%.1f",Meters2Feet(LeftVelocity),Meters2Feet(RightVelocity));
}

void Tank_Robot_Control::UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)
{
	double LeftVoltageToUse=min(LeftVoltage,1.0);
	double RightVoltageToUse=min(RightVoltage,1.0);
	if (!m_TankRobotProps.ReverseSteering)
	{
		m_LeftVoltage=LeftVoltageToUse;
		m_RightVoltage=RightVoltageToUse;
		m_Encoders.UpdateLeftRightVoltage(LeftVoltageToUse * m_TankRobotProps.VoltageScalar,RightVoltageToUse * m_TankRobotProps.VoltageScalar);
	}
	else
	{
		m_LeftVoltage=RightVoltageToUse;
		m_RightVoltage=LeftVoltageToUse;
		m_Encoders.UpdateLeftRightVoltage(RightVoltageToUse * m_TankRobotProps.VoltageScalar,LeftVoltageToUse * m_TankRobotProps.VoltageScalar);
	}
	m_Encoders.TimeChange();   //have this velocity immediately take effect
}


  /***************************************************************************************************************/
 /*												Tank_Wheel_UI													*/
/***************************************************************************************************************/

void Tank_Wheel_UI::Initialize(Entity2D::EventMap& em, const Wheel_Properties *props)
{
	m_props=*props;
	m_Rotation=0.0;
}

void Tank_Wheel_UI::UI_Init(Actor_Text *parent) 
{
	m_UIParent=parent;

	osg::Vec3 position(0.5*c_Scene_XRes_InPixels,0.5*c_Scene_YRes_InPixels,0.0f);

	m_Tread= new osgText::Text;
	m_Tread->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
	m_Tread->setCharacterSize(m_UIParent->GetFontSize());
	m_Tread->setFontResolution(10,10);
	m_Tread->setPosition(position);
	m_Tread->setAlignment(osgText::Text::CENTER_CENTER);
	m_Tread->setText(L"_");
	m_Tread->setUpdateCallback(m_UIParent);
}

void Tank_Wheel_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	if (AddOrRemove)
		if (m_Tread.valid()) geode->addDrawable(m_Tread);
	else
		if (m_Tread.valid()) geode->removeDrawable(m_Tread);
}

void Tank_Wheel_UI::update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading)
{
	const double FS=m_UIParent->GetFontSize();
	//Vec2d TreadRotPos(0.0,cos(m_Rotation)-0.3);  //good for " font
	Vec2d TreadRotPos(sin(m_Rotation)*-0.25,(cos(m_Rotation)*.5)+0.4);
	const Vec2d TreadOffset(m_props.m_Offset[0]+TreadRotPos[0],m_props.m_Offset[1]+TreadRotPos[1]);
	const Vec2d TreadLocalOffset=GlobalToLocal(Heading,TreadOffset);
	const osg::Vec3 TreadPos (parent_pos[0]+( TreadLocalOffset[0]*FS),parent_pos[1]+( TreadLocalOffset[1]*FS),parent_pos[2]);

	const double TreadColor=((sin(-m_Rotation) + 1.0)/2.0) * 0.8 + 0.2;
	m_Tread->setColor(osg::Vec4(TreadColor,TreadColor,TreadColor,1.0));

	if (m_Tread.valid())
	{
		m_Tread->setPosition(TreadPos);
		m_Tread->setRotation(FromLW_Rot_Radians(Heading,0.0,0.0));
	}
}

void Tank_Wheel_UI::Text_SizeToUse(double SizeToUse)
{
	if (m_Tread.valid()) m_Tread->setCharacterSize(SizeToUse);
}

void Tank_Wheel_UI::AddRotation(double RadiansToAdd)
{
	m_Rotation+=RadiansToAdd;
	if (m_Rotation>Pi2)
		m_Rotation-=Pi2;
	else if (m_Rotation<-Pi2)
		m_Rotation+=Pi2;
}

  /***************************************************************************************************************/
 /*												Tank_Robot_UI													*/
/***************************************************************************************************************/
void Tank_Robot_UI::UI_Init(Actor_Text *parent)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].UI_Init(parent);
}

void Tank_Robot_UI::Initialize(Entity2D::EventMap& em, const Entity_Properties *props)
{
	Vec2D Offsets[6]=
	{
		Vec2D(-1.6, 2.0),
		Vec2D( 1.6, 2.0),
		Vec2D(-1.6, 0.0),
		Vec2D( 1.6, 0.0),
		Vec2D(-1.6,-2.0),
		Vec2D( 1.6,-2.0),
	};
	for (size_t i=0;i<6;i++)
	{
		Tank_Wheel_UI::Wheel_Properties props;
		props.m_Offset=Offsets[i];
		props.m_Wheel_Diameter=m_TankRobot->GetTankRobotProps().WheelDiameter;
		m_Wheel[i].Initialize(em,&props);
	}
}

void Tank_Robot_UI::custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos)
{
	//just dispatch the update to the wheels (for now)
	for (size_t i=0;i<6;i++)
		m_Wheel[i].update(nv,draw,parent_pos,-m_TankRobot->GetAtt_r());
}

void Tank_Robot_UI::Text_SizeToUse(double SizeToUse)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].Text_SizeToUse(SizeToUse);
}

void Tank_Robot_UI::UpdateScene (osg::Geode *geode, bool AddOrRemove)
{
	for (size_t i=0;i<6;i++)
		m_Wheel[i].UpdateScene(geode,AddOrRemove);
}

void Tank_Robot_UI::TimeChange(double dTime_s)
{
	Tank_Robot &_=*m_TankRobot;
	for (size_t i=0;i<6;i++)
	{
		//For the linear velocities we'll convert to angular velocity and then extract the delta of this slice of time
		const double LinearVelocity=(i&1)?_.GetRightVelocity():_.GetLeftVelocity();
		const double PixelHackScale=m_Wheel[i].GetFontSize()/10.0;  //scale the wheels to be pixel aesthetic
		//Note: for UI... to make it pixel friendly always use 6 inches with the hack and not _.GetTankRobotProps().WheelDiameter
		const double RPS=LinearVelocity /  (PI * Inches2Meters(6.0) * PixelHackScale);
		const double AngularVelocity=RPS * Pi2;
		m_Wheel[i].AddRotation(AngularVelocity*dTime_s);
	}
}

#endif
