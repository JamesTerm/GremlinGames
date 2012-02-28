#pragma once


class FRC_2012_Control_Interface :	public Tank_Drive_Control_Interface,
									public Robot_Control_Interface,
									public Rotary_Control_Interface
{
public:
	//This is primarily used for updates to dashboard and driver station during a test build
	virtual void Robot_Control_TimeChange(double dTime_s)=0;
	//We need to pass the properties to the Robot Control to be able to make proper conversions.
	//The client code may cast the properties to obtain the specific data 
	virtual void Initialize(const Entity_Properties *props)=0;
};

struct FRC_2012_Robot_Props
{
public:
	typedef Framework::Base::Vec2d Vec2D;
	//typedef osg::Vec2d Vec2D;
	
	Vec2D PresetPositions[3];
	Vec2D KeyGrid[3][3];
	struct DeliveryCorrectionFields
	{
		double PowerCorrection;
		double YawCorrection;
	};
	DeliveryCorrectionFields KeyCorrections[3][3];
	size_t Coordinates_DiplayRow;
	size_t TargetVars_DisplayRow;
	size_t PowerVelocity_DisplayRow;
};

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		FRC_2012_Robot_Properties();
		virtual void LoadFromScript(Framework::Scripting::Script& script);

		const Rotary_Properties &GetTurretProps() const {return m_TurretProps;}
		const Rotary_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Rotary_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Rotary_Properties &GetConveyorProps() const {return m_ConveyorProps;}
		const Rotary_Properties &GetFlipperProps() const {return m_FlipperProps;}
		const Tank_Robot_Properties &GetLowGearProps() const {return m_LowGearProps;}
		const FRC_2012_Robot_Props &GetFRC2012RobotProps() const {return m_FRC2012RobotProps;}
		struct Control_Props
		{
			std::vector<UI_Controller::Controller_Element_Properties> EventList;
			std::string Controller;
		};
		typedef std::vector<Control_Props> Controls_List;
		const Controls_List &Get_RobotControls() const {return m_RobotControls;}
	private:
		typedef Tank_Robot_Properties __super;
		Rotary_Properties m_TurretProps,m_PitchRampProps,m_PowerWheelProps,m_ConveyorProps,m_FlipperProps;
		Tank_Robot_Properties m_LowGearProps;
		FRC_2012_Robot_Props m_FRC2012RobotProps;
		Controls_List m_RobotControls;
};

class FRC_2012_Goals;

class FRC_2012_Robot : public Tank_Robot
{
	public:
		enum SpeedControllerDevices
		{
			eTurret,
			ePitchRamp,
			ePowerWheels,
			eLowerConveyor,
			eMiddleConveyor,
			eFireConveyor,
			eFlippers
		};

		enum BoolSensorDevices
		{
			eLowerConveyor_Sensor,
			eMiddleConveyor_Sensor,
			eFireConveyor_Sensor
		};

		enum SolenoidDevices
		{
			eUseLowGear,		//If the OpenSolenoid() is called with true then it should be in low gear; otherwise high gear
			eRampDeployment
		};

		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		/// \param DefaultPresetIndex is a key preset index used in autonomous to pick a spot to shoot from
		/// 0=center 1=left 2=right
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,size_t DefaultPresetIndex=0,bool IsAutonomous=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		class Turret : public Rotary_Linear
		{
			private:
				FRC_2012_Robot * const m_pParent;
				double m_Velocity; //adds all axis velocities then assigns on the time change
			public:
				Turret(FRC_2012_Robot *parent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Linear __super;
				void Turret_SetRequestedVelocity(double Velocity) {m_Velocity+=Velocity;}
				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
		};

		class PitchRamp : public Rotary_Linear
		{
			public:
				PitchRamp(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition_Plus(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
		};

		class PowerWheels : public Rotary_Angular
		{
			public:
				PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				typedef Rotary_Angular __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetEncoderSafety(bool DisableFeedback) {__super::SetEncoderSafety(DisableFeedback);}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
				bool m_IsRunning;
		};

		class BallConveyorSystem
		{
			private:
				FRC_2012_Robot * const m_pParent;
				Rotary_Angular m_LowerConveyor,m_MiddleConveyor,m_FireConveyor;
				union ControlSignals
				{
					struct ControlSignals_rw
					{
						unsigned char Grip   : 1;
						unsigned char Squirt : 1;
						unsigned char Fire   : 1;
						unsigned char GripL  : 1;	//Manual grip low, medium, and high
						unsigned char GripM  : 1;
						unsigned char GripH  : 1;
					} bits;
					unsigned char raw;
				} m_ControlSignals;
			public:
				BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				void Initialize(Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_ControlSignals.bits.Fire==1;}
				IEvent::HandlerList ehl;

				void ResetPos() {m_LowerConveyor.ResetPos(),m_MiddleConveyor.ResetPos(),m_FireConveyor.ResetPos();}
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				void BindAdditionalEventControls(bool Bind);
				//Expose for goals
				void Fire(bool on) {m_ControlSignals.bits.Fire=on;}
			protected:
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_ControlSignals.bits.Grip=on;}
				void GripL(bool on) {m_ControlSignals.bits.GripL=on;}
				void GripM(bool on) {m_ControlSignals.bits.GripM=on;}
				void GripH(bool on) {m_ControlSignals.bits.GripH=on;}
				void Squirt(bool on) {m_ControlSignals.bits.Squirt=on;}

				void SetRequestedVelocity_FromNormalized(double Velocity);
		};

		class Flippers : public Rotary_Linear
		{
			private:
				typedef Rotary_Linear __super;
				FRC_2012_Robot * const m_pParent;
				bool m_Advance,m_Retract;
			public:
				Flippers(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:

				void Advance(bool on) {m_Advance=on;}
				void Retract(bool on) {m_Retract=on;}

				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
		};

	public:
		BallConveyorSystem &GetBallConveyorSystem();
		void SetPresetPosition(size_t index,bool IgnoreOrientation=false);
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy);
	private:
		void ApplyErrorCorrection();
		typedef  Tank_Robot __super;
		FRC_2012_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PitchRamp m_PitchRamp;
		PowerWheels m_PowerWheels;
		BallConveyorSystem m_BallConveyorSystem;
		Flippers m_Flippers;
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  //2d top view x,y of the target
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;  //1d z height (front view) of the target
		//cached during robot time change and applied to other systems when targeting is true
		double m_PitchAngle,m_LinearVelocity,m_HangTime;
		double m_YawErrorCorrection,m_PowerErrorCorrection;
		size_t m_DefaultPresetIndex;
		bool m_DisableTurretTargetingValue;
		bool m_POVSetValve;

		bool m_IsTargeting;
		void IsTargeting(bool on) {m_IsTargeting=on;}
		void SetTargetingOn() {IsTargeting(true);}
		void SetTargetingOff() {IsTargeting(false);}
		void SetTurretTargetingOff(bool on) {m_DisableTurretTargetingValue=on;}
		void SetTargetingValue(double Value);

		bool m_SetLowGear;
		void SetLowGear(bool on);
		void SetLowGearOn() {SetLowGear(true);}
		void SetLowGearOff() {SetLowGear(false);}
		void SetLowGearValue(double Value);

		void SetPreset1() {SetPresetPosition(0);}
		void SetPreset2() {SetPresetPosition(1);}
		void SetPreset3() {SetPresetPosition(2);}
		void SetPresetPOV (double value);
};

class FRC_2012_Goals
{
	public:
		static Goal *Get_ShootBalls(FRC_2012_Robot *Robot);
		static Goal *Get_ShootBalls_WithPreset(FRC_2012_Robot *Robot,size_t KeyIndex);
	private:
		class Fire : public AtomicGoal
		{
		private:
			FRC_2012_Robot &m_Robot;
			bool m_Terminate;
			bool m_IsOn;
		public:
			Fire(FRC_2012_Robot &robot,bool On);
			virtual void Activate() {m_Status=eActive;}
			virtual Goal_Status Process(double dTime_s);
			virtual void Terminate() {m_Terminate=true;}
		};

};


/// This contains all UI controls specific to this years robot.  Since we do not use files the primary use of this is specific keys assigned
class FRC_2012_UI_Controller : public UI_Controller
{
	public:
		FRC_2012_UI_Controller(Framework::UI::JoyStick_Binder &joy,AI_Base_Controller *base_controller=NULL);
};

