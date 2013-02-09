#pragma once


class FRC_2013_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
	#ifdef AI_TesterCode
	virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl)=0;
	#endif
};

struct FRC_2013_Robot_Props
{
public:
	//typedef Framework::Base::Vec2d Vec2D;
	typedef osg::Vec2d Vec2D;
	
	Vec2D PresetPositions[3];
	Vec2D KeyGrid[6][3];
	struct DeliveryCorrectionFields
	{
		double PowerCorrection;
		double PitchCorrection;
	};
	DeliveryCorrectionFields KeyCorrections[6][3];
	double FireTriggerDelay;		//Time for stable signal before triggering the fire
	double FireButtonStayOn_Time;   //Time to stay on before stopping the conveyors
	size_t Coordinates_DiplayRow;
	size_t TargetVars_DisplayRow;
	size_t PowerVelocity_DisplayRow;
	double YawTolerance;			//Used for drive yaw targeting (the drive is the turret) to avoid oscillation
	double Min_IntakeDrop;			//Used to determine the minimum drop of the intake during a fire operation

	struct Climb_Properties
	{
		//In theory lift and drop should be the same they can be negative in direction as well.  They may work where lift goes in one directions and the drop goes in the opposite
		double LiftDistance;
		double DropDistance;
	} Climb_Props;
	struct Autonomous_Properties
	{
		Vec2D RampLeft_ErrorCorrection_Offset;
		Vec2D RampRight_ErrorCorrection_Offset;
		Vec2D RampCenter_ErrorCorrection_Offset;
		double XLeftArc,XRightArc;
		struct WaitForBall_Info
		{
			double InitialWait;
			double TimeOutWait;			//If -1 then it is infinite and will not multi task a wait time (great for testing)
			double ToleranceThreshold;  //If zero then only the initial wait is used for each ball (or not using the wait for ball feature)
		} FirstBall_Wait,SecondBall_Wait; //We'll want to tweak the second ball a bit differently
		double MoveForward;				//Optional to move forward to use less power to shoot
		double TwoShotScaler;			//Scaler used for two point shots
	} Autonomous_Props;
};

class FRC_2013_Robot_Properties : public Tank_Robot_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		FRC_2013_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const Rotary_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Rotary_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Rotary_Properties &GetPowerSlowWheelProps() const {return m_PowerSlowWheelProps;}
		const Rotary_Properties &GetHelixProps() const {return m_HelixProps;}
		const Rotary_Properties &GetRollersProps() const {return m_RollersProps;}
		const Rotary_Properties &GetIntakeDeploymentProps() const {return m_IntakeDeploymentProps;}
		const Tank_Robot_Properties &GetClimbGearLiftProps() const {return m_ClimbGearLiftProps;}
		const Tank_Robot_Properties &GetClimbGearDropProps() const {return m_ClimbGearDropProps;}
		const FRC_2013_Robot_Props &GetFRC2013RobotProps() const {return m_FRC2013RobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		//typedef Tank_Robot_Properties __super;
		Rotary_Properties m_PitchRampProps,m_PowerWheelProps,m_PowerSlowWheelProps,m_HelixProps,m_RollersProps,m_IntakeDeploymentProps;
		Tank_Robot_Properties m_ClimbGearLiftProps;
		Tank_Robot_Properties m_ClimbGearDropProps;
		FRC_2013_Robot_Props m_FRC2013RobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

class FRC_2013_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			ePitchRamp,
			ePowerWheelFirstStage,
			ePowerWheelSecondStage,
			eHelix,
			eIntake_Deployment,
			eRollers
		};

		//Most likely will not need IR sensors
		enum BoolSensorDevices
		{
			eTest_Sensor
		};

		//Note: these shouldn't be written to directly but instead set the climb state which then will write to these
		//This will guarantee that they will be preserved in a mutually exclusive state
		enum SolenoidDevices
		{
			//These 3 cylinders work together:
			//neutral state is when all all neutral, and will be a transitional state; otherwise these are mutually exclusive
			eEngageDriveTrain,		//Cylinder 1 is on the drive train gear box
			eEngageLiftWinch,		//Cylinder 2 is on the lift winch
			eEngageDropWinch,		//Cylinder 3 is on the drop winch
			eFirePiston				//pneumatic 4... will engage on fire button per press
		};

		//You use a variation of selected states to do things
		//so driving would be:

		//cylinder 1 = drive
		//2 = neutral
		//3 = neutral

		//raising lift is
		//1 = neutral
		//2 = engaged
		//3 = neutral

		//dropping lift for climb
		//1 = neutral
		//2 = neutral
		//3= engaged
		enum ClimbState
		{
			eClimbState_Neutral,
			eClimbState_Drive,
			eClimbState_RaiseLift,
			//Drop lift has been divided into two phases... we'll want to give time for it to engage which the lift winch is still engaged, before releasing the lift winch
			eClimbState_DropLift,
			eClimbState_DropLift2
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		enum Targets
		{
			eCenterHighGoal,
			eLeftGoal,
			eRightGoal,
			eDefensiveKey
		};
		FRC_2013_Robot(const char EntityName[],FRC_2013_Control_Interface *robot_control,bool IsAutonomous=false);
		virtual~FRC_2013_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:

		class PitchRamp : public Rotary_Position_Control
		{
			public:
				PitchRamp(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				//typedef Rotary_Position_Control __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition_Plus(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2013_Robot * const m_pParent;
		};

		class PowerWheels
		{
			public:
				PowerWheels(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);

				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
				const Rotary_Velocity_Control &GetFirstStageShooter() const {return m_FirstStage;}
				const Rotary_Velocity_Control &GetSecondStageShooter() const {return m_SecondStage;}
				void TimeChange(double dTime_s);
				bool GetIsRunning() const {return m_IsRunning;}
			protected:
				//typedef Rotary_Velocity_Control __super;
				void SetRequestedVelocity_FromNormalized(double Velocity) {m_ManualVelocity=Velocity;}
				void SetRequestedVelocity_Axis_FromNormalized(double Velocity) {m_ManualAcceleration=Velocity;}
				void Set_FirstStage_RequestedVelocity_FromNormalized(double Velocity) {m_FirstStageManualVelocity=Velocity;}
				void SetEncoderSafety(bool DisableFeedback);
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
			private:
				FRC_2013_Robot * const m_pParent;
				Rotary_Velocity_Control m_SecondStage,m_FirstStage;
				double m_ManualVelocity,m_FirstStageManualVelocity;
				double m_ManualAcceleration;
				bool m_IsRunning;
		};

		class IntakeSystem
		{
			private:
				FRC_2013_Robot * const m_pParent;
				Rotary_Velocity_Control m_Helix,m_Rollers;
				class Intake_Deployment : public Rotary_Position_Control
				{
				private:
					//typedef Rotary_Position_Control __super;
					FRC_2013_Robot * const m_pParent;
					bool m_Advance,m_Retract;
					bool m_ChooseDropped; //cache last state that was used
				public:
					Intake_Deployment(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
					IEvent::HandlerList ehl;
					virtual void BindAdditionalEventControls(bool Bind);
					void SetIntendedPosition(double Position);
					bool GetChooseDropped() const {return m_ChooseDropped;}
				protected:
					void Advance();
					void Retract();

					//typedef Rotary_Position_Control __super;
					//events are a bit picky on what to subscribe so we'll just wrap from here
					void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

					void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
					virtual void TimeChange(double dTime_s);
				} m_IntakeDeployment;

				double m_FireDelayTrigger_Time; //Time counter of the value remaining in the on-to-delay state
				double m_FireStayOn_Time;  //Time counter of the value remaining in the on state
				bool m_FireDelayTriggerOn; //A valve mechanism that must meet time requirement to disable the delay
				bool m_FireStayOn;		   //A valve mechanism that must time out to turn off
				union ControlSignals
				{
					struct ControlSignals_rw
					{
						unsigned char Grip   : 1;
						unsigned char Squirt : 1;
						unsigned char Fire   : 1;
						unsigned char GripH  : 1;
					} bits;
					unsigned char raw;
				} m_ControlSignals;
			public:
				IntakeSystem(FRC_2013_Robot *pParent,Rotary_Control_Interface *robot_control);
				void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_ControlSignals.bits.Fire==1;}
				IEvent::HandlerList ehl;

				void ResetPos();
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				void BindAdditionalEventControls(bool Bind);
				//Expose for goals
				void Fire(bool on) {m_ControlSignals.bits.Fire=on;}
				void Squirt(bool on) {m_ControlSignals.bits.Squirt=on;}

			protected:
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_ControlSignals.bits.Grip=on;}

				void SetRequestedVelocity_FromNormalized(double Velocity);
		};

	public: //Autonomous public access (wind river has problems with friend technique)
		IntakeSystem &GetIntakeSystem();
		PowerWheels &GetPowerWheels();
		void SetTarget(Targets target);
		const FRC_2013_Robot_Properties &GetRobotProps() const;
		void SetClimbState(ClimbState climb_state);
		bool IsStopped() const;  //returns true if both encoders read zero on this iteration
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy);
	private:
		void ApplyErrorCorrection();
		//typedef  Tank_Robot __super;
		FRC_2013_Control_Interface * const m_RobotControl;
		PitchRamp m_PitchRamp;
		PowerWheels m_PowerWheels;
		IntakeSystem m_IntakeSystem;
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Targets m_Target;		//This allows us to change our target
		Vec2D m_DefensiveKeyPosition;
		void *m_UDP_Listener;

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  //2d top view x,y of the target
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;  //1d z height (front view) of the target
		//cached during robot time change and applied to other systems when targeting is true
		double m_PitchAngle,m_LinearVelocity,m_HangTime;
		double m_PitchErrorCorrection,m_PowerErrorCorrection;
		double m_DefensiveKeyNormalizedDistance;
		size_t m_DefaultPresetIndex;
		size_t m_AutonPresetIndex;  //used only because encoder tracking is disabled
		bool m_POVSetValve;

		bool m_IsTargeting;
		bool IsTargeting() const {return m_IsTargeting;}
		void SetTargeting(bool on) {m_IsTargeting=on;}
		void SetTargetingOn() {SetTargeting(true);}
		void SetTargetingOff() {SetTargeting(false);}
		void SetTargetingValue(double Value);

		enum DriveTargetSelection
		{
			eDrive_NoTarget,
			eDrive_Goal_Yaw,  //as name implies this only rotates (for now)
			eDrive_Frisbee
		};
		DriveTargetSelection m_DriveTargetSelection;

		bool m_SetClimbGear;
		bool m_SetClimbLeft,m_SetClimbRight;
		void SetClimbGear(bool on);
		void SetClimbGear_LeftButton(bool on);
		void SetClimbGear_RightButton(bool on);
		void SetClimbGearOn() {SetClimbGear(true);}
		void SetClimbGearOff() {SetClimbGear(false);}
		
		void SetPresetPOV (double value);

		void SetDefensiveKeyPosition(double NormalizedDistance) {m_DefensiveKeyNormalizedDistance=NormalizedDistance;}
		void SetDefensiveKeyOn();
		void SetDefensiveKeyOff() {m_Target=eCenterHighGoal;}

		void Robot_SetCreepMode(bool on);
};

class FRC_2013_Goals
{
	public:
		static Goal *Get_ShootBalls(FRC_2013_Robot *Robot,bool DoSquirt=false);
		static Goal *Get_FRC2013_Autonomous(FRC_2013_Robot *Robot,size_t KeyIndex,size_t TargetIndex,size_t RampIndex);
		static Goal *Climb(FRC_2013_Robot *Robot);
	private:
		class Fire : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			bool m_Terminate;
			bool m_IsOn;
			bool m_DoSquirt;  //If True it does the feed instead of fire
		public:
			Fire(FRC_2013_Robot &robot, bool On, bool DoSquirt=false);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class WaitForBall : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			double m_Tolerance;
			bool m_Terminate;
		public:
			WaitForBall(FRC_2013_Robot &robot,double Tolerance);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class ResetPosition : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			double m_Timer;  //keep track of how much time has elapsed
			double m_TimeStopped;  //keep track of how much *consecutive* time it has been stopped
			bool m_Terminate;
		public:
			ResetPosition(FRC_2013_Robot &robot);
			virtual void Activate();
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

		class ChangeClimbState : public AtomicGoal
		{
		private:
			FRC_2013_Robot &m_Robot;
			const FRC_2013_Robot::ClimbState m_ClimbState;
			double m_TimeAccrued;
			double m_TimeToWait;
			bool m_Terminate;
		public:
			ChangeClimbState(FRC_2013_Robot &robot,FRC_2013_Robot::ClimbState climb_state,double TimeToTakeEffect_s=1.00);
			virtual void Activate();
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};
};

#undef __TestXAxisServoDump__
class FRC_2013_Robot_Control : public FRC_2013_Control_Interface
{
	public:
		FRC_2013_Robot_Control();
		const FRC_2013_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void OpenSolenoid(size_t index,bool Open);
		virtual bool GetIsSolenoidOpen(size_t index) const;
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}

		#ifndef __TestXAxisServoDump__
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		#else
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		#endif

		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}

	protected: //from FRC_2013_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		//Note: This is only for AI Tester
		virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl);

		void TriggerFire(bool on) {m_FireSensor=on;}
		void SlowWheel(bool on) {m_SlowWheel=on;}

	protected:
		FRC_2013_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester2 m_Pitch_Pot,m_IntakeDeployment_Pot; //simulate the potentiometer and motor
		Encoder_Simulator m_PowerWheel_Enc,m_PowerSlowWheel_Enc,m_Helix_Enc,m_Rollers_Enc;  //simulate the encoder and motor
		KalmanFilter m_KalFilter_Arm;
		#ifdef __TestXAxisServoDump__
		double m_LastYawAxisSetting;  //needed to creep up the angle to position smoothly when testing servo code
		double m_LastLeftVelocity,m_LastRightVelocity;
		#endif
		//cache voltage values for display
		double m_PitchRampVoltage,m_PowerWheelVoltage,m_PowerSlowWheelVoltage,m_IntakeDeploymentVoltage;
		double m_HelixVoltage,m_RollersVoltage;
		double m_dTime_s;  //Stamp the current time delta slice for other functions to use
		bool m_FireSensor;
		bool m_SlowWheel;
		bool m_FirePiston;
};

class FRC_2013_Power_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Power_Wheel_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (it's constant)
};

class FRC_2013_Power_Slow_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Power_Slow_Wheel_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (it's constant)
};

class FRC_2013_Rollers_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Rollers_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
};

class FRC_2013_Fire_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2013_Fire_Conveyor_UI(FRC_2013_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2013_Robot_Control * const m_RobotControl;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2013_Robot_UI : public FRC_2013_Robot, public FRC_2013_Robot_Control
{
	public:
		FRC_2013_Robot_UI(const char EntityName[]);
	protected:
		virtual void TimeChange(double dTime_s);
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos); 
		virtual void Text_SizeToUse(double SizeToUse);
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

	private:
		Tank_Robot_UI m_TankUI;
		FRC_2013_Power_Wheel_UI m_PowerWheelUI;
		FRC_2013_Power_Slow_Wheel_UI m_PowerSlowWheelUI;
		FRC_2013_Rollers_UI m_Rollers;
		FRC_2013_Fire_Conveyor_UI m_Helix;
};
