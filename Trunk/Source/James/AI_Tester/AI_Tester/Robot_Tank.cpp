#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace std;

const double PI=M_PI;

Robot_Tank::Robot_Tank(const char EntityName[]) : Ship_Tester(EntityName), m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}
void Robot_Tank::UpdateVelocities()
{

}

void Robot_Tank::InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &LocalTorque)
{
	osg::Vec2d LocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	double LeftLinearVelocity=m_LeftLinearVelocity,RightLinearVelocity=m_RightLinearVelocity;
	double LeftMagnitude=fabs(m_LeftLinearVelocity);
	double RightMagnitude=fabs(m_RightLinearVelocity);
	double CommonMagnitude=min(LeftMagnitude,RightMagnitude);
	double RightAngularDelta;
	double LeftAngularDelta;
	//We do not care about x, but we may want to keep an eye for intense x forces
	LocalForce[0]=0.0;
	double Radius=GetDimensions()[0];
	//See if velocities are going in the same direction
	if (m_LeftLinearVelocity * m_RightLinearVelocity >= 0)
	{
		//First lets simplify the the overall velocity by transferring the common speed to local force
		double CommonVelocity=m_LeftLinearVelocity>0?CommonMagnitude:-CommonMagnitude; //put back in the correct direction
		LeftLinearVelocity-=CommonVelocity;
		RightLinearVelocity-=CommonVelocity;
		RightAngularDelta=LeftLinearVelocity / (2 * PI * Radius);
		LeftAngularDelta=-(RightLinearVelocity / (2 * PI * Radius));

		double NewYVelocity=CommonVelocity;
		//We also need to add displacement of the turn... when both wheels are going in opposite directions, this displacement will cancel out
		{
			double HalfRadius=Radius/2.0;
			double Height=(sin(RightAngularDelta) * HalfRadius) + (sin(-LeftAngularDelta) * HalfRadius);
			double Width=((1.0-cos(RightAngularDelta))*HalfRadius) + ((1.0-cos(LeftAngularDelta))*HalfRadius);
			double LinearAcceleration=Width-LocalVelocity[0];
			LocalForce[0]=LinearAcceleration*Mass;
			CommonVelocity+=Height;
		}
		double LinearAcceleration=CommonVelocity-LocalVelocity[1];
		LocalForce[1]=LinearAcceleration*Mass;
	}
	else
	{
		//going in different direction
		//Cancel out the opposing forces 
		double RemainderMagnitude=(LeftMagnitude-CommonMagnitude) + (RightMagnitude-CommonMagnitude);
		double RemainderVelocity;
		if (LeftMagnitude>RightMagnitude)
		{
			RemainderVelocity=m_LeftLinearVelocity>0?RemainderMagnitude:-RemainderMagnitude;
			LeftLinearVelocity-=RemainderVelocity;  //take off the remainder for equal opposing forces
		}
		else
		{
			RemainderVelocity=m_RightLinearVelocity>0?RemainderMagnitude:-RemainderMagnitude;
			RightLinearVelocity-=RemainderVelocity;  //take off the remainder for equal opposing forces
		}
		//To to put the remainder back into linear force
		double LinearAcceleration=RemainderVelocity-LocalVelocity[1];
		LocalForce[1]=LinearAcceleration*Mass;
		RightAngularDelta=LeftLinearVelocity / (2 * PI * Radius);
		LeftAngularDelta=RightLinearVelocity / (2 * PI * Radius);
	}
	//Now then we'll compute the torque
	double AngularAcceleration=(LeftAngularDelta+RightAngularDelta) - m_Physics.GetAngularVelocity();
	LocalTorque = AngularAcceleration * Mass;

}

void Robot_Tank::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities();
	osg::Vec2d NewLocalForce(LocalForce);
	double NewTorque=LocalTorque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque);
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,TorqueRestraint,dTime_s);
}

