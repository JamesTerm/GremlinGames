#pragma once

struct TractionModeProps
{
	double PID[3]; //p,i,d
	Ship_Properties ShipProperties;
	double InverseMaxForce;  //This is used to solve voltage at the acceleration level where the acceleration / force gets scaled down to voltage
	bool IsOpen;  //give ability to open or close loop for traction system  
	#ifdef AI_TesterCode
	double GearReduction;  //In reciprocal form of spread sheet
	#endif
};

class Butterfly_Robot_Properties : public Swerve_Robot_Properties
{
	public:
		Butterfly_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const TractionModeProps &GetTractionModeProps() const {return m_TractionModePropsProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		//Everything needed to switch to traction mode here
		TractionModeProps m_TractionModePropsProps;

		class ControlEvents : public LUA_Controls_Properties_Interface
		{
			protected: //from LUA_Controls_Properties_Interface
				virtual const char *LUA_Controls_GetEvents(size_t index) const; 
		};
		static ControlEvents s_ControlsEvents;
		LUA_Controls_Properties m_RobotControls;
};

class Butterfly_Robot : public Swerve_Robot
{
	public:
		//Since the DriveModeManager intercepts events internally... it will send a callback for further processing
		enum DriveMode
		{
			eOmniWheelDrive,
			eTractionDrive
		};
		enum SolenoidDevices
		{
			eUseLowGear
		};

		Butterfly_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		~Butterfly_Robot();
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
	protected:
		virtual Swerve_Drive *CreateDrive() {return new Butterfly_Drive(this);}

		virtual void BindAdditionalEventControls(bool Bind);
		virtual void BindAdditionalUIControls(bool Bind, void *joy);

		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode);
		DriveMode GetDriveMode() const {return m_DriveModeManager.GetMode();}
		virtual bool IsTractionMode() const {return (GetDriveMode()==eTractionDrive);}
	private:
		//This will change between omni wheel mode and traction drive
		class DriveModeManager
		{
		public:
			typedef Butterfly_Robot::DriveMode DriveMode;
			DriveModeManager(Butterfly_Robot *parent);

			//Cache the low gear properties
			void Initialize(const Butterfly_Robot_Properties &props);

			void SetMode(DriveMode Mode);
			void BindAdditionalEventControls(bool Bind);
			void BindAdditionalUIControls(bool Bind, void *joy);
			DriveMode GetMode() const {return m_CurrentMode;}
		protected:
			void SetLowGear(bool on);
			void SetLowGearOn() {SetLowGear(true);}
			void SetLowGearOff() {SetLowGear(false);}
			void SetLowGearValue(double Value);
		private:
			Butterfly_Robot *m_pParent;
			Butterfly_Robot_Properties m_ButterflyProps; //cache to obtain drive props
			TractionModeProps m_TractionModeProps;
			TractionModeProps m_OmniModeProps;
			DriveMode m_CurrentMode;
		} m_DriveModeManager;
};

class Butterfly_Robot_Control : public Swerve_Robot_Control
{
	protected:
		virtual void Initialize(const Entity_Properties *props);
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Open) {CloseSolenoid(index,!Open);}
	private:
		Butterfly_Robot_Properties m_ButterflyProps; //cache to obtain drive props
};

class Omni_Wheel_UI : public Wheel_UI
{
public:
	virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.3,0.3,1.0,1.0);}
	virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(0.1,0.1,1.0,1.0);}
};

class Butterfly_Robot_UI : public Swerve_Robot_UI
{
	public:
		typedef Butterfly_Robot::DriveMode DriveMode;
		Butterfly_Robot_UI(Swerve_Robot *SwerveRobot) : Swerve_Robot_UI(SwerveRobot) {}
		void DriveModeManager_SetMode_Callback(DriveMode Mode);
	protected:
		virtual Wheel_UI *Create_WheelUI() {return new Omni_Wheel_UI;}
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Butterfly_Robot_UI_Control : public Butterfly_Robot, public Butterfly_Robot_Control
{
	public:
		Butterfly_Robot_UI_Control(const char EntityName[]) : Butterfly_Robot(EntityName,this),Butterfly_Robot_Control(),
			m_ButterflyUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_ButterflyUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_ButterflyUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_ButterflyUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_ButterflyUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_ButterflyUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_ButterflyUI.UpdateScene(geode,AddOrRemove);}
		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode) 
		{	__super::DriveModeManager_SetMode_Callback(Mode);
			m_ButterflyUI.DriveModeManager_SetMode_Callback(Mode);
		}

	private:
		Butterfly_Robot_UI m_ButterflyUI;
};


//----------------------------------------------------------Nona-----------------------------------------------------------


class Nona_Robot : public Butterfly_Robot
{
	public:
		enum Nona_Robot_SpeedControllerDevices
		{
			eWheel_Kicker = Swerve_Robot::eNoSwerveRobotSpeedControllerDevices,
			eNoNonaRobotSpeedControllerDevices
		};

		Nona_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		~Nona_Robot();
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
	protected:
		virtual Swerve_Drive *CreateDrive();
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);

		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode);
	private:
		Rotary_Angular m_KickerWheel;  //apply control to kicker wheel
		Nona_Drive * const m_NonaDrive; //cache, avoid needing to dynamic cast each iteration
};

class Nona_Robot_Properties : public Butterfly_Robot_Properties
{
	public:
		Nona_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const Rotary_Properties &GetKickerWheelProps() const {return m_KickerWheelProps;}
	private:
		//Note the kicker wheel properties is a measurement of linear movement (not angular velocity)
		Rotary_Properties m_KickerWheelProps;
};

class Nona_Robot_Control : public Butterfly_Robot_Control
{
	public:
		Nona_Robot_Control();
	protected: //from Rotary_Control_Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);

		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		virtual void Reset_Encoders();
	protected:
		virtual void DisplayVoltage();
	private:
		Encoder_Simulator2 m_KickerWheelEncoder;
		double m_KickerWheelVoltage;
};

//class Omni_Wheel_UI_Nona : public Wheel_UI
//{
//	public:
//		virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.3,0.3,1.0,1.0);}
//		virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(0.1,0.1,1.0,1.0);}
//};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Nona_Robot_UI(Nona_Robot *NonaRobot) : m_NonaRobot(NonaRobot) {}
	public:
		typedef Butterfly_Robot::DriveMode DriveMode;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);

		void DriveModeManager_SetMode_Callback(DriveMode Mode);
	protected:
		virtual void UpdateVoltage(size_t index,double Voltage) {}
		virtual void CloseSolenoid(size_t index,bool Close) {}

	private:
		Nona_Robot * const m_NonaRobot;
		Omni_Wheel_UI m_Wheel[5];
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Nona_Robot_UI_Control : public Nona_Robot, public Nona_Robot_Control
{
	public:
		Nona_Robot_UI_Control(const char EntityName[]) : Nona_Robot(EntityName,this),Nona_Robot_Control(),
			m_NonaUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_NonaUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_NonaUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_NonaUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_NonaUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_NonaUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_NonaUI.UpdateScene(geode,AddOrRemove);}

		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode) 
		{	__super::DriveModeManager_SetMode_Callback(Mode);
			m_NonaUI.DriveModeManager_SetMode_Callback(Mode);
		}
	private:
		Nona_Robot_UI m_NonaUI;
};
