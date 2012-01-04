#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Tank_Drive_Control_Interface
{
	public:
		//This is primarily used for updates to dashboard and driver station during a test build
		virtual void Robot_Control_TimeChange(double dTime_s)=0;
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;
		virtual void Reset_Encoders()=0;

		//Encoders populate this with current velocity of motors
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity)=0;  ///< in meters per second
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage)=0;
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Tank_Robot : public Tank_Drive
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		void SetUseEncoders(bool UseEncoders) {m_UsingEncoders=UseEncoders;}
		virtual void TimeChange(double dTime_s);
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);

		static double RPS_To_LinearVelocity(double RPS);

	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
	private:
		//typedef  Tank_Drive __super;
		Tank_Drive_Control_Interface * const m_RobotControl;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		bool m_UsingEncoders;
		bool m_VoltageOverride;  //when true will kill voltage
		bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderHeading;
		Vec2D m_WheelDimensions; //cached from the FRC_2011_Robot_Properties
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Tank_Robot_Control : public Tank_Drive_Control_Interface
{
	public:
		Tank_Robot_Control();
		//This is only needed for simulation
		virtual void Robot_Control_TimeChange(double dTime_s);
	protected: //from Robot_Control_Interface
		virtual void Reset_Encoders();
		virtual void Initialize(const Entity_Properties *props);
		virtual void GetLeftRightVelocity(double &LeftVelocity,double &RightVelocity);
		virtual void UpdateLeftRightVoltage(double LeftVoltage,double RightVoltage);
	protected:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		Encoder_Tester m_Encoders;
		KalmanFilter m_KalFilter_Arm,m_KalFilter_EncodeLeft,m_KalFilter_EncodeRight;
		//cache voltage values for display
		double m_LeftVoltage,m_RightVoltage;
};

class Tank_Robot_Properties : public UI_Ship_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		Tank_Robot_Properties();

		//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
		//the width is a measurement of the the center of the wheel width to the other wheel
		const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
	private:
		Vec2D m_WheelDimensions;
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

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class Tank_Robot_UI : public Tank_Robot, public Tank_Robot_Control
{
	public:
		Tank_Robot_UI(const char EntityName[]) : Tank_Robot(EntityName,this),Tank_Robot_Control() {}
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);

		virtual void UI_Init(Actor_Text *parent);
		virtual void custom_update(osg::NodeVisitor *nv, osg::Drawable *draw,const osg::Vec3 &parent_pos);
		virtual void Text_SizeToUse(double SizeToUse);

		virtual void UpdateScene (osg::Geode *geode, bool AddOrRemove);

		virtual void TimeChange(double dTime_s);
	private:
		Tank_Wheel_UI m_Wheel[6];
};
