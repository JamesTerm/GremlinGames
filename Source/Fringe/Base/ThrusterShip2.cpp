#include "stdafx.h"
#include "Fringe.Base.h"

using namespace Fringe::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;

#undef __EnableOrientationResistance__  //This one can probably be removed
#undef __DisableShipSpeedBoost__
#undef __DisableEngineRampUp2__
#undef DEBUG_AFTERBURNER

  /***************************************************************************************************************/
 /*												ThrustShip2														*/
/***************************************************************************************************************/

const double Pi2=M_PI*2.0;
//We may want this to be a variable that can be set in some user settings; for now... const
const double c_AutoLevelDelayTime=3.0;

ClassFactoryT<ThrustShip2, Entity3D> ThrustShip2_ClassFactory("ThrustShip2", Entity3D::ClassFactory);

inline const osg::Vec3d Vec3Multiply (const osg::Vec3d &A,const osg::Vec3d &rhs)
{
	return osg::Vec3d(A[0]*rhs._v[0], A[1]*rhs._v[1], A[2]*rhs._v[2]);
}

osg::Quat FromLW_Rot_Radians(double H, double P, double R);

ThrustShip2::ThrustShip2() : m_controller(NULL),m_IntendedOrientationPhysics(m_IntendedOrientation),m_TS_RC_Controller(NULL),m_TorqueReported_Averager(0.0625)
{
	SetSimFlightMode(true);  //this sets up the initial speed as well
	SetStabilizeRotation(true); //This should always be true unless there is some ship failure
	//SetStabilizeRotation(false); //This is for testing
	SetEnableAutoLevel(false); //Set up autolevel FALSE by default (not on the reset call though)
	m_CoordinateTurns=true;  //TODO may want to provide accessor/mutator accessibility
	//m_CoordinateTurns=false;
	m_HeadingSpeedScale=1.0;
	m_LockShipHeadingToOrientation=m_LockShipRollToOrientation=false;  //usually this is false (especially for AI and Remote controllers)
	m_thrustState=TS_NotVisible;
	m_StabilizeRotation=true;
	#ifndef __DisableEngineRampUp2__
	m_Physics.SetUsingAccelerationRate(true);
	#endif

	m_controller = new AI_Base_Controller(*this);
}

ThrustShip2::~ThrustShip2()
{
	if (m_controller)
		delete m_controller;
	if (m_TS_RC_Controller)
		delete m_TS_RC_Controller;
	m_RC_Controller = m_TS_RC_Controller = NULL;
}


void ThrustShip2::ResetPos()
{
	__super::ResetPos();

	m_RequestedSpeed = 0.0;
	//m_Last_AccDel = 0.0;
	m_Last_RequestedSpeed=-1.0;
	m_rotVel_rad_s = m_rotDisplacement_rad = m_currAccel =	osg::Vec3d(0,0,0);
	m_IntendedOrientation=GetAtt_quat();
	m_IntendedOrientationPhysics.ResetVectors();
	m_LockRoll=true;
	m_AutoLevelDelay=0.0;
	m_GimbalLockTriggered=false;
	SetStabilizeRotation(true); //This should always be true unless there is some ship failure
	//SetStabilizeRotation(false); //This is for testing
	SetSimFlightMode(true);  //This one is a tough call... probably should do it on reset
}

void ThrustShip2::SetSimFlightMode(bool SimFlightMode)	
{
	//It seems that some people want/need to call this function repeatedly so I have included a valve branch here to prevent the debug flooding
	//And to not do extra work on the m_RequestedSpeed.
	if (m_SimFlightMode!=SimFlightMode)
	{
		m_RequestedSpeed=m_Physics.GetSpeed(m_Physics.GetLinearVelocity());
		m_SimFlightMode=SimFlightMode;	
		DebugOutput("SimFlightMode=%d\n",SimFlightMode);
	}
}

void ThrustShip2::SetEnableAutoLevel(bool EnableAutoLevel)
{
	m_EnableAutoLevel=EnableAutoLevel;
	DebugOutput("EnableAutoLevel=%d\n",EnableAutoLevel);
}

enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
const char* TS_EventNames[] = {
	"Ship.AfterBurner_Brake", "Ship.Brake", "Ship.Coast", "Ship.Thrust", "Ship.AfterBurner", "Ship.NotVisible"
};
ThrustShip2::eThrustState ThrustShip2::SetThrustState(ThrustShip2::eThrustState ts)
{
	//Apply a threshold averager here to avoid thrashing of animation sequences
	ts=m_thrustState_Average.GetValue(ts);
	// Watch for no changes
	if (ts == m_thrustState) return m_thrustState;

#ifdef DEBUG_AFTERBURNER
	if (IsLocallyControlled() && GetGameClient()->Get_UI_ActorScene())
		DOUT3(TS_EventNames[ts]);
#endif

	// Turn the old one off and the new one on
	if (TS_NotVisible != m_thrustState)
		GetEventMap()->EventOnOff_Map[TS_EventNames[m_thrustState]].Fire(false);
	m_thrustState = ts;
	if (TS_NotVisible != m_thrustState)
		GetEventMap()->EventOnOff_Map[TS_EventNames[m_thrustState]].Fire(true);
	return m_thrustState;
};

//	virtual void Initialize(GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, TransmittedEntity& te);

void ThrustShip2::SetRequestedSpeed(double Speed)
{
	ASSERT(IsLocallyControlled());
	SetSimFlightMode(true);
	#ifdef __DisableSpeedControl__
	m_RequestedSpeed=Speed;
	#else
	m_RequestedSpeed=MIN(Speed,GetMaxSpeed());
	#endif
}


osg::Vec3d ThrustShip2::GetArtificialHorizonComponent() const
{
	//Here is the simplest case where we use the global coordinates as our orientation
	osg::Vec3d GlobalForce(0.0,0.0,EARTH_G*Mass);
	//Now to present this force in how it applies locally to our ship
	osg::Vec3d ForceToApply(m_IntendedOrientation.conj() * GlobalForce);
	return ForceToApply;
}

double ThrustShip2::ComputeAutoLevelRollOffset()
{
	osg::Vec3d HorizonDownForce(GetArtificialHorizonComponent());
	// This is tricky Theta=atan y/x where Theta is the angle, y=the left/right component of the force, and x=the up/down component
	double AngleOffset=atan2(HorizonDownForce[0],HorizonDownForce[2]);  //Angle in radians

	//Gimbal lock control:  The idea here is to first detect a potential lock and use the upside down technique during the delay sequence
	//once the delay has expired
	if (fabs(AngleOffset)>2.5)
	{
		if (!m_GimbalLockTriggered)
		{
			m_AutoLevelDelay=c_AutoLevelDelayTime;
			m_GimbalLockTriggered=true;
		}
		//while its triggered avoid wobbles by doing the upside down lock technique
		if ((m_GimbalLockTriggered)&&(m_AutoLevelDelay>0.0))
		{
			//avoid Gimbal lock... this approach will reverse the angle where the auto level applies upside down
			if (AngleOffset>2.5)
				AngleOffset-=M_PI;
			else if (AngleOffset<-2.5)
				AngleOffset+=M_PI;
		}
	}
	else
		m_GimbalLockTriggered=false;  //no need to say if it was triggered to not trigger it


	#if 0
	{
		if (AngleOffset!=0.0)
			printf("\r %f radians          ",AngleOffset);
		//printf("\r %f degrees          ",RAD_2_DEG(AngleOffset));
		//osg::Vec3d Test=LocalTrajectoryForceToApply/Mass;
		//if (Test[0]!=0)
		//printf("lr=%f fb=%f ud=%f mps\n",Test[0],Test[1],Test[2]);
		//printf("\rlr=%f fb=%f ud=%f mps                       ",Test[0],Test[1],Test[2]);
	}
	#endif

	return AngleOffset;
}

double ThrustShip2::ComputeRollOffset(const osg::Vec3d &LocalTrajectoryForceToApply) const
{
	osg::Vec3d IntendedDown;
	{
		osg::Vec3d GlobalIntendedDown=(m_IntendedOrientation*osg::Vec3d(0,0,1))*(EARTH_G*Mass);
		IntendedDown=(m_IntendedOrientation.conj() * GlobalIntendedDown);
	}
	osg::Vec3d SummedForceVector=IntendedDown+LocalTrajectoryForceToApply;

	// This is tricky Theta=atan y/x where Theta is the angle, y=the left/right component of the force, and x=the up/down component
	double AngleOffset=atan2(SummedForceVector[0],SummedForceVector[2]);  //Angle in radians

	//Both the gimbal lock and dampen where around to deal with the oscillation problem, but it turns out that it was my ordering that caused this
	//So I no longer need the gimbal lock here, but I *may* need it for the Intended Orientation and the auto level feature
#if 0
	//avoid Gimbal lock... this approach will reverse the angle where the auto level applies upside down
	if (AngleOffset>2.5)
		AngleOffset-=M_PI;
	else if (AngleOffset<-2.5)
		AngleOffset+=M_PI;

#endif

	//I want to "give up" banking for negative g's so we simply give back the intended down's angle offset
	if (fabs(AngleOffset)>2.5)
		AngleOffset=atan2(IntendedDown[0],IntendedDown[2]);

	#if 0
	{
		printf("\r %f radians          ",AngleOffset);
		//printf("\r %f degrees          ",RAD_2_DEG(AngleOffset));
		//osg::Vec3d Test=LocalTrajectoryForceToApply/Mass;
		//if (Test[0]!=0)
			//printf("lr=%f fb=%f ud=%f mps\n",Test[0],Test[1],Test[2]);
			//printf("\rlr=%f fb=%f ud=%f mps                       ",Test[0],Test[1],Test[2]);
	}
	#endif

	return AngleOffset;
}

#undef __TestFullForce__

void ThrustShip2::ApplyTorqueThrusters(Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &Torque,const osg::Vec3d &TorqueRestraint,double dTime_s)
{  
	ASSERT(IsLocallyControlled());
	//ApplyTorqueThrusters
	//Note: desired speed is a separated variable isControlled from the ship's speed script, which we fine tune given the torque restraints
	//And also by minimizing the amount of G's pulled at the outer most edge of the ship... so for large ships that rotate this could be
	//significant, and you wouldn't want people slamming into the walls.
	//Note: if the speed is too high and the torque restraint is too low the ship will "wobble" because it trying to to go a faster speed that it
	//can "brake" for... ideally a little wobble is reasonable and this is isControlled by a good balance between desired speed and torque restraints

	osg::Vec3d TorqueToApply=PhysicsToUse.ComputeRestrainedTorque(Torque,TorqueRestraint,dTime_s);

	#if 0  //This case is only for test purposes (I will eventually remove)
	PhysicsToUse.ApplyTorque(TorqueToApply);
	#else
	PhysicsToUse.ApplyFractionalTorque(TorqueToApply,dTime_s,m_RadialArmDefault);
	#endif

	// Tell the RC Controller about it so it can be sent
	m_TS_RC_Controller->LocalTorque = m_TorqueReported_Averager.GetAverage(TorqueToApply);
}


///Putting force and torque together will make it possible to translate this into actual force with position
void ThrustShip2::ApplyThrusters(Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &LocalForce,const osg::Vec3d &LocalTorque,const osg::Vec3d &TorqueRestraint,double dTime_s)
{
	ASSERT(IsLocallyControlled());
	
	 //Apply force
	osg::Vec3d ForceToApply(GetAtt_quat() * LocalForce);

	#ifdef __TestFullForce__
	PhysicsToUse.ApplyForce(ForceToApply);
	#else
	#if 0
	if ((m_controller)&&(m_controller->IsUIControlled()))
		DOUT1 ("%f %f %f",ForceToApply[0],ForceToApply[1],ForceToApply[2]);
	#endif
	PhysicsToUse.ApplyFractionalForce(ForceToApply,dTime_s);
	#endif

	// I Tell the m_TS_RC_Controller about my Thrust so it can be sent
	m_TS_RC_Controller->LocalThrust = m_ThrustReported_Averager.GetAverage(LocalForce);

	// Apply Torque
	ApplyTorqueThrusters(PhysicsToUse,LocalTorque,TorqueRestraint,dTime_s);
}

void ThrustShip2::TestPosAtt_Delta(const osg::Vec3d pos_m, const osg::Vec3d att, double dTime_s)
{
	#if 0
		if ((m_controller)&&(m_controller->IsUIControlled()))
			DOUT1 ("%f %f %f %f",dTime_s,pos_m[0],pos_m[1],pos_m[2]);
	#endif
}


void ThrustShip2::Initialize(GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, TransmittedEntity& te)
{
	__super::Initialize(gs, em, te);

	ThrustShip2_TransmittedEntity* tste = dynamic_cast<ThrustShip2_TransmittedEntity*>(&te);
	ASSERT(tste);

	Mass  = m_Physics.GetMass();

	NormalFlight_Characteristics = tste->NormalFlight_Characteristics;
	Afterburner_Characteristics = tste->Afterburner_Characteristics;
	Braking_Characteristics = tste->Braking_Characteristics;
	
	//TODO I have found that I do not need these in acceleration form like I do for position, but unfortunately this means I'm inconsistent
	NormalFlight_Characteristics.HackTheMass(Mass);
	Afterburner_Characteristics.HackTheMass(Mass);
	Braking_Characteristics.HackTheMass(Mass);

	m_Physics.StructuralDmgGLimit = tste->StructuralDmgGLimit;
	m_Physics.GetPilotInfo().GLimit = tste->PilotGLimit;
	m_Physics.GetPilotInfo().PassOutTime_s = tste->PilotTimeToPassOut;
	m_Physics.GetPilotInfo().PassOutRecoveryTime_s = tste->PilotTimeToRecover;
	m_Physics.GetPilotInfo().MaxRecoveryTime_s = tste->PilotMaxTimeToRecover;

	osg::Vec3d RadiusOfConcentratedMass=m_Physics.GetRadiusOfConcentratedMass();
	m_IntendedOrientationPhysics.SetRadiusOfConcentratedMass(RadiusOfConcentratedMass);
	m_RadialArmDefault=Vec3Multiply(RadiusOfConcentratedMass,RadiusOfConcentratedMass);

	// We now know what we need for the Thrust Event Handler
	m_thrustEventHandler.SetThrusterShip(this, Mass, m_RadialArmDefault);
	m_thrustEventHandler.ResetFlightCharacteristics();
	m_thrustEventHandler.MaximizeFlightCharacteristics(NormalFlight_Characteristics);
	m_thrustEventHandler.MaximizeFlightCharacteristics(Afterburner_Characteristics);
	m_thrustEventHandler.MaximizeFlightCharacteristics(Braking_Characteristics);

	EngineRampAfterBurner = tste->EngineRampAfterBurner;
	EngineRampForward = tste->EngineRampForward;
	EngineRampReverse = tste->EngineRampReverse;
	EngineRampStrafe = tste->EngineRampStrafe;
	EngineDeceleration = tste->EngineDeceleration;

	//Pass these acceleration derivatives on to the Physics/Flight-Dynamics
	{
		Physics::FlightDynamics::LinearAccelerationRates &_=m_Physics.GetLinearAccelerationRates();
		_.AccDeltaPos=osg::Vec3d(EngineRampStrafe,EngineRampForward,EngineRampStrafe);
		_.AccDeltaNeg=osg::Vec3d(EngineRampStrafe,EngineRampReverse,EngineRampStrafe);
		osg::Vec3d Deceleration(EngineDeceleration,EngineDeceleration,EngineDeceleration);
		_.DecDeltaPos=_.DecDeltaNeg=Deceleration;
	}

	MAX_SPEED = tste->MAX_SPEED;
	ENGAGED_MAX_SPEED = tste->ENGAGED_MAX_SPEED;
	ACCEL = tste->ACCEL;
	BRAKE = tste->BRAKE;
	STRAFE = tste->STRAFE;
	AFTERBURNER_ACCEL = tste->AFTERBURNER_ACCEL;
	AFTERBURNER_BRAKE = tste->AFTERBURNER_BRAKE;

	dHeading = DEG_2_RAD(tste->dHeading);
	dPitch = DEG_2_RAD(tste->dPitch);
	dRoll = DEG_2_RAD(tste->dRoll);

	m_IntendedOrientation = GetAtt_quat();

	m_IntendedOrientationPhysics.SetMass(Mass);
	//m_controller = new AI_Base_Controller(*this, tste->CanUserPilot);
	m_controller->InitializeEventMaps(tste->CanUserPilot);
	m_controller->ReadScriptParameters(*tste);
}


void ThrustShip2::UpdateIntendedOrientaton(double dTime_s)
{
	if (!IsLocallyControlled()) return;
	osg::Vec3d rotVel,rotVelControlled;
	
	//distribute the rotation velocity to the correct case
	if (m_LockShipHeadingToOrientation)
		rotVelControlled=m_rotVel_rad_s;
	else
		rotVel=m_rotVel_rad_s;

	//Roll is dealt with separately since it may be possible to lock the roll without locking the heading (e.g. using keyboard roll with mouse)
	if (m_LockShipRollToOrientation)
	{
		rotVelControlled[2]=m_rotVel_rad_s[2];
		rotVel[2]=0;
	}
	else
	{
		rotVel[2]=m_rotVel_rad_s[2];
		rotVelControlled[2]=0;
	}

	//Make sure the look ahead is within a reasonable distance from the ship; otherwise the quat delta's may give error as to yaw and pitch corrections
	//To make this look smooth we will compute it as resistance
	double YawResistance=1.0,PitchResistance=1.0;
	#ifdef __EnableOrientationResistance__
	if (!m_UseHeadingSpeed || !m_UsePitchSpeed)
	{
		osg::Vec3d Offset(m_Physics.ComputeAngularDistance(m_IntendedOrientation));
		const double MaxLookAt=M_PI*0.4; //the max amount to look ahead
		YawResistance=fabs(Offset[0])<MaxLookAt?1.0-(fabs(Offset[0])/MaxLookAt):0.0;
		PitchResistance=fabs(Offset[1])<MaxLookAt?1.0-(fabs(Offset[    1])/MaxLookAt):0.0;
	}
	#endif

	//Doh!  It matters what order to multiply with quats!
	// From Rick: James, Why are we not multiplying by time here?  I think the m_rotVel_rad_s might be artificially high
	// From James: rotVel represents the delta to use at that given moment and should be artificially high as this gives you the
	// "snappiness" feeling when looking around this the mouse
	m_IntendedOrientation=FromLW_Rot_Radians(rotVel[0]*YawResistance,rotVel[1]*PitchResistance,rotVel[2])*m_IntendedOrientation;

	if ((m_EnableAutoLevel)&&(m_StabilizeRotation))
	{
		if (m_AutoLevelDelay<=0.0)
		{
			//Implement ensuring the roll is always aligned, so that the camera is also aligned
			if (rotVelControlled[2]==0)  //only apply "auto-roll" if roll is not manually being manipulated
			{
				if (m_LockRoll)
				{
					osg::Vec3d TorqueRestraint(GetFlightCharacteristics().MaxTorqueYaw,GetFlightCharacteristics().MaxTorquePitch,Mass*1.5);
					osg::Vec3d RollDelta=m_Physics.GetVelocityFromDistance_Angular(osg::Vec3d(0.0,0.0,-ComputeAutoLevelRollOffset()),TorqueRestraint,dTime_s);
					//Here is where the auto roll gets applied
					rotVelControlled[2]+=RollDelta[2];
				}
			}
		}
		else
			m_AutoLevelDelay-=dTime_s;
	}

	osg::Vec3d TorqueToApply(m_IntendedOrientationPhysics.GetTorqueFromVelocity(rotVelControlled,dTime_s));
	ApplyTorqueThrusters(m_IntendedOrientationPhysics,TorqueToApply,osg::Vec3d (GetFlightCharacteristics().MaxTorqueYaw,GetFlightCharacteristics().MaxTorquePitch,GetFlightCharacteristics().MaxTorqueRoll),dTime_s);
	{
		//Run physics update for displacement
		osg::Vec3d PositionDisplacement;
		osg::Vec3d RotationDisplacement;
		m_IntendedOrientationPhysics.TimeChangeUpdate(dTime_s,PositionDisplacement,RotationDisplacement);
		m_IntendedOrientation=FromLW_Rot_Radians(RotationDisplacement[0]*YawResistance,RotationDisplacement[1]*PitchResistance,RotationDisplacement[2])*m_IntendedOrientation;
	}
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::TimeChange(double dTime_s)
{
	// When not controlling (would normally be !IsLocallyControlled() but James overrides this for some tests) Only update the ship
	if (!RC_Controller_GetIsSending())
	{
		// Tell the physics about my current G Dempener so that the pilot might be affected properly
		GetPhysics().G_Dampener = GetFlightCharacteristics().G_Dampener;
		Ship::TimeChange(dTime_s);
		return;
	}

	// Update my controller
	if (m_controller)
		m_controller->UpdateController(dTime_s);

	// Find the current speed and use to determine the flight characteristics we will WANT to us
	osg::Vec3d LocalVelocity(GetAtt_quat().conj() * m_Physics.GetLinearVelocity());
	double currSpeed = LocalVelocity[1];
	bool manualMode = !((m_SimFlightMode)&&(m_currAccel[0]==0)&&(m_currAccel[2]==0));
	bool afterBurnerOn = (m_RequestedSpeed > GetEngaged_Max_Speed());
	#ifdef __DisableSpeedControl__
	bool afterBurnerBrakeOn = false;
	#else
	bool afterBurnerBrakeOn = (currSpeed > GetEngaged_Max_Speed());
	#endif
	const FlightCharacteristics& currFC((afterBurnerOn||afterBurnerBrakeOn) ? Afterburner_Characteristics : GetFlightCharacteristics());

	osg::Vec3d ForceToApply;

	//Enable to monitor current speed
	#if 0
	{
		osg::Vec3d Velocity=m_Physics.GetLinearVelocity();
		if (m_controller->IsUIControlled())
			printf("\r%s %f mps               ",GetID().c_str(),m_Physics.GetSpeed(Velocity));
		//printf("\r%f mph               ",m_Physics.GetSpeed(Velocity)*2.237);  //here's a cool quick conversion to get mph http://www.chrismanual.com/Intro/convfact.htm
	}
	#endif

	if (m_StabilizeRotation)
	{
		UpdateIntendedOrientaton(dTime_s);
		//Determine the angular distance from the intended orientation
		m_rotDisplacement_rad=-m_Physics.ComputeAngularDistance(m_IntendedOrientation);
	}
	else
	{
		m_IntendedOrientation=GetAtt_quat(); //If we can't stabilize the rotation then the intended orientation is slaved to the ship!
	}

	//Note: We use -1 for roll here to get a great effect on being in perfect sync to the intended orientation 
	//(provided the user doesn't exceed the turning speed of the roll)
	osg::Vec3d Ships_TorqueRestraint(currFC.MaxTorqueYaw,currFC.MaxTorquePitch,currFC.MaxTorqueRoll);
	if ((m_rotVel_rad_s[2]!=0)&&(m_LockShipRollToOrientation))
		Ships_TorqueRestraint[2]=-1.0;  //If we are manually rolling crank up to max for best effect to sync with orientation

	{
		//This will increase the ships speed if it starts to lag further behind
		#ifndef __DisableShipSpeedBoost__
		//For joystick and keyboard we can use -1 to lock to the intended quat
		if (m_LockShipHeadingToOrientation)
		{
			Ships_TorqueRestraint[0]=Ships_TorqueRestraint[1]=-1.0;  //we are locked to the orientation!
		}
		//Keep this around for testing purposes
		#if 0
		//I originally restricted this for Joystick and Keyboard, but now since the turning speeds are checked it should be fine for
		//the mouse to also benefit from this boost.  The ship should be able to turn as fast as its turning speed!
		else  
		{
			const bool CheatOnMouse=false;  //we may want this true for desired effect but its cheating (we would not want this for AI ships)
			if (CheatOnMouse)
			{
				osg::Vec3d Offset(m_Physics.ComputeAngularDistance(m_IntendedOrientation));
				//printf("\r %f roll       ",Offset[2]);
				const double MaxLookAt=M_PI*0.4; //the max amount to look ahead
				double YawBoost=fabs(Offset[0])<MaxLookAt?(fabs(Offset[0])/MaxLookAt):1.0;
				double PitchBoost=fabs(Offset[1])<MaxLookAt?(fabs(Offset[1])/MaxLookAt):1.0;

				// This boost is not quite right... it works ok, but this code should not be used anyhow
				Ships_TorqueRestraint[0]+=currFC.MaxTorqueYaw*YawBoost*10.0;
				Ships_TorqueRestraint[1]+=currFC.MaxTorquePitch*PitchBoost*10.0;
			}
		}
		#endif
		#endif
	}

	//Apply the restraints now... I need this to compute my roll offset
	osg::Vec3d AccRestraintPositive(currFC.MaxAccelRight,currFC.MaxAccelForward,currFC.MaxAccelUp);
	osg::Vec3d AccRestraintNegative(currFC.MaxAccelLeft,currFC.MaxAccelReverse,currFC.MaxAccelDown);

	if (!manualMode)
	{
		//This first system combined the speed request and the accel delta's as one but this runs into undesired effects with the accel deltas
		//The most undesired effect is that when no delta is applied neither should any extra force be applied.  There is indeed a distinction
		//between cruise control (e.g. slider) and using a Key button entry in this regard.  The else case here keeps these more separated where
		//you are either using one mode or the other
		#if 0
		bool ApplySpeedChange=true;
		if ((m_Last_AccDel!=m_currAccel[1])&&(m_currAccel[1]!=0)) //if user is changing his adjustments then reset the speed to current velocity
		{
			m_RequestedSpeed=currSpeed;
		}

		//DOUT2("%f %f %f",m_RequestedSpeed,currSpeed,m_RequestedSpeed-currSpeed);

		double SpeedDelta=(m_currAccel[1]*dTime_s);

		#ifndef __DisableSpeedControl__
		{
			if (m_currAccel[1]<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
			{
				if ((m_RequestedSpeed+SpeedDelta) < -ENGAGED_MAX_SPEED)
				{
					m_RequestedSpeed = -ENGAGED_MAX_SPEED;
					ApplySpeedChange = false;
				}
			}
			else 
			{
				double MaxSpeed=afterBurnerOn?MAX_SPEED:ENGAGED_MAX_SPEED;
				if ((m_RequestedSpeed+SpeedDelta) > MaxSpeed)
				{
					m_RequestedSpeed = MaxSpeed;
					ApplySpeedChange = false;
				}
			}
		}
		#endif

		if (ApplySpeedChange)
			m_RequestedSpeed+=SpeedDelta; //we just transfer the acceleration directly into our speed variable

		m_Last_AccDel=m_currAccel[1];
		//if (stricmp(GetName().c_str(),"Q33_2")==0)
		//	DOUT3("%f",m_RequestedSpeed);

				//Enable to monitor linear speed velocity and acceleration
		#if 0
		if (Velocity[1]!=0)
		{
			DebugOutput("Acceleration=%f,%f,%f\t",Acceleration[0],Acceleration[1],Acceleration[2]);
			DebugOutput("Velocity=%f,%f,%f\t",Velocity[0],Velocity[1],Velocity[2]);
			DebugOutput("Speed=%f\n",Speed);
		}
		#endif

		//This is a global orientation operation
		osg::Vec3d GlobalForce=m_Physics.GetForceFromVelocity(GetDirection()*m_RequestedSpeed,dTime_s);
		//so we'll need to convert to local
		ForceToApply=(GetAtt_quat().conj() * GlobalForce);

		//This shows that force is still being applied even after the release of the key
		#if 0
		if (stricmp(GetName().c_str(),"Q33_2")==0)
		{
			osg::Vec3d acc=GlobalForce/Mass;
			DOUT2("%f %f %f",acc[0],acc[1],acc[2]);
		}
		#endif

		#else
		double SpeedDelta=m_currAccel[1]*dTime_s;

		bool UsingRequestedSpeed=false;
		bool YawPitchActive=(fabs(m_rotDisplacement_rad[0]+m_rotDisplacement_rad[1])>0.001);

		//Note: m_RequestedSpeed is not altered with the speed delta, but it will keep up to date
		if (SpeedDelta!=0) //if user is changing his adjustments then reset the speed to current velocity
		{
			if (!YawPitchActive)
				m_RequestedSpeed=m_Last_RequestedSpeed=currSpeed+SpeedDelta;
			else
			{
				//If speeding/braking during hard turns do not use currSpeed as the centripetal forces will lower it
				m_RequestedSpeed+=SpeedDelta;
				m_Last_RequestedSpeed=m_RequestedSpeed;
				UsingRequestedSpeed=true;
			}
		}
		else
		{
			//If there is any turning while no deltas are on... kick on the requested speed
			if (YawPitchActive)
			{
				m_Last_RequestedSpeed=-1.0;  //active the requested speed mode by setting this to -1 (this will keep it on until a new Speed delta is used)
				UsingRequestedSpeed=true;
			}
			else
				UsingRequestedSpeed=(m_RequestedSpeed!=m_Last_RequestedSpeed);
		}

		//Just transfer the acceleration directly into our speed to use variable
		double SpeedToUse=(UsingRequestedSpeed)? m_RequestedSpeed:currSpeed+SpeedDelta;

		#if 0
		if (stricmp(GetName().c_str(),"Q33_2")==0)
		{
			//DOUT2("%f %f %f",m_RequestedSpeed,m_Last_RequestedSpeed,m_RequestedSpeed-m_Last_RequestedSpeed);
			//DOUT2("%f %f %f",m_RequestedSpeed,currSpeed,m_RequestedSpeed-currSpeed);
			//DOUT3("%f",SpeedDelta);
		}
		#endif

		#ifndef __DisableSpeedControl__
		{
			if (m_currAccel[1]<0) // Watch for braking too far backwards, we do not want to go beyond -ENGAGED_MAX_SPEED
			{
				if ((SpeedToUse) < -ENGAGED_MAX_SPEED)
				{
					SpeedToUse = -ENGAGED_MAX_SPEED;
					m_currAccel[1]=0.0;
				}
			}
			else 
			{
				double MaxSpeed=afterBurnerOn?MAX_SPEED:ENGAGED_MAX_SPEED;
				if ((SpeedToUse) > MaxSpeed)
				{
					SpeedToUse=MaxSpeed;
					m_RequestedSpeed=MaxSpeed;
					m_currAccel[1]=0.0;
				}
			}
		}
		#endif

		osg::Vec3d GlobalForce;
		if (UsingRequestedSpeed)
			GlobalForce=m_Physics.GetForceFromVelocity(GetDirection()*SpeedToUse,dTime_s);
		else
		{
			//We basically are zeroing the strafe here, and adding the forward/reverse element next
			GlobalForce=m_Physics.GetForceFromVelocity(GetDirection()*currSpeed,dTime_s);  
		}

		//so we'll need to convert to local
		ForceToApply=(GetAtt_quat().conj() * GlobalForce);

		if (!UsingRequestedSpeed)
			ForceToApply[1]+=m_currAccel[1] * Mass;
		//This shows no force being applied when key is released
		#if 0
		if (stricmp(GetName().c_str(),"Q33_2")==0)
		{
			osg::Vec3d acc=ForceToApply/Mass;
			DOUT2("%f %f %f",acc[0],acc[1],acc[2]);
			DOUT3("%f %f",SpeedToUse,Mass);
		}
		#endif

		#endif
	}
	else   //Manual mode
	{
		#ifndef __DisableSpeedControl__
		{
			//TODO we may want to compute the fractional forces to fill in the final room left, but before that the engine ramp up would need to
			//be taken into consideration as it currently causes it to slightly go beyond the boundary
			double MaxForwardSpeed=afterBurnerOn?MAX_SPEED:ENGAGED_MAX_SPEED;
			for (size_t i=0;i<3;i++)
			{
				double MaxSpeedThisAxis=i==1?MaxForwardSpeed:ENGAGED_MAX_SPEED;
				double SpeedDelta=(m_currAccel[1]*dTime_s);
				if ((LocalVelocity[i]+SpeedDelta>MaxSpeedThisAxis)&&(m_currAccel[i]>0))
						m_currAccel[i]=0.0;
				else if ((LocalVelocity[i]+SpeedDelta<-ENGAGED_MAX_SPEED)&&(m_currAccel[i]<0))
					m_currAccel[i]=0.0;
			}
		}
		#endif
		//Hand off m_curAccel to a local... we want to preserve the members state
		osg::Vec3d currAccel(m_currAccel);
		if ((m_SimFlightMode)&&((m_currAccel[0]!=0)||(m_currAccel[2]!=0)))
		{
			double CurrentRollOffset;
			{
				osg::Vec3d IntendedDown;
				{
					osg::Vec3d GlobalIntendedDown=(m_IntendedOrientation*osg::Vec3d(0,0,1))*(EARTH_G*Mass);
					IntendedDown=(GetAtt_quat().conj() * GlobalIntendedDown);
				}
				// This is tricky Theta=atan y/x where Theta is the angle, y=the left/right component of the force, and x=the up/down component
				CurrentRollOffset=atan2(IntendedDown[0],IntendedDown[2]);  //Angle in radians
			}
			//To be more strafe friendly we will keep the direction in line with the horizon (so you can repeatedly use it)
			double StrafeLR=cos(-CurrentRollOffset)*m_currAccel[0]+sin(CurrentRollOffset)*m_currAccel[2];
			double StrafeUD=sin(-CurrentRollOffset)*m_currAccel[0]+cos(CurrentRollOffset)*m_currAccel[2];
			currAccel[0]=StrafeLR;
			currAccel[2]=StrafeUD;
		}

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
		AccRestraintPositive[1]= afterBurnerOn ? AFTERBURNER_ACCEL : AFTERBURNER_BRAKE;
		//This is not perfect in that all the accelerated and deceleration vector elements need to have this ramp value for non-sliding mode
		//We may alternately consider putting it in slide mode when using afterburner
		m_Physics.GetLinearAccelerationRates().AccDeltaPos=osg::Vec3d(EngineRampAfterBurner,EngineRampAfterBurner,EngineRampAfterBurner);
		m_Physics.GetLinearAccelerationRates().DecDeltaPos=osg::Vec3d(EngineRampAfterBurner,EngineRampAfterBurner,EngineRampAfterBurner);
	}
	else
	{
		m_Physics.GetLinearAccelerationRates().AccDeltaPos=osg::Vec3d(EngineRampStrafe,EngineRampForward,EngineRampStrafe);
		m_Physics.GetLinearAccelerationRates().DecDeltaPos=osg::Vec3d(EngineDeceleration,EngineDeceleration,EngineDeceleration);
	}

	ForceToApply=m_Physics.ComputeRestrainedForce(ForceToApply,AccRestraintPositive*Mass,AccRestraintNegative*Mass,dTime_s);

	#ifndef __DisableEngineRampUp2__
	{
		const osg::Vec3d Target=ForceToApply/Mass;
		m_Physics.SetTargetAcceleration(Target);
		m_Physics.Acceleration_TimeChangeUpdate(dTime_s);
		ForceToApply=m_Physics.GetCurrentAcceleration()*Mass;
	}
	#endif __DisableEngineRampUp2__

	double roll_BankDisplacement;
	if (m_CoordinateTurns && m_StabilizeRotation)
	{
		{
			double CurrentRollVelocity=m_Physics.GetAngularVelocity()[2];
			if ((!m_LockRoll)&&(fabs(CurrentRollVelocity)<0.001))
			{
				m_LockRoll=true;
				m_AutoLevelDelay=c_AutoLevelDelayTime;
			}

		}

		//adjust roll offset for coordinated turns.  Note this must be called after the force restraints have been factored in
		if (m_rotVel_rad_s[2]==0)
		{
			osg::Vec3d ForceToUse;
			//This is a tough call... when strafing no banking
			if ((m_SimFlightMode)&&(m_currAccel[0]==0)&&(m_currAccel[2]==0))
			//if ((m_SimFlightMode)&&(m_CoordinateTurns))
				ForceToUse=ForceToApply;
			else
				ForceToUse=osg::Vec3d(0.0,0.0,0.0);  //In manual mode the ship still needs to align the roll to ensure camera/roll integrity

			//Doh! this causes the ship to unroll early... we will need to find another way to handle roll sensitivity
			//TODO put the 0.1 as a script variable called AutoRollSensitivity
			if (fabs(m_Physics.GetAngularVelocity()[0])+fabs(m_Physics.GetAngularVelocity()[1])<0.05)
				ForceToUse[0]=0.0;
			roll_BankDisplacement=-ComputeRollOffset(ForceToUse);
		}
	}

	//Implement ensuring the roll is always aligned, so that the camera is also aligned
	if (m_rotVel_rad_s[2]==0)  //only apply "auto-roll" if roll is not manually being manipulated
	{
		if ((m_LockRoll)&&(m_StabilizeRotation))
		{
			//Here is where the auto roll gets applied.  Note: to avoid over-compensation we completely override any pre-computed velocity
			//but then there shouldn't be any other velocity happening in theory.
			osg::Quat AlteredOrientaton(m_IntendedOrientation);
			AlteredOrientaton=FromLW_Rot_Radians(0.0,0.0,roll_BankDisplacement)*AlteredOrientaton;
			m_rotDisplacement_rad=-m_Physics.ComputeAngularDistance(AlteredOrientaton);
			Ships_TorqueRestraint[2]=currFC.MaxTorqueRoll;
		}
	}
	else
		m_LockRoll=false; //With this a user can change his orientation (no autolevel)

	osg::Vec3d TorqueToApply;
	if (m_StabilizeRotation)
	{
		//Here we have the original way to turn the ship
		//osg::Vec3d rotVel=m_Physics.GetVelocityFromDistance_Angular(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s);

		//And now the new way using the match velocity, Notice how I divide by time to get the right numbers
		//When this is locked to orientation (e.g. joystick keyboard) this will be ignored since the restraint is -1
		osg::Vec3d rotVel;

		{
			osg::Vec3d DistanceToUse=m_rotDisplacement_rad;
			#if 0
			if (m_controller->IsUIControlled() )
			{
				//TODO:  get correct direction given the current roll offset.  Then see if AI can work with it
				//If not determine way to access if auto pilot is on
				//I don't like to dynamic cast, but oh well... the AI (or autopilot) must not use this technique as it has bad consequences
				//UI_Controller *UI=dynamic_cast<UI_Controller *>(m_controller);
				//if (UI && (UI->GetAutoPilot()==false))
				if (false)
				{
					//m_rotDisplacement_rad extracts yaw and pitch delta's from a quaternion, and in the process will find the shortest distance to get there
					//If we have any velocity from m_rotVel_rad_s then we have the true intended direction from the user.  Usually the directions match
					//but when the delta's are large they will not, and in that case the user's direction will trump the quaternion's interpretation
					//when there is no velocity sensed the delta usually is small enough as the quaternion's interpretation should be adequate.
					//Note: Roll is not factored in this
					if (m_rotVel_rad_s[0]+m_rotVel_rad_s[1]!=0.0)
					{
						//The actual direction takes banking into account
						double Yaw  =cos(-roll_BankDisplacement)*m_rotVel_rad_s[0]+sin(roll_BankDisplacement)*m_rotVel_rad_s[1];
						double Pitch=sin(-roll_BankDisplacement)*m_rotVel_rad_s[0]+cos(roll_BankDisplacement)*m_rotVel_rad_s[1];
						DOUT2("%f %f %f",Yaw,Pitch,roll_BankDisplacement);
						osg::Vec2d IntendedDirection(Yaw,Pitch);
						//osg::Vec2d IntendedDirection(m_rotVel_rad_s[0],m_rotVel_rad_s[1]);
						osg::Vec2d DistanceVector(m_rotDisplacement_rad[0],m_rotDisplacement_rad[1]);
						//Note: the magnitude may be a bit conservative in the worst case scenario since the quaternion can find a shorter distance
						//However, we should still be able to work just fine with that.
						double Magnitude=DistanceVector.length();
						IntendedDirection.normalize();
						IntendedDirection*=Magnitude;
						DistanceToUse[0]=IntendedDirection[0];
						DistanceToUse[1]=IntendedDirection[1];
					}
				}
			}
			#endif
			//The match velocity needs to be in the same direction as the distance (It will not be if the ship is banking)
			osg::Vec3d MatchVel(0,0,0);
			if (m_rotVel_rad_s[0]+m_rotVel_rad_s[1]!=0.0)
			{
				osg::Vec2d IntendedDirection(DistanceToUse[0],DistanceToUse[1]);
				//osg::Vec2d IntendedDirection(m_rotVel_rad_s[0],m_rotVel_rad_s[1]);
				osg::Vec2d DistanceVector(m_rotDisplacement_rad[0],m_rotDisplacement_rad[1]);
				//Note: the magnitude may be a bit conservative in the worst case scenario since the quaternion can find a shorter distance
				//However, we should still be able to work just fine with that.
				double Magnitude=DistanceVector.length();
				IntendedDirection.normalize();
				IntendedDirection*=Magnitude;
				MatchVel[0]=IntendedDirection[0];
				MatchVel[1]=IntendedDirection[1];
			}

			rotVel=m_Physics.GetVelocityFromDistance_Angular_v2(DistanceToUse,Ships_TorqueRestraint,dTime_s,MatchVel);
		}
		//testing stuff  (eventually nuke this)
		//osg::Vec3d rotVel=m_Physics.GetVelocityFromDistance_Angular_v2(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s,osg::Vec3d(0,0,0));
		#if 0
		{
			//osg::Vec3d test2=m_Physics.GetVelocityFromDistance_Angular(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s);
			osg::Vec3d test=m_Physics.GetVelocityFromDistance_Angular_v2(m_rotDisplacement_rad,Ships_TorqueRestraint,dTime_s,osg::Vec3d(0,0,0));
			//DOUT2("%f %f %f",m_rotVel_rad_s[0],m_rotVel_rad_s[1],m_rotVel_rad_s[2]);
			DOUT2("%f %f %f",rotVel[0],test[0],rotVel[0]-test[0]);
			//DOUT2("%f %f %f",test2[0],test[0],test2[0]-test[0]);
			if (fabs(test[0])>=0.0001)
				DebugOutput("%f %f %f %f",rotVel[0],test[0],rotVel[0]-test[0],m_rotVel_rad_s[0]);
		}
		#endif

		//enforce the turning speeds
		#if 1
		//While joystick and keyboard never exceed the speeds, the mouse can... so cap it off here
		//Note: The m_HeadingSpeedScale applies to both yaw and pitch since we perform coordinated turns
		{
			osg::Vec3d SpeedRestraint(dHeading*m_HeadingSpeedScale,dPitch*m_HeadingSpeedScale,dRoll);
			double SmallestRatio=1.0;
			//This works similar to LocalTorque restraints; 
			//This method computes the smallest ratio needed to scale down the vectors.  It should give the maximum amount
			//of magnitude available without sacrificing the intended direction
			for (size_t i=0;i<2;i++) //only work with yaw and pitch
			{
				double AbsComponent=fabs(rotVel[i]);
				if (AbsComponent>SpeedRestraint[i])
				{
					double Temp=SpeedRestraint[i]/AbsComponent;
					SmallestRatio=Temp<SmallestRatio?Temp:SmallestRatio;
				}
			}
			rotVel[0]*=SmallestRatio;
			rotVel[1]*=SmallestRatio;
		}

		if (rotVel[2]>dRoll)
			rotVel[2]=dRoll;
		if (rotVel[2]<-dRoll)
			rotVel[2]=-dRoll;
		#endif
		//printf("\r%f %f %f mouse             ",m_rotDisplacement_rad[0],m_rotDisplacement_rad[1],m_rotDisplacement_rad[2]);
		TorqueToApply=m_Physics.GetTorqueFromVelocity(rotVel,dTime_s);
	}
	else
		TorqueToApply=m_rotVel_rad_s*Mass*dTime_s;


	//To be safe we reset these to zero (I'd put a critical section around these 3 lines of code if there are thread issues
	m_rotDisplacement_rad[0]=0.0;
	m_rotDisplacement_rad[1]=0.0;
	m_rotDisplacement_rad[2]=0.0;

	ApplyThrusters(m_Physics,ForceToApply,TorqueToApply,Ships_TorqueRestraint,dTime_s);

	// Now to run the time updates (displacement plus application of it)
	GetPhysics().G_Dampener = currFC.G_Dampener;
	Ship::TimeChange(dTime_s);

	// This function fires the various thruster events and updates the ThrsutState
	UpdateThrustState(m_TS_RC_Controller->LocalThrust, m_TS_RC_Controller->LocalTorque);

	m_controller->UpdateUI(dTime_s);

	//Reset my controller vars
	m_rotVel_rad_s=m_currAccel=osg::Vec3d(0,0,0);
}

//////////////////////////////////////////////////////////////////////////

void ThrustShip2::DestroyEntity(bool shotDown, osg::Vec3d collisionPt)
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
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::CancelAllControls()
{
	__super::CancelAllControls();
	if (m_controller && IsLocallyControlled())
		m_controller->CancelAllControls();
}
//////////////////////////////////////////////////////////////////////////

const FlightCharacteristics& ThrustShip2::GetFlightCharacteristics()
{
	if ((m_thrustState==TS_AfterBurner) || (m_thrustState==TS_AfterBurner_Brake))
		return Afterburner_Characteristics;
	else if (m_thrustState==TS_Brake)
		return Braking_Characteristics;
	else return NormalFlight_Characteristics;
}
//////////////////////////////////////////////////////////////////////////

RC_Controller& ThrustShip2::GetRC_Controller()
{
	if (!m_RC_Controller)
	{
		m_TS_RC_Controller = new ThrustShip2_RC_Controller(this);
		m_RC_Controller = m_TS_RC_Controller;
	}
	return *m_RC_Controller;
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::UpdateThrustState(const osg::Vec3d& localThrust, const osg::Vec3d& localTorque)
{
	//None of this matters if the ship is not showing
	if (!IsShowing())
	{
		SetThrustState(TS_NotVisible);
		return;
	}

	// We want to find a thrust state based on the localThrust
	eThrustState newThrustState = TS_Coast;
	double normThrust = NormalFlight_Characteristics.MaxAccelForward * GetMass();
	double normBrake = Braking_Characteristics.MaxAccelReverse * GetMass();
	osg::Vec3d localVel = GetAtt_quat().conj() * GetPhysics().GetLinearVelocity();
	double currSpeed = localVel[1];
	double maxSpeed = this->GetEngaged_Max_Speed() * 1.3;

#ifdef DEBUG_AFTERBURNER
	if (IsLocallyControlled() && GetGameClient()->Get_UI_ActorScene())
		DOUT4("localThrust[1] = %0.5i, normThrust = %0.5i", (int)localThrust[1]/1000, (int)normThrust/1000);
#endif

	// If we are going faster than our engaged speed in any direction...
	if (currSpeed > maxSpeed)
	{
		if (localThrust[1] < normBrake*-0.01) // Only when braking in afterburner
			newThrustState = TS_AfterBurner_Brake;
		else newThrustState = TS_AfterBurner; // All other times for Afterburner
	}
	else if (currSpeed < -maxSpeed)
	{
		// Flying BACKWARDS in an Afterburner state
		newThrustState = TS_AfterBurner_Brake;
	}
	else
	{
		if (localThrust[1] > normThrust*1.1) // Afterburner can start early
			newThrustState = TS_AfterBurner;
		if (localThrust[1] > normThrust*0.01) // Just a little thrust, not much more than 0
			newThrustState = TS_Thrust;
		else if (localThrust[1] < normBrake*-0.01) // Just a little brake, not much less than 0
			newThrustState = TS_Brake;
	}

	SetThrustState(newThrustState);

	// Fire individual Thrust Events
	m_thrustEventHandler.ApplyThrustAndTorque(localThrust, localTorque);
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::Show(bool show)
{
	Fringe::Base::Ship::Show(show);
	if (!show)
		SetThrustState(TS_NotVisible);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ThrustShip2_RC_Controller::PopulateStandardEntityUpdate(RakNet::BitStream& entityUpdateBS)
{
	__super::PopulateStandardEntityUpdate(entityUpdateBS);
	entityUpdateBS.Write(LocalThrust[0]); entityUpdateBS.Write(LocalThrust[1]); entityUpdateBS.Write(LocalThrust[2]);
	entityUpdateBS.Write(LocalTorque[0]); entityUpdateBS.Write(LocalTorque[1]); entityUpdateBS.Write(LocalTorque[2]);
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2_RC_Controller::ReadStandardEntityUpdate(RakNet::BitStream& entityUpdateBS, double msgTime_s)
{
	__super::ReadStandardEntityUpdate(entityUpdateBS, msgTime_s);
	entityUpdateBS.Read(LocalThrust[0]); entityUpdateBS.Read(LocalThrust[1]); entityUpdateBS.Read(LocalThrust[2]);
	entityUpdateBS.Read(LocalTorque[0]); entityUpdateBS.Read(LocalTorque[1]); entityUpdateBS.Read(LocalTorque[2]);
	m_parent->UpdateThrustState(LocalThrust, LocalTorque);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::ThrustEventHandler::SetThrusterShip(ThrustShip2* ts, double mass, const osg::Vec3d& RadiusOfConcentratedMass)
{
#ifdef TEST_THRUST_EVENTS
	m_ts = ts;
#endif

	Entity3D::EventMap* em = ts->GetEventMap();
	m_mass = mass;
	m_RadiusOfConcentratedMass = RadiusOfConcentratedMass;

	// Thrust alone
	Thrust_F=&(em->EventValue_Map["Thrust_F"]);
	Thrust_B=&(em->EventValue_Map["Thrust_B"]);
	Thrust_L=&(em->EventValue_Map["Thrust_L"]);
	Thrust_R=&(em->EventValue_Map["Thrust_R"]);
	Thrust_U=&(em->EventValue_Map["Thrust_U"]);
	Thrust_D=&(em->EventValue_Map["Thrust_D"]);

	// Forward, Heading Pitch
	Thrust_FR=&(em->EventValue_Map["Thrust_FR"]);
	Thrust_FL=&(em->EventValue_Map["Thrust_FL"]);
	Thrust_FU=&(em->EventValue_Map["Thrust_FU"]);
	Thrust_FD=&(em->EventValue_Map["Thrust_FD"]);
	Thrust_FRU=&(em->EventValue_Map["Thrust_FRU"]);
	Thrust_FRD=&(em->EventValue_Map["Thrust_FRD"]);
	Thrust_FLU=&(em->EventValue_Map["Thrust_FLU"]);
	Thrust_FLD=&(em->EventValue_Map["Thrust_FLD"]);

	// Backward, Heading Pitch
	Thrust_BR=&(em->EventValue_Map["Thrust_BR"]);
	Thrust_BL=&(em->EventValue_Map["Thrust_BL"]);
	Thrust_BU=&(em->EventValue_Map["Thrust_BU"]);
	Thrust_BD=&(em->EventValue_Map["Thrust_BD"]);
	Thrust_BRU=&(em->EventValue_Map["Thrust_BRU"]);
	Thrust_BRD=&(em->EventValue_Map["Thrust_BRD"]);
	Thrust_BLU=&(em->EventValue_Map["Thrust_BLU"]);
	Thrust_BLD=&(em->EventValue_Map["Thrust_BLD"]);

	// Left Thrust, Heading, Roll
	Thrust_LR0=&(em->EventValue_Map["Thrust_LR0"]);
	Thrust_LL0=&(em->EventValue_Map["Thrust_LL0"]);
	Thrust_L0L=&(em->EventValue_Map["Thrust_L0L"]);
	Thrust_L0R=&(em->EventValue_Map["Thrust_L0R"]);
	Thrust_LLL=&(em->EventValue_Map["Thrust_LLL"]);
	Thrust_LLR=&(em->EventValue_Map["Thrust_LLR"]);
	Thrust_LRL=&(em->EventValue_Map["Thrust_LRL"]);
	Thrust_LRR=&(em->EventValue_Map["Thrust_LRR"]);

	// Right Thrust, Heading, Roll
	Thrust_RR0=&(em->EventValue_Map["Thrust_RR0"]);
	Thrust_RL0=&(em->EventValue_Map["Thrust_RL0"]);
	Thrust_R0L=&(em->EventValue_Map["Thrust_R0L"]);
	Thrust_R0R=&(em->EventValue_Map["Thrust_R0R"]);
	Thrust_RLL=&(em->EventValue_Map["Thrust_RLL"]);
	Thrust_RLR=&(em->EventValue_Map["Thrust_RLR"]);
	Thrust_RRL=&(em->EventValue_Map["Thrust_RRL"]);
	Thrust_RRR=&(em->EventValue_Map["Thrust_RRR"]);

	// Up Thrust, Pitch, Roll
	Thrust_UD=&(em->EventValue_Map["Thrust_UD"]);
	Thrust_UU=&(em->EventValue_Map["Thrust_UU"]);
	Thrust_UR=&(em->EventValue_Map["Thrust_UR"]);
	Thrust_UL=&(em->EventValue_Map["Thrust_UL"]);
	Thrust_UUR=&(em->EventValue_Map["Thrust_UUR"]);
	Thrust_UUL=&(em->EventValue_Map["Thrust_UUL"]);
	Thrust_UDL=&(em->EventValue_Map["Thrust_UDL"]);
	Thrust_UDR=&(em->EventValue_Map["Thrust_UDR"]);

	// Down Thrust, Pitch, Roll
	Thrust_DD=&(em->EventValue_Map["Thrust_DD"]);
	Thrust_DU=&(em->EventValue_Map["Thrust_DU"]);
	Thrust_DR=&(em->EventValue_Map["Thrust_DR"]);
	Thrust_DL=&(em->EventValue_Map["Thrust_DL"]);
	Thrust_DUR=&(em->EventValue_Map["Thrust_DUR"]);
	Thrust_DUL=&(em->EventValue_Map["Thrust_DUL"]);
	Thrust_DDL=&(em->EventValue_Map["Thrust_DDL"]);
	Thrust_DDR=&(em->EventValue_Map["Thrust_DDR"]);
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::ThrustEventHandler::ResetFlightCharacteristics()
{
	m_flightCharacteristics.MaxAccelLeft = 0.0;
	m_flightCharacteristics.MaxAccelRight = 0.0;
	m_flightCharacteristics.MaxAccelUp = 0.0;
	m_flightCharacteristics.MaxAccelDown = 0.0;
	m_flightCharacteristics.MaxAccelForward = 0.0;
	m_flightCharacteristics.MaxAccelReverse = 0.0;
	m_flightCharacteristics.MaxTorquePitch = 0.0;
	m_flightCharacteristics.MaxTorqueRoll = 0.0;
	m_flightCharacteristics.MaxTorqueYaw = 0.0;
}
//////////////////////////////////////////////////////////////////////////

#ifndef max
#define max(a,b) (a) > (b) ? (a) : (b)
#endif
void ThrustShip2::ThrustEventHandler::MaximizeFlightCharacteristics(const FlightCharacteristics& maxMe)
{
	m_flightCharacteristics.MaxAccelLeft = max(m_flightCharacteristics.MaxAccelLeft, maxMe.MaxAccelLeft);
	m_flightCharacteristics.MaxAccelRight = max(m_flightCharacteristics.MaxAccelRight, maxMe.MaxAccelRight);
	m_flightCharacteristics.MaxAccelUp = max(m_flightCharacteristics.MaxAccelUp, maxMe.MaxAccelUp);
	m_flightCharacteristics.MaxAccelDown = max(m_flightCharacteristics.MaxAccelDown, maxMe.MaxAccelDown);
	m_flightCharacteristics.MaxAccelForward = max(m_flightCharacteristics.MaxAccelForward, maxMe.MaxAccelForward);
	m_flightCharacteristics.MaxAccelReverse = max(m_flightCharacteristics.MaxAccelReverse, maxMe.MaxAccelReverse);
	m_flightCharacteristics.MaxTorquePitch = max(m_flightCharacteristics.MaxTorquePitch, maxMe.MaxTorquePitch);
	m_flightCharacteristics.MaxTorqueRoll = max(m_flightCharacteristics.MaxTorqueRoll, maxMe.MaxTorqueRoll);
	m_flightCharacteristics.MaxTorqueYaw = max(m_flightCharacteristics.MaxTorqueYaw, maxMe.MaxTorqueYaw);
}
//////////////////////////////////////////////////////////////////////////

void ThrustShip2::ThrustEventHandler::ApplyThrustAndTorque(const osg::Vec3d& LocalForce, const osg::Vec3d& LocalTorque)
{
	//James TODO...
	//Unfortunately the HackTheMass() needs to be out so that we can use the m_flightCharacteristics for angular acceleration like we do for
	//force.  Until then the MaxAccelRot will patch it for now.

	// Change all limits and current values to acceleration
	osg::Vec3d MaxAccelRot;
	MaxAccelRot[1] = m_flightCharacteristics.MaxTorquePitch / m_mass;
	MaxAccelRot[2] = m_flightCharacteristics.MaxTorqueRoll / m_mass;
	MaxAccelRot[0] = m_flightCharacteristics.MaxTorqueYaw / m_mass;
	osg::Vec3d LocalAccel(LocalForce / m_mass);
	osg::Vec3d LocalRotAccel(LocalTorque / m_mass);

	//DOUT2("%f %f %f",LocalRotAccel[0],MaxAccelRot[0],LocalRotAccel[0]/MaxAccelRot[0]);
	osg::Vec3d MaxAccel_Pos(m_flightCharacteristics.MaxAccelLeft,m_flightCharacteristics.MaxAccelForward,m_flightCharacteristics.MaxAccelDown);
	osg::Vec3d MaxAccel_Neg(m_flightCharacteristics.MaxAccelRight,m_flightCharacteristics.MaxAccelReverse,m_flightCharacteristics.MaxAccelUp);

	// Find the actual values
	double Fo = (LocalAccel[1] > 0.0) ?( LocalAccel[1]/MaxAccel_Pos[1]) : 0.0;
	double Bk = (LocalAccel[1] < 0.0) ?(-LocalAccel[1]/MaxAccel_Neg[1]) : 0.0;
	double Le = (LocalAccel[0] > 0.0) ?( LocalAccel[0]/MaxAccel_Pos[0]) : 0.0;
	double Rt = (LocalAccel[0] < 0.0) ?(-LocalAccel[0]/MaxAccel_Neg[0]) : 0.0;
	double Dn = (LocalAccel[2] > 0.0) ?( LocalAccel[2]/MaxAccel_Pos[2]) : 0.0;
	double Up = (LocalAccel[2] < 0.0) ?(-LocalAccel[2]/MaxAccel_Neg[2]) : 0.0;

	double Hl = (LocalRotAccel[0] > 0.0) ?( LocalRotAccel[0]/MaxAccelRot[0]) : 0.0;
	double Hr = (LocalRotAccel[0] < 0.0) ?(-LocalRotAccel[0]/MaxAccelRot[0]) : 0.0;
	double Pd = (LocalRotAccel[1] > 0.0) ?( LocalRotAccel[1]/MaxAccelRot[1]) : 0.0;
	double Pu = (LocalRotAccel[1] < 0.0) ?(-LocalRotAccel[1]/MaxAccelRot[1]) : 0.0;
	double Rl = (LocalRotAccel[2] > 0.0) ?( LocalRotAccel[2]/MaxAccelRot[2]) : 0.0;
	double Rr = (LocalRotAccel[2] < 0.0) ?(-LocalRotAccel[2]/MaxAccelRot[2]) : 0.0;

#ifdef TEST_THRUST_EVENTS
	if (m_ts->IsLocallyControlled() && m_ts->GetGameClient()->Get_UI_ActorScene())
		DOUT3("%0.4i / %0.4i", (int)(Rl), (int)(MaxAccelRot[2]));
#endif

	// Fire all the appropriate events with the right values with limits
	// Thrust alone
	Thrust_F->Fire(Fo);
	Thrust_B->Fire(Bk);
	Thrust_L->Fire(Le);
	Thrust_R->Fire(Rt);
	Thrust_U->Fire(Up);
	Thrust_D->Fire(Dn);

	// Forward, Heading Pitch
	Thrust_FR->Fire((Fo+Hr) / 2.0);
	Thrust_FL->Fire((Fo+Hl) / 2.0);
	Thrust_FU->Fire((Fo+Pu) / 2.0);
	Thrust_FD->Fire((Fo+Pd) / 2.0);
	Thrust_FRU->Fire((Fo+Hr+Pu) / 3.0);
	Thrust_FRD->Fire((Fo+Hr+Pd) / 3.0);
	Thrust_FLU->Fire((Fo+Hl+Pu) / 3.0);
	Thrust_FLD->Fire((Fo+Hl+Pd) / 3.0);

	// Backward, Heading Pitch
	Thrust_BR->Fire((Bk+Hr) / 2.0);
	Thrust_BL->Fire((Bk+Hl) / 2.0);
	Thrust_BU->Fire((Bk+Pu) / 2.0);
	Thrust_BD->Fire((Bk+Pd) / 2.0);
	Thrust_BRU->Fire((Bk+Hr+Pu) / 3.0);
	Thrust_BRD->Fire((Bk+Hr+Pd) / 3.0);
	Thrust_BLU->Fire((Bk+Hl+Pu) / 3.0);
	Thrust_BLD->Fire((Bk+Hl+Pd) / 3.0);

	// Left Thrust, Heading, Roll
	Thrust_LR0->Fire((Le+Hr) / 2.0);
	Thrust_LL0->Fire((Le+Hl) / 2.0);
	Thrust_L0L->Fire((Le+Rl) / 2.0);
	Thrust_L0R->Fire((Le+Rr) / 2.0);
	Thrust_LLL->Fire((Le+Hl+Rl) / 3.0);
	Thrust_LLR->Fire((Le+Hl+Rr) / 3.0);
	Thrust_LRL->Fire((Le+Hr+Rl) / 3.0);
	Thrust_LRR->Fire((Le+Hr+Rr) / 3.0);

	// Right Thrust, Heading, Roll
	Thrust_RR0->Fire((Rt+Hr) / 2.0);
	Thrust_RL0->Fire((Rt+Hl) / 2.0);
	Thrust_R0L->Fire((Rt+Rl) / 2.0);
	Thrust_R0R->Fire((Rt+Rr) / 2.0);
	Thrust_RLL->Fire((Rt+Hl+Rl) / 3.0);
	Thrust_RLR->Fire((Rt+Hl+Rr) / 3.0);
	Thrust_RRL->Fire((Rt+Hr+Rl) / 3.0);
	Thrust_RRR->Fire((Rt+Hr+Rr) / 3.0);

	// Up Thrust, Pitch, Roll
	Thrust_UD->Fire((Up+Pd) / 2.0);
	Thrust_UU->Fire((Up+Pu) / 2.0);
	Thrust_UR->Fire((Up+Rr) / 2.0);
	Thrust_UL->Fire((Up+Rl) / 2.0);
	Thrust_UUR->Fire((Up+Pu+Rr) / 3.0);
	Thrust_UUL->Fire((Up+Pu+Rl) / 3.0);
	Thrust_UDL->Fire((Up+Pd+Rl) / 3.0);
	Thrust_UDR->Fire((Up+Pd+Rr) / 3.0);

	// Down Thrust, Pitch, Roll
	Thrust_DD->Fire((Dn+Pd) / 2.0);
	Thrust_DU->Fire((Dn+Pu) / 2.0);
	Thrust_DR->Fire((Dn+Rr) / 2.0);
	Thrust_DL->Fire((Dn+Rl) / 2.0);
	Thrust_DUR->Fire((Dn+Pu+Rr) / 3.0);
	Thrust_DUL->Fire((Dn+Pu+Rl) / 3.0);
	Thrust_DDL->Fire((Dn+Pd+Rl) / 3.0);
	Thrust_DDR->Fire((Dn+Pd+Rr) / 3.0);
}
//////////////////////////////////////////////////////////////////////////

/// Called as we are losing control of this entity
void ThrustShip2::SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner)
{
	// Update the target information if the same owner
	if (sameOwner)
	{
		Entity3D* target = m_controller->GetTarget();
		int tgtID = target ? target->GetNetworkID() : -1;
		sendControlBS.Write(tgtID);
	}
	__super::SendFinalUpdate(playerThatWantsControl, sendControlBS, sameOwner);
}
//////////////////////////////////////////////////////////////////////////

/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
void ThrustShip2::RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner)
{
	if (sameOwner)
	{
		int tgtID; recvControlBS.Read(tgtID);
		m_controller->SetTarget((tgtID > -1) ? dynamic_cast<DestroyableEntity*>(GetGameClient()->FindEntity(tgtID)) : NULL);
	}
	__super::RecvFinalUpdate(recvControlBS, playerThatWantsControl, msgTime_s, sameOwner);
}
//////////////////////////////////////////////////////////////////////////