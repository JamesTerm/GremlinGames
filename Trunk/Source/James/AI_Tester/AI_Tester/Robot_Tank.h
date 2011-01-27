#pragma once

class Robot_Tank : public Ship_Tester
{
	public:
		Robot_Tank(const char EntityName[]);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
	protected:
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
	private:
		void UpdateVelocities();
		void InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &LocalTorque);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};
