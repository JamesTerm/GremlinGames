#pragma once

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
	private:
		void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		void InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &Torque,double dTime_s);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};
