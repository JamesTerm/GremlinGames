#pragma once


class FRC_2014_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
	#ifdef Robot_TesterCode
	virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl)=0;
	#endif
};

struct FRC_2014_Robot_Props
{
public:
	struct Catapult
	{
		double ArmToGearRatio;
		double PotentiometerToArmRatio;
		double ChipShotAngle;
		double GoalShotAngle;
	} Catapult_Robot_Props;
	struct Intake
	{
		double ArmToGearRatio;
		double PotentiometerToArmRatio;
		double Stowed_Angle;
		double Deployed_Angle;
		double Squirt_Angle;
	} Intake_Robot_Props;
	struct Autonomous_Properties
	{
		double BallTargetDistance; //used to tweak desired distance away from ball for targeting
		bool IsSupportingHotSpot;  //Are we supporting hot spot targeting (this will provide hint of orientation)
	} Autonomous_Props;
	struct BallTargeting
	{
		double CameraOffsetScalar;  //used to tweak the reduced deltas
		double LatencyCounterThreshold;  //Used to control interval of camera samples
	} BallTargeting_Props;
};

class FRC_2014_Robot_Properties : public Tank_Robot_Properties
{
	public:
		FRC_2014_Robot_Properties();
		virtual void LoadFromScript(Scripting::Script& script);

		const Rotary_Properties &GetTurretProps() const {return m_TurretProps;}
		const Rotary_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Rotary_Properties &GetWinchProps() const {return m_WinchProps;}
		const Rotary_Properties &GetIntake_ArmProps() const {return m_Intake_ArmProps;}
		const Rotary_Properties &GetIntakeRollersProps() const {return m_IntakeRollersProps;}

		const Tank_Robot_Properties &GetLowGearProps() const {return m_LowGearProps;}
		const FRC_2014_Robot_Props &GetFRC2014RobotProps() const {return m_FRC2014RobotProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
		const Control_Assignment_Properties &Get_ControlAssignmentProps() const {return m_ControlAssignmentProps;}
	private:
		#ifndef Robot_TesterCode
		typedef Tank_Robot_Properties __super;
		#endif
		Rotary_Properties m_TurretProps,m_PitchRampProps,m_WinchProps,m_Intake_ArmProps,m_IntakeRollersProps;
		Tank_Robot_Properties m_LowGearProps;
		FRC_2014_Robot_Props m_FRC2014RobotProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

const char * const csz_FRC_2014_Robot_SpeedControllerDevices_Enum[] =
{
	"winch","intake_arm_1","intake_arm_2","left_drive_3","right_drive_3","rollers"
};

const char * const csz_FRC_2014_Robot_SolenoidDevices_Enum[] =
{
	"use_low_gear","release_clutch","catcher_shooter","catcher_intake"
};

const char * const csz_FRC_2014_Robot_BoolSensorDevices_Enum[] =
{
	"intake_min_1","intake_max_1","intake_min_2","intake_max_2","catapult_limit"
};


class FRC_2014_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			eWinch,
			eIntakeArm1,
			eIntakeArm2,
			eLeftDrive3,
			eRightDrive3,
			eRollers
		};

		static SpeedControllerDevices GetSpeedControllerDevices_Enum (const char *value)
		{	return Enum_GetValue<SpeedControllerDevices> (value,csz_FRC_2014_Robot_SpeedControllerDevices_Enum,_countof(csz_FRC_2014_Robot_SpeedControllerDevices_Enum));
		}

		enum SolenoidDevices
		{
			eUseLowGear,		//If the OpenSolenoid() is called with true then it should be in low gear; otherwise high gear
			eReleaseClutch,     //If true it is released if false it is engaged
			eCatcherShooter,
			eCatcherIntake
		};

		static SolenoidDevices GetSolenoidDevices_Enum (const char *value)
		{	return Enum_GetValue<SolenoidDevices> (value,csz_FRC_2014_Robot_SolenoidDevices_Enum,_countof(csz_FRC_2014_Robot_SolenoidDevices_Enum));
		}

		enum BoolSensorDevices
		{
			eIntakeMin1,
			eIntakeMax1,
			eIntakeMin2,
			eIntakeMax2,
			eCatapultLimit
		};

		static BoolSensorDevices GetBoolSensorDevices_Enum (const char *value)
		{	return Enum_GetValue<BoolSensorDevices> (value,csz_FRC_2014_Robot_BoolSensorDevices_Enum,_countof(csz_FRC_2014_Robot_BoolSensorDevices_Enum));
		}

		FRC_2014_Robot(const char EntityName[],FRC_2014_Control_Interface *robot_control,bool IsAutonomous=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D_Kind::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		class Turret
		{
			private:
				FRC_2014_Robot * const m_pParent;
				double m_Velocity; //adds all axis velocities then assigns on the time change
			public:
				Turret(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control);
				virtual ~Turret() {}
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
				double GetCurrentVelocity() const {return m_Velocity;}
				virtual void TimeChange(double dTime_s);
			protected:
				void Turret_SetRequestedVelocity(double Velocity) {m_Velocity+=Velocity;}
		};

		class PitchRamp
		{
			private:
				FRC_2014_Robot * const m_pParent;
				double m_Velocity; 
			public:
				PitchRamp(FRC_2014_Robot *pParent,Rotary_Control_Interface *robot_control);
				virtual ~PitchRamp() {}
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
				virtual void ResetPos();
				double GetCurrentVelocity() const {return m_Velocity;}
				virtual void TimeChange(double dTime_s);
			protected:
				void Pitch_SetRequestedVelocity(double Velocity) {m_Velocity+=Velocity;}
		};

		class Winch : public Rotary_Position_Control
		{
			public:
				Winch(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//given the raw potentiometer converts to the arm angle
				double PotentiometerRaw_To_Arm_r(double raw) const;
				void Fire_Catapult(bool ReleaseClutch);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void BindAdditionalEventControls(bool Bind);
				void Advance(bool on);
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
				virtual bool DidHitMaxLimit();

			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				void SetChipShot();
				void SetGoalShot();
				FRC_2014_Robot * const m_pParent;
				bool m_Advance;
		};

		class Intake_Arm : public Rotary_Position_Control
		{
			public:
				Intake_Arm(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//The parent needs to call initialize
				double HeightToAngle_r(double Height_m) const;
				double Arm_AngleToHeight_m(double Angle_r) const;
				double AngleToHeight_m(double Angle_r) const;
				double GetPosRest();
				//given the raw potentiometer converts to the arm angle
				double PotentiometerRaw_To_Arm_r(double raw) const;
				void CloseRist(bool Close);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void BindAdditionalEventControls(bool Bind);
				void Advance(bool on);
				void Retract(bool on);
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);

				virtual bool DidHitMinLimit();
				virtual bool DidHitMaxLimit();
			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Position_Control __super;
				#endif
				void SetStowed();
				void SetDeployed();
				void SetSquirt();  //optional if we need to go back further to eject balls
				FRC_2014_Robot * const m_pParent;
				bool m_Advance, m_Retract;
		};
		class Intake_Rollers : public Rotary_Velocity_Control
		{
			public:
				Intake_Rollers(FRC_2014_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on);
				void Squirt(bool on);
			protected:
				//Intercept the time change to send out voltage
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				#ifndef Robot_TesterCode
				typedef Rotary_Velocity_Control __super;
				#endif
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				FRC_2014_Robot * const m_pParent;
				bool m_Grip,m_Squirt;
		};

	public: //Autonomous public access (wind river has problems with friend technique)
		const FRC_2014_Robot_Properties &GetRobotProps() const;
	protected:
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);
		//used to blend turret and pitch controls into the drive itself
		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s);
	private:
		#ifndef Robot_TesterCode
		typedef  Tank_Robot __super;
		#endif
		FRC_2014_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PitchRamp m_PitchRamp;
		Winch m_Winch;
		Intake_Arm m_Intake_Arm;
		Intake_Rollers m_Intake_Rollers;
		FRC_2014_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Vec2D m_DefensiveKeyPosition;
		double m_LatencyCounter;

		double m_PitchAngle,m_LinearVelocity,m_HangTime;
		double m_YawErrorCorrection,m_PowerErrorCorrection;
		double m_DefensiveKeyNormalizedDistance;
		size_t m_DefaultPresetIndex;
		size_t m_AutonPresetIndex;  //used only because encoder tracking is disabled
		//cached during robot time change and applied to other systems when targeting is true
		double m_YawAngle;

		bool m_DisableTurretTargetingValue;
		bool m_POVSetValve;

		bool m_SetLowGear;
		void SetLowGear(bool on);
		void SetLowGearOn() {SetLowGear(true);}
		void SetLowGearOff() {SetLowGear(false);}
		void SetLowGearValue(double Value);

		bool  m_SetDriverOverride;
		void SetDriverOverride(bool on);

		bool m_IsBallTargeting;
		bool IsBallTargeting() const {return m_IsBallTargeting;}
		void SetBallTargeting(bool on) {m_IsBallTargeting=on;}
		void SetBallTargeting_Off(bool off) {SetBallTargeting(!off);}
		void SetBallTargetingOn() {SetBallTargeting(true);}
		void SetBallTargetingOff() {SetBallTargeting(false);}

		bool m_CatcherShooter;
		void SetCatcherShooter(bool on);
		void SetCatcherShooterOn() {SetCatcherShooter(true);}
		void SetCatcherShooterOff() {SetCatcherShooter(false);}

		bool m_CatcherIntake;
		void SetCatcherIntake(bool on);
		void SetCatcherIntakeOn() {SetCatcherIntake(true);}
		void SetCatcherIntakeOff() {SetCatcherIntake(false);}

		#ifdef Robot_TesterCode
		void TestAutonomous();
		void GoalComplete();
		#endif
};

namespace FRC_2014_Goals
{
	Goal *Get_FRC2014_Autonomous(FRC_2014_Robot *Robot);
};

#undef __TestControlAssignments__
#if defined Robot_TesterCode && !defined __TestControlAssignments__

class FRC_2014_Robot_Control : public FRC_2014_Control_Interface
{
	public:
		FRC_2014_Robot_Control();

		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		FRC_2014_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2014_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}
	protected: //from FRC_2014_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		//Note: This is only for Robot Tester
		virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl);

	protected:
		FRC_2014_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester2 m_Winch_Pot, m_IntakeArm_Pot;
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_WinchVoltage,m_IntakeArmVoltage;
};
#else

class FRC_2014_Robot_Control : public RobotControlCommon, public FRC_2014_Control_Interface
{
	public:
		FRC_2014_Robot_Control(bool UseSafety=true);
		virtual ~FRC_2014_Robot_Control();

		//This is called per enabled session to enable (on not) things dynamically (e.g. compressor)
		void ResetPos();
		#ifndef Robot_TesterCode
		void SetSafety(bool UseSafety) {m_TankRobotControl.SetSafety(UseSafety);}
		#endif

		FRC_2014_Control_Interface &AsControlInterface() {return *this;}

		const FRC_2014_Robot_Properties &GetRobotProps() const {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		virtual bool GetBoolSensorState(size_t index);
		virtual void CloseSolenoid(size_t index,bool Close) {OpenSolenoid(index,!Close);}
		virtual void OpenSolenoid(size_t index,bool Open);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}
	protected: //from RobotControlCommon
		virtual size_t RobotControlCommon_Get_Victor_EnumValue(const char *name) const
		{	return FRC_2014_Robot::GetSpeedControllerDevices_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DigitalInput_EnumValue(const char *name) const  
		{	return FRC_2014_Robot::GetBoolSensorDevices_Enum(name);
		}
		virtual size_t RobotControlCommon_Get_DoubleSolenoid_EnumValue(const char *name) const  
		{	return FRC_2014_Robot::GetSolenoidDevices_Enum(name);
		}
	protected: //from FRC_2014_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		#ifdef Robot_TesterCode
		virtual void BindAdditionalEventControls(bool Bind,GG_Framework::Base::EventMap *em,IEvent::HandlerList &ehl);
		#endif

	protected:
		FRC_2014_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Compressor *m_Compressor;
		double m_WinchVoltage;  //used in simulation but no harm in leaving enabled for wind-river
		//All digital input reads are done on time change and cached to avoid multiple reads to the FPGA
		bool m_Limit_IntakeMin1,m_Limit_IntakeMin2,m_Limit_IntakeMax1,m_Limit_IntakeMax2;
		bool m_Limit_Catapult;
};

#endif //Robot_TesterCode

#ifdef Robot_TesterCode
///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2014_Robot_UI : public FRC_2014_Robot, public FRC_2014_Robot_Control
{
	public:
		FRC_2014_Robot_UI(const char EntityName[]);
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
};
#endif //Robot_TesterCode
