#pragma once

#if 0
///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Robot_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Arm()=0; 
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void UpdateVoltage(size_t index,double Voltage)=0;
		///This is a implemented by reading the potentiometer and converting its value to correspond to the arm's current angle
		///This is in radians of the arm's gear ratio
		///TODO break this apart to reading pure analog values and have the potentiometer conversion happen within the robot
		virtual double GetArmCurrentPosition()=0;
		/// \param The index is ordinal enumerated to specific robot's interpretation
		/// \see subclass for enumeration specifics
		virtual void CloseSolenoid(size_t index,bool Close)=0;  //true=close false=open
};
#endif

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Swerve_Robot : public Swerve_Drive
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Swerve_Robot(const char EntityName[],Robot_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		void SetUseEncoders(bool UseEncoders) {m_UsingEncoders=UseEncoders;}
		virtual void TimeChange(double dTime_s);
		//This is a work-around (since robot control also has TimeChange)
		virtual void UI_TimeChange(double dTime_s) {}
		static double RPS_To_LinearVelocity(double RPS);

		//Accessors needed for setting goals
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		//virtual void BindAdditionalEventControls(bool Bind);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
	private:
		//typedef  Tank_Drive __super;
		Robot_Control_Interface * const m_RobotControl;

		//PIDController2 m_PIDController_Left,m_PIDController_Right;
		//double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration

		bool m_UsingEncoders;
		//bool m_VoltageOverride;  //when true will kill voltage
		//bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		//Vec2D m_EncoderGlobalVelocity;  //cache for later use
		//double m_EncoderHeading;
};

#if 0
///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Robot_Control : public Robot_Control_Interface
{
	public:
		Robot_Control(FRC_2011_Robot *Robot);
		//This is only needed for simulation
		virtual void TimeChange(double dTime_s);
	protected: //from Robot_Control_Interface
		//Will reset various members as needed (e.g. Kalman filters)
		virtual void Reset_Arm(); 
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
		//virtual void UpdateVoltage(size_t index,double Voltage); this is overridden
		//pacify this by returning its current value
		virtual double GetArmCurrentPosition();
	protected:
		FRC_2011_Robot * const m_Robot;
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
		Potentiometer_Tester m_Potentiometer; //simulate a real potentiometer for calibration testing
		Encoder_Tester m_Encoders;
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		//cache voltage values for display
		double m_LeftVoltage,m_RightVoltage,m_ArmVoltage,m_RollerVoltage;
		bool m_Deployment,m_Claw,m_Rist;
};
#endif

//TODO move
class Wheel_UI
{
	public:
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
	private:
		Actor_Text *m_UIParent;
		Wheel_Properties m_props;
		osg::ref_ptr<osgText::Text> m_Front,m_Back,m_Tread; //Tread is really a line that helps show speed
		double m_Rotation,m_Swivel;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Swerve_Robot_UI : public Swerve_Robot, public Robot_Control
{
	public:
		Swerve_Robot_UI(const char EntityName[]) : Swerve_Robot(EntityName,this),Robot_Control() {}
	protected:
		virtual void UpdateVoltage(size_t index,double Voltage) {}
		virtual void CloseSolenoid(size_t index,bool Close) {}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void UI_TimeChange(double dTime_s);
	private:
		Wheel_UI m_Wheel[4];
};

#if 0
class Swerve_Robot_Properties : public UI_Ship_Properties
{
	public:
		Swerve_Robot_Properties();
		//I'm not going to implement script support mainly due to lack of time, but also this is a specific object that
		//most likely is not going to be sub-classed (i.e. sealed)... if this turns out different later we can implement
		//virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Ship_1D_Properties &GetArmProps() const {return m_ArmProps;}
		const Ship_1D_Properties &GetClawProps() const {return m_ClawProps;}
	private:
		Ship_1D_Properties m_ArmProps,m_ClawProps;
};
#endif
