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
		~Swerve_Robot();
		IEvent::HandlerList ehl;
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		void SetUseEncoders(bool UseEncoders) {m_UsingEncoders=UseEncoders;}
		virtual void TimeChange(double dTime_s);
		static double RPS_To_LinearVelocity(double RPS);

		//Accessors needed for setting goals
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		//virtual void BindAdditionalEventControls(bool Bind);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
		virtual const SwerveVelocities &GetSwerveVelocities() const {return m_Swerve_Robot_Velocities;}

		//Get the sweet spot between the update and interpolation to avoid oscillation 
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
	private:
		//typedef  Tank_Drive __super;
		Robot_Control_Interface * const m_RobotControl;

		//The driving module consists of a swivel motor and the driving motor for a wheel.  It manages / converts the intended direction and speed to 
		//actual direction and velocity (i.e. works in reverse) as well as working with sensor feedback (e.g. potentiometer, encoder) for error
		//correction of voltage computation.
		class DrivingModule
		{
			public:
				DrivingModule(const char EntityName[],Robot_Control_Interface *robot_control);
				struct DrivingModule_Props
				{
					const Ship_1D_Properties *Swivel_Props;
					const Ship_1D_Properties *Drive_Props;
				};
				virtual void Initialize(GG_Framework::Base::EventMap& em,const DrivingModule_Props *props=NULL);
				virtual void TimeChange(double dTime_s);
				void SetIntendedSwivelDirection(double direction) {m_IntendedSwivelDirection=direction;}
				void SetIntendedDriveVelocity(double Velocity) {m_IntendedDriveVelocity=Velocity;}
				//I have no problem exposing read-only access to these :)
				const Ship_1D &GetSwivel() {return m_Swivel;}
				const Ship_1D &GetDrive() {return m_Drive;}
				void ResetPos() {m_Drive.ResetPos(),m_Swivel.ResetPos();}
			private:
				std::string m_ModuleName,m_SwivelName,m_DriveName;
				Ship_1D m_Swivel;  //apply control to swivel mechanism
				Ship_1D m_Drive;  //apply control to drive motor
				//Pass along the intended swivel direction and drive velocity
				double m_IntendedSwivelDirection,m_IntendedDriveVelocity;

				Robot_Control_Interface * const m_RobotControl;
		} *m_DrivingModule[4]; //FL, FR, RL, RR  The four modules used  (We could put 6 here if we want)

		bool m_UsingEncoders;
		Vec2D m_WheelDimensions; //cached from the Swerve_Robot_Properties
		SwerveVelocities m_Swerve_Robot_Velocities;
};

class Swerve_Robot_Properties : public UI_Ship_Properties
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		Swerve_Robot_Properties();
		const Ship_1D_Properties &GetSwivelProps() const {return m_SwivelProps;}
		const Ship_1D_Properties &GetDriveProps() const {return m_DriveProps;}
		//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
		//the width is a measurement of the the center of the wheel width to the other wheel
		const Vec2D &GetWheelDimensions() const {return m_WheelDimensions;}
	private:
		//Note the drive properties is a measurement of linear movement (not angular velocity)
		Ship_1D_Properties m_SwivelProps,m_DriveProps;
		Vec2D m_WheelDimensions;
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
		Wheel_UI() : m_UIParent(NULL) {}
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

		virtual void TimeChange(double dTime_s);
	private:
		Wheel_UI m_Wheel[4];
};
