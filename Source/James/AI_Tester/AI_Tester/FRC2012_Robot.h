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
			eFireConveyor
		};

		enum BoolSensorDevices
		{
			eLowerConveyor_Sensor,
			eMiddleConveyor_Sensor,
			eFireConveyor_Sensor
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		//TODO change UseEncoders to be passive
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

	protected:
		class Turret : public Rotary_Linear
		{
			public:
				Turret(Rotary_Control_Interface *robot_control,FRC_2012_Robot *parent);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
				virtual void TimeChange(double dTime_s);
			private:
				FRC_2012_Robot * const m_pParent;
		};

		class PitchRamp : public Rotary_Linear
		{
			public:
				PitchRamp(Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetIntendedPosition(double Position);

				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
		};

		class PowerWheels : public Rotary_Angular
		{
			public:
				PowerWheels(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetEncoderSafety(bool DisableFeedback) {__super::SetEncoderSafety(DisableFeedback);}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
			private:
				FRC_2012_Robot * const m_pParent;
				bool m_IsRunning;
		};

		class BallConveyorSystem
		{
			private:
				FRC_2012_Robot * const m_pParent;
				//typedef Ship_1D __super;
				Rotary_Angular m_LowerConveyor,m_MiddleConveyor,m_FireConveyor;
				bool m_Grip,m_Squirt,m_Fire;
			public:
				BallConveyorSystem(FRC_2012_Robot *pParent,Rotary_Control_Interface *robot_control);
				virtual void Initialize(GG_Framework::Base::EventMap& em,const Entity1D_Properties *props=NULL);
				bool GetIsFireRequested() const {return m_Fire;}

				IEvent::HandlerList ehl;
				//public access needed for goals
				void Fire(bool on) {m_Fire=on;}
				//Using meaningful terms to assert the correct direction at this level
				void Grip(bool on) {m_Grip=on;}
				void Squirt(bool on) {m_Squirt=on;}

				void ResetPos() {m_LowerConveyor.ResetPos(),m_MiddleConveyor.ResetPos(),m_FireConveyor.ResetPos();}
				//Intercept the time change to send out voltage
				void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			protected:
				void SetRequestedVelocity_FromNormalized(double Velocity);
		};
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		//typedef  Tank_Robot __super;
		FRC_2012_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PitchRamp m_PitchRamp;
		PowerWheels m_PowerWheels;
		BallConveyorSystem m_BallConveyorSystem;

		//This is adjusted depending on location for correct bank-shot angle trajectory, note: the coordinate system is based where 0,0 is the 
		//middle of the game playing field
		Vec2D m_TargetOffset;  
		//This is adjusted depending on doing a bank shot or swishing 
		double m_TargetHeight;
		bool m_IsTargeting;
};

//-------------------------------------------------------------------------------------------------------------------------------------------------
///These classes below are for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct and provides some idea to work needed in WindRiver

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		FRC_2012_Robot_Properties();

		const Ship_1D_Properties &GetTurretProps() const {return m_TurretProps;}
		const Ship_1D_Properties &GetPitchRampProps() const {return m_PitchRampProps;}
		const Ship_1D_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}
		const Ship_1D_Properties &GetConveyorProps() const {return m_ConveyorProps;}

	private:
		Rotary_Properties m_TurretProps,m_PitchRampProps,m_PowerWheelProps,m_ConveyorProps;
};

class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	public:
		FRC_2012_Robot_Control();
		const FRC_2012_Robot_Properties &GetRobotProps() {return m_RobotProps;}
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
	protected: //from Tank_Drive_Control_Interface
		virtual void Reset_Encoders() {m_pTankRobotControl->Reset_Encoders();}
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity) {m_pTankRobotControl->GetLeftRightVelocity(LeftVelocity,RightVelocity);}
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage) {m_pTankRobotControl->UpdateLeftRightVoltage(LeftVoltage,RightVoltage);}
		virtual void Tank_Drive_Control_TimeChange(double dTime_s) {m_pTankRobotControl->Tank_Drive_Control_TimeChange(dTime_s);}
	protected: //from Rotary Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage) {UpdateVoltage(index,Voltage);}

	protected: //from FRC_2011_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Robot_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);

	protected:
		FRC_2012_Robot_Properties m_RobotProps;  //saves a copy of all the properties
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester2 m_Turret_Pot,m_Pitch_Pot; //simulate the potentiometer and motor
		Encoder_Simulator m_PowerWheel_Enc,m_LowerConveyor_Enc,m_MiddleConveyor_Enc,m_FireConveyor_Enc;  //simulate the encoder and motor
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_TurretVoltage,m_PitchRampVoltage,m_PowerWheelVoltage,m_LowerConveyorVoltage,m_MiddleConveyorVoltage,m_FireConveyorVoltage;
};

class FRC_2012_Turret_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		struct Turret_Properties
		{
			double YOffset;
		};

		FRC_2012_Turret_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		virtual void Initialize(Entity2D::EventMap& em, const Turret_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
		Actor_Text *m_UIParent;
		Turret_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Turret; 
};

class Side_Wheel_UI
{
public:
	Side_Wheel_UI() : m_UIParent(NULL),m_Rotation(0.0) {}
	typedef osg::Vec2d Vec2D;
	typedef osg::Vec4d Vec4D;

	struct Wheel_Properties
	{
		Vec2D m_Offset;  //Placement of the wheel in reference to the parent object (default 0,0)
		Vec4D m_Color;
		const wchar_t *m_TextDisplay;
	};

	void UI_Init(Actor_Text *parent);

	//Client code can manage the properties
	virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
	//Keep virtual for special kind of wheels
	virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading);
	virtual void Text_SizeToUse(double SizeToUse);

	virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);
	//This will add to the existing rotation and normalize
	void AddRotation(double RadiansToAdd);
	double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
private:
	Actor_Text *m_UIParent;
	Wheel_Properties m_props;
	osg::ref_ptr<osgText::Text> m_Wheel; 
	double m_Rotation;

	Vec4D m_Color;
	const wchar_t *m_TextDisplay;
};

class FRC_2012_Power_Wheel_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Power_Wheel_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
		double m_PowerWheelMaxSpeed;  //cache to avoid all the hoops of getting it (its constant)
};

class FRC_2012_Lower_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Lower_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

class FRC_2012_Middle_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Middle_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

class FRC_2012_Fire_Conveyor_UI : public Side_Wheel_UI
{
	public:
		FRC_2012_Fire_Conveyor_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control) {}
		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		virtual void TimeChange(double dTime_s);
	private:
		FRC_2012_Robot_Control * const m_RobotControl;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2012_Robot_UI : public FRC_2012_Robot, public FRC_2012_Robot_Control
{
	public:
		FRC_2012_Robot_UI(const char EntityName[]);
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
		FRC_2012_Turret_UI m_TurretUI;
		FRC_2012_Power_Wheel_UI m_PowerWheelUI;
		FRC_2012_Lower_Conveyor_UI m_LowerConveyor;
		FRC_2012_Middle_Conveyor_UI m_MiddleConveyor;
		FRC_2012_Fire_Conveyor_UI m_FireConveyor;
};
