#pragma once

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Robot_Control_Interface
{
	public:
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;

		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity)=0;  ///< in meters per second
		virtual void UpdateArmVelocity(double Velocity)=0;  ///< in meters per second
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC_2011_Robot : public Robot_Tank
{
	public:
		FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control);
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
		virtual void ResetPos();

		class Robot_Arm : public Ship_1D
		{
			public:
				Robot_Arm(const char EntityName[],Robot_Control_Interface *robot_control);
				IEvent::HandlerList ehl;
				static double HeightToAngle_r(double Height_m);
				static double AngleToHeight_m(double Angle_r);
			protected:
				//Intercept the time change to obtain current height as well as sending out the desired velocity
				virtual void TimeChange(double dTime_s);
				virtual void BindAdditionalEventControls(bool Bind);
			private:
				void SetRequestedVelocity_FromNormalized(double Velocity);
				void SetPos0feet();
				void SetPos3feet();
				void SetPos6feet();
				void SetPos9feet();
				Robot_Control_Interface * const m_RobotControl;
				double m_LastNormalizedVelocity;  //this is managed direct from being set to avoid need for precision tolerance
		};

		//Accessor needed for setting goals
		Robot_Arm &GetArm() {return m_Arm;}
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		virtual void TimeChange(double dTime_s);
		virtual void BindAdditionalEventControls(bool Bind);
	private:
		Robot_Control_Interface * const m_RobotControl;
		Robot_Arm m_Arm;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Robot_Control : public Robot_Control_Interface
{
	protected: //from Robot_Control_Interface
		virtual void Initialize(const Entity_Properties *props);
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity);
		virtual void UpdateArmVelocity(double Velocity);
	private:
		double m_RobotMaxSpeed;  //cache this to covert velocity to motor setting
		double m_ArmMaxSpeed;
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2011_Robot_tester : public FRC_2011_Robot, public Robot_Control
{
	public:
		FRC_2011_Robot_tester(const char EntityName[]) : FRC_2011_Robot(EntityName,this) {}
};

class FRC_2011_Robot_Properties : public UI_Ship_Properties
{
	public:
		FRC_2011_Robot_Properties();
		//I'm not going to implement script support mainly due to lack of time, but also this is a specific object that
		//most likely is not going to be sub-classed (i.e. sealed)... if this turns out different later we can implement
		//virtual void LoadFromScript(GG_Framework::Logic::Scripting::Script& script);
		const Ship_1D_Properties &GetArmProps() const {return m_ArmProps;}
	private:
		Ship_1D_Properties m_ArmProps;
};