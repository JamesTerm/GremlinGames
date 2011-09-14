#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Robot_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Arm()=0; 
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void UpdateVoltage(size_t index,double Voltage)=0;
		///This is a implemented by reading the potentiometer and converting its value to correspond to the arm's current angle
		///This is in radians of the arm's gear ratio
		///TODO break this apart to reading pure analog values and have the potentiometer conversion happen within the robot
		virtual double GetArmCurrentPosition()=0;
		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void CloseSolenoid(size_t index,bool Close)=0;  //true=close false=open
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC_2011_Robot : public Robot_Tank
{
	public:
		enum SolenoidDevices
		{
			eDeployment,
			eClaw,
			eRist
		};
		enum SpeedControllerDevices
		{
			eArm,
			eRollers
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		void SetUseEncoders(bool UseEncoders) {m_UsingEncoders=UseEncoders;}
		virtual void TimeChange(double dTime_s);
		static double RPS_To_LinearVelocity(double RPS);
		void CloseDeploymentDoor(bool Close);

		//TODO for now it it does not matter, but the ship 1d does not normalize to 2pi yet, this class would take advantage of this
		//but fortunately there currently is no use for keep track of the rotation, this however will matter if we have things which
		//rotate within the game
		class Robot_Claw : public Ship_1D
		{
			public:
				Robot_Claw(const char EntityName[],Robot_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//public access needed for goals
				void CloseClaw(bool Close);
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				//typedef Ship_1D __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				Robot_Control_Interface * const m_RobotControl;
				bool m_Grip,m_Squirt;
		};
		class Robot_Arm : public Ship_1D
		{
			public:
				Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//The parent needs to call initialize
				virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				static double HeightToAngle_r(double Height_m);
				static double Arm_AngleToHeight_m(double Angle_r);
				static double AngleToHeight_m(double Angle_r);
				static double GetPosRest();
				//given the raw potentiometer converts to the arm angle
				static double PotentiometerRaw_To_Arm_r(double raw);
				void CloseRist(bool Close);
				virtual void ResetPos();
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void PosDisplacementCallback(double posDisplacement_m);
			private:
				//typedef Ship_1D __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetPotentiometerSafety(double Value);
				void SetPosRest();
				void SetPos0feet();
				void SetPos3feet();
				void SetPos6feet();
				void SetPos9feet();
				Robot_Control_Interface * const m_RobotControl;
				PIDController2 m_PIDController;
				double m_LastPosition;  //used for calibration
				double m_CalibratedScaler; //used for calibration
				double m_LastTime; //used for calibration
				double m_MaxSpeedReference; //used for calibration
				bool m_UsingPotentiometer; //dynamically able to turn off (e.g. panic button)
				bool m_VoltageOverride;  //when true will kill voltage
		};

		//Accessors needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
		Robot_Claw &GetClaw() {return m_Claw;}
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		virtual void BindAdditionalEventControls(bool Bind);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
	private:
		//typedef  Robot_Tank __super;
		Robot_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
		Robot_Claw m_Claw;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		bool m_UsingEncoders;
		bool m_VoltageOverride;  //when true will kill voltage
		bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderHeading;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Robot_Control : public Robot_Control_Interface
{
	public:
		Robot_Control(FRC_2011_Robot *Robot);
		//This is only needed for simulation
		virtual void TimeChange(double dTime_s);
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Reset_Arm(); 
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		//virtual void UpdateVoltage(size_t index,double Voltage); this is overridden
		//pacify this by returning its current value
		virtual double GetArmCurrentPosition();
	protected:
		FRC_2011_Robot * const m_Robot;
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		Potentiometer_Tester m_Potentiometer; //simulate a real potentiometer for calibration testing
		Encoder_Tester m_Encoders;
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		//cache voltage values for display
		double m_LeftVoltage,m_RightVoltage,m_ArmVoltage,m_RollerVoltage;
		bool m_Deployment,m_Claw,m_Rist;
};

class Robot_Control_2011 : public Robot_Control
{
	public:
		Robot_Control_2011(FRC_2011_Robot *Robot) : Robot_Control(Robot) {}
		//See FRC_2011_Robot for enumerations
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close);
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2011_Robot_tester : public FRC_2011_Robot, public Robot_Control_2011
{
	public:
		FRC_2011_Robot_tester(const char EntityName[]) : FRC_2011_Robot(EntityName,this),Robot_Control_2011(this) {}
};

class FRC_2011_Robot_Properties : public UI_Ship_Properties
{
	public:
		FRC_2011_Robot_Properties();
		//I'm not going to implement script support mainly due to lack of time, but also this is a specific object that
		//most likely is not going to be sub-classed (i.e. sealed)... if this turns out different later we can implement
		//virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Ship_1D_Properties &GetArmProps() const {return m_ArmProps;}
		const Ship_1D_Properties &GetClawProps() const {return m_ClawProps;}
	private:
		Ship_1D_Properties m_ArmProps,m_ClawProps;
};

class Goal_OperateSolenoid : public AtomicGoal
{
	private:
		FRC_2011_Robot &m_Robot;
		const FRC_2011_Robot::SolenoidDevices m_SolenoidDevice;
		bool m_Terminate;
		bool m_IsClosed;
	public:
		Goal_OperateSolenoid(FRC_2011_Robot &robot,FRC_2011_Robot::SolenoidDevices SolenoidDevice,bool Close);
		virtual void Activate() {m_Status=eActive;}
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate() {m_Terminate=true;}
};
