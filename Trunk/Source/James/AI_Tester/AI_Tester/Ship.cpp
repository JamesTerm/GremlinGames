#include "stdafx.h"
#include "AI_Tester.h"

using namespace AI_Tester;
using namespace GG_Framework::Base;
using namespace osg;

#undef __EnableOrientationResistance__  //This one can probably be removed
#undef __DisableShipSpeedBoost__
#undef __DisableSpeedControl__  //This one is great for test purposes
#undef DEBUG_AFTERBURNER

bool g_DisableEngineRampUp2=false;

namespace Scripting=GG_Framework::Logic::Scripting;
//namespace Scripting=Framework::Scripting;

const double Pi=M_PI;
const double Pi2=M_PI*2.0;
const double Half_Pi=M_PI/2.0;

  /***************************************************************************************************************/
 /*													Ship_2D														*/
/***************************************************************************************************************/

namespace AI_Tester
{

inline const Vec2d Vec2Multiply (const Vec2d &A,const Vec2d &rhs)
{
	return Vec2d(A[0]*rhs._v[0], A[1]*rhs._v[1]);
}

inline Vec2d LocalToGlobal(double Heading,const Vec2d &LocalVector)
{
	return Vec2d(sin(Heading)*LocalVector[1]+cos(-Heading)*LocalVector[0],
					  cos(Heading)*LocalVector[1]+sin(-Heading)*LocalVector[0]);
}
inline Vec2d GlobalToLocal(double Heading,const Vec2d &GlobalVector)
{
	return Vec2d(sin(-Heading)*GlobalVector[1]+cos(Heading)*GlobalVector[0],
					  cos(-Heading)*GlobalVector[1]+sin(Heading)*GlobalVector[0]);
}

//This is really Local to Global for just the Y Component
inline Vec2d GetDirection(double Heading,double Intensity)
{
	return Vec2d(sin(Heading)*Intensity,cos(Heading)*Intensity);
}

}

//Quat FromLW_Rot_Radians(double H, double P, double R);

Ship_2D::Ship_2D(const char EntityName[]) : Ship(EntityName),
	m_controller(NULL),m_IntendedOrientationPhysics(m_IntendedOrientation),m_TorqueReported_Averager(0.0625)
{
	SetSimFlightMode(true);  //this sets up the initial speed as well
	SetStabilizeRotation(true); //This should always be true unless there is some ship failure
	//SetStabilizeRotation(false); //This is for testing
	m_CoordinateTurns=true;  //TODO may want to provide accessor/mutator accessibility
	//m_CoordinateTurns=false;
	m_HeadingSpeedScale=1.0;
	m_LockShipHeadingToOrientation=false;  //usually this is false (especially for AI and Remote controllers)
	m_thrustState=TS_NotVisible;
	m_StabilizeRotation=true;

	m_Physics.SetUsingAccelerationRate(!g_DisableEngineRampUp2);

	ResetPos();
}

Ship_2D::~Ship_2D()
{
	delete m_controller;
	m_controller=NULL;
}


void Ship_2D::ResetPos()
{
	__super::ResetPos();
	//Setting this to true allows starting from an arbitrary position and it not auto adjust to the intended orientation
	//for AI and Remote... they will implicitly clear this once the send the first intended orientation
	m_LockShipHeadingToOrientation=true;
	m_RequestedVelocity[0]=m_RequestedVelocity[1] = 0.0;
	//m_Last_AccDel = 0.0;
	m_Last_RequestedVelocity[0]=m_Last_RequestedVelocity[1]=-1.0;
	m_rotAccel_rad_s = m_rotDisplacement_rad = 0.0;
	m_currAccel =	Vec2d(0,0);
	m_IntendedOrientation=GetAtt_r();
	m_IntendedOrientationPhysics.ResetVectors();
	SetStabilizeRotation(true); //This should always be true unless there is some ship failure
	//SetStabilizeRotation(false); //This is for testing
	SetSimFlightMode(true);  //This one is a tough call... probably should do it on reset
}

void Ship_2D::SetSimFlightMode(bool SimFlightMode)	
{
	//It seems that some people want/need to call this function repeatedly so I have included a valve branch here to prevent the debug flooding
	//And to not do extra work on the m_RequestedVelocity.
	if (m_SimFlightMode!=SimFlightMode)
	{
		//Vec2d LocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
		//m_RequestedVelocity=LocalVelocity;
		//unfortunately a slide turn maneuver requires this, but fortunately is is for UI.  This is not perfect if the user intended to go backwards
		//but that would not be something desirable
		m_RequestedVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity()); 
		m_SimFlightMode=SimFlightMode;	
		DebugOutput("SimFlightMode=%d\n",SimFlightMode);
	}
}

enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
const char* TS_EventNames[] = {
	"Ship.AfterBurner_Brake", "Ship.Brake", "Ship.Coast", "Ship.Thrust", "Ship.AfterBurner", "Ship.NotVisible"
};
Ship_2D::eThrustState Ship_2D::SetThrustState(Ship_2D::eThrustState ts)
{
	//Apply a threshold averager here to avoid thrashing of animation sequences
	ts=m_thrustState_Average.GetValue(ts);
	// Watch for no changes
	//if (ts == m_thrustState) return m_thrustState;
	return m_thrustState;
};

void Ship_2D::SetRequestedVelocity(double Velocity)
{
	//assert(IsLocallyControlled());
	SetSimFlightMode(true);
	if (Velocity>0.0)
		m_RequestedVelocity[1]=MIN(Velocity,GetMaxSpeed());
	else
		m_RequestedVelocity[1]=MAX(Velocity,-GetMaxSpeed());
	m_RequestedVelocity[0]=0;
}

void Ship_2D::SetRequestedVelocity(Vec2D Velocity)
{
	//Note: with 2D submissions we do not need to manually set sim mode 
	//assert(IsLocallyControlled());
	m_RequestedVelocity=Velocity;  //I'm not doing speed checking for this one (this will be checked later anyhow)
	//DOUT5("%f %f",m_RequestedVelocity[0],m_RequestedVelocity[1]);
}

void Ship_2D::SetIntendedOrientation(double IntendedOrientation,bool Absolute)
{
	m_LockShipHeadingToOrientation=false; //this must be false for this to work (if not already)
	m_IntendedOrientation=Absolute?IntendedOrientation: NormalizeRotation2(m_IntendedOrientation+IntendedOrientation);
}

void Ship_2D::SetCurrentAngularAcceleration(double Acceleration,bool LockShipHeadingToOrientation) 
{	
	m_LockShipHeadingToOrientation=LockShipHeadingToOrientation;
	m_rotAccel_rad_s=Acceleration;
}


#if 0
Vec3d Ship_2D::GetArtificialHorizonComponent() const
{
	//Here is the simplest case where we use the global coordinates as our orientation
	Vec3d GlobalForce(0.0,0.0,EARTH_G*Mass);
	//Now to present this force in how it applies locally to our ship
	Vec3d ForceToApply(m_IntendedOrientation.conj() * GlobalForce);
	return ForceToApply;
}
#endif
#undef __TestFullForce__


void Ship_2D::ApplyTorqueThrusters(PhysicsEntity_2D &PhysicsToUse,double Torque,double TorqueRestraint,double dTime_s)
{  
	//assert(IsLocallyControlled());
	//ApplyTorqueThrusters
	//Note: desired speed is a separated variable isControlled from the ship's speed script, which we fine tune given the torque restraints
	//And also by minimizing the amount of G's pulled at the outer most edge of the ship... so for large ships that rotate this could be
	//significant, and you wouldn't want people slamming into the walls.
	//Note: if the speed is too high and the torque restraint is too low the ship will "wobble" because it trying to to go a faster speed that it
	//can "brake" for... ideally a little wobble is reasonable and this is isControlled by a good balance between desired speed and torque restraints

	double TorqueToApply=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);

	#if 0  //This case is only for test purposes (I will eventually remove)
	PhysicsToUse.ApplyTorque(TorqueToApply);
	#else
	PhysicsToUse.ApplyFractionalTorque(TorqueToApply,dTime_s,m_RadialArmDefault);
	#endif
}


///Putting force and torque together will make it possible to translate this into actual force with position
void Ship_2D::ApplyThrusters(PhysicsEntity_2D &PhysicsToUse,const Vec2d &LocalForce,double LocalTorque,double TorqueRestraint,double dTime_s)
{
	//assert(IsLocallyControlled());
	
	 //Apply force
	Vec2d ForceToApply=LocalToGlobal(GetAtt_r(),LocalForce);

	PhysicsToUse.ApplyFractionalForce(ForceToApply,dTime_s);

	// Apply Torque
	ApplyTorqueThrusters(PhysicsToUse,LocalTorque,TorqueRestraint,dTime_s);
}

void Ship_2D::TestPosAtt_Delta(const Vec2d pos_m, double att, double dTime_s)
{
	#if 0
		if (m_controller->IsUIControlled())
			DOUT1 ("%f %f %f %f",dTime_s,pos_m[0],pos_m[1],pos_m[2]);
	#endif
}

AI_Base_Controller *Ship_2D::Create_Controller() 
{
	return new AI_Base_Controller(*this);
}

void Ship_2D::UpdateShipProperties(const Ship_Props &props)
{
	m_ShipProps.UpdateShipProperties(props);
	dHeading=props.dHeading;
	MAX_SPEED=props.MAX_SPEED;
	ENGAGED_MAX_SPEED=props.ENGAGED_MAX_SPEED;
	ACCEL=props.ACCEL;
	BRAKE=props.BRAKE;
	STRAFE=props.STRAFE;
	AFTERBURNER_ACCEL=props.AFTERBURNER_ACCEL;
	AFTERBURNER_BRAKE=props.AFTERBURNER_BRAKE;

	EngineRampAfterBurner=props.EngineRampAfterBurner;
	EngineRampForward=props.EngineRampForward;
	EngineRampReverse=props.EngineRampReverse;
	EngineRampStrafe=props.EngineRampStrafe;
	EngineDeceleration=props.EngineDeceleration;

	MaxAccelLeft=props.MaxAccelLeft;
	MaxAccelRight=props.MaxAccelRight;
	MaxTorqueYaw=props.MaxTorqueYaw * m_Physics.GetMass();
}

void Ship_2D::Initialize(Entity2D::EventMap& em,const Entity_Properties *props)
{
	if (!m_controller)
		m_controller = Create_Controller();
	__super::Initialize(em,props);
	const Ship_Properties *ship_props=dynamic_cast<const Ship_Properties *>(props);
	if (ship_props)
	{
		UpdateShipProperties(ship_props->GetShipProps());
		m_ShipProps=*ship_props;
	}
	else
	{
		double Scale=0.2;  //we must scale everything down to see on the view
		MAX_SPEED = 2000.0 * Scale;
		ENGAGED_MAX_SPEED = 400.0 * Scale;
		ACCEL = 60.0 * Scale;
		BRAKE = 50.0 * Scale;
		STRAFE = BRAKE; //could not find this one
		AFTERBURNER_ACCEL = 107.0 * Scale;
		AFTERBURNER_BRAKE = BRAKE;

		double RAMP_UP_DUR = 1.0;
		double RAMP_DOWN_DUR = 1.0;
		EngineRampAfterBurner= AFTERBURNER_ACCEL/RAMP_UP_DUR;
		EngineRampForward= ACCEL/RAMP_UP_DUR;
		EngineRampReverse= BRAKE/RAMP_UP_DUR;
		EngineRampStrafe= STRAFE/RAMP_UP_DUR;
		EngineDeceleration= ACCEL/RAMP_DOWN_DUR;

		MaxAccelLeft=40.0 * Scale;
		MaxAccelRight=40.0 * Scale;
		//MaxAccelForward=87.0 * Scale;
		//MaxAccelReverse=70.0 * Scale;
		MaxTorqueYaw=2.5;
		dHeading = DEG_2_RAD(270.0);
	}

	Camera_Restraint=G_Dampener=1.0;
	Mass  = m_Physics.GetMass();

	//For now I don't really care about these numbers yet, so I'm pulling from the q33
	m_Physics.StructuralDmgGLimit = 10.0;
	m_Physics.GetPilotInfo().GLimit = 8.0;
	m_Physics.GetPilotInfo().PassOutTime_s = 10.0;
	m_Physics.GetPilotInfo().PassOutRecoveryTime_s = 1.0;
	m_Physics.GetPilotInfo().MaxRecoveryTime_s = 10.0;

	double RadiusOfConcentratedMass=m_Physics.GetRadiusOfConcentratedMass();
	m_IntendedOrientationPhysics.SetRadiusOfConcentratedMass(RadiusOfConcentratedMass);
	m_RadialArmDefault=RadiusOfConcentratedMass*RadiusOfConcentratedMass;

	//Pass these acceleration derivatives on to the Physics/Flight-Dynamics
	{
		FlightDynamics_2D::LinearAccelerationRates &_=m_Physics.GetLinearAccelerationRates();
		_.AccDeltaPos=Vec2d(EngineRampStrafe,EngineRampForward);
		_.AccDeltaNeg=Vec2d(EngineRampStrafe,EngineRampReverse);
		Vec2d Deceleration(EngineDeceleration,EngineDeceleration);
		_.DecDeltaPos=_.DecDeltaNeg=Deceleration;
	}

	m_IntendedOrientation = GetAtt_r();
	m_IntendedOrientationPhysics.SetMass(Mass);
}


void Ship_2D::UpdateIntendedOrientaton(double dTime_s)
{
	double rotVel=0.0,rotVelControlled=0.0;
	
	//distribute the rotation velocity to the correct case
	if (m_LockShipHeadingToOrientation)
		rotVelControlled=m_rotAccel_rad_s;
	else
		rotVel=m_rotAccel_rad_s;

	//Make sure the look ahead is within a reasonable distance from the ship; otherwise the quat delta's may give error as to yaw and pitch corrections
	//To make this look smooth we will compute it as resistance
	double YawResistance=1.0;
	#ifdef __EnableOrientationResistance__
	if (!m_UseHeadingSpeed)
	{
		Vec3d Offset(m_Physics.ComputeAngularDistance(m_IntendedOrientation));
		const double MaxLookAt=M_PI*0.4; //the max amount to look ahead
		YawResistance=fabs(Offset[0])<MaxLookAt?1.0-(fabs(Offset[0])/MaxLookAt):0.0;
	}
	#endif

	// From Rick: James, Why are we not multiplying by time here?  I think the m_rotAccel_rad_s might be artificially high
	// From James: rotVel represents the delta to use at that given moment and should be artificially high as this gives you the
	// "snappiness" feeling when looking around this the mouse
	m_IntendedOrientation+=rotVel*YawResistance;

	double TorqueToApply=m_IntendedOrientationPhysics.GetTorqueFromVelocity(rotVelControlled,dTime_s);
	ApplyTorqueThrusters(m_IntendedOrientationPhysics,TorqueToApply,MaxTorqueYaw,dTime_s);
	{
		//Run physics update for displacement
		Vec2d PositionDisplacement;
		double RotationDisplacement;
		m_IntendedOrientationPhysics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
		m_IntendedOrientation+=RotationDisplacement*YawResistance;
	}
}

//////////////////////////////////////////////////////////////////////////

#undef _TestIndendedDirction_properties__

void Ship_2D::TimeChange(double dTime_s)
{
	// Update my controller
	m_controller->UpdateController(dTime_s);

	// Find the current velocity and use to determine the flight characteristics we will WANT to us
	//Vec3d LocalVelocity(GetAtt_quat().conj() * m_Physics.GetLinearVelocity());
	Vec2d LocalVelocity=GlobalToLocal(GetAtt_r(),m_Physics.GetLinearVelocity());
	double currVelocity = LocalVelocity[1];
	bool manualMode = !m_SimFlightMode;
	bool afterBurnerOn = (m_RequestedVelocity[1] > GetEngaged_Max_Speed());
	bool afterBurnerBrakeOn = (fabs(currVelocity) > GetEngaged_Max_Speed());
	//const FlightCharacteristics& currFC((afterBurnerOn||afterBurnerBrakeOn) ? Afterburner_Characteristics : GetFlightCharacteristics());

	Vec2d ForceToApply;
	//Enable to monitor current speed
	#if 0
	{
		Vec3d Velocity=m_Physics.GetLinearVelocity();
		if (m_controller->IsUIControlled())
			printf("\r%s %f mps               ",GetID().c_str(),m_Physics.GetSpeed(Velocity));
		//printf("\r%f mph               ",m_Physics.GetSpeed(Velocity)*2.237);  //here's a cool quick conversion to get mph http://www.chrismanual.com/Intro/convfact.htm
	}
	#endif

	if (m_StabilizeRotation)
	{

		//Note: I use to have the intended orientation lead and apply physics on it to result in the desired lock on effect, but this proved to be problematic
		//if the ship was unable to keep up with the intended rate.  That is actually a good error test to keep around here, but true locking to ship will
		//slave the intended orientation to the ship
		//  [1/12/2012 Terminator]

		#ifdef _TestIndendedDirction_properties__
		UpdateIntendedOrientaton(dTime_s);

		//Determine the angular distance from the intended orientation
		m_rotDisplacement_rad=-m_Physics.ComputeAngularDistance(m_IntendedOrientation);
		#else
		if (m_LockShipHeadingToOrientation)
		{
			m_rotDisplacement_rad=m_rotAccel_rad_s;
			m_IntendedOrientation=GetAtt_r();
		}
		else
		{
			UpdateIntendedOrientaton(dTime_s);
			m_rotDisplacement_rad=-m_Physics.ComputeAngularDistance(m_IntendedOrientation);
		}
		#endif
	}
	else
	{
		m_IntendedOrientation=GetAtt_r(); //If we can't stabilize the rotation then the intended orientation is slaved to the ship!
	}

	const double Ships_TorqueRestraint=MaxTorqueYaw;

	//All of this disabling torque restraint only worked when the lock to orientation applied the restraint there... now that this is gone
	//there is no reason to ever change the restraint

	//Note: We use -1 for roll here to get a great effect on being in perfect sync to the intended orientation 
	//(provided the user doesn't exceed the turning speed of the roll)
	//{
	//	//This will increase the ships speed if it starts to lag further behind
	//	#ifndef __DisableShipSpeedBoost__
	//	//For joystick and keyboard we can use -1 to lock to the intended quat
	//	if (m_LockShipHeadingToOrientation)
	//	{
	//		Ships_TorqueRestraint=-1.0;  //we are locked to the orientation!
	//	}
	//	#endif
	//}

	//Apply the restraints now... I need this to compute my roll offset
	Vec2d AccRestraintPositive(MaxAccelRight,m_ShipProps.GetMaxAccelForward(currVelocity));
	Vec2d AccRestraintNegative(MaxAccelLeft,m_ShipProps.GetMaxAccelReverse(currVelocity));

	if (!manualMode)
	{
		//This first system combined the velocity request and the accel delta's as one but this runs into undesired effects with the accel deltas
		//The most undesired effect is that when no delta is applied neither should any extra force be applied.  There is indeed a distinction
		//between cruise control (e.g. slider) and using a Key button entry in this regard.  The else case here keeps these more separated where
		//you are either using one mode or the other
		Vec2d VelocityDelta=m_currAccel*dTime_s;

		bool UsingRequestedVelocity=false;
		bool YawPitchActive=(fabs(m_rotDisplacement_rad)>0.001);

		//Note: m_RequestedVelocity is not altered with the velocity delta, but it will keep up to date
		if (VelocityDelta[1]!=0) //if user is changing his adjustments then reset the velocity to current velocity
		{
			if (!YawPitchActive)
				m_RequestedVelocity=m_Last_RequestedVelocity=LocalVelocity+VelocityDelta;
			else
			{
				//If speeding/braking during hard turns do not use currVelocity as the centripetal forces will lower it
				m_RequestedVelocity+=VelocityDelta;
				m_Last_RequestedVelocity=m_RequestedVelocity;
				UsingRequestedVelocity=true;
			}
		}
		else
		{
			//If there is any turning while no deltas are on... kick on the requested velocity
			if (YawPitchActive)
			{
				//active the requested velocity mode by setting this to 0 (this will keep it on until a new velocity delta is used)
				//TODO work out a new system for resetting this I cannot use zero, but -1 is not correct either since we can use
				//negative direction
				m_Last_RequestedVelocity[0]=m_Last_RequestedVelocity[1]=-1.0;  
				UsingRequestedVelocity=true;
			}
			else
				UsingRequestedVelocity=(m_RequestedVelocity!=m_Last_RequestedVelocity);
		}

		//Just transfer the acceleration directly into our velocity to use variable
		Vec2d VelocityToUse=(UsingRequestedVelocity)? m_RequestedVelocity:LocalVelocity+VelocityDelta;

		#if 0
		if (stricmp(GetName().c_str(),"Q33_2")==0)
		{
			//DOUT2("%f %f %f",m_RequestedVelocity,m_Last_RequestedVelocity,m_RequestedVelocity-m_Last_RequestedVelocity);
			//DOUT2("%f %f %f",m_RequestedVelocity,currVelocity,m_RequestedVelocity-currVelocity);
			//DOUT3("%f",VelocityDelta);
		}
		#endif

		#ifndef __DisableSpeedControl__
		{
			if (m_currAccel[1]<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
			{
				if ((VelocityToUse[1]) < -ENGAGED_MAX_SPEED)
				{
					m_RequestedVelocity[1] = VelocityToUse[1] = -ENGAGED_MAX_SPEED;
					m_currAccel[1]=0.0;
				}
			}
			else 
			{
				double MaxSpeed=afterBurnerOn?MAX_SPEED:ENGAGED_MAX_SPEED;
				if ((VelocityToUse[1]) > MaxSpeed)
				{
					m_RequestedVelocity[1]=VelocityToUse[1]=MaxSpeed;
					m_currAccel[1]=0.0;
				}
			}
		}
		#endif

		Vec2d GlobalForce;
		if (UsingRequestedVelocity)
		{
			GlobalForce=m_Physics.GetForceFromVelocity(LocalToGlobal(GetAtt_r(),VelocityToUse),dTime_s);
			//Allow subclass to evaluate the requested velocity in use;
			RequestedVelocityCallback(VelocityToUse[1],dTime_s);
		}
		else
		{
			//We basically are zeroing the strafe here, and adding the forward/reverse element next
			GlobalForce=m_Physics.GetForceFromVelocity(GetDirection(GetAtt_r(),currVelocity),dTime_s);  
			//Allow subclass to evaluate the requested velocity in use;
			RequestedVelocityCallback(currVelocity,dTime_s);
		}

		//so we'll need to convert to local
		//ForceToApply=(GetAtt_quat().conj() * GlobalForce);
		ForceToApply=GlobalToLocal(GetAtt_r(),GlobalForce);

		#if 0
		if (fabs(LocalVelocity[1])>0.0)
			printf("v=%.2f ",(ForceToApply[1]/Mass) / AccRestraintPositive[1]);
		#endif


		if (!UsingRequestedVelocity)
			ForceToApply[1]+=m_currAccel[1] * Mass;
		//This shows no force being applied when key is released
		#if 0
		if (stricmp(GetName().c_str(),"Q33_2")==0)
		{
			Vec3d acc=ForceToApply/Mass;
			DOUT2("%f %f",acc[0],acc[1]);
			DOUT3("%f %f",VelocityToUse,Mass);
		}
		#endif
	}
	else   //Manual mode
	{
		#ifndef __DisableSpeedControl__
		{
			//TODO we may want to compute the fractional forces to fill in the final room left, but before that the engine ramp up would need to
			//be taken into consideration as it currently causes it to slightly go beyond the boundary
			double MaxForwardSpeed=afterBurnerOn?MAX_SPEED:ENGAGED_MAX_SPEED;
			for (size_t i=0;i<2;i++)
			{
				double MaxSpeedThisAxis=i==1?MaxForwardSpeed:ENGAGED_MAX_SPEED;
				double VelocityDelta=(m_currAccel[1]*dTime_s);
				if ((LocalVelocity[i]+VelocityDelta>MaxSpeedThisAxis)&&(m_currAccel[i]>0))
						m_currAccel[i]=0.0;
				else if ((LocalVelocity[i]+VelocityDelta<-ENGAGED_MAX_SPEED)&&(m_currAccel[i]<0))
					m_currAccel[i]=0.0;
			}
		}
		#endif
		//Hand off m_curAccel to a local... we want to preserve the members state
		Vec2d currAccel(m_currAccel);

		#ifdef __TestFullForce__
		ForceToApply=currAccel*Mass*dTime_s;
		#else
		ForceToApply=currAccel*Mass;
		#endif
	}


	//for afterburner up the forward restraint
	if (afterBurnerOn || afterBurnerBrakeOn)
	{
		// Set the maximum restraint values based on Burning or Braking afterburners
		//TODO this property should be the MaxAcceleration type of property where AFTERBURNER_ACCEL is used for keyboard acceleration
		//AccRestraintPositive[1]= afterBurnerOn ? AFTERBURNER_ACCEL : AFTERBURNER_BRAKE;

		//This is not perfect in that all the accelerated and deceleration vector elements need to have this ramp value for non-sliding mode
		//We may alternately consider putting it in slide mode when using afterburner
		m_Physics.GetLinearAccelerationRates().AccDeltaPos=Vec2d(EngineRampAfterBurner,EngineRampAfterBurner);
		m_Physics.GetLinearAccelerationRates().DecDeltaPos=Vec2d(EngineRampAfterBurner,EngineRampAfterBurner);
	}
	else
	{
		m_Physics.GetLinearAccelerationRates().AccDeltaPos=Vec2d(EngineRampStrafe,EngineRampForward);
		m_Physics.GetLinearAccelerationRates().DecDeltaPos=Vec2d(EngineDeceleration,EngineDeceleration);
	}

	ForceToApply=m_Physics.ComputeRestrainedForce(ForceToApply,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s);

	if (!g_DisableEngineRampUp2)
	{
		const Vec2d Target=ForceToApply/Mass;
		m_Physics.SetTargetAcceleration(Target);
		m_Physics.Acceleration_TimeChangeUpdate(dTime_s);
		ForceToApply=m_Physics.GetCurrentAcceleration()*Mass;
	}

	double TorqueToApply;
	if (m_StabilizeRotation)
	{
		//Here we have the original way to turn the ship
		//Vec3d rotVel=m_Physics.GetVelocityFromDistance_Angular(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s);

		//And now the new way using the match velocity, Notice how I divide by time to get the right numbers
		//When this is locked to orientation (e.g. joystick keyboard) this will be ignored since the restraint is -1
		double rotVel;

		if (!m_LockShipHeadingToOrientation)
		{
			double DistanceToUse=m_rotDisplacement_rad;
			//The match velocity needs to be in the same direction as the distance (It will not be if the ship is banking)
			double MatchVel=0.0;
			rotVel=m_Physics.GetVelocityFromDistance_Angular(DistanceToUse,Ships_TorqueRestraint * m_ShipProps.GetRotateToScaler(DistanceToUse),
				dTime_s,MatchVel,!m_LockShipHeadingToOrientation);
		}
		else
			rotVel=m_rotDisplacement_rad;
		//testing stuff  (eventually nuke this)
		//Vec3d rotVel=m_Physics.GetVelocityFromDistance_Angular_v2(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s,Vec3d(0,0,0));
		#if 0
		{
			//Vec3d test2=m_Physics.GetVelocityFromDistance_Angular(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s);
			Vec3d test=m_Physics.GetVelocityFromDistance_Angular_v2(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s,Vec3d(0,0,0));
			//DOUT2("%f %f %f",m_rotAccel_rad_s[0],m_rotAccel_rad_s[1],m_rotAccel_rad_s[2]);
			DOUT2("%f %f %f",rotVel[0],test[0],rotVel[0]-test[0]);
			//DOUT2("%f %f %f",test2[0],test[0],test2[0]-test[0]);
			if (fabs(test[0])>=0.0001)
				DebugOutput("%f %f %f %f",rotVel[0],test[0],rotVel[0]-test[0],m_rotAccel_rad_s[0]);
		}
		#endif

		//enforce the turning speeds
		#if 1
		//While joystick and keyboard never exceed the speeds, the mouse can... so cap it off here
		//Note: The m_HeadingSpeedScale applies to both yaw and pitch since we perform coordinated turns
		{
			double SpeedRestraint=dHeading*m_HeadingSpeedScale;
			double SmallestRatio=1.0;
			//This works similar to LocalTorque restraints; 
			//This method computes the smallest ratio needed to scale down the vectors.  It should give the maximum amount
			//of magnitude available without sacrificing the intended direction
			{
				double AbsComponent=fabs(rotVel);
				if (AbsComponent>SpeedRestraint)
				{
					double Temp=SpeedRestraint/AbsComponent;
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
			rotVel*=SmallestRatio;
		}
		#endif
		//printf("\r%f %f            ",m_rotDisplacement_rad,rotVel);
		TorqueToApply=m_Physics.GetTorqueFromVelocity(rotVel,dTime_s);
		#if 0
		if (fabs(rotVel)>0.0)
			printf("v=%.2f ",TorqueToApply / Ships_TorqueRestraint);
		#endif

	}
	else
		TorqueToApply=m_rotAccel_rad_s*Mass*dTime_s;


	//To be safe we reset this to zero (I'd put a critical section around this line of code if there are thread issues
	m_rotDisplacement_rad=0.0;

	ApplyThrusters(m_Physics,ForceToApply,TorqueToApply,Ships_TorqueRestraint,dTime_s);

	// Now to run the time updates (displacement plus application of it)
	GetPhysics().G_Dampener = G_Dampener;
	__super::TimeChange(dTime_s);

	m_controller->UpdateUI(dTime_s);

	#ifdef _TestNoIndendedDirction_properties__
	if (m_LockShipHeadingToOrientation)
		m_IntendedOrientation=GetAtt_r();
	#endif

	//Reset my controller vars
	m_rotAccel_rad_s=0.0;
	m_currAccel=Vec2d(0,0);
}

//////////////////////////////////////////////////////////////////////////

void Ship_2D::CancelAllControls()
{
	//__super::CancelAllControls();
	//	m_controller->CancelAllControls();
}

//////////////////////////////////////////////////////////////////////////
#if 0
void Ship_2D::DestroyEntity(bool shotDown, Vec3d collisionPt)
{
	__super::DestroyEntity(shotDown, collisionPt);

	//Reset all of the intended vectors and thrusters.
	// Anything that would keep the ship from flying with its current velocities. (linear and angular)
	if (IsLocallyControlled())
	{
		SetSimFlightMode(false);
		SetStabilizeRotation(false);
	}
}
#endif

void Ship_2D::BindAdditionalUIControls(bool Bind,void *joy)
{
	m_ShipProps.Get_ShipControls().BindAdditionalUIControls(Bind,joy);
}


  /***********************************************************************************************************************************/
 /*															Ship_Properties															*/
/***********************************************************************************************************************************/


//These must be in the same order as they are in Ship_Properties::Ship_Type
const char * const csz_RobotNames[] =
{
	"RobotTank",
	"RobotSwerve",
	"RobotButterfly",
	"RobotNona",
	"Robot2011",
	"Robot2012",
	"Robot2013"
};

Ship_Properties::Ship_Properties() : m_ShipControls(&s_ControlsEvents)
{
	Ship_Props props;
	memset(&props,0,sizeof(Ship_Props));

	props.dHeading = DEG_2_RAD(270.0);

	double Scale=0.2;  //we must scale everything down to see on the view
	props.MAX_SPEED = 2000.0 * Scale;
	props.ENGAGED_MAX_SPEED = 400.0 * Scale;
	props.ACCEL = 60.0 * Scale;
	props.BRAKE = 50.0 * Scale;
	props.STRAFE = props.BRAKE; //could not find this one
	props.AFTERBURNER_ACCEL = 107.0 * Scale;
	props.AFTERBURNER_BRAKE = props.BRAKE;

	props.MaxAccelLeft=40.0 * Scale;
	props.MaxAccelRight=40.0 * Scale;
	props.MaxAccelForward=props.MaxAccelForward_High=87.0 * Scale;
	props.MaxAccelReverse=props.MaxAccelReverse_High=70.0 * Scale;
	props.MaxTorqueYaw=2.5;

	double RAMP_UP_DUR = 1.0;
	double RAMP_DOWN_DUR = 1.0;
	props.EngineRampAfterBurner= props.AFTERBURNER_ACCEL/RAMP_UP_DUR;
	props.EngineRampForward= props.ACCEL/RAMP_UP_DUR;
	props.EngineRampReverse= props.BRAKE/RAMP_UP_DUR;
	props.EngineRampStrafe= props.STRAFE/RAMP_UP_DUR;
	props.EngineDeceleration= props.ACCEL/RAMP_DOWN_DUR;
	props.RotateTo_TorqueDegradeScalar=props.RotateTo_TorqueDegradeScalar_High=1.0;
	m_ShipProps=props;
};

const char *Ship_Properties::SetUpGlobalTable(Scripting::Script& script)
{
	Ship_Props &props=m_ShipProps;
	const char* err;
	props.ShipType=Ship_Props::eDefault;
	m_EntityName="Ship";
	err = script.GetGlobalTable("Ship");
	if (err)
	{
		for (size_t i=0;i<_countof(csz_RobotNames);i++)
		{
			err = script.GetGlobalTable(csz_RobotNames[i]);
			if (!err)
			{
				props.ShipType=(Ship_Props::Ship_Type)(i+1);
				m_EntityName=csz_RobotNames[i];
				break;
			}
		}
	}
	return err;
}

//declared as global to avoid allocation on stack each iteration
const char * const g_Ship_Controls_Events[] = 
{
	"Joystick_SetCurrentSpeed_2","Joystick_SetCurrentSpeed","Analog_Turn","POV_Turn","Turn_180",
	"Joystick_SetLeftVelocity","Joystick_SetRightVelocity",
	"SlideHold","Slide","Stop","Thrust","Brake","Analog_StrafeRight","None"
};

const char *Ship_Properties::ControlEvents::LUA_Controls_GetEvents(size_t index) const
{
	return (index<_countof(g_Ship_Controls_Events))?g_Ship_Controls_Events[index] : NULL;
}
Ship_Properties::ControlEvents Ship_Properties::s_ControlsEvents;


void Ship_Properties::LoadFromScript(Scripting::Script& script)
{
	Ship_Props &props=m_ShipProps;
	const char* err=NULL;
	{
		double dHeading;
		err = script.GetField("dHeading", NULL, NULL, &dHeading);
		if (!err)
			props.dHeading=DEG_2_RAD(dHeading);
		else
			script.GetField("heading_rad", NULL, NULL, &props.dHeading);

		err = script.GetField("ACCEL", NULL, NULL, &props.ACCEL);
		err = script.GetField("BRAKE", NULL, NULL, &props.BRAKE);
		err = script.GetField("STRAFE", NULL, NULL, &props.STRAFE);
		if (err)
			props.STRAFE=props.BRAKE; 		//Give strafe the default of the brake

		props.AFTERBURNER_ACCEL=props.ACCEL;  //Give afterburner acceleration rate the same as thrusters for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &props.AFTERBURNER_ACCEL);

		props.AFTERBURNER_BRAKE=props.BRAKE;  //Give afterburner brake rate the same as brakes for default
		err = script.GetField("AFTERBURNER_ACCEL", NULL, NULL, &props.AFTERBURNER_BRAKE);

		// By defaults, we want all of the engines to ramp up to full in this amount of time
		double RAMP_UP_DUR = 1.0;
		double RAMP_DOWN_DUR = 1.0;

		err = script.GetField("RAMP_UP_DUR", NULL, NULL, &RAMP_UP_DUR);
		err = script.GetField("RAMP_DOWN_DUR", NULL, NULL, &RAMP_DOWN_DUR);
		const double MIN_RAMP = 0.00001;	// Avoid stupid numbers, div by 0 errors, etc.
		if (RAMP_UP_DUR < MIN_RAMP) RAMP_UP_DUR = MIN_RAMP;	
		if (RAMP_DOWN_DUR < MIN_RAMP) RAMP_DOWN_DUR = MIN_RAMP;

		err = script.GetField("EngineDeceleration", NULL, NULL, &props.EngineDeceleration);
		if (err) props.EngineDeceleration= props.ACCEL/RAMP_DOWN_DUR;
		err = script.GetField("EngineRampStrafe", NULL, NULL, &props.EngineRampStrafe);
		if (err) props.EngineRampStrafe= props.STRAFE/RAMP_UP_DUR;
		err = script.GetField("EngineRampForward", NULL, NULL, &props.EngineRampForward);
		if (err) props.EngineRampForward= props.ACCEL/RAMP_UP_DUR;
		err = script.GetField("EngineRampReverse", NULL, NULL, &props.EngineRampReverse);
		if (err) props.EngineRampReverse= props.BRAKE/RAMP_UP_DUR;
		err = script.GetField("EngineRampAfterBurner", NULL, NULL, &props.EngineRampAfterBurner);
		if (err) props.EngineRampAfterBurner= props.AFTERBURNER_ACCEL/RAMP_UP_DUR;


		script.GetField("MaxAccelLeft", NULL, NULL, &props.MaxAccelLeft);
		script.GetField("MaxAccelRight", NULL, NULL, &props.MaxAccelRight);
		script.GetField("MaxAccelForward", NULL, NULL, &props.MaxAccelForward);
		err=script.GetField("MaxAccelForward_High", NULL, NULL, &props.MaxAccelForward_High);
		if (err)
			props.MaxAccelForward_High=props.MaxAccelForward;

		script.GetField("MaxAccelReverse", NULL, NULL, &props.MaxAccelReverse);
		err=script.GetField("MaxAccelReverse_High", NULL, NULL, &props.MaxAccelReverse_High);
		if (err)
			props.MaxAccelReverse_High=props.MaxAccelReverse;

		script.GetField("MaxTorqueYaw", NULL, NULL, &props.MaxTorqueYaw);
		script.GetField("rotate_to_scale", NULL, NULL, &props.RotateTo_TorqueDegradeScalar);
		err=script.GetField("rotate_to_scale_high", NULL, NULL, &props.RotateTo_TorqueDegradeScalar_High);
		if (err)
			props.RotateTo_TorqueDegradeScalar_High=props.RotateTo_TorqueDegradeScalar;

		err = script.GetField("MAX_SPEED", NULL, NULL, &props.MAX_SPEED);
		err = script.GetField("ENGAGED_MAX_SPEED", NULL, NULL, &props.ENGAGED_MAX_SPEED);
		if (err)
			props.ENGAGED_MAX_SPEED=props.MAX_SPEED;

		err = script.GetFieldTable("controls");
		if (!err)
		{
			m_ShipControls.LoadFromScript(script);
			script.Pop();
		}
	}

	// Let the base class finish things up
	__super::LoadFromScript(script);
}

//This is depreciated... may need to review game use-case
#if 0
void Ship_Properties::Initialize(Ship_2D *NewShip) const
{
	const Ship_Props &props=m_ShipProps;
	NewShip->dHeading=props.dHeading;
	NewShip->MAX_SPEED=props.MAX_SPEED;
	NewShip->ENGAGED_MAX_SPEED=props.ENGAGED_MAX_SPEED;
	NewShip->ACCEL=props.ACCEL;
	NewShip->BRAKE=props.BRAKE;
	NewShip->STRAFE=props.STRAFE;
	NewShip->AFTERBURNER_ACCEL=props.AFTERBURNER_ACCEL;
	NewShip->AFTERBURNER_BRAKE=props.AFTERBURNER_BRAKE;

	NewShip->EngineRampAfterBurner=props.EngineRampAfterBurner;
	NewShip->EngineRampForward=props.EngineRampForward;
	NewShip->EngineRampReverse=props.EngineRampReverse;
	NewShip->EngineRampStrafe=props.EngineRampStrafe;
	NewShip->EngineDeceleration=props.EngineDeceleration;

	NewShip->MaxAccelLeft=props.MaxAccelLeft;
	NewShip->MaxAccelRight=props.MaxAccelRight;
	//NewShip->MaxAccelForward=props.MaxAccelForward;
	//NewShip->MaxAccelReverse=props.MaxAccelReverse;
	NewShip->MaxTorqueYaw=props.MaxTorqueYaw;
}
#endif

void Ship_Properties::UpdateShipProperties(const Ship_Props &props)
{
	m_ShipProps=props;
}

double Ship_Properties::GetMaxAccelForward(double Velocity) const
{
	const Ship_Props &props=m_ShipProps;
	const double ratio = fabs(Velocity)/props.MAX_SPEED;
	const double  &Low=props.MaxAccelForward;
	const double &High=props.MaxAccelForward_High;
	return (ratio * High) + ((1.0-ratio) * Low);
}

double Ship_Properties::GetMaxAccelReverse(double Velocity) const
{
	const Ship_Props &props=m_ShipProps;
	const double ratio = fabs(Velocity)/props.MAX_SPEED;
	const double  &Low=props.MaxAccelReverse;
	const double &High=props.MaxAccelReverse_High;
	return (ratio * High) + ((1.0-ratio) * Low);
}

double Ship_Properties::GetRotateToScaler(double Distance) const
{
	const Ship_Props &props=m_ShipProps;
	double DistanceMagnitude=fabs(Distance);
	while (DistanceMagnitude>M_PI)
		DistanceMagnitude-=Pi2;
	const double ratio = fabs(DistanceMagnitude)/Pi;
	const double  &Low=props.RotateTo_TorqueDegradeScalar;
	const double &High=props.RotateTo_TorqueDegradeScalar_High;
	return (ratio * High) + ((1.0-ratio) * Low);
}

  /***********************************************************************************************************************************/
 /*														UI_Ship_Properties															*/
/***********************************************************************************************************************************/

UI_Ship_Properties::UI_Ship_Properties()
{
	m_TextImage="*";
	m_UI_Dimensions[0]=1,m_UI_Dimensions[1]=1;
};

void UI_Ship_Properties::Initialize(const char **TextImage,osg::Vec2d &Dimension) const
{
	*TextImage=m_TextImage.c_str();
	Dimension[0]=m_UI_Dimensions[0];
	Dimension[1]=m_UI_Dimensions[1];
}

void UI_Ship_Properties::LoadFromScript(Scripting::Script& script)
{
	const char* err=NULL;
	{
		//Get the ships UI
		err = script.GetFieldTable("UI");
		if (!err)
		{
			err = script.GetField("Length", NULL, NULL,&m_UI_Dimensions[1]);
			ASSERT_MSG(!err, err);
			err = script.GetField("Width", NULL, NULL,&m_UI_Dimensions[0]);
			ASSERT_MSG(!err, err);
			err = script.GetField("TextImage",&m_TextImage,NULL,NULL);
			ASSERT_MSG(!err, err);
			script.Pop();
		}

	}
	__super::LoadFromScript(script);
}
