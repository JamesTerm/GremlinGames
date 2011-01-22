// GuardianVR.Base Mech.cpp
#include "stdafx.h"
#include "GuardianVR.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GuardianVR::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;
using namespace GG_Framework::UI;

ClassFactoryT<Mech, Entity3D> Mech_ClassFactory("Mech", Entity3D::ClassFactory);

void Mech::Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te)
{
	DEBUG_CANNON_FIRE("Mech::Initialize(...)\n");
	__super::Initialize(gs, em, te);

	// Get the Transmitted Entity that has Cannons
	Mech_TransmittedEntity* mte = dynamic_cast<Mech_TransmittedEntity*>(&te);
	ASSERT(mte)
	{
		std::vector<CannonDesc>::iterator pos;
		unsigned index = 0;
		for (pos = mte->Cannons.begin(); pos != mte->Cannons.end(); ++pos)
		{
			CannonDesc& cd = *pos;
			double range = cd.Round_Speed * cd.Round_Life;
			if (range > m_cannonRange)
				m_cannonRange = range;
			if (cd.Round_Speed > m_cannonSpeed)
				m_cannonSpeed = cd.Round_Speed;
			m_Cannons.push_back(new Cannon(++index, *this, cd));
		}
	}

	// The heading limit
	dHeading = DEG_2_RAD(mte->dHeading);
	dPitch = DEG_2_RAD(mte->dPitch);

	// Our intended orientation starts the same as the original attitude
	m_IntendedOrientation = GetAtt_quat();
	m_intendedOrientation_Vec[0] = te.Heading;
	m_intendedOrientation_Vec[1] = te.Pitch;

	// Listen for the main firing weapons
	em.EventOnOff_Map["Mech.TryFireMainWeapon"].Subscribe(ehl, *this, &Mech::TryFireMainWeapon);

	// Listen for SPAWN being turned on and off
	em.EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &Mech::OnSpawn);

	m_controller = new Mech_Controller(*this, true);
	m_controller->ReadScriptParameters(*mte);
}
//////////////////////////////////////////////////////////////////////////

Mech::~Mech()
{
	std::vector<Cannon*>::iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
		delete (*pos);
	m_Cannons.clear();
}
//////////////////////////////////////////////////////////////////////////

void Mech::GetWeaponStatus(Cannon::TempStatus& tempStatus, double& tempLevel, double& restartLevel) const
{
	tempLevel = 0.0;
	restartLevel = 0.0;
	tempStatus = Cannon::CANNON_Cold;

	std::vector<Cannon*>::const_iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
	{
		double t, rs;
		Cannon::TempStatus ts;
		(*pos)->GetCannonTemp(ts, t, rs);
		if (t > tempLevel)
			tempLevel = t;
		if (rs > restartLevel)
			restartLevel = rs;
		if ((int)ts < (int)tempStatus)
			tempStatus = ts;
	}
}
//////////////////////////////////////////////////////////////////////////

void Mech::UpdateIntendedOrientaton(double dTime_s)
{
	// Apply the offset based on the current velocity, used for pointing the camera
	// Start by JUST adding up the intended orientation vectors, which keep heading and pitch seperate
	m_intendedOrientation_Vec += m_rotVel_rad_s*dTime_s;

	// Watch for pitch being too high or too low, avoid any pitch above 89 degrees
	static const double pitch89deg_rad = DEG_2_RAD(89.0);
	if (m_intendedOrientation_Vec[1] > pitch89deg_rad)
		m_intendedOrientation_Vec[1] = pitch89deg_rad;
	else if (m_intendedOrientation_Vec[1] < -pitch89deg_rad)
		m_intendedOrientation_Vec[1] = -pitch89deg_rad;

	// Then do the quaternion.  Notice we are NOT using FromLW_Rot_Radians, because we want the heading and pitch applied
	// in a different order
	m_IntendedOrientation=
		osg::Quat(
		-m_intendedOrientation_Vec[1]	, osg::Vec3d(1,0,0),
		 m_intendedOrientation_Vec[0]	, osg::Vec3d(0,0,1),
		-m_intendedOrientation_Vec[2]	, osg::Vec3d(0,1,0));
}
//////////////////////////////////////////////////////////////////////////

void Mech::ApplyLocalTorque(Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &Torque,double dTime_s)
{  
	// We are not needing to restrain the torque at all
	PhysicsToUse.ApplyFractionalTorque(Torque,dTime_s,PhysicsToUse.GetRadiusOfConcentratedMass());
}

void Mech::ApplyLocalForce(Physics::PhysicsEntity &PhysicsToUse,const osg::Vec3d &LocalForce,double dTime_s)
{
	// Translate the local force into a global one by post-multiplying with our current attitude
	PhysicsToUse.ApplyFractionalForce(GetAtt_quat() * LocalForce,dTime_s);
}

// Here are the different motion events, they need to be mutually exclusive
// It is a good idea to use these actual strings to simplify the comparisons
// Each of these will be tied to separate events that are all on/off, but only one of them will be on at any one time
const char* MECH_Standing = "Standing";
const char* MECH_TurnL = "TurningL";
const char* MECH_TurnR = "TurningR";

void Mech::OnSpawn(bool on)
{
	// When spawning is turned off, turn off any associated motions
	if (m_currMotionState)
	{
		GetEventMap()->EventOnOff_Map[m_currMotionState].Fire(false);
		m_currMotionState = NULL;
	}
}

void Mech::TimeChange(double dTime_s)
{
	// Let the controller do its thing, which will call our Turn functions, etc.
	m_controller->UpdateController(dTime_s);

	osg::Vec3d ForceToApply, TorqueToApply;

	// Be sure to set this motion state to something besides standing.
	// Not sure how this happens while being destroyed :(
	const char* nextMotionState = MECH_Standing;

	// Handle all of the turning and other motion, be sure to 
	{
		// Update the Intended Orientation based on mouse, joystick, and keyboard input
		UpdateIntendedOrientaton(dTime_s);

		// Determine the angular distance from the intended orientation, so we know how far we need to go
		osg::Quat intendedHeading=FromLW_Rot_Radians(m_intendedOrientation_Vec[0],0.0,0.0);
		osg::Vec3d rotDisplacement_rad=-m_Physics.ComputeAngularDistance(intendedHeading);

		// We would LOVE to be able to get there in only one frame.  What does the angular velocity need to be?
		osg::Vec3d desiredRotVel_radpersec = rotDisplacement_rad / dTime_s;

		// Make sure we are never trying to go faster than our scripted limit
		if (desiredRotVel_radpersec[0] < -dHeading)
			desiredRotVel_radpersec[0] = -dHeading;
		else if (desiredRotVel_radpersec[0] > dHeading)
			desiredRotVel_radpersec[0] = dHeading;

		// Account for how fast we are ALREADY rotating
		desiredRotVel_radpersec -= m_Physics.GetAngularVelocity();

		// Show turning one way or the other here too slow a turn is one rev in one minute
		static const double SlowTurn = 2.0*M_PI / 60.0;
		if (desiredRotVel_radpersec[0] > SlowTurn)
			nextMotionState = MECH_TurnL;
		else if (desiredRotVel_radpersec[0] < -SlowTurn)
			nextMotionState = MECH_TurnR;

		// Figure out the amount of torque required, we will not use any restraints, only that applied by dHeading above.
		TorqueToApply=desiredRotVel_radpersec*this->GetMass()/dTime_s;
	}

	// Apply the forces and torque locally
	ApplyLocalForce(m_Physics,ForceToApply,dTime_s);
	ApplyLocalTorque(m_Physics,TorqueToApply,dTime_s);

	// Fire the events for our current motion
	if (m_currMotionState != nextMotionState)
	{
		if (m_currMotionState)
			GetEventMap()->EventOnOff_Map[m_currMotionState].Fire(false);
		m_currMotionState=nextMotionState;
		if (m_currMotionState)
			GetEventMap()->EventOnOff_Map[m_currMotionState].Fire(true);
	}

	//Now to run the time updates (displacement plus application of it)
	__super::TimeChange(dTime_s);

	// The UI now needs to know about the changes
	m_controller->UpdateUI(dTime_s);

	//Reset my controller vars
	m_rotVel_rad_s=osg::Vec3d(0,0,0);
}

//////////////////////////////////////////////////////////////////////////

void Mech::CancelAllControls()
{
	__super::CancelAllControls();
	TryFireMainWeapon(false);
}
//////////////////////////////////////////////////////////////////////////

void Mech::OnAsyncLoadComplete()
{
	__super::OnAsyncLoadComplete();

	DEBUG_CANNON_FIRE("Mech::OnAsyncLoadComplete()\n");
	// Let each cannon do its complete
	std::vector<Cannon*>::iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
		(*pos)->OnAsyncLoadComplete();
}
//////////////////////////////////////////////////////////////////////////

void Mech::TryFireMainWeapon(bool on)
{
	std::vector<Cannon*>::iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
		(*pos)->OnFireCannons(on);
}
//////////////////////////////////////////////////////////////////////////

void Mech::SetPlayerControlled(bool isControlled)
{
	if (IsLocallyControlled())
		m_controller->HookUpUI(isControlled);
	__super::SetPlayerControlled(isControlled);
}
//////////////////////////////////////////////////////////////////////////

void Mech_ChaseVehicle_Imp::Reset(GG_Framework::Logic::Entity3D& entity)
{
	__super::Reset(entity);
	if (!m_bbActor)
	{
		m_bbActor = Self_Entity->Get_BB_Actor();
		ASSERT(m_bbActor);
		m_camPosOffset = GG_Framework::UI::OSG::FindChildNode(m_bbActor.get(),	"COCKPIT_NOMOFFSET");
	}
}
//////////////////////////////////////////////////////////////////////////

void Mech_ChaseVehicle_Imp::GetLookAtVectors(bool usingPOV, const osg::Quat &intendedOrientation, double dTime_s, 
					  osg::Vec3d& fixedEye, osg::Vec3d& lookDir, osg::Vec3d& upDir)
{
	// Make sure we found the node
	if (m_camPosOffset.valid() && !usingPOV)
	{
		// Get the relative camera position and the other vectors are based on the Matrix offset between the node and the parent BB
		osg::Matrix M = GG_Framework::UI::OSG::GetNodeMatrix(m_camPosOffset.get(), NULL, m_bbActor.get());
		osg::Vec3d relCamPos = M.getTrans();
		osg::Vec3d cockpitCtr = GetCockpitCtr();

		// The vectors we will return.  fixedEye is based on relCamPos, but rotated about cockpitCtr
		fixedEye = Self_Entity->GetPos_m() + cockpitCtr + (intendedOrientation * (relCamPos-cockpitCtr));
		lookDir = (osg::Vec3d(0.0,1.0,0.0)*M) - relCamPos;
		upDir = (osg::Vec3d(0.0,0.0,1.0)*M) - relCamPos;
	}
	else	// Otherwise JUST use the base class
		GG_Framework::AppReuse::ChaseVehicle_Imp::GetLookAtVectors(usingPOV, intendedOrientation, dTime_s, fixedEye, lookDir, upDir);
}
//////////////////////////////////////////////////////////////////////////





