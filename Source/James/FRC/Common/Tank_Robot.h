#pragma once

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
	typedef Framework::Base::Vec2d Vec2D;
	//typedef osg::Vec2d Vec2D;

	//This is a measurement of the width x length of the wheel base, where the length is measured from the center axis of the wheels, and
	//the width is a measurement of the the center of the wheel width to the other wheel
	Vec2D WheelDimensions;
	double WheelDiameter;
	double MotorToWheelGearRatio;  //Used to interpolate RPS of the encoder to linear velocity
	double LeftPID[3]; //p,i,d
	double RightPID[3]; //p,i,d
};

class Tank_Robot_UI;

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class Tank_Robot : public Tank_Drive
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		Tank_Robot(const char EntityName[],Tank_Drive_Control_Interface *robot_control,bool UseEncoders=false);
		IEvent::HandlerList ehl;
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();
		void SetUseEncoders(bool UseEncoders) {m_UsingEncoders=UseEncoders;}
		virtual void TimeChange(double dTime_s);
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
	protected:
		//friend Tank_Robot_UI;

		virtual void ComputeDeadZone(double &LeftVoltage,double &RightVoltage);
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		virtual void RequestedVelocityCallback(double VelocityToUse,double DeltaTime_s);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		virtual const Vec2D &GetWheelDimensions() const {return m_TankRobotProps.WheelDimensions;}
		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	private:
		typedef  Tank_Drive __super;
		Tank_Drive_Control_Interface * const m_RobotControl;
		PIDController2 m_PIDController_Left,m_PIDController_Right;
		double m_CalibratedScaler_Left,m_CalibratedScaler_Right; //used for calibration
		bool m_UsingEncoders;
		bool m_VoltageOverride;  //when true will kill voltage
		bool m_UseDeadZoneSkip; //Manages when to use the deadzone (mainly false during autonomous deceleration)
		Vec2D m_EncoderGlobalVelocity;  //cache for later use
		double m_EncoderHeading;
		Tank_Robot_Props m_TankRobotProps; //cached in the Initialize from specific robot
};

class Tank_Robot_Properties : public Ship_Properties
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;

		Tank_Robot_Properties();

		const Tank_Robot_Props &GetTankRobotProps() const {return m_TankRobotProps;}
	protected:
		Tank_Robot_Props m_TankRobotProps;
};
