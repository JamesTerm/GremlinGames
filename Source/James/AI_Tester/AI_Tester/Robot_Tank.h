#pragma once

///This kind of ship will convert the torque and force into a two fixed point force/thrust system like that of a tank.  It also interpolates the left
///right velocities back into the torque and force.  Unlike a ship, it will always absorb any lateral forces as it is assumed that the entity will not
///skid in those directions.  This means it cannot strafe, and will behave much like a vehicle on land rather than a ship in space.
class Robot_Tank : public Ship_Tester
{
	public:
		Robot_Tank(const char EntityName[]);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();
	protected:
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
	private:
		void InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &Torque,double dTime_s);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};

///This is the interface to control the robot.  It is presented in a generic way that is easily compatible to the ship and robot tank
class Robot_Control_Interface
{
	public:
		//We need to pass the properties to the Robot Control to be able to make proper conversions.
		//The client code may cast the properties to obtain the specific data 
		virtual void Initialize(const Entity_Properties *props)=0;

		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity)=0;  ///< in meters per second
		virtual void UpdateArmHeight(double Height_m)=0;  ///< in meters
};

///This is a specific robot that is a robot tank and is composed of an arm, it provides addition methods to control the arm, and applies updates to
///the Robot_Control_Interface
class FRC_2011_Robot : public Robot_Tank
{
	public:
		FRC_2011_Robot(const char EntityName[],Robot_Control_Interface *robot_control);
		virtual void Initialize(Entity2D::EventMap& em, const Entity_Properties *props=NULL);
	protected:
		//This method is the perfect moment to obtain the new velocities and apply to the interface
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
	private:
		Robot_Control_Interface * const m_RobotControl;
};

///This class is a dummy class to use for simulation only.  It does however go through the conversion process, so it is useful to monitor the values
///are correct
class Robot_Control : public Robot_Control_Interface
{
	protected: //from Robot_Control_Interface
		virtual void Initialize(const Entity_Properties *props);
		virtual void UpdateLeftRightVelocity(double LeftVelocity,double RightVelocity);
		virtual void UpdateArmHeight(double Height_m);
	private:
		double m_ENGAGED_MAX_SPEED;  //cache this to covert velocity to motor setting
};

///This is only for the simulation where we need not have client code instantiate a Robot_Control
class FRC_2011_Robot_tester : public FRC_2011_Robot, public Robot_Control
{
	public:
		FRC_2011_Robot_tester(const char EntityName[]) : FRC_2011_Robot(EntityName,this) {}
};