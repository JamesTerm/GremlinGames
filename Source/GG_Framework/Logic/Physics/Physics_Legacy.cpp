//This file will contain depreciated methods as well as some notes on varios formulas attempted in the past

#include "stdafx.h"
#include "GG_Framework.Logic.Physics.h"


using namespace GG_Framework::Logic::Physics;
using namespace GG_Framework::Base;
using namespace std;

const double Pi2=M_PI*2.0;
//The actual force between two objects are f=(G m1 m2)/ r^2
//For example g = ( G * (Me->5.98E+24)) / (Re->6.38E+6)^2 = 9.8 m/s^2
//G is ideal to compute forces on ships from various planets
const double G=6.673E-11;

/*
http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm

heading = atan2(2*qy*qw-2*qx*qz , 1 - 2*qy^2 - 2*qz^2)
attitude = asin(2*qx*qy + 2*qz*qw) 
bank = atan2(2*qx*qw-2*qy*qz , 1 - 2*qx^2 - 2*qz^2)

except when qx*qy + qz*qw = 0.5 (north pole)
which gives:
heading = 2 * atan2(x,w)
bank = 0
and when qx*qy + qz*qw = -0.5 (south pole)
which gives:
heading = -2 * atan2(x,w)
bank = 0 

*/

void QuatToEulerAngles (const osg::Quat &q,double &heading,double &attitude,double &bank) 
{
	const osg::Quat q1(q.z(),q.y(),q.x(),q.w());
	double test = q1.x()*q1.y() + q1.z()*q1.w();
	if (test > 0.499) 
	{ // singularity at north pole
		//heading = 2 * atan2(q1.x(),q1.w());
		heading = atan2(q1.x(),q1.w());
		attitude = M_PI/2;
		bank = 0;
		return;
	}	
	if (test < -0.499) 
	{ // singularity at south pole
		//heading = -2 * atan2(q1.x(),q1.w());
		heading = atan2(q1.x(),q1.w());
		attitude = - M_PI/2;
		bank = 0;
		return;
	}
	double sqw = q1.w()*q1.w();    
	double sqx = q1.x()*q1.x();    
	double sqy = q1.y()*q1.y();    
	double sqz = q1.z()*q1.z();
	double unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor

#if 0  //This is the original
	heading = atan2(2*q1.y()*q1.w()-2*q1.x()*q1.z() , 1 - 2*sqy - 2*sqz);
	attitude = asin(2*test);
	bank = atan2(2*q1.x()*q1.w()-2*q1.y()*q1.z() , 1 - 2*sqx - 2*sqz);
#else //This one is optimized
	heading = atan2(2*q1.y()*q1.w()-2*q1.x()*q1.z() , sqx - sqy - sqz + sqw);
	attitude = asin(2*test/unit);
	//attitude = asin(2*qx*qy + 2*qz*qw) 
	//attitude = asin((2*q1.x()*q1.y()) + (2*q1.z()*q1.w())); 
	bank = atan2(2*q1.x()*q1.w()-2*q1.y()*q1.z() , -sqx + sqy - sqz + sqw);
#endif 
}


//This does some minor variation of the result to conform to how I work with the Vec3
osg::Vec3d QuatToEulerAngles (const osg::Quat &q1)
{
	double heading,attitude,bank;
	QuatToEulerAngles(q1,heading,attitude,bank);
	//attitude hack...
	//Not sure why attitude gives a smaller range so I'm scaling it here: :(
	attitude*=2;

	//Not sure why these are shuffled around
	return osg::Vec3d(bank+M_PI,attitude+M_PI,heading+M_PI);
}


//osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Angular_v2(const osg::Vec3d &Distance,const osg::Vec3d &Restraint,double DeltaTime_s, const osg::Vec3d& matchVel)
//Given the distance compute the time needed
//This is a rearranged equation of v * t * 1/2 * a * t^2
//Place the division first keeps the multiply small
//double Time=sqrt(2.0*(DistanceLength/Acceleration));
//(sqrt(v^2+2*a*d)-v)/a)
//double Time=(sqrt((v * v) + 2.0 * Acceleration[i] * DistanceLength)-v)/Acceleration[i];

//osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
//double v=fabs(matchVel[i]);
//if (matchVel[i] * DistanceDirection < 0.0)
//v=-v;
//Given the distance compute the time needed
//This is a rearranged equation of v * t * 1/2 * a * t^2
//Place the division first keeps the multiply small
//double Time=sqrt(2.0*(DistanceLength/Acceleration));
//(sqrt(v^2+2*a*d)-v)/a)
//double Time=(sqrt((v * v) + 2.0 * Acceleration * DistanceLength)-v)/Acceleration;

//This one is the one that caught my attention to not work with the velocity within the time computation as they could go in two different directions
//osg::Vec3d PhysicsEntity::GetVelocityFromDistance_Linear_v1(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
//TODO determine if this should ever be negative, also I may need to rethink this because the distance direction and matching velocity could be
//and typically are in two directions.  Ironically in its current state give overall good results.
//  [9/20/2009 JamesK]
//double v=matchVel.length();
//		double Time=(sqrt((v * v) + 2.0 * AccelerationMagnitude * dDistance)-v)/AccelerationMagnitude;

//osg::Vec3d FlightDynamics::GetVelocityFromDistance_Linear(const osg::Vec3d &Distance,const osg::Vec3d &ForceRestraintPositive,const osg::Vec3d &ForceRestraintNegative,double DeltaTime_s, const osg::Vec3d& matchVel)
//double v=fabs(matchVel[i]);
//if (matchVel[i] * DistanceDirection < 0.0)
//v=-v;
//(sqrt(v^2+2*a*d)-v)/a)
//double Time=(sqrt((v * v) + 2.0 * Acceleration * DistanceLength)-v)/Acceleration;


////To match velocity we boost the restraint to compensate the stop distance
//double matchVelSqaured=matchVel[i]*matchVel[i];
////Note we only boost if the velocity is in the same direction is the distance
//if (DistanceDirection*matchVel[i]>0)
//Acceleration+=matchVelSqaured;
