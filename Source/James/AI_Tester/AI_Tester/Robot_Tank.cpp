#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace std;

const double PI=M_PI;

Robot_Tank::Robot_Tank(const char EntityName[]) : Ship_Tester(EntityName), m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}

void Robot_Tank::ResetPos()
{
	m_LeftLinearVelocity=m_RightLinearVelocity=0.0;
	__super::ResetPos();
}

void Robot_Tank::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);
	double LinearVelocityDelta;

	//First we compute the Y component force to the velocities in the direction that it currently is facing
	//I'm writing this out so I can easily debug
	{
		double AccelerationDelta=LocalForce[1]/Mass;
		LinearVelocityDelta=AccelerationDelta*dTime_s;
	}

	#if 0
	//determine direction
	double ForceHeading;
	{
		osg::Vec2d LocalForce_norm(LocalForce);
		LocalForce_norm.normalize();
		ForceHeading=atan2(LocalForce_norm[0],LocalForce_norm[1]);
		//DOUT2("x=%f y=%f h=%f\n",LocalForce[0],LocalForce[1],RAD_2_DEG(ForceHeading));
	}
	#endif
	double LeftDelta,RightDelta;
	//Now to blend the torque into the velocities
	{
		double Radius=GetDimensions()[0];
		//first convert to angular acceleration
		double AccelerationDelta=Torque/Mass;
		double AngularVelocityDelta=AccelerationDelta*dTime_s;
		//Convert the angular velocity into linear velocity
		double AngularVelocityDelta_linear=AngularVelocityDelta * (2 * PI * Radius);
		//I'm keeping this first attempt, I like it because it is simple and reliable however, when going forward in fast speeds the torque will clobber the
		//linear force with abrupt stopping 

		LeftDelta=(AngularVelocityDelta_linear/2)+LinearVelocityDelta;
		RightDelta=(-AngularVelocityDelta_linear/2)+LinearVelocityDelta;

		#if 1
		osg::Vec2d NewDelta(LeftDelta,RightDelta);
		osg::Vec2d CurrentVelocity(m_LeftLinearVelocity,m_RightLinearVelocity);
		for (size_t i=0;i<2;i++)
		{
			if (CurrentVelocity[i] * AngularVelocityDelta_linear >0.0)
			{
				//The left velocity is about to increase see if the linear velocity is going in the same direction
				if ((CurrentVelocity[i] * LinearVelocityDelta)>0.0)
					NewDelta[i]/=2;  //average out the linear and the angular
			}
			//Now to apply the final force restraint
			double Restraint=(CurrentVelocity[i]>0.0)?MaxAccelForward:MaxAccelReverse;
			if (Restraint>0.0)  //test for -1
			{
				Restraint=min(fabs(NewDelta[i]),Restraint);
				if (NewDelta[i]<0.0)
					Restraint*=-1.0;  //restore the negative sign
				NewDelta[i]=Restraint;
			}
		}
		LeftDelta=NewDelta[0];
		RightDelta=NewDelta[1];

		#endif


	}
	//Now to apply to the velocities with speed control  
	double NewVelocity=m_LeftLinearVelocity+LeftDelta;
	if (fabs(NewVelocity)>ENGAGED_MAX_SPEED)
		NewVelocity=(NewVelocity>0)?ENGAGED_MAX_SPEED:-ENGAGED_MAX_SPEED;
	m_LeftLinearVelocity=NewVelocity;

	NewVelocity=m_RightLinearVelocity+RightDelta;
	if (fabs(NewVelocity)>ENGAGED_MAX_SPEED)
		NewVelocity=(NewVelocity>0)?ENGAGED_MAX_SPEED:-ENGAGED_MAX_SPEED;
	m_RightLinearVelocity=NewVelocity;	

	DOUT2("left=%f right=%f \n",m_LeftLinearVelocity,m_RightLinearVelocity);
}

void Robot_Tank::InterpolateThrusterChanges(osg::Vec2d &LocalForce,double &Torque,double dTime_s)
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
	Torque = (AngularAcceleration * Mass) / dTime_s;
}

void Robot_Tank::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const osg::Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	osg::Vec2d NewLocalForce(LocalForce);
	double NewTorque=Torque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,-1,dTime_s);
}

