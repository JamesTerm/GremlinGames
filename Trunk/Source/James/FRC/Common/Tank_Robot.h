#pragma once
#undef __UseScalerPID__

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
	typedef Framework::Base::Vec2d Vec2D;
	//typedef osg::Vec2d Vec2D;

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double LeftPID[3]; //p,i,d
	double RightPID[3]; //p,i,d
	double InputLatency;  //Used with PID to help avoid oscillation in the error control (We can make one for each if needed)
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double LeftMaxSpeedOffset;	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	double RightMaxSpeedOffset;
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen;  //This property only applies in teleop
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	//Note: I cannot imagine one side ever needing to be different from another (PID can solve if that is true)
	//Currently supporting 4 terms in polynomial equation
	double Polynomial[5];  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	//for instance the hiking viking has both of these false, while the admiral has the right encoder reversed
	bool LeftEncoderReversed,RightEncoderReversed;
};

class Tank_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Tank_Robot : public Tank_Drive
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		virtual void TimeChange(double dTime_s);
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
	protected:
		//friend Tank_Robot_UI;

		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		#ifdef __UseScalerPID__
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		#endif
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual const Vec2D &GetWheelDimensions() const {return m_TankRobotProps.WheelDimensions;}
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	protected:
		bool m_IsAutonomous;
	private:
		typedef  Tank_Drive __super;
		Tank_Drive_Control_Interface * const m_RobotControl;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		#ifdef __UseScalerPID__
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		#else
		double m_ErrorOffset_Left,m_ErrorOffset_Right; //used for calibration
		#endif
		bool m_UsingEncoders;
		#ifdef __UseScalerPID__
		bool m_VoltageOverride;  //when true will kill voltage
		#endif
		bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderHeading;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
		LatencyFilter m_PID_Input_Latency_Left,m_PID_Input_Latency_Right;
};

class Tank_Robot_Properties : public Ship_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		Tank_Robot_Properties();
		virtual void LoadFromScript(Framework::Scripting::Script& script);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	protected:
		Tank_Robot_Props m_TankRobotProps;
	private:
		typedef Ship_Properties __super;
};
