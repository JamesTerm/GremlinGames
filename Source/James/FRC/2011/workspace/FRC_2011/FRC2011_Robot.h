#pragma once

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

		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
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
				void OpenClaw(bool Open);
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				typedef Ship_1D __super;
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
				virtual void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
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
				typedef Ship_1D __super;
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
		typedef  Robot_Tank __super;
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

class FRC_2011_Robot_Properties : public Ship_Properties
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
