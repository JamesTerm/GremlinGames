#pragma once

///This kind of ship will convert the torque and force into a two fixed point force/thrust system like that of a tank.  It also interpolates the left
///right velocities back into the torque and force.  Unlike a ship, it will always absorb any lateral forces as it is assumed that the entity will not
///skid in those directions.  This means it cannot strafe, and will behave much like a vehicle on land rather than a ship in space.
class Tank_Drive : public Ship_Tester
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Tank_Drive(const char EntityName[]);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();
	protected:
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//This method converts the given left right velocities into a form local linear velocity and angular velocity
		void InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
		//override the wheel dimensions, which by default are the entities dimensions (a good approximation)
		virtual const Vec2D &GetWheelDimensions() const {return GetDimensions();}
	private:
		//typedef Ship_2D __super;
		void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};

class Swerve_Drive : public Ship_Tester
{
	public:
		//typedef Framework::Base::Vec2d Vec2D;
		typedef osg::Vec2d Vec2D;
		Swerve_Drive(const char EntityName[]);
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
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();

		double GetIntendedVelocitiesFromIndex(size_t index) const; //This is sealed always using m_Velocities
		double GetSwerveVelocitiesFromIndex(size_t index) const; //This is sealed always using m_Velocities
	protected:
		//Overload this for optimal time between the update and position to avoid oscillation
		virtual void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		//Unlike in robot tank We'll only cache the values to work with in the Apply Thrusters, and apply them only to the inject displacement
		//This way when swerve is unable to deliver due to error and limitations, the actual control will not be compromised
		void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
		virtual bool InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement);

		virtual void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		//This method converts the given left right velocities into a form local linear velocity and angular velocity
		void InterpolateVelocities(SwerveVelocities Velocities,Vec2D &LocalVelocity,double &AngularVelocity,double dTime_s);
		//Override this if the actual velocities are different than the intended velocities as InterpolateVelocities() calls this accessor
		virtual const SwerveVelocities &GetSwerveVelocities() const {return m_Velocities;}
		const SwerveVelocities &GetIntendedVelocities() const {return m_Velocities;}
		//override the wheel dimensions, which by default are the entities dimensions (a good approximation)
		virtual const Vec2D &GetWheelDimensions() const {return GetDimensions();}
	private:
		//typedef Ship_2D __super;
		SwerveVelocities m_Velocities;
		Vec2D m_CachedLocalForce,m_CachedLinearVelocity;
		double m_CachedTorque,m_CachedAngularVelocity;
};