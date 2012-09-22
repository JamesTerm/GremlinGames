#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Swerve_Drive_Control_Interface : public Rotary_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;
};

struct Swerve_Robot_Props
{
	//typedef Framework::Base::Vec2d Vec2D;
	typedef osg::Vec2d Vec2D;

	//Currently supporting 4 terms in polynomial equation
	//Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	double Polynomial_Wheel[5];
	double Polynomial_Swivel[5];  

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double Wheel_PID[3]; //p,i,d
	double Swivel_PID[3]; //p,i,d
	double InputLatency;  //Used with PID to help avoid oscillation in the error control (We can make one for each if needed)
	double HeadingLatency; //Should be about 100ms + Input Latency... this will establish intervals to sync up the heading with entity
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double MaxSpeedOffset[4];	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	Vec2D DriveTo_ForceDegradeScalar;  //Used for way point driving in autonomous in conjunction with max force to get better deceleration precision
	double SwivelRange;  //Value in radians of the swivel range 0 is infinite
	double TankSteering_Tolerance; //used to help controls drive straight
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxForce;  //This is used to solve voltage at the acceleration level where the acceleration / force gets scaled down to voltage
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen_Wheel,IsOpen_Swivel;  //give ability to open or close loop for wheel or swivel system  
	//This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool PID_Console_Dump_Wheel[4];  
	bool PID_Console_Dump_Swivel[4];
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	bool EncoderReversed_Wheel[4];
	bool EncoderReversed_Swivel[4];
};

class Swerve_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Swerve_Robot : public Ship_Tester,
					 public Swerve_Drive_Interface
{
	public:
		//Note these order in the same as SwerveVelocities::SectionOrder
		enum Swerve_Robot_SpeedControllerDevices
		{
			eWheel_FL,
			eWheel_FR,
			eWheel_RL,
			eWheel_RR,
			eSwivel_FL,
			eSwivel_FR,
			eSwivel_RL,
			eSwivel_RR,
			eNoSwerveRobotSpeedControllerDevices
		};

		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Swerve_Robot(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		~Swerve_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		void SetIsAutonomous(bool IsAutonomous);
		virtual void TimeChange(double dTime_s);

		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
		const Swerve_Drive_Control_Interface &GetRobotControl() const {return *m_RobotControl;}
		//Accessors needed for setting goals
	protected:
		friend Swerve_Robot_UI;

		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		//virtual void BindAdditionalEventControls(bool Bind);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual void SetAttitude(double radians);  //from ship tester

		//Get the sweet spot between the update and interpolation to avoid oscillation 
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);

		virtual Vec2D Get_DriveTo_ForceDegradeScalar() const {return m_SwerveRobotProps.DriveTo_ForceDegradeScalar;}
		virtual Swerve_Drive *CreateDrive() {return new Swerve_Drive(this);}
		virtual void DestroyDrive();

		//from Swerve_Drive_Interface
		virtual const SwerveVelocities &GetSwerveVelocities() const {return m_Swerve_Robot_Velocities;}
		virtual Vec2D GetLinearVelocity_ToDisplay() {return GlobalToLocal(GetAtt_r(),m_EncoderGlobalVelocity);}
		virtual double GetAngularVelocity_ToDisplay() {return m_EncoderAngularVelocity;}

		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,double dTime_s) 
			{m_TankSteering.UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,*this,dTime_s);
			}
		virtual void BindAdditionalEventControls(bool Bind) 
			{m_TankSteering.BindAdditionalEventControls(Bind,GetEventMap(),ehl);
			}
	protected:  //from Vehicle_Drive_Common_Interface
		virtual const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
		virtual double GetWheelTurningDiameter() const {return m_WheelDimensions.length();}
		virtual double Vehicle_Drive_GetAtt_r() const {return GetAtt_r();}
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const {return GetPhysics();}
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW() {return GetPhysics();}

		bool m_IsAutonomous;
		Swerve_Drive_Control_Interface * const m_RobotControl;
	private:
		Swerve_Drive * const m_VehicleDrive;
		//typedef  Tank_Drive __super;

		//The driving module consists of a swivel motor and the driving motor for a wheel.  It manages / converts the intended direction and speed to 
		//actual direction and velocity (i.e. works in reverse) as well as working with sensor feedback (e.g. potentiometer, encoder) for error
		//correction of voltage computation.
		class DrivingModule
		{
			public:
				DrivingModule(const char EntityName[],Swerve_Drive_Control_Interface *robot_control,size_t SectionOrder);
				struct DrivingModule_Props
				{
					const  Rotary_Properties *Swivel_Props;
					const  Rotary_Properties *Drive_Props;
				};
				virtual void Initialize(GG_Framework::Base::EventMap& em,const DrivingModule_Props *props=NULL);
				virtual void TimeChange(double dTime_s);
				void SetIntendedSwivelDirection(double direction) {m_IntendedSwivelDirection=direction;}
				void SetIntendedDriveVelocity(double Velocity) {m_IntendedDriveVelocity=Velocity;}
				//I have no problem exposing read-only access to these :)
				const Rotary_Linear &GetSwivel() const {return m_Swivel;}
				const Rotary_Angular &GetDrive() const {return m_Drive;}

				void ResetPos() {m_Drive.ResetPos(),m_Swivel.ResetPos();}
				
			private:
				std::string m_ModuleName,m_SwivelName,m_DriveName;
				Rotary_Linear m_Swivel;  //apply control to swivel mechanism
				Rotary_Angular m_Drive;  //apply control to drive motor
				//Pass along the intended swivel direction and drive velocity
				double m_IntendedSwivelDirection,m_IntendedDriveVelocity;

				Swerve_Drive_Control_Interface * const m_RobotControl;
		} *m_DrivingModule[4]; //FL, FR, RL, RR  The four modules used  (We could put 6 here if we want)

		bool m_UsingEncoders;
		Vec2D m_WheelDimensions; //cached from the Swerve_Robot_Properties
		SwerveVelocities m_Swerve_Robot_Velocities;
		Vec2D m_EncoderGlobalVelocity;
		double m_EncoderAngularVelocity;
		Swerve_Robot_Props m_SwerveRobotProps; //cached in the Initialize from specific robot

		//These help to manage the latency, where the heading will only reflect injection changes on the latency intervals
		double m_Heading;  //We take over the heading from physics
		double m_HeadingUpdateTimer;
		Tank_Steering m_TankSteering;  //adding controls for tank steering
	public:
		double GetSwerveVelocitiesFromIndex(size_t index) const {return m_VehicleDrive->GetSwerveVelocitiesFromIndex(index);}
};

class Swerve_Robot_Properties : public UI_Ship_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		Swerve_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);

		const Rotary_Properties &GetSwivelProps() const {return m_SwivelProps;}
		const Rotary_Properties &GetDriveProps() const {return m_DriveProps;}
		//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
		//the width is a measurement of the the center of the wheel width to the other wheel
		const Vec2D &GetWheelDimensions() const {return m_SwerveRobotProps.WheelDimensions;}
		const Swerve_Robot_Props &GetSwerveRobotProps() const {return m_SwerveRobotProps;}
	private:
		//Note the drive properties is a measurement of linear movement (not angular velocity)
		Rotary_Properties m_SwivelProps,m_DriveProps;
		Swerve_Robot_Props m_SwerveRobotProps;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Swerve_Robot_Control : public Swerve_Drive_Control_Interface
{
	public:
		Swerve_Robot_Control();
		void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}

		//This is only needed for simulation
	protected: //from Rotary_Control_Interface
		virtual void Reset_Rotary(size_t index=0); 
		virtual double GetRotaryCurrentPorV(size_t index=0);
		virtual void UpdateRotaryVoltage(size_t index,double Voltage);

		//from Swerve_Drive_Control_Interface
		virtual void Swerve_Drive_Control_TimeChange(double dTime_s);
		virtual void Initialize(const Entity_Properties *props);
		virtual void Reset_Encoders();

		double RPS_To_LinearVelocity(double RPS);
	protected:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		Potentiometer_Tester2 m_Potentiometers[4]; //simulate a real potentiometer for calibration testing
		Encoder_Simulator2 m_Encoders[4];
		//cache voltage values for display
		double m_EncoderVoltage[4],m_PotentiometerVoltage[4];
		Swerve_Robot_Props m_SwerveRobotProps; //cached in the Initialize from specific robot
		bool m_DisplayVoltage;
};

class Wheel_UI
{
	public:
		Wheel_UI() : m_UIParent(NULL) {}
		virtual ~Wheel_UI() {}
		typedef osg::Vec2d Vec2D;

		struct Wheel_Properties
		{
			Vec2D m_Offset;  //Placement of the wheel in reference to the parent object (default 0,0)
			double m_Wheel_Diameter; //in meters default 0.1524  (6 inches)
		};

		void UI_Init(Actor_Text *parent);

		//Client code can manage the properties
		virtual void Initialize(Entity2D::EventMap& em, const Wheel_Properties *props=NULL);
		//Keep virtual for special kind of wheels
		virtual void update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos,double Heading);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);
		//Where 0 is up and 1.57 is right and -1.57 is left
		void SetSwivel(double SwivelAngle){m_Swivel=-SwivelAngle;}
		//This will add to the existing rotation and normalize
		void AddRotation(double RadiansToAdd);
		double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
	protected:
		virtual osg::Vec4 GetFrontWheelColor() const {return osg::Vec4(0.0,1.0,0.0,1.0);}
		virtual osg::Vec4 GetBackWheelColor() const {return osg::Vec4(1.0,0.0,0.0,1.0);}
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Front,m_Back,m_Tread; //Tread is really a line that helps show speed
		double m_Rotation,m_Swivel;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Swerve_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Swerve_Robot_UI(Swerve_Robot *SwerveRobot);
		~Swerve_Robot_UI();
	public:
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);

	protected:
		virtual void UpdateVoltage(size_t index,double Voltage) {}
		virtual void CloseSolenoid(size_t index,bool Close) {}
		virtual Wheel_UI *Create_WheelUI() {return new Wheel_UI;}
		virtual void Destroy_WheelUI(Wheel_UI *wheel_ui) {delete wheel_ui;}
	private:
		Swerve_Robot * const m_SwerveRobot;
		Wheel_UI *m_Wheel[4];
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Swerve_Robot_UI_Control : public Swerve_Robot, public Swerve_Robot_Control
{
	public:
		Swerve_Robot_UI_Control(const char EntityName[]) : Swerve_Robot(EntityName,this),Swerve_Robot_Control(),
			m_SwerveUI(this) {}

	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_SwerveUI.TimeChange(dTime_s);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_SwerveUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_SwerveUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
			{m_SwerveUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_SwerveUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_SwerveUI.UpdateScene(geode,AddOrRemove);}

	private:
		Swerve_Robot_UI m_SwerveUI;
};
