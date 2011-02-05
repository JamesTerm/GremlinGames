#pragma once

class Robot_Tank : public Ship_Tester
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		Robot_Tank(const char EntityName[]);
		double GetLeftVelocity() const {return m_LeftLinearVelocity;}
		double GetRightVelocity() const {return m_RightLinearVelocity;}
		// Places the ship back at its initial position and resets all vectors
		virtual void ResetPos();
	protected:
		//This will convert the force into both motor velocities and interpolate the final torque and force to apply
		virtual void ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s);
	private:
		typedef Ship_2D __super;
		void UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double Torque,double TorqueRestraint,double dTime_s);
		void InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s);
		double m_LeftLinearVelocity,m_RightLinearVelocity;
};
