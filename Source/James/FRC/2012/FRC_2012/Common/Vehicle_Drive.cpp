#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "Vehicle_Drive.h"

using namespace Framework::Base;
using namespace std;

const double PI=M_PI;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*													Vehicle_Drive_Common															*/
/***********************************************************************************************************************************/

void Vehicle_Drive_Common::Vehicle_Drive_Common_ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	//Just cache these here to not pollute the control
	m_CachedLocalForce=LocalForce;
	m_CachedTorque=Torque;
	m_CachedLinearVelocity=PhysicsToUse.GetLinearVelocity();
	m_CachedAngularVelocity=PhysicsToUse.GetAngularVelocity();

	InterpolateThrusterChanges(m_CachedLocalForce,m_CachedTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
}

bool Vehicle_Drive_Common::Vehicle_Drive_Common_InjectDisplacement(PhysicsEntity_2D &PhysicsToUse,double DeltaTime_s,double Att_r,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	const bool _InjectedDisplacement=true;
	if (_InjectedDisplacement)
	{
		Vec2d computedLinearVelocity=PhysicsToUse.GetLinearVelocity();
		double computedAngularVelocity=PhysicsToUse.GetAngularVelocity();
		PhysicsToUse.SetLinearVelocity(m_CachedLinearVelocity);
		PhysicsToUse.SetAngularVelocity(m_CachedAngularVelocity);
		PhysicsToUse.ApplyFractionalForce(LocalToGlobal(Att_r,m_CachedLocalForce),DeltaTime_s);
		PhysicsToUse.ApplyFractionalTorque(m_CachedTorque,DeltaTime_s);
		PhysicsToUse.TimeChangeUpdate(DeltaTime_s,PositionDisplacement,RotationDisplacement);

		//We must set this back so that the PID can compute the entire error
		PhysicsToUse.SetLinearVelocity(computedLinearVelocity);
		PhysicsToUse.SetAngularVelocity(computedAngularVelocity);
	}
	return _InjectedDisplacement;
}


  /***********************************************************************************************************************************/
 /*															Tank_Drive																*/
/***********************************************************************************************************************************/

Tank_Drive::Tank_Drive(const char EntityName[]) : Ship_Tester(EntityName), m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}

void Tank_Drive::ResetPos()
{
	m_LeftLinearVelocity=m_RightLinearVelocity=0.0;
	__super::ResetPos();
}

void Tank_Drive::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);

	//L is the vehicle’s wheelbase
	//const double L=GetWheelDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetWheelDimensions()[0];

	Vec2d CurrentVelocity=GlobalToLocal(GetAtt_r(),PhysicsToUse.GetLinearVelocity());
	const double FWD=((LocalForce[1]/Mass)*dTime_s)+CurrentVelocity[1];
	//FWD=IsZero(FWD)?0.0:FWD;
	double RCW=(TorqueRestrained/Mass)*dTime_s+PhysicsToUse.GetAngularVelocity();
	//RCW=fabs(RCW)<0.3?0.0:RCW;
	double RPS=RCW / Pi2;
	RCW=RPS * (PI * W);  //W is really diameter

	m_LeftLinearVelocity = FWD + RCW;
	m_RightLinearVelocity = FWD - RCW;

	#if 0
	DOUT2("%f %f",FWD,RCW);
	DOUT4("%f %f ",m_LeftLinearVelocity,m_RightLinearVelocity);
	#endif
	//DOUT4("%f %f",FWD,RCW);  //Test accuracy
}

void Tank_Drive::InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2d &LocalVelocity,double &AngularVelocity,double dTime_s)
{
	//L is the vehicle’s wheelbase
	//const double L=GetWheelDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetWheelDimensions()[0];

	//const double FWD = (LeftLinearVelocity*cos(1.0)+RightLinearVelocity*cos(1.0))/2.0;
	const double FWD = (LeftLinearVelocity + RightLinearVelocity) / 2.0;
	//const double STR = (LeftLinearVelocity*sin(0.0)+ RightLinearVelocity*sin(0.0))/2.0;
	const double STR = 0.0;

	//const double HP=PI/2;
	//const double HalfDimLength=GetWheelDimensions().length()/2;

	//Here we go it is finally working I just needed to take out the last division
	const double omega = ((LeftLinearVelocity) + (RightLinearVelocity*-1))/2.0;

	LocalVelocity[0]=STR;
	LocalVelocity[1]=FWD;

	AngularVelocity=(omega / (PI * W)) * Pi2;
	//This is a safety to avoid instability
	#if 0
	AngularVelocity=IsZero(omega)?0.0:omega;
	if (AngularVelocity>20.0)
		AngularVelocity=20.0;
	else if (AngularVelocity<-20.0)
		AngularVelocity=-20.0;
	#endif

	#if 0
	DOUT2("%f %f",FWD,omega);
	DOUT4("%f %f ",m_LeftLinearVelocity,m_RightLinearVelocity);
	#endif
	//DOUT5("%f %f",FWD,omega);
}

void Tank_Drive::InterpolateThrusterChanges(Vec2d &LocalForce,double &Torque,double dTime_s)
{
	Vec2d OldLocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	Vec2d LocalVelocity;
	double AngularVelocity;
	InterpolateVelocities(m_LeftLinearVelocity,m_RightLinearVelocity,LocalVelocity,AngularVelocity,dTime_s);

	Vec2d LinearAcceleration=LocalVelocity-OldLocalVelocity;
	LocalForce=(LinearAcceleration * Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=AngularVelocity - m_Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
}

#if 0
void Tank_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	Vec2d NewLocalForce(LocalForce);
	double NewTorque=Torque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,-1,dTime_s);
}
#else
void Tank_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	Vehicle_Drive_Common_ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);	
	//__super::ApplyThrusters(PhysicsToUse,GetCachedLocalForce(),LocalTorque,-1,dTime_s);
	__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,-1,dTime_s);
}
#endif

bool Tank_Drive::InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	//For test purposes
	//return false;
	return Vehicle_Drive_Common_InjectDisplacement(m_Physics,DeltaTime_s,GetAtt_r(),PositionDisplacement,RotationDisplacement);
}


  /***********************************************************************************************************************************/
 /*														Swerve_Drive																*/
/***********************************************************************************************************************************/

Swerve_Drive::Swerve_Drive(const char EntityName[]) : Ship_Tester(EntityName)
{
	memset(&m_Velocities,0,sizeof(SwerveVelocities));
}

void Swerve_Drive::ResetPos()
{
	memset(&m_Velocities,0,sizeof(SwerveVelocities));
	__super::ResetPos();
}

void Swerve_Drive::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);

	//L is the vehicle’s wheelbase
	const double L=GetWheelDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetWheelDimensions()[0];

	//const double R = sqrt((L*L)+(W*W));
	const double R = GetWheelDimensions().length();

	//Allow around 2-3 degrees of freedom for rotation.  While manual control worked fine without it, it is needed for
	//targeting goals (e.g. follow ship)

	Vec2d CurrentVelocity=GlobalToLocal(GetAtt_r(),PhysicsToUse.GetLinearVelocity());
	const double STR=((LocalForce[0]/Mass)*dTime_s)+CurrentVelocity[0];
	//STR=IsZero(STR)?0.0:STR;
	const double FWD=((LocalForce[1]/Mass)*dTime_s)+CurrentVelocity[1];
	//FWD=IsZero(FWD)?0.0:FWD;
	double RCW=(TorqueRestrained/Mass)*dTime_s+PhysicsToUse.GetAngularVelocity();
	//RCW=fabs(RCW)<0.3?0.0:RCW;
	double RPS=RCW / Pi2;
	RCW=RPS * (PI * R);  //R is really diameter

	const double A = STR - RCW*(L/R);
	const double B = STR + RCW*(L/R);
	const double C = FWD - RCW*(W/R);
	const double D = FWD + RCW*(W/R);
	SwerveVelocities::uVelocity::Explicit &_=m_Velocities.Velocity.Named;

	_.sFL = sqrt((B*B)+(D*D)); _.aFL = atan2(B,D);
	_.sFR = sqrt((B*B)+(C*C)); _.aFR = atan2(B,C);
	_.sRL = sqrt((A*A)+(D*D)); _.aRL = atan2(A,D);
	_.sRR = sqrt((A*A)+(C*C)); _.aRR = atan2(A,C);

	#if 0
	DOUT2("%f %f %f",FWD,STR,RCW);
	DOUT4("%f %f %f %f",_.sFL,_.sFR,_.sRL,_.sRR);
	DOUT5("%f %f %f %f",_.aFL,_.aFR,_.aRL,_.aRR);
	#endif
	//DOUT4("%f %f %f",FWD,STR,RCW);  //Test accuracy

}

void Swerve_Drive::InterpolateVelocities(SwerveVelocities Velocities,Vec2d &LocalVelocity,double &AngularVelocity,double dTime_s)
{
	SwerveVelocities::uVelocity::Explicit &_=Velocities.Velocity.Named;
	//L is the vehicle’s wheelbase
	const double L=GetWheelDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetWheelDimensions()[0];
	const double D = GetWheelDimensions().length();

	const double FWD = (_.sFR*cos(_.aFR)+_.sFL*cos(_.aFL)+_.sRL*cos(_.aRL)+_.sRR*cos(_.aRR))/4;

	const double STR = (_.sFR*sin(_.aFR)+_.sFL*sin(_.aFL)+_.sRL*sin(_.aRL)+_.sRR*sin(_.aRR))/4;
	const double HP=PI/2;
	//const double HalfDimLength=GetWheelDimensions().length()/2;

	//Here we go it is finally working I just needed to take out the last division
	const double omega = ((_.sFR*cos(atan2(W,L)+(HP-_.aFR))+_.sFL*cos(atan2(-W,L)+(HP-_.aFL))
		+_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))+_.sRR*cos(atan2(W,-L)+(HP-_.aRR)))/4);

	//const double omega = (((_.sFR*cos(atan2(W,L)+(HP-_.aFR))/4)+(_.sFL*cos(atan2(-W,L)+(HP-_.aFL))/4)
	//	+(_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))/4)+(_.sRR*cos(atan2(W,-L)+(HP-_.aRR))/4)));

	LocalVelocity[0]=STR;
	LocalVelocity[1]=FWD;

	AngularVelocity=(omega / (PI * D)) * Pi2;
	//This is a safety to avoid instability
	#if 0
	AngularVelocity=IsZero(omega)?0.0:omega;
	if (AngularVelocity>20.0)
		AngularVelocity=20.0;
	else if (AngularVelocity<-20.0)
		AngularVelocity=-20.0;
	#endif

	#if 0
	DOUT2("%f %f %f",FWD,STR,AngularVelocity);
	DOUT4("%f %f %f %f",_.sFL,_.sFR,_.sRL,_.sRR);
	DOUT5("%f %f %f %f",_.aFL,_.aFR,_.aRL,_.aRR);
	#endif
	//DOUT5("%f %f %f",FWD,STR,omega);  //Test accuracy
}

void Swerve_Drive::InterpolateThrusterChanges(Vec2d &LocalForce,double &Torque,double dTime_s)
{
	Vec2d OldLocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	Vec2d LocalVelocity;
	double AngularVelocity;
	InterpolateVelocities(GetSwerveVelocities(),LocalVelocity,AngularVelocity,dTime_s);

	Vec2d LinearAcceleration=LocalVelocity-OldLocalVelocity;
	LocalForce=(LinearAcceleration * Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=AngularVelocity - m_Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
}

void Swerve_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	Vehicle_Drive_Common_ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);	
	__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,-1,dTime_s);
}

bool Swerve_Drive::InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	return Vehicle_Drive_Common_InjectDisplacement(m_Physics,DeltaTime_s,GetAtt_r(),PositionDisplacement,RotationDisplacement);
}

double Swerve_Drive::GetIntendedVelocitiesFromIndex(size_t index) const
{
	return m_Velocities.Velocity.AsArray[index];
}

double Swerve_Drive::GetSwerveVelocitiesFromIndex(size_t index) const
{
	return GetSwerveVelocities().Velocity.AsArray[index];
}
