#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;

Robot_Tank::Robot_Tank(const char EntityName[]) : Ship_Tester(EntityName), m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}

void Robot_Tank::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	class Internal
	{
		public:
			void UpdateVelocities()
			{

			}
			void InterpolateThrusterChanges(osg::Vec2d &LocalForce,double LocalTorque)
			{
			}
	} _;
	_.UpdateVelocities();
	osg::Vec2d NewLocalForce(LocalForce);
	double NewTorque=LocalTorque;
	_.InterpolateThrusterChanges(NewLocalForce,NewTorque);
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,TorqueRestraint,dTime_s);
}

