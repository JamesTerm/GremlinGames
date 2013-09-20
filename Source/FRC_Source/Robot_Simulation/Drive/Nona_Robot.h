#pragma once

struct TractionModeProps
{
	double PID[3]; //p,i,d
	Ship_Properties ShipProperties;
	double InverseMaxAccel;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel;  //used for deceleration case
	bool IsOpen;  //give ability to open or close loop for traction system  
	#ifdef Robot_TesterCode
	double GearReduction;  //In reciprocal form of spread sheet
	#endif
};

class DRIVE_API Butterfly_Robot_Properties : public Swerve_Robot_Properties
{
	public:
		Butterfly_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const TractionModeProps &GetTractionModeProps() const {return m_TractionModePropsProps;}
		const LUA_Controls_Properties &Get_RobotControls() const {return m_RobotControls;}
	private:
		//typedef Swerve_Robot_Properties __super;
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

class DRIVE_API Butterfly_Robot : public Swerve_Robot
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
		virtual void BindAdditionalUIControls(bool Bind, void *joy, void *key);

		virtual void DriveModeManager_SetMode_Callback(DriveMode Mode);
		DriveMode GetDriveMode() const {return m_DriveModeManager.GetMode();}
		virtual bool IsTractionMode() const {return (GetDriveMode()==eTractionDrive);}
	private:
		//typedef Swerve_Robot __super;
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
			void BindAdditionalUIControls(bool Bind, void *joy, void *key);
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

class DRIVE_API Butterfly_Robot_Control : public Swerve_Robot_Control
{
	protected:
		virtual void Initialize(const Entity_Properties *props);
		virtual void CloseSolenoid(size_t index,bool Close);
		virtual void OpenSolenoid(size_t index,bool Open) {CloseSolenoid(index,!Open);}
	private:
		Butterfly_Robot_Properties m_ButterflyProps; //cache to obtain drive props
};



//----------------------------------------------------------Nona-----------------------------------------------------------

class DRIVE_API Nona_Robot : public Butterfly_Robot
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
		//typedef Butterfly_Robot __super;
		
		Rotary_Velocity_Control m_KickerWheel;  //apply control to kicker wheel
		Nona_Drive * m_NonaDrive; //cache, avoid needing to dynamic cast each iteration
};

class DRIVE_API Nona_Robot_Properties : public Butterfly_Robot_Properties
{
	public:
		Nona_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const Rotary_Properties &GetKickerWheelProps() const {return m_KickerWheelProps;}
	private:
		//typedef Butterfly_Robot_Properties __super;
		//Note the kicker wheel properties is a measurement of linear movement (not angular velocity)
		Rotary_Properties m_KickerWheelProps;
};

class DRIVE_API Nona_Robot_Control : public Butterfly_Robot_Control
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
