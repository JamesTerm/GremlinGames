#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;

  /***********************************************************************************************************************************/
 /*													Vehicle_Drive_Common															*/
/***********************************************************************************************************************************/

//Note: these member variables are arguably written to before read; however, I'm initializing to not mistake first iteration as corrupted data
Vehicle_Drive_Common::Vehicle_Drive_Common(Vehicle_Drive_Common_Interface *VehicleProps) : m_VehicleProps(VehicleProps),
	m_CachedLocalForce(Vec2D()),m_CachedTorque(0.0)
{
}

void Vehicle_Drive_Common::Vehicle_Drive_Common_ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	//Client code will call this explicitly from within Apply Thrusters
	//UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);

	//These are set in InterpolateThrusterChanges
	//m_CachedLocalForce=LocalForce;
	//m_CachedTorque=Torque;

	//Just cache these here to not pollute the control
	m_CachedLinearVelocity=PhysicsToUse.GetLinearVelocity();
	m_CachedAngularVelocity=PhysicsToUse.GetAngularVelocity();

	//Client code will call this explicitly from within Apply Thrusters... using accessors to the cached data
	//InterpolateThrusterChanges(m_CachedLocalForce,m_CachedTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
}

bool Vehicle_Drive_Common::Vehicle_Drive_Common_InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	const bool _InjectedDisplacement=true;
	if (_InjectedDisplacement)
	{
		PhysicsEntity_2D &PhysicsToUse=m_VehicleProps->Vehicle_Drive_GetPhysics_RW();
		const double Att_r=m_VehicleProps->Vehicle_Drive_GetAtt_r();
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

void Vehicle_Drive_Common::InterpolateThrusterChanges(Vec2D &LocalForce,double &Torque,double dTime_s)
{
	m_CachedLocalForce=LocalForce;
	m_CachedTorque=Torque;
}

  /***********************************************************************************************************************************/
 /*															Tank_Drive																*/
/***********************************************************************************************************************************/

Tank_Drive::Tank_Drive(Vehicle_Drive_Common_Interface *Parent) : Vehicle_Drive_Common(Parent),m_pParent(Parent),
	m_LeftLinearVelocity(0.0),m_RightLinearVelocity(0.0)
{
}

void Tank_Drive::ResetPos()
{
	m_LeftLinearVelocity=m_RightLinearVelocity=0.0;
	//No longer aggregating, but could callback to Vehicle_Drive_Common if needed (currently not needed)
	//__super::ResetPos();
}

void Tank_Drive::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);

	const double D=m_pParent->GetWheelTurningDiameter();

	Vec2d CurrentVelocity=GlobalToLocal(m_pParent->Vehicle_Drive_GetAtt_r(),PhysicsToUse.GetLinearVelocity());
	const double Mass=m_pParent->Vehicle_Drive_GetPhysics().GetMass();
	const double FWD=((LocalForce[1]/Mass)*dTime_s)+CurrentVelocity[1];
	//FWD=IsZero(FWD)?0.0:FWD;
	double RCW=(TorqueRestrained/Mass)*dTime_s+PhysicsToUse.GetAngularVelocity();
	//RCW=fabs(RCW)<0.3?0.0:RCW;
	double RPS=RCW / Pi2;
	RCW=RPS * (Pi * D);  //D is the turning diameter

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
	const double D=m_pParent->GetWheelTurningDiameter();

	//const double FWD = (LeftLinearVelocity*cos(1.0)+RightLinearVelocity*cos(1.0))/2.0;
	const double FWD = (LeftLinearVelocity + RightLinearVelocity) / 2.0;
	//const double STR = (LeftLinearVelocity*sin(0.0)+ RightLinearVelocity*sin(0.0))/2.0;
	const double STR = 0.0;

	//const double HP=Pi/2;
	//const double HalfDimLength=GetWheelDimensions().length()/2;

	//Here we go it is finally working I just needed to take out the last division
	const double omega = ((LeftLinearVelocity) + (RightLinearVelocity*-1))/2.0;

	LocalVelocity[0]=STR;
	LocalVelocity[1]=FWD;

	AngularVelocity=(omega / (Pi * D)) * Pi2;
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
	const PhysicsEntity_2D &Physics=m_pParent->Vehicle_Drive_GetPhysics();
	const double Mass=Physics.GetMass();
	Vec2d OldLocalVelocity=GlobalToLocal(m_pParent->Vehicle_Drive_GetAtt_r(),Physics.GetLinearVelocity());
	Vec2d LocalVelocity;
	double AngularVelocity;
	InterpolateVelocities(m_LeftLinearVelocity,m_RightLinearVelocity,LocalVelocity,AngularVelocity,dTime_s);

	Vec2d LinearAcceleration=LocalVelocity-OldLocalVelocity;
	LocalForce=(LinearAcceleration * Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=AngularVelocity - Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
	//call super for it to cache internally
	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Tank_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	Vehicle_Drive_Common_ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);	
	//This is now aggregated so no need to call super
	//__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
}

bool Tank_Drive::InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	//For test purposes
	//return false;
	return Vehicle_Drive_Common_InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);
}


  /***********************************************************************************************************************************/
 /*														Swerve_Drive																*/
/***********************************************************************************************************************************/

Swerve_Drive::Swerve_Drive(Swerve_Drive_Interface *Parent) :  Vehicle_Drive_Common(Parent),m_pParent(Parent)
{
	memset(&m_Velocities,0,sizeof(SwerveVelocities));
}

void Swerve_Drive::ResetPos()
{
	memset(&m_Velocities,0,sizeof(SwerveVelocities));
	//No longer aggregating, but could callback to Vehicle_Drive_Common if needed (currently not needed)
	//__super::ResetPos();
}

void Swerve_Drive::UpdateVelocities(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	const double Mass=m_pParent->Vehicle_Drive_GetPhysics().GetMass();
	double TorqueRestrained=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);
	const Vec2D &WheelDimensions=m_pParent->GetWheelDimensions();

	//L is the vehicle’s wheelbase
	const double L=WheelDimensions[1];
	//W is the vehicle’s track width
	const double W=WheelDimensions[0];

	//const double R = sqrt((L*L)+(W*W));
	const double R = m_pParent->GetWheelTurningDiameter();

	//Allow around 2-3 degrees of freedom for rotation.  While manual control worked fine without it, it is needed for
	//targeting goals (e.g. follow ship)

	Vec2d CurrentVelocity=GlobalToLocal(m_pParent->Vehicle_Drive_GetAtt_r(),PhysicsToUse.GetLinearVelocity());
	const double STR=((LocalForce[0]/Mass)*dTime_s)+CurrentVelocity[0];
	//STR=IsZero(STR)?0.0:STR;
	const double FWD=((LocalForce[1]/Mass)*dTime_s)+CurrentVelocity[1];
	//FWD=IsZero(FWD)?0.0:FWD;
	double RCW=(TorqueRestrained/Mass)*dTime_s+PhysicsToUse.GetAngularVelocity();
	//RCW=fabs(RCW)<0.3?0.0:RCW;
	double RPS=RCW / Pi2;
	RCW=RPS * (Pi * R);  //R is really diameter

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
	const Vec2D &WheelDimensions=m_pParent->GetWheelDimensions();
	//L is the vehicle’s wheelbase
	const double L=WheelDimensions[1];
	//W is the vehicle’s track width
	const double W=WheelDimensions[0];
	const double D = m_pParent->GetWheelTurningDiameter();

	const double FWD = (_.sFR*cos(_.aFR)+_.sFL*cos(_.aFL)+_.sRL*cos(_.aRL)+_.sRR*cos(_.aRR))/4;

	const double STR = (_.sFR*sin(_.aFR)+_.sFL*sin(_.aFL)+_.sRL*sin(_.aRL)+_.sRR*sin(_.aRR))/4;
	const double HP=Pi/2;
	//const double HalfDimLength=GetWheelDimensions().length()/2;

	//Here we go it is finally working I just needed to take out the last division
	const double omega = ((_.sFR*cos(atan2(W,L)+(HP-_.aFR))+_.sFL*cos(atan2(-W,L)+(HP-_.aFL))
		+_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))+_.sRR*cos(atan2(W,-L)+(HP-_.aRR)))/4);

	//const double omega = (((_.sFR*cos(atan2(W,L)+(HP-_.aFR))/4)+(_.sFL*cos(atan2(-W,L)+(HP-_.aFL))/4)
	//	+(_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))/4)+(_.sRR*cos(atan2(W,-L)+(HP-_.aRR))/4)));

	LocalVelocity[0]=STR;
	LocalVelocity[1]=FWD;

	AngularVelocity=(omega / (Pi * D)) * Pi2;
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
	const PhysicsEntity_2D &Physics=m_pParent->Vehicle_Drive_GetPhysics();
	const double Mass=Physics.GetMass();
	Vec2d OldLocalVelocity=GlobalToLocal(m_pParent->Vehicle_Drive_GetAtt_r(),Physics.GetLinearVelocity());
	Vec2d LocalVelocity;
	double AngularVelocity;
	InterpolateVelocities(m_pParent->GetSwerveVelocities(),LocalVelocity,AngularVelocity,dTime_s);

	Vec2d LinearAcceleration=LocalVelocity-OldLocalVelocity;
	LocalForce=(LinearAcceleration * Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=AngularVelocity - Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
	//call super for it to cache internally
	__super::InterpolateThrusterChanges(LocalForce,Torque,dTime_s);
}

void Swerve_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2D &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	Vehicle_Drive_Common_ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
	//This is now aggregated so no need to call super
	//__super::ApplyThrusters(PhysicsToUse,LocalForce,LocalTorque,TorqueRestraint,dTime_s);
}

bool Swerve_Drive::InjectDisplacement(double DeltaTime_s,Vec2D &PositionDisplacement,double &RotationDisplacement)
{
	return Vehicle_Drive_Common_InjectDisplacement(DeltaTime_s,PositionDisplacement,RotationDisplacement);
}

double Swerve_Drive::GetIntendedVelocitiesFromIndex(size_t index) const
{
	return m_Velocities.Velocity.AsArray[index];
}

double Swerve_Drive::GetSwerveVelocitiesFromIndex(size_t index) const
{
	return m_pParent->GetSwerveVelocities().Velocity.AsArray[index];
}
