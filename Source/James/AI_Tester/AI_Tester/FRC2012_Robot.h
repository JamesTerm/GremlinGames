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
			ePowerWheels
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		//TODO change UseEncoders to be passive
		FRC_2012_Robot(const char EntityName[],FRC_2012_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		virtual void TimeChange(double dTime_s);

		class Turret : public Rotary_Linear
		{
			public:
				Turret(Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
			protected:
				virtual void BindAdditionalEventControls(bool Bind);
				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity) {__super::SetRequestedVelocity_FromNormalized(Velocity);}
				void SetPotentiometerSafety(bool DisableFeedback) {__super::SetPotentiometerSafety(DisableFeedback);}
		};

		class PowerWheels : public Rotary_Angular
		{
			public:
				PowerWheels(Rotary_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
			protected:
				virtual void BindAdditionalEventControls(bool Bind);
				//typedef Rotary_Linear __super;
				//events are a bit picky on what to subscribe so we'll just wrap from here
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetEncoderSafety(bool DisableFeedback) {__super::SetEncoderSafety(DisableFeedback);}
				void SetIsRunning(bool IsRunning) {m_IsRunning=IsRunning;}
			private:
				bool m_IsRunning;
		};
	protected:
		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		//typedef  Tank_Robot __super;
		FRC_2012_Control_Interface * const m_RobotControl;
		Turret m_Turret;
		PowerWheels m_PowerWheels;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class FRC_2012_Robot_Control : public FRC_2012_Control_Interface
{
	public:
		FRC_2012_Robot_Control();
		//This is only needed for simulation
	protected: //from Robot_Control_Interface
		virtual void UpdateVoltage(size_t index,double Voltage);
		//Solenoid not used
		virtual void CloseSolenoid(size_t index,bool Close) {}
		virtual void OpenSolenoid(size_t index,bool Close) {}
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
		Tank_Robot_Control m_TankRobotControl;
		Tank_Drive_Control_Interface * const m_pTankRobotControl;  //This allows access to protected members
		Potentiometer_Tester2 m_Turret_Pot; //simulate the potentiometer and motor
		Encoder_Simulator m_PowerWheel_Enc;  //simulate the encoder and motor
		KalmanFilter m_KalFilter_Arm;
		//cache voltage values for display
		double m_TurretVoltage,m_PowerWheelVoltage;
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

class FRC_2012_Power_Wheel_UI
{
public:
	FRC_2012_Power_Wheel_UI(FRC_2012_Robot_Control *robot_control) : m_RobotControl(robot_control),m_UIParent(NULL),m_Rotation(0.0) {}
	typedef osg::Vec2d Vec2D;

	struct Wheel_Properties
	{
		Vec2D m_Offset;  //Placement of the wheel in reference to the parent object (default 0,0)
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
	virtual void TimeChange(double dTime_s);
private:
	FRC_2012_Robot_Control * const m_RobotControl;
	Actor_Text *m_UIParent;
	Wheel_Properties m_props;
	osg::ref_ptr<osgText::Text> m_Wheel; 
	double m_Rotation;
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
};

class FRC_2012_Robot_Properties : public Tank_Robot_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		FRC_2012_Robot_Properties();

		const Ship_1D_Properties &GetTurretProps() const {return m_TurretProps;}
		const Ship_1D_Properties &GetPowerWheelProps() const {return m_PowerWheelProps;}

	private:
		Rotary_Properties m_TurretProps,m_PowerWheelProps;
};
