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

void Robot_Tank::InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &LocalTorque,double dTime_s)
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
	double NewVelocityY;
	//See if velocities are going in the same direction
	if (m_LeftLinearVelocity * m_RightLinearVelocity >= 0)
	{
		//First lets simplify the the overall velocity by transferring the common speed to local force
		double CommonVelocity=m_LeftLinearVelocity>0?CommonMagnitude:-CommonMagnitude; //put back in the correct direction
		LeftLinearVelocity-=CommonVelocity;
		RightLinearVelocity-=CommonVelocity;
		NewVelocityY=CommonVelocity;
	}
	else
	{
		//going in different direction
		NewVelocityY=0;  //nothing to do... the common code will cancel them out
	}

	RightAngularDelta=LeftLinearVelocity / (2 * PI * Radius);
	LeftAngularDelta=-(RightLinearVelocity / (2 * PI * Radius));

	//We also need to add displacement of the left over turn..
	{
		double HalfRadius=Radius/2.0;
		double Height=(sin(RightAngularDelta) * HalfRadius) + (sin(-LeftAngularDelta) * HalfRadius);
		double Width=((1.0-cos(RightAngularDelta))*HalfRadius) + (-(1.0-cos(LeftAngularDelta))*HalfRadius);
		double LinearAcceleration=Width-LocalVelocity[0];

		LocalForce[0]=(LinearAcceleration*Mass) / dTime_s;
		NewVelocityY+=Height;
	}
	double LinearAcceleration=NewVelocityY-LocalVelocity[1];
	LocalForce[1]=(LinearAcceleration*Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=(LeftAngularDelta+RightAngularDelta) - m_Physics.GetAngularVelocity();
	LocalTorque = (AngularAcceleration * Mass) / dTime_s;
}

void Robot_Tank::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities();
	osg::Vec2d NewLocalForce(LocalForce);
	double NewTorque=LocalTorque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,TorqueRestraint,dTime_s);
}

