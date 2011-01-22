#include "stdafx.h"
#include "GG_Framework.Logic.Physics.h"
#include "..\..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic::Physics;
using namespace GG_Framework::Base;
using namespace std;

const double Pi2=M_PI*2.0;
//The actual force between two objects are f=(G m1 m2)/ r^2
//For example g = ( G * (Me->5.98E+24)) / (Re->6.38E+6)^2 = 9.8 m/s^2
//G is ideal to compute forces on ships from various planets
const double G=6.673E-11;


inline bool PosBNE(double val,double t)
{
	return !(fabs(val-t)<1E-3);
}


inline const osg::Vec3d Vec3Multiply (const osg::Vec3d &A,const osg::Vec3d &rhs)
{
	return osg::Vec3d(A[0]*rhs._v[0], A[1]*rhs._v[1], A[2]*rhs._v[2]);
}

inline const osg::Vec3d Vec3_abs (const osg::Vec3d &A)
{
	return osg::Vec3d(fabs(A[0]),fabs( A[1]),fabs( A[2]));
}
inline const osg::Vec3d Vec3_min (const osg::Vec3d &A,const osg::Vec3d &B)
{
	return osg::Vec3d(min(A[0],B[0]),min(A[1],B[1]),min(A[2],B[2]));
}


namespace GG_Framework
{
	namespace Logic
	{
		namespace Physics
		{

//Normalize it in the bound of Pi2
inline void NormalizeVec3ToPi2(osg::Vec3d &A)
{
	for (size_t i=0;i<3;i++)
	{
		if (A[i]<0)
			A[i]+=Pi2;
		else if (A[i]>Pi2)
			A[i]-=Pi2;
	}
}

inline void NormalizeVec3ToPi2wDirection(osg::Vec3d &A)
{
	for (size_t i=0;i<3;i++)
	{
		if (A[i]<-Pi2)
			A[i]+=Pi2;
		else if (A[i]>Pi2)
			A[i]-=Pi2;
	}
}

		}
	}
}


void PhysicsEntity::SetMass(double mass)
{
	m_EntityMass=mass;
}
double PhysicsEntity::GetMass() const
{
	return m_EntityMass;
}

void PhysicsEntity::CopyFrom(const PhysicsEntity& rhs)
{
	m_LinearVelocity= rhs.m_LinearVelocity;
	m_LinearAcceleration= rhs.m_LinearAcceleration;
	m_AngularVelocity= rhs.m_AngularVelocity;
	m_AngularAcceleration= rhs.m_AngularAcceleration;
	m_TrackAccTime=rhs.m_TrackAccTime;
	m_LinearAccQueue = rhs.m_LinearAccQueue;
	m_TorqueAccQueue = rhs.m_TorqueAccQueue;
}

void PhysicsEntity::ResetVectors()
{
	m_LinearVelocity= osg::Vec3d(0,0,0);
	m_LinearAcceleration= osg::Vec3d(0,0,0);
	m_AngularVelocity= osg::Vec3d(0,0,0);
	m_AngularAcceleration= osg::Vec3d(0,0,0);
	m_TrackAccTime=0.0;
	while (!m_LinearAccQueue.empty())
		m_LinearAccQueue.pop();
	while (!m_TorqueAccQueue.empty())
		m_TorqueAccQueue.pop();
#ifdef __TestInput__
	printf("Enter Test Input\nInput >");
	scanf("%lf",&m_TestInput);
#endif
}

PhysicsEntity::PhysicsEntity()
{
	//Plug in some good defaults
	m_EntityMass=500; //about 5000 pounds
	//m_EntityMass=200; //about 2000 pounds
	m_StaticFriction=0.8;
	m_KineticFriction=0.2;
	m_AngularInertiaCoefficient=1.0;
	m_RadiusOfConcentratedMass=osg::Vec3d(1.0,1.0,1.0);
	m_SummedExternalForces=osg::Vec3d(0,0,0);
	m_lastTime_s=0.0;


	ResetVectors();
}

void PhysicsEntity::SetFriction(double StaticFriction,double KineticFriction)
{
	m_StaticFriction=StaticFriction;
	m_KineticFriction=KineticFriction;
}

void PhysicsEntity::SetAngularInertiaCoefficient(double AngularInertiaCoefficient)
{
	m_AngularInertiaCoefficient=AngularInertiaCoefficient;
}

void PhysicsEntity::SetRadiusOfConcentratedMass(const osg::Vec3d &RadiusOfConcentratedMass)
{
	m_RadiusOfConcentratedMass=RadiusOfConcentratedMass;
}

const osg::Vec3d &PhysicsEntity::GetRadiusOfConcentratedMass() const
{
	return m_RadiusOfConcentratedMass;
}

void PhysicsEntity::SetLinearVelocity( const osg::Vec3d &LinearVelocity)
{ 
	m_LinearVelocity=LinearVelocity;
}
osg::Vec3d PhysicsEntity::GetLinearVelocity() const
{
	return m_LinearVelocity;
}

osg::Vec3d PhysicsEntity::GetLinearAcceleration() const
{
	return m_LinearAcceleration;
}

double PhysicsEntity::GetSpeed(const osg::Vec3d &Velocity)
{
	//This is basically the distance formula in 3d space
	return sqrt((Velocity[0]*Velocity[0])+
				(Velocity[1]*Velocity[1])+
				(Velocity[2]*Velocity[2]));
}

void PhysicsEntity::SetAngularVelocity( const osg::Vec3d &AngularVelocity)
{
	m_AngularVelocity=AngularVelocity;
}
osg::Vec3d PhysicsEntity::GetAngularAcceleration() const
{
	return m_AngularAcceleration;
}

osg::Vec3d PhysicsEntity::GetAngularVelocity() const
{
	return m_AngularVelocity;
}

void PhysicsEntity::ApplyFractionalForce( const osg::Vec3d &force,double FrameDuration)
{
	//I'm writing this out so I can easily debug
	osg::Vec3d AccelerationDelta=force/m_EntityMass;
	osg::Vec3d VelocityDelta=AccelerationDelta*FrameDuration;
	m_LinearVelocity+=VelocityDelta;

	//if (AccelerationDelta[1]!=0)
	//	DebugOutput("Acc%f Vel%f\n",AccelerationDelta[1],m_LinearVelocity[1]);
}
void PhysicsEntity::ApplyFractionalTorque( const osg::Vec3d &torque,double FrameDuration,const osg::Vec3d &RadialArmDistance)
{
	osg::Vec3d AccelerationDelta=GetAngularAccelerationDelta(torque,RadialArmDistance);
	osg::Vec3d VelocityDelta=AccelerationDelta*FrameDuration;
	m_AngularVelocity+=VelocityDelta;
}

void PhysicsEntity::ApplyFractionalForce( const osg::Vec3d &force, const osg::Vec3d &point,double FrameDuration )
{
	//Use this as a "get by" if the code doesn't work properly
#if 0
	ApplyFractionalForce(force,FrameDuration);
	return;
#endif

#if 1  // James Technique

	//Here is a rough draft to solve in 2 dimensions
	//A=atan2(py,px)   point
	//M=pi/2 - A
	//L=atan2(fy,fx)  force
	//N=L+M
	//Y=sin(N)*f.length = contribution for force
	//X=cos(N)*f.length = contribution for torque
	
	//This finds the corresponding "x" and "y" component for each iteration
	//0=left right 1=front back 2=up down... so:
	//top view 0,1  side view 1,2 front view 0,2 
	size_t Gety[3]={1,2,2};
	size_t Getx[3]={0,1,0};

	osg::Vec3d TorqueToApply;
	osg::Vec3d ForceToApply;
	osg::Vec3d RadialArmDistance;
	for (size_t i=0;i<3;i++)
	{
		double A=atan2(point[Gety[i]],point[Getx[i]]);
		double M=(M_PI/2) - A;
		double L=atan2(-force[Gety[i]],-force[Getx[i]]);
		double N=L+M;

		double ForceLength= sqrt((force[Gety[i]]*force[Gety[i]])+(force[Getx[i]]*force[Getx[i]]));
		RadialArmDistance[i]= sqrt((point[Gety[i]]*point[Gety[i]])+(point[Getx[i]]*point[Getx[i]]));
		//I've reserved a special case for ships which haven't specified  their radius size, in which case we simply factor out the radial arm too
		if ((m_RadiusOfConcentratedMass[i]==1.0)&&(RadialArmDistance[i]>1.0)) RadialArmDistance[i]=1.0;

		//Fr = t   ... We should multiply force by the radial arm distance to get the torque
		//but instead,  we pass it off to physics where the multiply gets applied directly against the Radius of Concentrated Mass
		//We could multiply here but doing it the other way keeps the torque value low, which also makes it easier to debug
		TorqueToApply[i]=(cos(N)*ForceLength); 
		//The Pitch axis works in reverse?
		if (i==1)
			TorqueToApply[i]=-TorqueToApply[i];
	}

	osg::Vec3d vecToCenter = -point;
	//Note we should be able to support a point set at 0,0,0 in which case we use the force itself as the direction... otherwise a zero'd point
	//results in a zero'd vector which would omit applying the force
	if (vecToCenter.length2()==0.0)
		vecToCenter=-force;

	vecToCenter.normalize();

	ForceToApply = vecToCenter * (force * vecToCenter);

	ApplyFractionalForce(ForceToApply,FrameDuration);
	ApplyFractionalTorque(TorqueToApply,FrameDuration,RadialArmDistance);

#else  // Rick's Method uses the dot product again, but 
	osg::Vec3d vecToCenter = -point;
	vecToCenter.normalize();
	osg::Vec3d LinearForceToApply = vecToCenter * (force * vecToCenter);
	//This idea could work (maybe) by using the cross product, but I'm not sure how to do that
	osg::Vec3d TorqueToApply = force - LinearForceToApply;

	// Here is the linear aspect of the force
	ApplyFractionalForce(LinearForceToApply,FrameDuration);

	// [Rick Notes]
	// This part does not seem to be working.  The TorqueToApply seems correct to me, but I am thinking that
	// perhaps the units are off, maybe they need to be converted again, or they are in Euler Angles?
	ApplyFractionalTorque(TorqueToApply,FrameDuration,point);
#endif
}


void PhysicsEntity::ApplyForce( const osg::Vec3d &force )
{
	if (force[0]+force[1]+force[2]==0.0)
		return;
	//sum(Force)=Mass * Acceleration
	//In this method all force is applied directly to the center of mass
	osg::Vec3d AccelerationDelta=force/m_EntityMass;

	m_LinearAcceleration+=AccelerationDelta;
	DebugOutput("ApplyForce %f %d\n",m_LinearAcceleration[1],m_LinearAccQueue.size());

	//Now to stamp the delta and time to turn off after one second
	AccStamp stamp;
	stamp.Acceleration=AccelerationDelta;
	stamp.Time=m_TrackAccTime;
	m_LinearAccQueue.push(stamp);
}

inline osg::Vec3d PhysicsEntity::GetAngularAccelerationDelta(const osg::Vec3d &torque,const osg::Vec3d &RadialArmDistance)
{
	// [Rick Notes] It seems we could use a vector CROSS product here, but I may not be quite understanding these units
	//				I may not have the directions right either ;)  Vector CROSS product might be a good tool for this,
	//				but I need to get a better understanding of how it works with what you have.  Is the returned value in Euler angles?
	/* We want a cross product here, and divide by the mass and angular inertia
	return (RadialArmDistance^torque) / (m_EntityMass*m_AngularInertiaCoefficient);

	// [Rick Notes], Why divide by the arm distance?  Shouldn't we be multiplying?  Why square that, and along just the component?
	// We divide by I to solve for a... see formula below
	*/

	// t=Ia 
	//I=sum(m*r^2) or sum(AngularCoef*m*r^2)

	osg::Vec3d ret;
	for (size_t i=0;i<3;i++)
	{
		//Avoid division by zero... no radial arm distance no acceleration!
		if (RadialArmDistance[i]==0)
		{
			ret[i]=0;
			continue;
		}
		//Doing it this way keeps the value of torque down to a reasonable level
		// [Rick Notes]  What does a "Reasonable Level" mean?  Perhaps we should see the equation somewhere
		// I forgot what the equation was and I get a bit lost.
		double RadiusRatio(m_RadiusOfConcentratedMass[i]*m_RadiusOfConcentratedMass[i]/RadialArmDistance[i]);
		assert(RadiusRatio!=0);  //no-one should be using a zero sized radius!
		ret[i]=(torque[i]/(m_AngularInertiaCoefficient*m_EntityMass*RadiusRatio));
	}
	return ret;
}

void PhysicsEntity::ApplyTorque( const osg::Vec3d &torque )
{
	// [Rick Notes] I think what you were trying to do here is just check to make sure the incoming torque parameter
	//  has some value before you do anything to it.  What you really want to do is make sure the length is non 0,
	//  but I do not think adding the components will give you the effect you are wanting(imagine e.g. (1,-1,0)).  
	//  Finding the length has that expensive sqrt(),
	//  but the OSG guys gave us length2(), probably just for this kind of stuff.
	/*
	if (torque[0]+torque[1]+torque[2]==0.0)
		return;
		*/
	if (torque.length2() == 0.0)
		return;

	osg::Vec3d AccelerationDelta=GetAngularAccelerationDelta(torque);

	m_AngularAcceleration+=(AccelerationDelta);

	//Now to stamp the delta and time to turn off after one second
	AccStamp stamp;
	stamp.Acceleration=AccelerationDelta;
	stamp.Time=m_TrackAccTime;
	m_TorqueAccQueue.push(stamp);
}



osg::Vec3d PhysicsEntity::GetForceFromVelocity(const osg::Vec3d &vDesiredVelocity,double DeltaTime_s)
{
	osg::Vec3d DeltaVelocity=(vDesiredVelocity-GetLinearVelocity());
	//A=Delta V / Delta T
	osg::Vec3d Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply Force restrictions later)


	//if (Acceleration!=osg::Vec3d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	osg::Vec3d Force= (Acceleration-(m_LinearAcceleration*DeltaTime_s)) * m_EntityMass;
	//if (PosBNE(Force[0],0)||(PosBNE(Force[1],0))||(PosBNE(Force[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Force[0],Force[1],Force[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],m_AngularAcceleration[2],m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Force[2]);

	return Force;
}

osg::Vec3d PhysicsEntity::GetTorqueFromVelocity( const osg::Vec3d &vDesiredVelocity,double DeltaTime_s)
{

	//TODO input torque restraints from script (this would be due to the capabilities of the engines)
	//Note: desired speed is a separated variable controlled from the ship's speed script, which we fine tune given the torque restraints
	//And also by minimizing the amount of G's pulled at the outer most edge of the ship... so for large ships that rotate this could be
	//significant, and you wouldn't want people slamming into the walls.
	//Note: if the speed is too high and the torque restraint is too low the ship will "wobble" because it trying to to go a faster speed that it
	//can "brake" for... ideally a little wobble is reasonable and this is controlled by a good balance between desired speed and torque restraints


	osg::Vec3d DeltaVelocity=(vDesiredVelocity-GetAngularVelocity());
	//A=Delta V / Delta T
	osg::Vec3d Acceleration=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply torque restrictions later)

	//if (Acceleration!=osg::Vec3d(0,0,0))
	//	printf(" x=%f,y=%f,z=%f\n",Acceleration[0],Acceleration[1],Acceleration[2]);
	//Now that we know what the acceleration needs to be figure out how much force to get it there
	osg::Vec3d Torque= (Acceleration-(m_AngularAcceleration*DeltaTime_s)) * m_EntityMass;
	//if (PosBNE(Torque[0],0)||(PosBNE(Torque[1],0))||(PosBNE(Torque[2],0)))
	//	printf("tx=%f,ty=%f,tz=%f\n",Torque[0],Torque[1],Torque[2]);

	//if (PosBNE(Heading[2],0.0))
	//	DebugOutput(" s=%f,a=%f,w=%f,h=%f,z=%f,t=%f\n",Distance[2],m_AngularAcceleration[2],m_AngularVelocity[2],Heading[2],CurrentOrientation[2],Torque[2]);

	return Torque;
}

osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Angular(const osg::Vec3d &Distance,const osg::Vec3d &Restraint,double DeltaTime_s)
{
	osg::Vec3d ret;
	//This is how many radians the ship is capable to turn for this given time frame
	osg::Vec3d Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	for (size_t i=0;i<3;i++)
	{
		//first compute which direction to go
		double DistanceDirection=Distance[i];
		if (IsZero(DistanceDirection))
		{
			ret[i]=0.0;
			continue;
		}

		if (DistanceDirection>M_PI)
			DistanceDirection-=Pi2;
		else if (DistanceDirection<-M_PI)
			DistanceDirection+=Pi2;
		double DistanceLength=fabs(DistanceDirection);

		double IdealSpeed=fabs(DistanceDirection/DeltaTime_s);

		if (Restraint[i]!=-1)
		{
			//Given the distance compute the time needed
			//Time = sqrt( Distance / Acceleration  )
			//double Time=sqrt(fabs(DistanceDirection/Acceleration[i]));
			double Time=sqrt(2.0*(DistanceLength/Acceleration[i]));

			//Now compute maximum speed for this time
			double MaxSpeed=Acceleration[i]*Time;
			ret[i]=min(IdealSpeed,MaxSpeed);
		}
		else
			ret[i]=IdealSpeed;  //i.e. god speed

		if (DistanceDirection<0)
			ret[i]=-ret[i];
	}
	return ret;
}

osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Angular_v2(const osg::Vec3d &Distance,const osg::Vec3d &Restraint,double DeltaTime_s, const osg::Vec3d& matchVel)
{
	osg::Vec3d ret;

	//This is how many radians the ship is capable to turn for this given time frame
	osg::Vec3d Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	for (size_t i=0;i<3;i++)
	{
		//first compute which direction to go
		double DistanceDirection=Distance[i];
		DistanceDirection-=matchVel[i]*DeltaTime_s;
		if (IsZero(DistanceDirection))
		{
			ret[i]=matchVel[i];
			continue;
		}

		if (DistanceDirection>M_PI)
			DistanceDirection-=Pi2;
		else if (DistanceDirection<-M_PI)
			DistanceDirection+=Pi2;
		double DistanceLength=fabs(DistanceDirection);

		//Ideal speed needs to also be normalized
		double IDS=Distance[i];
		if (IDS>M_PI)
			IDS-=Pi2;
		else if (IDS<-M_PI)
			IDS+=Pi2;

		double IdealSpeed=fabs(IDS/DeltaTime_s);

		if (Restraint[i]!=-1)
		{
			//Given the distance compute the time needed
			//Place the division first keeps the multiply small
			double Time=sqrt(2.0*(DistanceLength/Acceleration[i]));

			//Now compute maximum speed for this time
			double MaxSpeed=DistanceLength/Time;
			ret[i]=min(IdealSpeed,MaxSpeed);

			if (DistanceDirection<0)
				ret[i]=-ret[i];
			ret[i]+=matchVel[i];
		}
		else
		{
			ret[i]=IdealSpeed;  //i.e. god speed
			if (IDS<0)
				ret[i]=-ret[i];
		}
	}
	return ret;
}

inline osg::Vec3d PhysicsEntity::GetVelocityFromCollision(osg::Vec3d ThisVelocityToUse,double otherEntityMass,osg::Vec3d otherEntityVelocity)
{
	//almost not quite
	//return (m_LinearVelocity*(m_EntityMass-otherEntityMass)) / (m_EntityMass+otherEntityMass);

	/// en.wikipedia.org/wiki/Elastic_collision
	// Here is the equation
	// ((vel1 ( mass1 - mass2 )) + (2 * mass2 * vel2))) / (m1+m2)
	osg::Vec3d ret= (ThisVelocityToUse *(m_EntityMass-otherEntityMass));
	ret+= (otherEntityVelocity*(2 * otherEntityMass));
	ret/= (m_EntityMass+otherEntityMass);
	return ret;
}


//just like GetVelocityFromDistance_Angular except we do not normalize the DistanceDirection
osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
{
	osg::Vec3d ret;
	//These are initialized as we go
	double Acceleration;
	double Restraint;

	for (size_t i=0;i<3;i++)
	{
		double DistanceDirection=Distance[i];
		DistanceDirection-=matchVel[i]*DeltaTime_s;
		if (IsZero(DistanceDirection))
		{
			ret[i]=matchVel[i];
			continue;
		}
		double DistanceLength=fabs(DistanceDirection);

		//Compose our restraint and acceleration based on the component direction
		Restraint=(DistanceDirection>0)?ForceRestraintPositive[i]:ForceRestraintNegative[i];
		Acceleration=(Restraint/m_EntityMass);

		double IdealSpeed=fabs(Distance[i]/DeltaTime_s);

		if (Restraint!=-1)
		{
			//Given the distance compute the time needed
			//Place the division first keeps the multiply small
			double Time=sqrt(2.0*(DistanceLength/Acceleration));

			//Now compute maximum speed for this time
			double MaxSpeed=DistanceLength/Time;
			ret[i]=min(IdealSpeed,MaxSpeed);

			if (DistanceDirection<0.0)
				ret[i]=-ret[i];
			ret[i]+=matchVel[i];
		}
		else
		{
			ret[i]=IdealSpeed;  //i.e. god speed
			if (Distance[i]<0.0)
				ret[i]=-ret[i];
		}
	}
	return ret;
}

osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Linear_v1(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
{
	osg::Vec3d ret;
	osg::Vec3d DistToUse=Distance; //save the original distance as it is const anyhow

	DistToUse-=(matchVel*DeltaTime_s);
	double dDistance=DistToUse.length();
	if (IsZero(dDistance))
		return matchVel;

	//This is how many meters per second the ship is capable to stop for this given time frame
	osg::Vec3d Restraint;
	//Compute the restraint based off of its current direction
	for (size_t i=0;i<3;i++)
		Restraint[i]=DistToUse[i]>0?ForceRestraintPositive[i]:ForceRestraintNegative[i];

	osg::Vec3d Acceleration=(Restraint/m_EntityMass); //obtain acceleration

	double IdealSpeed=Distance.length()/DeltaTime_s;
	double AccelerationMagnitude=GetSpeed(Acceleration);
	double Time=sqrt(2.0*(dDistance/AccelerationMagnitude));

	double MaxSpeed=dDistance/Time;
	double SpeedToUse=min(IdealSpeed,MaxSpeed);

	//DebugOutput("d=%f i=%f m=%f\n",Distance[1],IdealSpeed,MaxSpeed);
	//Now to give this magnitude a direction based of the velocity
	double scale=SpeedToUse/dDistance;
	ret=DistToUse*scale;
	ret+=matchVel;
	return ret;
}

osg::Vec3d PhysicsEntity::ComputeRestrainedTorque(const osg::Vec3d &Torque,const osg::Vec3d &TorqueRestraint,double dTime_s)
{  
	osg::Vec3d TorqueToApply=Torque;

	if (TorqueRestraint[0]!=-1)
	{
		double SmallestRatio=1.0;
		//Apply Torque restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
		//of magnitude available without sacrificing the intended direction
		for (size_t i=0;i<2;i++) //only work with yaw and pitch
		{
			double AbsComponent=fabs(TorqueToApply[i]);
			if ((AbsComponent>TorqueRestraint[i]) && (TorqueRestraint[i] > 0.0))
			{
				double Temp=TorqueRestraint[i]/AbsComponent;
				SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
			}
		}
		TorqueToApply[0]*=SmallestRatio;
		TorqueToApply[1]*=SmallestRatio;
	}

	if (TorqueRestraint[2]>0.0)
	{
		//Apply torque restraint on roll
		if (fabs(TorqueToApply[2])>TorqueRestraint[2])
			TorqueToApply[2]=TorqueToApply[2]>0? TorqueRestraint[2]:-TorqueRestraint[2];
	}
	
	return TorqueToApply;
}


osg::Vec3d PhysicsEntity::ComputeRestrainedForce(const osg::Vec3d &LocalForce,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double dTime_s)
{
	osg::Vec3d ForceToApply=LocalForce;
	if (ForceRestraintPositive[0]!=-1)
	{
		double SmallestRatio=1.0;
		//Apply Force restraints; This method computes the smallest ratio needed to scale down the vector.  It should give the maximum amount
		//of magnitude available without sacrificing the intended direction
		for (size_t i=0;i<3;i++)
		{
			double Temp;
			//separate the positive and negative coordinates
			if (LocalForce[i]>0)
			{
				if (LocalForce[i]>ForceRestraintPositive[i])
				{
					Temp=ForceRestraintPositive[i]/LocalForce[i];
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
			else
			{
				double AbsComponent=fabs(LocalForce[i]);
				if (AbsComponent>ForceRestraintNegative[i])
				{
					Temp=ForceRestraintNegative[i]/AbsComponent;
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
		}
		ForceToApply*=SmallestRatio;
		//Test my restraints
		//printf("\r lr %f fr %f ud %f                ",LocalForce[0],LocalForce[1],LocalForce[2]);
	}
	return ForceToApply;
}

void PhysicsEntity::TimeChangeUpdate(double DeltaTime_s,osg::Vec3d &PositionDisplacement,osg::Vec3d &RotationDisplacement)
{
	if (!m_LinearAccQueue.empty())
	{
		//Transfer the acceleration to velocity
		//This may be a bit misleading but the acceleration is being transfered to kinetic energy, once this has been transfered the force
		//is no longer applied (and velocity will have the energy)
		osg::Vec3d VelocityDelta=m_LinearAcceleration*DeltaTime_s;

		m_LinearVelocity+=VelocityDelta;

		//Test to see if any queued items have exceeded a second
		while ((!m_LinearAccQueue.empty())&&(m_TrackAccTime-m_LinearAccQueue.front().Time>=1.0))
		{
			m_LinearAcceleration-=m_LinearAccQueue.front().Acceleration;
			DebugOutput("remove ApplyForce %f %d\n",m_LinearAcceleration[1],m_LinearAccQueue.size());
			m_LinearAccQueue.pop();
		}
		//Due to precision loss ensure the acceleration is reset to zero (It should be very close to zero anyhow)
		if (m_LinearAccQueue.empty())
			m_LinearAcceleration=osg::Vec3d(0,0,0);
	}
	if (!m_TorqueAccQueue.empty())
	{
		osg::Vec3d VelocityDelta=m_AngularAcceleration*DeltaTime_s;
		m_AngularVelocity+=VelocityDelta;
		//Test to see if any queued items have exceeded a second
		while ((!m_TorqueAccQueue.empty())&&(m_TrackAccTime-m_TorqueAccQueue.front().Time>=1.0))
		{
			m_AngularAcceleration-=m_TorqueAccQueue.front().Acceleration;
			m_TorqueAccQueue.pop();
		}
		//Due to precision loss ensure the acceleration is reset to zero (It should be very close to zero anyhow)
		if (m_TorqueAccQueue.empty())
			m_AngularAcceleration=osg::Vec3d(0,0,0);
	}
	//I shouldn't need to worry about overflow, but if so we can make it reset
	m_TrackAccTime+=DeltaTime_s;
	

	//Transfer the velocity to displacement
	RotationDisplacement = m_AngularVelocity * DeltaTime_s;
	PositionDisplacement = m_LinearVelocity * DeltaTime_s;
}


  /************************************************************************************************************************/
 /*										                  FlightDynamics							                     */
/************************************************************************************************************************/

void FlightDynamics::init()
{
	//populate LinearAccelerationRates with some good defaults
	const osg::Vec3d AccDelta(30.0,30.0,30.0);
	const osg::Vec3d DecDelta(60.0,60.0,60.0);
	LinearAccelerationRates &_=m_LinearAccelerationRates;
	_.AccDeltaNeg=_.AccDeltaPos=AccDelta;
	_.DecDeltaNeg=_.DecDeltaPos=DecDelta;
	m_UsingAccelerationRate=false;
	StructuralDmgGLimit = 10.0;
	G_Dampener = 1.0;
}

FlightDynamics::FlightDynamics() : m_QuatToUse(m_DefaultQuat), m_Pilot(this)
{
	init();
}

FlightDynamics::FlightDynamics(const osg::Quat &quatToUse) : m_QuatToUse(quatToUse), m_Pilot(this)
{
	init();
}

void FlightDynamics::ResetVectors()
{
	__super::ResetVectors();
	if (m_QuatToUse==m_DefaultQuat)
		m_DefaultQuat=osg::Quat(0,0,0,1);
	m_CurrentAcceleration=m_TargetAcceleration=osg::Vec3d(0.0,0.0,0.0);
	m_Pilot.ResetVectors();
}

osg::Vec3d FlightDynamics::ComputeAngularDistance(const osg::Vec3d &lookDir,const osg::Vec3d &UpDir)
{
	osg::Vec3d ret(0.0,0.0,0.0);
	////////////////////////////////////////////////////////////////////////////////////////////////
	//Default 0,1,0   Yaw,Pitch,Roll
	//Look up negative, down positive
	//Look left negative right positive


	//printf("\r %f %f %f camera                       ",lookDir[0],lookDir[1],lookDir[2]);
	osg::Vec3d CurrentOrientation(m_QuatToUse.conj()*lookDir);
	//printf("\r %f %f %f ship                       ",CurrentOrientation[0],CurrentOrientation[1],CurrentOrientation[2]);
	ret[0]=atan2(CurrentOrientation[0],CurrentOrientation[1]);  //Angle in radians
	if (CurrentOrientation[1]>0.0)
		ret[1]=atan2(CurrentOrientation[2],CurrentOrientation[1]);  //Angle in radians
	else
		ret[1]=atan2(CurrentOrientation[2],-(CurrentOrientation[1]));  //Angle in radians
	#if 0
	{
		printf("\r %f %f radians          ",ret[0],ret[1]);
		//printf("\r %f %f degrees          ",RAD_2_DEG(YawAngleOffset),RAD_2_DEG(PitchAngleOffset));

	}
	#endif
	osg::Vec3d RollOrientation(m_QuatToUse.conj()*UpDir);
	//printf("\r %f %f %f ship                       ",RollOrientation[0],RollOrientation[1],RollOrientation[2]);
	ret[2]=atan2(RollOrientation[0],RollOrientation[2]);  //Angle in radians
	//printf("\r %f degrees          ",RAD_2_DEG(RollOffset));
	return ret;
}

osg::Vec3d FlightDynamics::ComputeAngularDistance(const osg::Quat &Orientation)
{
	osg::Vec3d LookDir(Orientation*osg::Vec3d(0,1,0));
	osg::Vec3d UpDir(Orientation*osg::Vec3d(0,0,1));
	return ComputeAngularDistance(LookDir,UpDir);
}

static osg::Quat FromLW_Rot_Radians(double H, double P, double R)
{
	return osg::Quat(
		H	, osg::Vec3d(0,0,1),
		-P	, osg::Vec3d(1,0,0),
		-R	, osg::Vec3d(0,1,0));
}

void FlightDynamics::TimeChangeUpdate(double DeltaTime_s,osg::Vec3d &PositionDisplacement,osg::Vec3d &RotationDisplacement)
{
	__super::TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);
	m_Pilot.Update(DeltaTime_s, G_Dampener);
	if (m_QuatToUse==m_DefaultQuat)
		m_DefaultQuat=FromLW_Rot_Radians(RotationDisplacement[0],RotationDisplacement[1],RotationDisplacement[2])*m_DefaultQuat;
}


FlightDynamics::LinearAccelerationRates &FlightDynamics::GetLinearAccelerationRates()
{
	return m_LinearAccelerationRates;
}

void FlightDynamics::SetTargetAcceleration(const osg::Vec3d &TargetAcceleration)
{
	m_TargetAcceleration=TargetAcceleration;
}

osg::Vec3d FlightDynamics::GetAcceleration_Delta(double dTime_s,const osg::Vec3d &TargetAcceleration,bool Clipping)
{
	//implicitly initialized in case we need to do no work
	osg::Vec3d ret(0.0,0.0,0.0);
	//Note: this is somewhat simplified when we cross-over the zero thresh-hold, where instead of blending one derivative against the other
	//it will follow through with first tagged one.  Since the deltas are encapsulated it should work, because it will still
	//accurately predict the force that will be applied.  The amount of error for this case should be minimal enough to not be noticeable.
	//We can provide different engine models in the future
	//Compute the acceleration given the derivatives
	LinearAccelerationRates &_=m_LinearAccelerationRates;
	for (size_t i=0;i<3;i++)
	{
		if ((TargetAcceleration[i]==m_CurrentAcceleration[i])&&(Clipping))
			continue;
		//determine if we are going in a positive or negative direction
		if (TargetAcceleration[i]>m_CurrentAcceleration[i]) //We are going in a positive direction
		{
			double Delta=m_CurrentAcceleration[i]>0?_.AccDeltaPos[i]:_.DecDeltaNeg[i];
			Delta*=dTime_s;
			if (Clipping)
				ret[i]=std::min(Delta,TargetAcceleration[i]-m_CurrentAcceleration[i]);
			else
				ret[i]=Delta;
		}
		else //We are going in a negative direction
		{
			double Delta=m_CurrentAcceleration[i]>0?_.DecDeltaPos[i]:_.AccDeltaNeg[i];
			Delta*=dTime_s;
			if (Clipping)
				ret[i]=std::max(-Delta,TargetAcceleration[i]-m_CurrentAcceleration[i]);
			else
				ret[i]=Delta;
		}
	}
	return ret;
}

osg::Vec3d FlightDynamics::GetAcceleration_Delta (double dTime_s)
{
	return GetAcceleration_Delta(dTime_s,m_TargetAcceleration);
}

void FlightDynamics::Acceleration_TimeChangeUpdate(double dTime_s)
{
	const osg::Vec3d Acceleration_Delta=GetAcceleration_Delta(dTime_s);
	m_CurrentAcceleration+=Acceleration_Delta;
}

osg::Vec3d FlightDynamics::GetForceFromVelocity(const osg::Vec3d &vDesiredVelocity,double DeltaTime_s)
{
	osg::Vec3d Force;
	if (!m_UsingAccelerationRate)
		Force=__super::GetForceFromVelocity(vDesiredVelocity,DeltaTime_s);
	else
	{
		osg::Vec3d Zerod=osg::Vec3d(0.0,0.0,0.0);
		osg::Vec3d Acceleration=Zerod;
		const osg::Vec3d DeltaVelocity=(vDesiredVelocity-GetLinearVelocity());
		//compute the maximum deceleration speed, since we need to reach 0 by the time we reach our desired velocity
		//Note: these are in local orientation so they need to be converted to global
		const osg::Vec3d MaxDeceleration=(m_QuatToUse * GetAcceleration_Delta(DeltaTime_s,osg::Vec3d(0.0,0.0,0.0),false));
		osg::Vec3d Global_CurrentAcceleration(m_QuatToUse * m_CurrentAcceleration);

		
		//A=Delta V / Delta T
		const osg::Vec3d NewAcceleration_target=DeltaVelocity/DeltaTime_s;  //This may be pretty quick (apply Force restrictions later)

		{
			double MaxDeceleration_Length=MaxDeceleration.length();
			double CurrentAcceleration_Length=Global_CurrentAcceleration.length();
			double SpeedUpTime=CurrentAcceleration_Length?DeltaVelocity.length()/(CurrentAcceleration_Length*DeltaTime_s):0.0;
			double SpeedDownTime=MaxDeceleration_Length?CurrentAcceleration_Length/MaxDeceleration_Length:0.0;
			//Choose whether to speed up or slow down based on which would take longer to do
			if ((SpeedUpTime>SpeedDownTime)||(MaxDeceleration_Length>=CurrentAcceleration_Length))
				Acceleration=NewAcceleration_target;
			else
				Acceleration=DeltaVelocity/SpeedDownTime;

		}
		Force= Acceleration * m_EntityMass;
	}
	return Force;
}

osg::Vec3d FlightDynamics::GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
{
	osg::Vec3d ret;

	if (!m_UsingAccelerationRate)
		ret=__super::GetVelocityFromDistance_Linear(Distance,ForceRestraintPositive,ForceRestraintNegative,DeltaTime_s,matchVel);
	else
	{
		const osg::Vec3d MaxDeceleration=GetAcceleration_Delta(1.0,osg::Vec3d(0.0,0.0,0.0),false);
#if 0
		//I thought there may be a problem with solving each element separately so I wrote this version
		double dDistance=Distance.length();
		const double RampError=12.0;

		//double AccelerationMagnitude=Acceleration.length();
		double AccelerationMagnitude=ForceRestraintNegative[1]/m_EntityMass; //for now this is the amount of reverse thrust, but I need to work this out at some point

		double Time=sqrt(2.0*dDistance/AccelerationMagnitude);
		ret=(dDistance>RampError)?Distance/max(Time,DeltaTime_s) : osg::Vec3d(0,0,0);

		//DebugOutput("d=%f %f ds=%f s=%f\n",Distance[1],dDistance,ret.length(),m_LinearVelocity.length());

#else
		//Solving each element gives the advantage of using strafe techniques, we are assuming all calls work with local orientation
		for (size_t i=0;i<3;i++)
		{
			double DistanceDirection=Distance[i];
			DistanceDirection-=matchVel[i]*DeltaTime_s;
			if (IsZero(DistanceDirection))
			{
				ret[i]=matchVel[i];
				continue;
			}

			double DistanceLength=fabs(DistanceDirection);

			//Compose our restraint and acceleration based on the component direction
			double Restraint=(DistanceDirection>0)?ForceRestraintPositive[i]:ForceRestraintNegative[i];
			double Acceleration=(Restraint/m_EntityMass);

			double MaxDeceleration_Length=fabs(MaxDeceleration[i]);

			//This number is a work in progress
			const double RampError=2.0;

			{
				//Solve the Acceleration... given a distance what would it have to be going its fastest ramp rate
				double Time=sqrt(2.0*(DistanceLength/MaxDeceleration[i]));
				Acceleration=min(Acceleration,MaxDeceleration_Length*Time);
			}
			double Time=sqrt(2.0*(DistanceLength/Acceleration));

			//using distance/DTime in the max helps taper off the last frame to not over compensate
			ret[i]=DistanceLength/max(Time,DeltaTime_s);
			//ret[i]=DistanceLength/max(Time,DeltaTime_s);


			//if (i==1)
				//DebugOutput("Distance=%f,Time=%f,Speed=%f,vel=%f\n",DistanceDirection,Time,ret[i],m_LinearVelocity[i]);
				//DebugOutput("Distance=%f,Time=%f,Speed=%f,acc=%f\n",DistanceDirection,Time,ret[i],Acceleration);

			if (DistanceLength>RampError)
			{
				if (DistanceDirection<0)
					ret[i]=-ret[i];
				ret[i]+=matchVel[i];
			}
			else
				ret[i]=matchVel[i];
		}
#endif
	}
	return ret;

}

void FlightDynamics::Pilot::ResetVectors()
{
	m_Gs = INITIALIZE_Gs+2.0;
	m_blackOUTredOUT = 0.0;
}

FlightDynamics::Pilot::Pilot (FlightDynamics *parent) : m_pParent(parent), GLimit(6.5),
PassOutTime_s(5.0), // Time it takes to pass out
PassOutRecoveryTime_s(1.0), // Time to get OUT of pass out and start recovery
MaxRecoveryTime_s(20.0)	// So we do not pass out forever
{
	ResetVectors();
}


void FlightDynamics::Pilot::Update(double dTime_s, double G_Dampener)
{
	//TODO I know this is not taking into account of local head position, but we have not supported this yet
	//I think when we do we can derive this info from the rotation delta.  Also it is probably more "correct" to measure the
	//velocity as this takes external forces into account, but given the issues with the time deltas, m_CurrentAcceleration
	//will give the smoothest results.  I'd be tempted to use the velocity for gravitational cases when we have them, but then
	//I'd like to address the time deltas being more consistent as well.
	osg::Vec3 headAccel = m_pParent->m_CurrentAcceleration;  //This is local orientation

	// We need to have passed a couple of frames before things have normalized
	if (m_Gs >= INITIALIZE_Gs)
		m_Gs -= 1.0;
	else	
	{
		static const osg::Vec3d POS_G_VECTOR(0.0, 0.6, 0.8);
		// Use a dot product to find the component of the acceleration that lies along that vector, +/-, and divide it by 1G
		m_Gs = (headAccel*POS_G_VECTOR*G_Dampener) / EARTH_G; //note I'm already in local orientation

		// We want to REALLY buffer the g's (in case of collisions and such)
		double GsToUse = m_G_Averager.GetAverage(m_Gs);

		// Some limits used to determine if we have passed out
		const double passOutLevel = 1.0 + (PassOutRecoveryTime_s/PassOutTime_s);
		const double maxPassOutLevel = 1.0 + (MaxRecoveryTime_s/PassOutTime_s);

		// Find the amount of blackout / red out
		if ((GsToUse > GLimit) && (m_blackOUTredOUT >= 0.0))
		{
			m_blackOUTredOUT += (1.0+(GsToUse-GLimit))*dTime_s / PassOutTime_s;
			if ((m_blackOUTredOUT > 1.0) && (m_blackOUTredOUT < passOutLevel))
			{
				// We Just Blacked Out
				m_blackOUTredOUT = passOutLevel;
			}
		}
		else if ((GsToUse < -GLimit) && (m_blackOUTredOUT <= 0.0))
		{
			m_blackOUTredOUT -= (1.0-(GsToUse+GLimit))*dTime_s / PassOutTime_s;
			if ((m_blackOUTredOUT < -1.0) && (m_blackOUTredOUT > -passOutLevel))
			{
				// We just Red' Out
				m_blackOUTredOUT = -passOutLevel;

			}
		}
		else
		{
			// Recovery
			double recovery = dTime_s / PassOutTime_s;
			if (m_blackOUTredOUT < -recovery)
				m_blackOUTredOUT += recovery;
			else if (m_blackOUTredOUT > recovery)
				m_blackOUTredOUT -= recovery;
			else
			{
				// We are recovered, no longer passed out
				m_blackOUTredOUT = 0.0;
			}
		}

		// Watch for going WAY overboard on the black/red out
		if (m_blackOUTredOUT > maxPassOutLevel)
			m_blackOUTredOUT = maxPassOutLevel;
		else if (m_blackOUTredOUT < -maxPassOutLevel)
			m_blackOUTredOUT = -maxPassOutLevel;

#ifdef DISABLE_BLACKOUT
		m_Gs = 0.0;
		m_blackOUTredOUT = 0.0;
#endif
		//printf("\r G's: (%6.1f), Blackout Line: %6.1f", GsToUse, m_blackOUTredOUT);
	}
}
