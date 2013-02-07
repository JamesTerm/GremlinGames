#pragma once
#undef __Tank_UseScalerPID__

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Tank_Drive_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Tank_Drive_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
};

struct Tank_Robot_Props
{
	//typedef Framework::Base::Vec2d Vec2D;
	typedef osg::Vec2d Vec2D;

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double VoltageScalar;		//Used to handle reversed voltage wiring
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double LeftPID[3]; //p,i,d
	double RightPID[3]; //p,i,d
	double HeadingLatency; //Should be about 100ms + Input Latency... this will establish intervals to sync up the heading with entity
	double PrecisionTolerance;  //Used to manage voltage override and avoid oscillation
	double LeftMaxSpeedOffset;	//These are used to align max speed to what is reported by encoders (Encoder MaxSpeed - Computed MaxSpeed)
	double RightMaxSpeedOffset;
	double TankSteering_Tolerance; //used to help controls drive straight
	Vec2D DriveTo_ForceDegradeScalar;  //Used for way point driving in autonomous in conjunction with max force to get better deceleration precision
	size_t Feedback_DiplayRow;  //Choose a row for display -1 for none (Only active if __DebugLUA__ is defined)
	bool IsOpen;  //This property only applies in teleop
	bool PID_Console_Dump;  //This will dump the console PID info (Only active if __DebugLUA__ is defined)
	bool ReverseSteering;  //This will fix if the wiring on voltage has been reversed (e.g. voltage to right turns left side)
	//Note: I cannot imagine one side ever needing to be different from another (PID can solve if that is true)
	//Currently supporting 4 terms in polynomial equation
	double Polynomial[5];  //Here is the curve fitting terms where 0th element is C, 1 = Cx^1, 2 = Cx^2, 3 = Cx^3 and so on...
	//This may be computed from stall torque and then torque at wheel (does not factor in traction) to linear in reciprocal form to avoid division
	//or alternatively solved empirically.  Using zero disables this feature
	double InverseMaxAccel_Left,InverseMaxAccel_Right;  //This is used to solve voltage at the acceleration level where the acceleration / max acceleration gets scaled down to voltage
	double InverseMaxDecel_Left,InverseMaxDecel_Right;  //used for deceleration case
	//Different robots may have the encoders flipped or not which must represent the same direction of both treads
	//for instance the hiking viking has both of these false, while the admiral has the right encoder reversed
	bool LeftEncoderReversed,RightEncoderReversed;
};

class Tank_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Tank_Robot : public Ship_Tester,
				   public Vehicle_Drive_Common_Interface
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool IsAutonomous=false);
		virtual ~Tank_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		void Reset(bool ResetPosition=true);
		/// \param ResetPos typically true for autonomous and false for dynamic use
		void SetUseEncoders(bool UseEncoders,bool ResetPosition=true);
		void SetIsAutonomous(bool IsAutonomous);
		virtual void TimeChange(double dTime_s);
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		//Give ability to change properties
		void UpdateTankProps(const Tank_Robot_Props &TankProps);
	protected:
		friend Tank_Robot_UI;

		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		#ifdef __UseScalerPID__
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		#endif
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
		virtual void SetAttitude(double radians);  //from ship tester
		virtual Vec2D Get_DriveTo_ForceDegradeScalar() const {return m_TankRobotProps.DriveTo_ForceDegradeScalar;}
		virtual Tank_Drive *CreateDrive() {return new Tank_Drive(this);}
		virtual void DestroyDrive();
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void ResetPos();
		virtual void UpdateController(double &AuxVelocity,Vec2D &LinearAcceleration,double &AngularAcceleration,bool &LockShipHeadingToOrientation,double dTime_s) 
			{m_TankSteering.UpdateController(AuxVelocity,LinearAcceleration,AngularAcceleration,*this,LockShipHeadingToOrientation,dTime_s);
			}
		virtual void BindAdditionalEventControls(bool Bind) 
			{m_TankSteering.BindAdditionalEventControls(Bind,GetEventMap(),ehl);
			}

	protected:  //from Vehicle_Drive_Common_Interface
		virtual const Vec2D &GetWheelDimensions() const {return m_TankRobotProps.WheelDimensions;}
		//Note by default a 6WD Tank Robot is assumed to set length for a 4WD (or half the total length of 6)
		virtual double GetWheelTurningDiameter() const {return m_TankRobotProps.WheelDimensions.length();}
		virtual double Vehicle_Drive_GetAtt_r() const {return GetAtt_r();}
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const {return GetPhysics();}
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW() {return GetPhysics();}
	protected:
		bool m_IsAutonomous;
	private:
		//typedef  Tank_Drive __super;
		Tank_Drive_Control_Interface * const m_RobotControl;
		Tank_Drive * const m_VehicleDrive;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		#ifdef __UseScalerPID__
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		#else
		double m_ErrorOffset_Left,m_ErrorOffset_Right; //used for calibration
		#endif
		bool m_UsingEncoders;
		#ifdef __UseScalerPID__
		bool m_VoltageOverride;  //when true will kill voltage
		#endif
		bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderAngularVelocity;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
		//These help to manage the latency, where the heading will only reflect injection changes on the latency intervals
		double m_Heading;  //We take over the heading from physics
		double m_HeadingUpdateTimer;
		double m_PreviousLeftVelocity,m_PreviousRightVelocity; //used to compute acceleration
		Tank_Steering m_TankSteering;  //adding controls for tank steering
	public:
		double GetLeftVelocity() const {return m_VehicleDrive->GetLeftVelocity();}
		double GetRightVelocity() const {return m_VehicleDrive->GetRightVelocity();}
};

class Tank_Robot_Properties : public UI_Ship_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		Tank_Robot_Properties();
		virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	protected:
		Tank_Robot_Props m_TankRobotProps;
	private:
		//typedef Ship_Properties __super;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Tank_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Tank_Robot_Control();
		//This is only needed for simulation
		virtual void Tank_Drive_Control_TimeChange(double dTime_s);
		double GetLeftVoltage() const {return m_LeftVoltage;}
		double GetRightVoltage() const {return m_RightVoltage;}
		void SetDisplayVoltage(bool display) {m_DisplayVoltage=display;}

		void SetLeftRightReverseDirectionEncoder(bool Left_reverseDirection,bool Right_reverseDirection) 
		{	m_Encoders.SetLeftRightReverseDirectionEncoder(Left_reverseDirection,Right_reverseDirection);
		}
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		double RPS_To_LinearVelocity(double RPS);
	protected:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		Encoder_Tester m_Encoders;
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		//cache voltage values for display
		double m_LeftVoltage,m_RightVoltage;
		bool m_DisplayVoltage;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
};

//This is a simplified version of the wheel UI without the swivel or the graphics to show direction (only the tread)
class Tank_Wheel_UI
{
	public:
		Tank_Wheel_UI() : m_UIParent(NULL) {}
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
		//This will add to the existing rotation and normalize
		void AddRotation(double RadiansToAdd);
		double GetFontSize() const {return m_UIParent?m_UIParent->GetFontSize():10.0;}
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Tread; //Tread is really a line that helps show speed
		double m_Rotation;
};

class Tank_Robot_UI
{
	public:
		typedef osg::Vec2d Vec2D;

		Tank_Robot_UI(Tank_Robot *tank_robot) : m_TankRobot(tank_robot) {}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);
	private:
		Tank_Robot * const m_TankRobot;
		Tank_Wheel_UI m_Wheel[6];
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Tank_Robot_UI_Control : public Tank_Robot, public Tank_Robot_Control
{
	public:
		Tank_Robot_UI_Control(const char EntityName[]) : Tank_Robot(EntityName,this),Tank_Robot_Control(),
		m_TankUI(this) {}
	protected:
		virtual void TimeChange(double dTime_s) 
		{
			__super::TimeChange(dTime_s);
			m_TankUI.TimeChange(dTime_s);
			SetDisplayVoltage(m_controller->GetUIController()?true:false);
		}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL)
		{
			__super::Initialize(em,props);
			m_TankUI.Initialize(em,props);
		}

	protected:   //from EntityPropertiesInterface
		virtual void UI_Init(Actor_Text *parent) {m_TankUI.UI_Init(parent);}
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos) 
		{m_TankUI.custom_update(nv,draw,parent_pos);}
		virtual void Text_SizeToUse(double SizeToUse) {m_TankUI.Text_SizeToUse(SizeToUse);}
		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove) {m_TankUI.UpdateScene(geode,AddOrRemove);}

	private:
		Tank_Robot_UI m_TankUI;
};
