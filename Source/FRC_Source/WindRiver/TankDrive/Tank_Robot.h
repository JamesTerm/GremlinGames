#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Tank_Drive_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Tank_Drive_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
};

struct Tank_Robot_Props
{
	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar_Left,VoltageScalar_Right;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double LeftPID[3]; //p,i,d
	double RightPID[3]; //p,i,d
	double HeadingLatency; //Should be about 100ms + Input Latency... this will establish intervals to sync up the heading with entity
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double LeftMaxSpeedOffset;	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	double RightMaxSpeedOffset;
	double TankSteering_Tolerance; //used to help controls drive straight
	Vec2D DriveTo_ForceDegradeScalar;  //Used for way point driving in autonomous in conjunction with max force to get better deceleration precision
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen,HasEncoders;  //This property only applies in teleop
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	bool UseAggressiveStop;  //If true, will use adverse force to assist in stopping.
	//Note: I cannot imagine one side ever needing to be different from another (PID can solve if that is true)
	//Currently supporting 4 terms in polynomial equation
	PolynomialEquation_forth_Props Voltage_Terms;  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	PolynomialEquation_forth_Props Force_Terms;
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel_Left,InverseMaxAccel_Right;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel_Left,InverseMaxDecel_Right;  //used for deceleration case
	//This scalars work with the local force directly to be added to the voltage
	double ForwardLinearGainAssist_Scalar;
	//double RightLinearGainAssist_Scalar;  --until we have a take drive that can strafe, this is not needed
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	//for instance the hiking viking has both of these false, while the admiral has the right encoder reversed
	bool LeftEncoderReversed,RightEncoderReversed;
	double Positive_DeadZone_Left,Positive_DeadZone_Right;
	double Negative_DeadZone_Left,Negative_DeadZone_Right;  //These must be in negative form
};

class Tank_Robot_UI;
#ifndef Robot_TesterCode
#define DRIVE_API
#endif

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class DRIVE_API Tank_Robot : public Ship_Tester,
				   public Vehicle_Drive_Common_Interface
{
	public:
		Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		virtual ~Tank_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		void Reset(bool ResetPosition=true);
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		void SetIsAutonomous(bool IsAutonomous);
		virtual void TimeChange(double dTime_s);
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		//Give ability to change properties
		void UpdateTankProps(const Tank_Robot_Props &TankProps);
	protected:
		#ifdef Robot_TesterCode
		friend Tank_Robot_UI;
		#endif
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		#ifdef __UseScalerPID__
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		#endif
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
		virtual void SetAttitude(double radians);  //from ship tester
		virtual Vec2D Get_DriveTo_ForceDegradeScalar() const {return m_TankRobotProps.DriveTo_ForceDegradeScalar;}
		virtual Tank_Drive *CreateDrive() {return new Tank_Drive(this);}
		virtual void DestroyDrive();
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void ResetPos();
		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s) 
			{m_TankSteering.UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,*this,LockShipHeadingToOrientation,dTime_s);
			}
		virtual void BindAdditionalEventControls(bool Bind) 
			{m_TankSteering.BindAdditionalEventControls(Bind,GetEventMap(),ehl);
			}
		//this may need to be overridden for robots that need it on for certain cases like 2012 needing it on for low gear
		virtual bool GetUseAgressiveStop() const;

		static void InitNetworkProperties(const Tank_Robot_Props &props,const Ship_Props &ship_props);  //This will GetVariables of all properties needed to tweak PID and gain assists
		static void NetworkEditProperties(Tank_Robot_Props &props,Ship_Props &ship_props);  //This will GetVariables of all properties needed to tweak PID and gain assists

	protected:  //from Vehicle_Drive_Common_Interface
		virtual const Vec2D &GetWheelDimensions() const {return m_TankRobotProps.WheelDimensions;}
		//Note by default a 6WD Tank Robot is assumed to set length for a 4WD (or half the total length of 6)
		virtual double GetWheelTurningDiameter() const {return m_TankRobotProps.WheelDimensions.length();}
		virtual double Vehicle_Drive_GetAtt_r() const {return GetAtt_r();}
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const {return GetPhysics();}
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW() {return GetPhysics();}
	protected:
		bool m_IsAutonomous;
	private:
		#ifndef Robot_TesterCode
		typedef  Ship_Tester __super;
		#endif
		Tank_Drive_Control_Interface * const m_RobotControl;
		Tank_Drive *m_VehicleDrive;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration (in coast mode)
		double m_ErrorOffset_Left,m_ErrorOffset_Right; //used for calibration in brake (a.k.a. aggressive stop) mode
		bool m_UsingEncoders;
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderAngularVelocity;
		//cache in their native form (I don't want to assume lower level is caching)
		double m_Encoder_LeftVelocity,m_Encoder_RightVelocity;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
		//These help to manage the latency, where the heading will only reflect injection changes on the latency intervals
		double m_Heading;  //We take over the heading from physics
		double m_HeadingUpdateTimer;
		double m_PreviousLeftVelocity,m_PreviousRightVelocity; //used to compute acceleration
		Tank_Steering m_TankSteering;  //adding controls for tank steering
		PolynomialEquation_forth m_VoltagePoly,m_ForcePoly;
	public:
		double GetLeftVelocity() const {return m_VehicleDrive->GetLeftVelocity();}
		double GetRightVelocity() const {return m_VehicleDrive->GetRightVelocity();}
};

#ifndef Robot_TesterCode
typedef Ship_Properties UI_Ship_Properties;
#endif

class DRIVE_API Tank_Robot_Properties : public UI_Ship_Properties
{
	public:
		Tank_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
		#ifdef Robot_TesterCode
		const EncoderSimulation_Props &GetEncoderSimulationProps() const {return m_EncoderSimulation.GetEncoderSimulationProps();}
		EncoderSimulation_Props &EncoderSimulationProps() {return m_EncoderSimulation.EncoderSimulationProps();}
		#endif
	protected:
		Tank_Robot_Props m_TankRobotProps;
	private:
		#ifndef Robot_TesterCode
		typedef Ship_Properties __super;
		#else
		EncoderSimulation_Properties m_EncoderSimulation;
		#endif
};

#ifdef Robot_TesterCode

class DRIVE_API Tank_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Tank_Robot_Control();
		//This is only needed for simulation
		virtual void Tank_Drive_Control_TimeChange(double dTime_s);
		double GetLeftVoltage() const {return m_LeftVoltage;}
		double GetRightVoltage() const {return m_RightVoltage;}
		void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}

		void SetLeftRightReverseDirectionEncoder(bool Left_reverseDirection,bool Right_reverseDirection) 
		{	m_Encoders.SetLeftRightReverseDirectionEncoder(Left_reverseDirection,Right_reverseDirection);
		}
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		double RPS_To_LinearVelocity(double RPS);
	protected:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		Encoder_Tester m_Encoders;
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		//cache voltage values for display
		double m_LeftVoltage,m_RightVoltage;
		bool m_DisplayVoltage;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
	private:
		//Used for diagnostics, but also may be used for path align information
		void InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use
};

#endif
