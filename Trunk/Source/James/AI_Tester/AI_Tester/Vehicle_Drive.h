#pragma once

class Vehicle_Drive_Common_Interface
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		//override the wheel dimensions, which by default are the entities dimensions (a good approximation)
		virtual const Vec2D &GetWheelDimensions() const =0;
		//This returns the measurement of the turning diameter where the wheels turn within themselves
		//usually for a 4 wheel drive this is length from corner to corner, and for a 6 wheel drive this is just the track width
		virtual double GetWheelTurningDiameter() const =0;
		virtual double Vehicle_Drive_GetAtt_r() const=0;
		virtual const PhysicsEntity_2D &Vehicle_Drive_GetPhysics() const=0;
		//This method will help me identify write operation cases easier
		virtual PhysicsEntity_2D &Vehicle_Drive_GetPhysics_RW()=0;
};

class Vehicle_Drive_Common
{
	public:
		Vehicle_Drive_Common(Vehicle_Drive_Common_Interface *VehicleProps);
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;

		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s)=0;

		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		//Unlike in robot tank We'll only cache the values to work with in the Apply Thrusters, and apply them only to the inject displacement
		//This way when swerve is unable to deliver due to error and limitations, the actual control will not be compromised
		void Vehicle_Drive_Common_ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		bool Vehicle_Drive_Common_InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
		//TODO verify this is not needed
		//const Vec2D &GetCachedLocalForce() {return m_CachedLocalForce;}
	protected:
		Vehicle_Drive_Common_Interface * const m_VehicleProps;
	private:
		Vec2D m_CachedLocalForce,m_CachedLinearVelocity;
		double m_CachedTorque,m_CachedAngularVelocity;
};

///This kind of ship will convert the torque and force into a two fixed point force/thrust system like that of a tank.  It also interpolates the left
///right velocities back into the torque and force.  Unlike a ship, it will always absorb any lateral forces as it is assumed that the entity will not
///skid in those directions.  This means it cannot strafe, and will behave much like a vehicle on land rather than a ship in space.
class Tank_Drive : public Vehicle_Drive_Common
{
	private:
		Vehicle_Drive_Common_Interface * const m_pParent;
		//typedef Ship_2D __super;
		double m_LeftLinearVelocity,m_RightLinearVelocity;
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Tank_Drive(Vehicle_Drive_Common_Interface *Parent);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		//This method converts the given left right velocities into a form local linear velocity and angular velocity
		void InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
		//Overload this for optimal time between the update and position to avoid oscillation
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);

		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
};

struct SwerveVelocities
{
	enum SectionOrder
	{
		eFrontLeft,
		eFrontRight,
		eRearLeft,
		eRearRight
	};
	SwerveVelocities()
	{
		memset(this,0,sizeof(SwerveVelocities));
	}
	union uVelocity
	{
		struct Explicit
		{
			double sFL,sFR,sRL,sRR; //wheel tangential speeds in MPS
			double aFL,aFR,aRL,aRR; //wheel angles in radians clockwise from straight ahead
		} Named;
		double AsArray[8];
	} Velocity;
};

class Swerve_Drive_Interface : public Vehicle_Drive_Common_Interface
{
	public:
	virtual const SwerveVelocities &GetSwerveVelocities() const =0;
};

class Swerve_Drive : public Vehicle_Drive_Common
{
	private:
		Swerve_Drive_Interface * const m_pParent;
		//typedef Ship_2D __super;
		SwerveVelocities m_Velocities;
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Swerve_Drive(Swerve_Drive_Interface *Parent);
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		double GetIntendedVelocitiesFromIndex(size_t index) const; //This is sealed always using m_Velocities
		double GetSwerveVelocitiesFromIndex(size_t index) const; //This is sealed always using m_Velocities
		//Overload this for optimal time between the update and position to avoid oscillation
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);

		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);
	protected:
		//This method converts the given left right velocities into a form local linear velocity and angular velocity
		void InterpolateVelocities(SwerveVelocities Velocities,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
		const SwerveVelocities &GetIntendedVelocities() const {return m_Velocities;}
};