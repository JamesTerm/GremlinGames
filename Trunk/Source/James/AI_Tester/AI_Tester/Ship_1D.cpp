#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;

#undef __DisableShipSpeedBoost__
#undef __DisableSpeedControl__  //This one is great for test purposes


  /***************************************************************************************************************/
 /*													Ship_1D														*/
/***************************************************************************************************************/

Ship_1D::Ship_1D(const char EntityName[]) : Entity1D(EntityName)
	
{
	SetSimFlightMode(true);  //this sets up the initial speed as well
	m_LockShipToPosition=false;  //usually this is false (especially for AI and Remote controllers)
	ResetPos();
}

Ship_1D::~Ship_1D()
{
}


void Ship_1D::ResetPos()
{
	__super::ResetPos();

	m_RequestedVelocity = m_currAccel =0.0;
	m_Last_RequestedVelocity=-1.0;
	m_IntendedPosition=0.0;
	m_IntendedPositionPhysics.ResetVectors();
	//m_Physics.ResetVectors(); called from entity 1D's reset
	SetSimFlightMode(true);  //This one is a tough call... probably should do it on reset
}

void Ship_1D::SetSimFlightMode(bool SimFlightMode)	
{
	//It seems that some people want/need to call this function repeatedly so I have included a valve branch here to prevent the debug flooding
	//And to not do extra work on the m_RequestedVelocity.
	if (m_SimFlightMode!=SimFlightMode)
	{
		m_RequestedVelocity=m_Physics.GetLinearVelocity();
		m_SimFlightMode=SimFlightMode;	
		DebugOutput("SimFlightMode=%d\n",SimFlightMode);
	}
}

void Ship_1D::SetRequestedVelocity(double Velocity)
{
	SetSimFlightMode(true);
	if (Velocity>0.0)
		m_RequestedVelocity=MIN(Velocity,GetMaxSpeed());
	else
		m_RequestedVelocity=MAX(Velocity,-GetMaxSpeed());
}


void Ship_1D::Initialize(EventMap& em,const Entity1D_Properties *props)
{
	__super::Initialize(em,props);
	const Ship_1D_Properties *ship_props=dynamic_cast<const Ship_1D_Properties *>(props);
	if (ship_props)
	{
		ship_props->Initialize(this);
	}
	else
	{
		MAX_SPEED = 1.0;
		ACCEL = 0.25;
		BRAKE = 0.25;

		MaxAccelForward=0.25;
		MaxAccelReverse=0.25;
	}
	Mass  = m_Physics.GetMass();

	m_IntendedPosition = 0.0;
	m_IntendedPositionPhysics.SetMass(Mass);
}


void Ship_1D::UpdateIntendedPosition(double dTime_s)
{
	double Vel=0.0,VelControlled=0.0;
	
	//distribute the rotation velocity to the correct case
	if (m_LockShipToPosition)
		VelControlled=m_currAccel;
	else
		Vel=m_currAccel;


	m_IntendedPosition+=Vel;

	double ForceToApply=m_IntendedPositionPhysics.GetForceFromVelocity(VelControlled,dTime_s);
	//compute the restrained force here
	ForceToApply=m_IntendedPositionPhysics.ComputeRestrainedForce(ForceToApply,MaxAccelForward*Mass,MaxAccelReverse*Mass,dTime_s);
	m_IntendedPositionPhysics.ApplyFractionalForce(ForceToApply,dTime_s);
	{
		//Run physics update for displacement
		double PositionDisplacement;
		m_IntendedPositionPhysics.TimeChangeUpdate(dTime_s,PositionDisplacement);
		m_IntendedPosition+=PositionDisplacement;
	}
}
//////////////////////////////////////////////////////////////////////////

void Ship_1D::TimeChange(double dTime_s)
{

	// Find the current speed and use to determine the flight characteristics we will WANT to us
	double LocalVelocity=m_Physics.GetLinearVelocity();
	double currFwdVel = LocalVelocity;
	bool manualMode = !((m_SimFlightMode)&&(m_currAccel==0));

	double ForceToApply;
	double posDisplacement_m=0;

	{
		UpdateIntendedPosition(dTime_s);
		//Determine the angular distance from the intended orientation
		posDisplacement_m=m_IntendedPosition-GetPos_m();
	}

	//Note: We use -1 for roll here to get a great effect on being in perfect sync to the intended orientation 
	//(provided the user doesn't exceed the turning speed of the roll)
	double Ships_ForceRestraint=MaxAccelForward*Mass;
	{
		//This will increase the ships speed if it starts to lag further behind
		#ifndef __DisableShipSpeedBoost__
		//For joystick and keyboard we can use -1 to lock to the intended quat
		if (m_LockShipToPosition)
		{
			Ships_ForceRestraint=-1.0;  //we are locked to the orientation!
		}
		#endif
	}

	//Apply the restraints now... I need this to compute my roll offset
	double AccRestraintPositive=MaxAccelForward;
	double AccRestraintNegative=MaxAccelReverse;

	//Unlike in 2D the intended position and speed control now reside in the same velocity to apply force.  To implement, we'll branch depending on
	//which last UI control was used (i.e. LockShipToPosition) so manual controls like joystick manipulate the speed controls, while automated controls
	//use the intended branch.  I could clean up the UpdateIntendedPosition with this change, but for now I'll keep the functionality intact.
	if (m_LockShipToPosition)
	{
		if (!manualMode)
		{
			//This first system combined the speed request and the accel delta's as one but this runs into undesired effects with the accel deltas
			//The most undesired effect is that when no delta is applied neither should any extra force be applied.  There is indeed a distinction
			//between cruise control (e.g. slider) and using a Key button entry in this regard.  The else case here keeps these more separated where
			//you are either using one mode or the other
			double SpeedDelta=m_currAccel*dTime_s;

			bool UsingRequestedSpeed=false;
			bool YawPitchActive=(fabs(posDisplacement_m)>0.001);

			//Note: m_RequestedVelocity is not altered with the speed delta, but it will keep up to date
			if (SpeedDelta!=0) //if user is changing his adjustments then reset the speed to current velocity
			{
				if (!YawPitchActive)
					m_RequestedVelocity=m_Last_RequestedVelocity=currFwdVel+SpeedDelta;
				else
				{
					//If speeding/braking during hard turns do not use currFwdVel as the centripetal forces will lower it
					m_RequestedVelocity+=SpeedDelta;
					m_Last_RequestedVelocity=m_RequestedVelocity;
					UsingRequestedSpeed=true;
				}
			}
			else
			{
				//If there is any turning while no deltas are on... kick on the requested speed
				if (YawPitchActive)
				{
					m_Last_RequestedVelocity=-1.0;  //active the requested speed mode by setting this to -1 (this will keep it on until a new Speed delta is used)
					UsingRequestedSpeed=true;
				}
				else
					UsingRequestedSpeed=(m_RequestedVelocity!=m_Last_RequestedVelocity);
			}

			//Just transfer the acceleration directly into our speed to use variable
			double SpeedToUse=(UsingRequestedSpeed)? m_RequestedVelocity:currFwdVel+SpeedDelta;


			#ifndef __DisableSpeedControl__
			{
				if (m_currAccel<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
				{
					if ((SpeedToUse) < -MAX_SPEED)
					{
						SpeedToUse = -MAX_SPEED;
						m_currAccel=0.0;
					}
				}
				else 
				{
					double MaxSpeed=MAX_SPEED;
					if ((SpeedToUse) > MaxSpeed)
					{
						SpeedToUse=MaxSpeed;
						m_RequestedVelocity=MaxSpeed;
						m_currAccel=0.0;
					}
				}
			}
			#endif

			if (UsingRequestedSpeed)
				ForceToApply=m_Physics.GetForceFromVelocity(SpeedToUse,dTime_s);
			else
				ForceToApply=m_Physics.GetForceFromVelocity(currFwdVel,dTime_s);  

			if (!UsingRequestedSpeed)
				ForceToApply+=m_currAccel * Mass;
		}
		else   //Manual mode
		{
			#ifndef __DisableSpeedControl__
			{
				{
					double SpeedDelta=m_currAccel*dTime_s;
					if ((LocalVelocity+SpeedDelta>MAX_SPEED)&&(m_currAccel>0))
							m_currAccel=0.0;
					else if ((LocalVelocity+SpeedDelta<-MAX_SPEED)&&(m_currAccel<0))
						m_currAccel=0.0;
				}
			}
			#endif
			ForceToApply=m_currAccel*Mass;
		}
		ForceToApply=m_Physics.ComputeRestrainedForce(ForceToApply,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s);
	}
	else
	{
		double Vel;

		{
			double DistanceToUse=posDisplacement_m;
			//The match velocity needs to be in the same direction as the distance (It will not be if the ship is banking)
			double MatchVel=0.0;
			Vel=m_Physics.GetVelocityFromDistance_Linear(DistanceToUse,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s,MatchVel);
		}

		#ifndef __DisableSpeedControl__
		{
			if (m_currAccel<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
			{
				if ((Vel) < -MAX_SPEED)
				{
					Vel = -MAX_SPEED;
					m_currAccel=0.0;
				}
			}
			else 
			{
				double MaxSpeed=MAX_SPEED;
				if ((Vel) > MaxSpeed)
				{
					Vel=MaxSpeed;
					m_RequestedVelocity=MaxSpeed;
					m_currAccel=0.0;
				}
			}
		}
		#endif

		ForceToApply=m_Physics.GetForceFromVelocity(Vel,dTime_s);
	}


	//To be safe we reset this to zero (I'd put a critical section around this line of code if there are thread issues
	posDisplacement_m=0.0;

	//ApplyThrusters(m_Physics,ForceToApply,TorqueToApply,Ships_TorqueRestraint,dTime_s);
	m_Physics.ApplyFractionalForce(ForceToApply,dTime_s);


	// Now to run the time updates (displacement plus application of it)
	Entity1D::TimeChange(dTime_s);

	m_currAccel=0.0;
}
