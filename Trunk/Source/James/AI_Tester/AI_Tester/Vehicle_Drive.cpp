#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;
using namespace std;

//const double PI=M_PI;
const double Pi2=M_PI*2.0;

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
		Vec2d LocalForce_norm(LocalForce);
		LocalForce_norm.normalize();
		ForceHeading=atan2(LocalForce_norm[0],LocalForce_norm[1]);
		//DOUT2("x=%f y=%f h=%f\n",LocalForce[0],LocalForce[1],RAD_2_DEG(ForceHeading));
	}
	#endif
	double LeftDelta,RightDelta;
	//Now to blend the torque into the velocities
	{
		double Width=GetDimensions()[0];
		//first convert to angular acceleration
		double AccelerationDelta=TorqueRestrained/Mass;
		double AngularVelocityDelta=AccelerationDelta*dTime_s;
		//Convert the angular velocity into linear velocity
		double AngularVelocityDelta_linear=AngularVelocityDelta * Width;
		//I'm keeping this first attempt, I like it because it is simple and reliable however, when going forward in fast speeds the torque will clobber the
		//linear force with abrupt stopping 
		Vec2d CurrentVelocity(m_LeftLinearVelocity,m_RightLinearVelocity);
		{
			//Scale down the amount of torque based on current speed... this helps not slow down the linear force when turning
			double FilterScaler=1.0 - (CurrentVelocity.length() / (ENGAGED_MAX_SPEED*2.0));
			AngularVelocityDelta_linear*=FilterScaler;
		}

		LeftDelta=(AngularVelocityDelta_linear/2)+LinearVelocityDelta;
		RightDelta=(-AngularVelocityDelta_linear/2)+LinearVelocityDelta;

		#if 1
		Vec2d NewDelta(LeftDelta,RightDelta);
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

	//DOUT2("left=%f right=%f Ang=%f\n",m_LeftLinearVelocity,m_RightLinearVelocity,RAD_2_DEG(m_Physics.GetAngularVelocity()));
}

void Tank_Drive::InterpolateVelocities(double LeftLinearVelocity,double RightLinearVelocity,Vec2d &LocalVelocity,double &AngularVelocity,double dTime_s)
{
	double LeftMagnitude=fabs(LeftLinearVelocity);
	double RightMagnitude=fabs(RightLinearVelocity);
	double CommonMagnitude=min(LeftMagnitude,RightMagnitude);
	double RightAngularDelta;
	double LeftAngularDelta;
	//We do not care about x, but we may want to keep an eye for intense x forces
	double Width=GetDimensions()[0];
	double NewVelocityY;
	//See if velocities are going in the same direction
	if (LeftLinearVelocity * RightLinearVelocity >= 0)
	{
		//First lets simplify the the overall velocity by transferring the common speed to local force
		double CommonVelocity=LeftLinearVelocity>0?CommonMagnitude:-CommonMagnitude; //put back in the correct direction
		LeftLinearVelocity-=CommonVelocity;
		RightLinearVelocity-=CommonVelocity;
		NewVelocityY=CommonVelocity;
	}
	else
	{
		//going in different direction
		NewVelocityY=0;  //nothing to do... the common code will cancel them out
	}

	RightAngularDelta=LeftLinearVelocity / (Pi2 * Width);
	LeftAngularDelta=-(RightLinearVelocity / (Pi2 * Width));

	//We also need to add displacement of the left over turn..
	{
		double RAD_Slice=RightAngularDelta *Pi2 * dTime_s;
		double LAD_Slice=LeftAngularDelta * Pi2 * dTime_s;
		double Radius=Width/2.0;
		double Height=(sin(RAD_Slice) * Radius) + (sin(-LAD_Slice) * Radius);
		double Width=((1.0-cos(RAD_Slice))*Radius) + (-(1.0-cos(LAD_Slice))*Radius);
		LocalVelocity[0]=Width;
		NewVelocityY+=(Height / dTime_s);
	}
	LocalVelocity[1]=NewVelocityY;

	AngularVelocity=((LeftAngularDelta+RightAngularDelta)*Pi2);
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

void Tank_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	Vec2d NewLocalForce(LocalForce);
	double NewTorque=Torque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,-1,dTime_s);
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
	const double L=GetDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetDimensions()[0];

	//const double R = sqrt((L*L)+(W*W));
	const double R = GetDimensions().length();

	//Allow around 2-3 degrees of freedom and about the safe for strafe.  While manual control worked fine without it, it is needed for
	//targeting goals (e.g. follow ship)

	Vec2d CurrentVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	double STR=((LocalForce[0]/Mass)*dTime_s)+CurrentVelocity[0];
	STR=fabs(STR)<0.3?0.0:STR;
	double FWD=((LocalForce[1]/Mass)*dTime_s)+CurrentVelocity[1];
	FWD=IsZero(FWD)?0.0:FWD;
	double RCW=(TorqueRestrained/Mass)*dTime_s+m_Physics.GetAngularVelocity();
	RCW=fabs(RCW)<0.3?0.0:RCW;

	const double A = STR - RCW*(L/R);
	const double B = STR + RCW*(L/R);
	const double C = FWD - RCW*(W/R);
	const double D = FWD + RCW*(W/R);
	SwerveVelocities &_=m_Velocities;

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
	SwerveVelocities &_=Velocities;
	//L is the vehicle’s wheelbase
	const double L=GetDimensions()[1];
	//W is the vehicle’s track width
	const double W=GetDimensions()[0];

	const double FWD = (_.sFR*cos(_.aFR)+_.sFL*cos(_.aFL)+_.sRL*cos(_.aRL)+_.sRR*cos(_.aRR))/4;

	const double STR = (_.sFR*sin(_.aFR)+_.sFL*sin(_.aFL)+_.sRL*sin(_.aRL)+_.sRR*sin(_.aRR))/4;
	const double HP=PI/2;
	//const double HalfDimLength=GetDimensions().length()/2;

	//This one was close but not quite correct
	//const double HP=HalfDimLength;
	//const double omega = ((_.sFR*cos(atan2(W,L)+(HP-_.aFR))+_.sFL*cos(atan2(-W,L)+(HP-_.aFL))
	//	+_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))+_.sRR*cos(atan2(W,-L)+(HP-_.aRR)))/4)/HalfDimLength;

	const double omega = (((_.sFR*cos(atan2(W,L)+(HP-_.aFR))/4)+(_.sFL*cos(atan2(-W,L)+(HP-_.aFL))/4)
		+(_.sRL*cos(atan2(-W,-L)+(HP-_.aRL))/4)+(_.sRR*cos(atan2(W,-L)+(HP-_.aRR))/4)));

	LocalVelocity[0]=STR;
	LocalVelocity[1]=FWD;

	AngularVelocity=omega;
	//This is a safety to avoid instability
	#if 0
	AngularVelocity=IsZero(omega)?0.0:omega;
	if (AngularVelocity>20.0)
		AngularVelocity=20.0;
	else if (AngularVelocity<-20.0)
		AngularVelocity=-20.0;
	#endif

	#if 1
	DOUT2("%f %f %f",FWD,STR,omega);
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
	InterpolateVelocities(m_Velocities,LocalVelocity,AngularVelocity,dTime_s);

	Vec2d LinearAcceleration=LocalVelocity-OldLocalVelocity;
	LocalForce=(LinearAcceleration * Mass) / dTime_s;

	//Now then we'll compute the torque
	double AngularAcceleration=AngularVelocity - m_Physics.GetAngularVelocity();
	Torque = (AngularAcceleration * Mass) / dTime_s;
}

void Swerve_Drive::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double Torque,double TorqueRestraint,double dTime_s)
{
	UpdateVelocities(PhysicsToUse,LocalForce,Torque,TorqueRestraint,dTime_s);
	Vec2d NewLocalForce(LocalForce);
	double NewTorque=Torque;
	InterpolateThrusterChanges(NewLocalForce,NewTorque,dTime_s);
	//No torque restraint... restraints are applied during the update of velocities
	__super::ApplyThrusters(PhysicsToUse,NewLocalForce,NewTorque,-1,dTime_s);
}

double Swerve_Drive::GetVelocities(size_t index) const
{
	double ret;
	switch(index)
	{
	case 0: ret=m_Velocities.aFL; break;
	case 1: ret=m_Velocities.aFR; break;
	case 2: ret=m_Velocities.aRL; break;
	case 3: ret=m_Velocities.aRR; break;
	case 4: ret=m_Velocities.sFL; break;
	case 5: ret=m_Velocities.sFR; break;
	case 6: ret=m_Velocities.sRL; break;
	case 7: ret=m_Velocities.sRR; break;
	}
	return ret;
}
