#include "stdafx.h"
#include "Fringe.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace Fringe::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace GG_Framework::AppReuse;

// #define DISABLE_AI_CANNONS
#undef AI_IGNORE_G

// AI_DUMMY_FACTOR 0.3, make it lower (closer to 0) to make them better, make it higher to make them worse
#define AI_DUMMY_FACTOR 0.5

AI_Base_Controller::AI_Base_Controller(ThrustShip2& ship) : 
	m_ship(ship), m_currWayPointIndex(0), m_firingCannons(false), m_numKills(0), 
	m_numHits(0), m_UI_Controller(NULL), m_AI_recoverGs(false), m_AI_Reaction(*this), m_jinkFromCollision(NULL)
{
	// Start with an empty bank of targets
	for (unsigned i = 0; i < 10; ++i)
		m_targetBank[i] = NULL;
}

void AI_Base_Controller::InitializeEventMaps(bool canUserPilot)
{
	m_canUserPilot=canUserPilot;
	GG_Framework::Logic::Entity3D::EventMap* em = m_ship.GetEventMap();

	// Know when we are feeling G forces
	//em->EventValue_Map["FeelingG"].Subscribe(ehl, *this, &AI_Base_Controller::FeelingG);

	em->Event_Map["TARGET_PREV"].Subscribe(ehl, *this, &AI_Base_Controller::TargetPrev);
	em->Event_Map["TARGET_NEXT"].Subscribe(ehl, *this, &AI_Base_Controller::TargetNext);
	em->Event_Map["TARGET_TGT_YOU"].Subscribe(ehl, *this, &AI_Base_Controller::TargetTgtYou);

	// Add some event handlers for destruction and cannon hits
	em->EventPt_Map["DESTROY"].Subscribe(ehl, *this, &AI_Base_Controller::ShipDestroyed);
	m_ship.CannonRoundHit.Subscribe(ehl, *this, &AI_Base_Controller::OnCannonRoundHit);

	em->Event_Map["TARGET_INSIGHT"].Subscribe(ehl, *this, &AI_Base_Controller::TargetInSight);

	em->Event_Map["TARGET_1"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank1);
	em->Event_Map["TARGET_2"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank2);
	em->Event_Map["TARGET_3"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank3);
	em->Event_Map["TARGET_4"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank4);
	em->Event_Map["TARGET_5"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank5);
	em->Event_Map["TARGET_6"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank6);
	em->Event_Map["TARGET_7"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank7);
	em->Event_Map["TARGET_8"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank8);
	em->Event_Map["TARGET_9"].Subscribe(ehl, *this, &AI_Base_Controller::UseTargetBank9);

	em->Event_Map["SET_TARGET_1"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank1);
	em->Event_Map["SET_TARGET_2"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank2);
	em->Event_Map["SET_TARGET_3"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank3);
	em->Event_Map["SET_TARGET_4"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank4);
	em->Event_Map["SET_TARGET_5"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank5);
	em->Event_Map["SET_TARGET_6"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank6);
	em->Event_Map["SET_TARGET_7"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank7);
	em->Event_Map["SET_TARGET_8"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank8);
	em->Event_Map["SET_TARGET_9"].Subscribe(ehl, *this, &AI_Base_Controller::SetTargetBank9);

	em->Event_Map["MATCH_TARGET_SPEED"].Subscribe(ehl, *this, &AI_Base_Controller::MatchTargetSpeed);

	em->EventValue_Map["SET_TARGET"].Subscribe(ehl, *this, &AI_Base_Controller::NetworkSetTarget);

	//Set up the ship to autopilot by default
	{
		// We always want auto-level on when in auto-pilot
		m_ship.SetEnableAutoLevel(true);
		// For auto-pilot mode, we MUST set simulated flight mode if we have an auto-pilot route
		bool hasAutoPilotRoute = HasAutoPilotRoute();
		m_ship.SetStabilizeRotation(hasAutoPilotRoute);
		m_ship.SetSimFlightMode(hasAutoPilotRoute);
	}

	// Listen for the collision event
	em->Collision.Subscribe(ehl, *this, &AI_Base_Controller::OnCollision);
}

void AI_Base_Controller::OnCollision(Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s)
{
	// Jink away from the direction the bullet came from, only if we are fighters or something small
	if (m_ship.GetCollisionIndex() < Freighter)
	{
		osg::Vec3d bulletVector(otherEntity.GetPhysics().GetLinearVelocity() * -1.0);
		if (m_jinkFromCollision)
			*m_jinkFromCollision = bulletVector;
		m_jinkFromCollision = new osg::Vec3d(bulletVector);
	}
}

void AI_Base_Controller::ReadScriptParameters(ThrustShip2_TransmittedEntity& te)
{
	// Just grab the way points from the script
	WayPoints = te.WayPoints;

	//We should only call these once here
	// If we are this far, make sure that we are using non-assisted mode
	if (WayPoints.size())
	{
		m_ship.SetSimFlightMode(true);
		m_ship.SetEnableAutoLevel(true);
	}

	m_currWayPointIndex = 0;
	ShipToFollow = te.ShipToFollow;
	TargetShip.ShipID = te.TARGET_SHIP;

	//TODO these may be moved to the ships sensors area
	LEAD_RET_OSGV = te.LEAD_RET_OSGV;
	INRANGE_LEAD_RET_OSGV = te.INRANGE_LEAD_RET_OSGV;
	FWD_RET_OSGV = te.FWD_RET_OSGV;
}

bool AI_Base_Controller::Try_SetUIController(UI_Controller *controller)
{
	//So far we have had access to all.  We may want to derive some logic here or establish it at this level (to be determined). 
	m_UI_Controller=controller;
	return true;
}

void AI_Base_Controller::CancelAllControls()  
{
	if (m_UI_Controller) 
		m_UI_Controller->CancelAllControls();
}

void AI_Base_Controller::MatchTargetSpeed()
{
	GG_Framework::AppReuse::DestroyableEntity* target = GetTarget();
	if (target && m_UI_Controller)
		m_UI_Controller->MatchSpeed(target->GetLinearVelocity().length());
}

void AI_Base_Controller::UpdateTargetLeadPoint()
{
	GG_Framework::AppReuse::DestroyableEntity* target = GetTarget();
	double weaponSpeed = m_ship.GetWeaponsSpeed();
	if (target && (weaponSpeed > 0.0))
	{
		osg::Vec3d V_target(target->GetPhysics().GetLinearVelocity());
		osg::Vec3d V_this(m_ship.GetPhysics().GetLinearVelocity());
		osg::Vec3d V_rel(V_target-V_this);	// Relative velocity, as if we were sitting still
		osg::Vec3d P_this(m_ship.GetPos_m());
		osg::Vec3d P_target(target->GetPos_m());

		// We will work through 3 iterations of finding the time to target with our weapon and the distance
		// We might want to work out a way to optimize this
		double Dist_0 = (P_target-P_this).length();
		double t_0 = Dist_0 / weaponSpeed;
		osg::Vec3d Lead_1 = P_target + V_rel * t_0;

		double Dist_1 = (Lead_1-P_this).length();
		double t_1 = Dist_1 / weaponSpeed;
		osg::Vec3d Lead_2 = P_target + V_rel * t_1;

		double Dist_2 = (Lead_2-P_this).length();
		double t_2 = Dist_2 / weaponSpeed;
		m_targetLeadPt = P_target + V_rel * t_2;

		m_ship.SetGunTarget(&m_targetLeadPt);
	}
	else
	{
		// Just set the lead point ahead of our position
		m_targetLeadPt = m_ship.GetPos_m() + m_ship.GetAtt_quat() * osg::Vec3d(0,10000,0);
		m_ship.SetGunTarget(NULL);
	}

}

void AI_Base_Controller::UpdateUI(double dTime_s)
{
	UpdateTargetLeadPoint();
	if (m_UI_Controller)
		m_UI_Controller->UpdateUI(dTime_s);
}

//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::GetCanUserPilot()
{
	return (m_canUserPilot && !m_ship.IsBeingDestroyed());
}


void AI_Base_Controller::UpdateTargetAndFollow()
{
	// Grab the ship the first time
	if (!TargetShip.Ship && !TargetShip.ShipID.empty())
		SetTarget(dynamic_cast<DestroyableEntity*>(m_ship.GetGameClient()->FindEntity(TargetShip.ShipID)));
	if(!ShipToFollow.Ship && !ShipToFollow.ShipID.empty())
		ShipToFollow.Ship = dynamic_cast<DestroyableEntity*>(m_ship.GetGameClient()->FindEntity(ShipToFollow.ShipID));
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::UpdateController_Base(double dTime_s)
{
	// Make sure the targets are updated all the time
	UpdateTargetAndFollow();

	// If the ship is being destroyed, or it is not showing, or not handling the G's well,
	// it should not still try to do any more kinds of AI reactions.
	if (m_ship.IsBeingDestroyed() || !m_ship.IsShowing() || !HandleGs(dTime_s))
	{
		ResetAI_Reaction();
		return;
	}

	// Watch for needing to jink
	if (m_jinkFromCollision)
	{
		// Find the vector related to my current velocity and where the collision came from
		osg::Vec3d currVel = m_ship.GetPhysics().GetLinearVelocity();
		osg::Vec3d normalDir = (currVel ^ *m_jinkFromCollision);
		osg::Vec3d globalPos = m_ship.GetPos_m();
		UpdateIntendedLocation(globalPos+normalDir,globalPos+currVel, 1.0, NULL);
		delete (m_jinkFromCollision); m_jinkFromCollision = NULL;
	}

	// Is the AI Reaction ready for a new update?
	if (m_AI_Reaction.ReadyForNextUpdate())
		UpdateAI_Reaction(dTime_s);

	// The AI_Reaction then handles the rest
	m_AI_Reaction.FlyToNextLocation(dTime_s);
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::ResetAI_Reaction()
{
	if (m_jinkFromCollision)
		delete (m_jinkFromCollision); m_jinkFromCollision = NULL;
	m_AI_Reaction.Reset();
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::HandleGs(double dTime_s)
{
#ifndef AI_IGNORE_G

	double G_PilotLimit = m_ship.GetPhysics().GetPilotInfo().GLimit;
	double G_SLIDE_LIMIT = G_PilotLimit * 1.1;
	double G_START = G_PilotLimit * 0.3;
	double G_END = G_PilotLimit * 0.8;

	// It should also not do it if pulling too many Gs
	double l_Gs=m_ship.GetPhysics().GetPilotInfo().Get_Gs();
	if (m_AI_recoverGs)
	{
		// No longer recovering
		if ((l_Gs < G_START) && (l_Gs > -G_START))
			m_AI_recoverGs = false;
	}
	else if ((l_Gs > G_END) || (l_Gs < -G_END))
	{
		// We need to recover a bit
		m_AI_recoverGs = true;
	}

	if ((l_Gs > G_SLIDE_LIMIT) || (l_Gs < -G_SLIDE_LIMIT)) // Straighten out if pulling LOTS of G's
		m_ship.SetSimFlightMode(false);	// Puts us in slide mode

	if (m_AI_recoverGs)
	{
		// Always stop firing guns if pulling some G's
		AI_TryFireCannons(NULL);			
		return false;
	}

	// Otherwise we can start again
	m_ship.SetSimFlightMode(true);
#endif
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::FlyWayPoints(double dTime_s)
{
	unsigned numWayPoints = WayPoints.size();
	if (!numWayPoints) return false;

	// Base a tolerance2 for how close we want to get to the way point based on the current velocity,
	// within a second of reaching the way point, just move to the next one
	double tolerance2 = (m_ship.m_Physics.GetLinearVelocity().length2() * 1.0) + 0.1; // (will keep it within one meter even if not moving)

	// Find the NEXT waypoint to fly to if we are too close to fly to this one
	// Watch for all waypoints in the list being too close
	osg::Vec3d currPos = m_ship.GetPos_m();
	unsigned currIndex = m_currWayPointIndex;
	while ((WayPoints[m_currWayPointIndex].Position-currPos).length2() < tolerance2)
	{
		//This can be set this late, because no turns will happen until the next point is reached
		m_ship.SetHeadingSpeedScale(WayPoints[m_currWayPointIndex].TurnSpeedScaler);
		++m_currWayPointIndex;
		if (m_currWayPointIndex == numWayPoints)
			m_currWayPointIndex = 0;

		// If we are back to what we had, we know all way points are too close, just stop
		if (m_currWayPointIndex == currIndex)
		{
			SetShipSpeed(0.0);
			return true;
		}
	}

	// TODO:  We COULD Do things here like make a spline to the next two way-points, 
	// adjust the velocity based on James' functions and the ship's capabilities, to the NEXT way point
	// avoid collisions with other things, etc.

	// Now we can go to that way point without it being too close
	WayPoint wp = WayPoints[m_currWayPointIndex];
	UpdateIntendedLocation(wp.Position,wp.Position, wp.Power, NULL);
	return true;
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::UpdateAI_Reaction(double dTime_s)
{
	// Either we are targeting ...
	if (TargetShip.Ship)
	{
		bool targetOK = TrackTarget(TargetShip);
		AI_TryFireCannons(targetOK ? TargetShip.Ship : NULL);
		if (targetOK)
			return;
	}

	// .. Or Flying Way Points ...
	if (FlyWayPoints(dTime_s))
		return;

	// ... Or Following another ship
	if (ShipToFollow.Ship)
		TrackFollowShip(ShipToFollow);
}
//////////////////////////////////////////////////////////////////////////

AI_Base_Controller::AI_Reaction::AI_Reaction(AI_Base_Controller& controller) : m_controller(controller), m_shouldMatchVel(false)
{
	m_readyToFly = false;
	m_timeSinceLastUpdate = m_timeTillUpdateOK = 0.0;
	m_oversteer = 1.0;
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::AI_Reaction::UpdateIntendedLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, osg::Vec3d* matchVel)
{
	// All interpolated positions are relative to where the ship is to start
	m_shipStartingPos = m_controller.m_ship.GetPos_m();

	// PositionPoints will move from where the Trajectory tells us to where we want to eventually be
	m_startingPositionPoint = TrajectoryPoint;
	m_endingPositionPoint = PositionPoint;

	// Find the distances and vectors we want for trajectories
	osg::Vec3d startTraj(m_globalTrajectoryPoint - m_shipStartingPos);
	m_startingDist = startTraj.normalize();
	if (m_startingDist < 1)
	{
		if (m_controller.m_ship.GetGameClient()->Get_UI_ActorScene())
			printf("SMALL m_startingDist\n");
		m_startingDist = 1.0;
		startTraj = m_controller.m_ship.GetAtt_quat() * osg::Vec3d(0,1,0);
	}
	m_startingQuat.makeRotate(osg::Vec3d(0,1,0), startTraj);

	osg::Vec3d endTraj(TrajectoryPoint - m_shipStartingPos);
	m_endingDist = endTraj.normalize();
	if (m_endingDist < 1)
	{
		if (m_controller.m_ship.GetGameClient()->Get_UI_ActorScene())
			printf("SMALL m_endingDist\n");
		m_endingDist = 1.0;
		endTraj = m_controller.m_ship.GetAtt_quat() * osg::Vec3d(0,1,0);
	}
	m_endingQuat.makeRotate(osg::Vec3d(0,1,0), endTraj);

	// Find the time till next update based on how far away the angle is from straight forward for the ship
	double timeTill90 = 1.2;
	double minTime = 0.25;
	m_controller.GetTurningReactionTimes(minTime, timeTill90);
	double cosAngle = (m_controller.m_ship.GetAtt_quat() * osg::Vec3d(0,1,0)) * endTraj;
	double OneMinusCosSq = (1.0 - cosAngle) * (1.0 - cosAngle);
	m_timeTillUpdateOK = (OneMinusCosSq * timeTill90) + minTime;

	// Determine the amount of over steer possible as well based on the cosAngle;
	m_oversteer = 1.0 + ((RAND_GEN()-0.5) * OneMinusCosSq * m_controller.GetRandomOversteerFactor());


	// None of this stuff needs to be interpolated
	m_power = power;
	if (matchVel)
	{
		m_matchVel = *matchVel;
		m_shouldMatchVel = true;
	}
	else
		m_shouldMatchVel = false;

	// We have gotten at least one update, so we are able to fly
	m_readyToFly = true;

	// Restart the clock to get the calculation
	m_timeSinceLastUpdate = 0.0;
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::AI_Reaction::ReadyForNextUpdate()
{
	// Has there been enough time?
	if (m_timeSinceLastUpdate >= m_timeTillUpdateOK)
	{
		m_readyToFly = false; 
		return true;
	}
	else return false;
}
void AI_Base_Controller::AI_Reaction::Reset()
{
	// Reset our focus to straight ahead (1km or so)
	m_globalTrajectoryPoint = m_globalTrajectoryPoint = 
		m_controller.m_ship.GetPos_m() + (m_controller.m_ship.GetAtt_quat() * osg::Vec3d(0,1000,0));
	m_timeSinceLastUpdate = m_timeTillUpdateOK = 0.0;
	m_readyToFly = false;
	m_oversteer = 1.0;
}

void AI_Base_Controller::AI_Reaction::FlyToNextLocation(double dTime_s)
{
	if (m_readyToFly)
	{
		// Find out how fat we are interpolating
		m_timeSinceLastUpdate += dTime_s;
		double T = (m_timeTillUpdateOK>0.0) ? (m_timeSinceLastUpdate / m_timeTillUpdateOK) : 1.0;
		if (T > 1.0) T = 1.0;
		T *= m_oversteer; // We want to oversteer a bit, depending on lots of factors

		double OneMinusT = 1.0 - T;

		// Interpolate the global position point
		m_globalTrajectoryPoint = (m_startingPositionPoint*OneMinusT) + (m_endingPositionPoint*T);

		double dist = (m_startingDist*OneMinusT) + (m_endingDist*T);
		osg::Quat Q; Q.slerp(T, m_startingQuat, m_endingQuat);
		m_globalTrajectoryPoint = m_shipStartingPos + (Q * osg::Vec3d(0,dist,0));

		m_controller.DriveToLocation(
			m_globalTrajectoryPoint, m_globalTrajectoryPoint, m_power, dTime_s, 
			m_shouldMatchVel ? &m_matchVel : NULL);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

double AI_Base_Controller::GetRandomOversteerFactor()
{
	// How are the pilots handling the G's?
	double Gs = m_ship.GetPhysics().GetPilotInfo().Get_Gs();
	if (Gs < 0.0) Gs *= -1.0;

	return (1.0 + (Gs / (m_ship.GetPhysics().GetPilotInfo().GLimit*0.8))) * AI_DUMMY_FACTOR;
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::GetTurningReactionTimes(double& minTime_s, double& timeTill90_s)
{
	double Gs = m_ship.GetPhysics().GetPilotInfo().Get_Gs();
	if (Gs < 0.0) Gs *= -1.0;
	double Gmod = Gs / (m_ship.GetPhysics().GetPilotInfo().GLimit*0.8);
	timeTill90_s = (1.0 + 0.1*(Gs*Gs)) * AI_DUMMY_FACTOR;
	minTime_s = (0.1 * (Gs*Gs)) * AI_DUMMY_FACTOR;
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::UpdateController(double dTime_s)
{
	// Is the player controlling this ship, or is the AI?
	if (m_UI_Controller)
	{
		// The player is controlling with a UI, tell my AI_Reaction to reset so it starts over next time
		m_UI_Controller->UpdateController(dTime_s);
	}
	else
		UpdateController_Base(dTime_s);
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::TrackTarget(FollowShip& targetFollow)
{
	ASSERT_MSG(targetFollow.Ship, GG_Framework::Base::BuildString("Could not find the Ship to follow: %s\n)", targetFollow.ShipID.c_str()).c_str());
	if (targetFollow.Ship->IsShowing() && !targetFollow.Ship->IsBeingDestroyed())
	{
		osg::Vec3d globalGoalPositionPoint=targetFollow.Ship->GetPos_m();
		osg::Vec3d globalGoalTrajectoryPoint = globalGoalPositionPoint;
		osg::Vec3d globalGoalVel = globalGoalVel = targetFollow.Ship->GetPhysics().GetLinearVelocity();

		m_ship.SetHeadingSpeedScale(1.0);
		//Init the trajectory offset (late since we depend on the bounding sphere)
		osg::Vec3d myShipSize = m_ship.GetPhysics().GetRadiusOfConcentratedMass();
		osg::Vec3d targetShipSize = targetFollow.Ship->GetPhysics().GetRadiusOfConcentratedMass();

		double minFollowDist = myShipSize[1] + targetShipSize[1] + (m_ship.GetWeaponsRange() * 0.2);

		// Make the goal trajectory point base on the lead position
		globalGoalTrajectoryPoint = GetTargetLeadPoint();

		// If we are TOO close, just drive forward to get some distance
		if ((m_ship.GetPos_m()-targetFollow.Ship->GetPos_m()).length2() < (minFollowDist*minFollowDist))
		{
			globalGoalPositionPoint+= (m_ship.GetPos_m()-targetFollow.Ship->GetPos_m()) * 10.0;
			UpdateIntendedLocation(globalGoalTrajectoryPoint,globalGoalPositionPoint, 1.0, NULL);
		}
		else
		{
			// Follow on the 6 o'clock position, offset for close to where I am already
			double followDist = myShipSize[1] + targetShipSize[1] + (m_ship.GetWeaponsRange() * 0.8);
			targetFollow.RelPosition = osg::Vec3d(0.0, -followDist, 0.0);
			globalGoalPositionPoint+=((targetFollow.Ship->GetAtt_quat()*targetFollow.RelPosition) +
				(m_ship.GetAtt_quat()*targetFollow.RelPosition))*0.5;

			UpdateIntendedLocation(globalGoalTrajectoryPoint,globalGoalPositionPoint, 1.0, &globalGoalVel);
		}
		return true;
	}
	else
	{
		// Just drive straight so we can deal with avoidance
		osg::Vec3d globalGoalPositionPoint = m_ship.GetPos_m() + m_ship.GetVelocity_m_s();
		UpdateIntendedLocation(globalGoalPositionPoint,globalGoalPositionPoint, 1.0, NULL);
		return false;
	}
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::TrackFollowShip(FollowShip& followMe)
{
	ASSERT_MSG(followMe.Ship, GG_Framework::Base::BuildString("Could not find the Ship to follow: %s\n)", followMe.ShipID.c_str()).c_str());
	if (followMe.Ship->IsShowing() && !followMe.Ship->IsBeingDestroyed())
	{
		// This is the "correct" offset position
		osg::Vec3d globalGoalPositionPoint;
		osg::Vec3d globalGoalTrajectoryPoint;
		osg::Vec3d globalGoalVelocity;
		globalGoalPositionPoint=globalGoalTrajectoryPoint=followMe.Ship->GetPos_m();
		globalGoalVelocity = followMe.Ship->GetPhysics().GetLinearVelocity();

		//Init the trajectory offset (late since we depend on the bounding sphere)
		if (!followMe.IsTrajectoryPositionComputed)
		{
			followMe.TrajectoryPosition=followMe.RelPosition;
			followMe.TrajectoryPosition[1] += 100.0;	// Just point forward
			followMe.IsTrajectoryPositionComputed=true;
		}

		globalGoalPositionPoint+=followMe.Ship->GetAtt_quat()*followMe.RelPosition;
		globalGoalTrajectoryPoint  +=followMe.Ship->GetAtt_quat()*followMe.TrajectoryPosition;

		// The globalGoalGlobalOrient position is either in front so we can turn to it
		// AND/OR it is far enough away where we SHOULD make a heavy turn into it.
		UpdateIntendedLocation(globalGoalTrajectoryPoint,globalGoalPositionPoint, 1.0, &globalGoalVelocity);
		return true;
	}
	else
	{
		// Just drive straight so we can deal with avoidance
		osg::Vec3d globalGoalPositionPoint = m_ship.GetPos_m() + m_ship.GetVelocity_m_s();
		UpdateIntendedLocation(globalGoalPositionPoint,globalGoalPositionPoint, 1.0, NULL);
		return false;
	}
	
}
//////////////////////////////////////////////////////////////////////////

DestroyableEntity* AI_Base_Controller::GetTarget() 
{
	if (TargetShip.Ship && TargetShip.Ship->IsShowing())
		return TargetShip.Ship;
	else return NULL;
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::HasAutoPilotRoute()
{
	return (WayPoints.size() > 0) || (ShipToFollow.Ship || !ShipToFollow.ShipID.empty()) || (TargetShip.Ship || !TargetShip.ShipID.empty());
}


void AI_Base_Controller::UpdateIntendedLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, osg::Vec3d* matchVel)
{
	// Watch for collision avoidance issues (we should have a check here if there are ships or controllers too dumb to try)
	if (TryAvoidCollision(PositionPoint))
	{
		// We might collide, so set the trajectory in the same direction to help avoid the collision
		TrajectoryPoint = PositionPoint;
	}

	// TODO: We may want to do any evasive maneuvering here

	// Store the value in the AI_Reaction
	m_AI_Reaction.UpdateIntendedLocation(TrajectoryPoint, PositionPoint, power, matchVel);
}

// This will only be called from the m_AI_Reaction
void AI_Base_Controller::DriveToLocation(osg::Vec3d TrajectoryPoint,osg::Vec3d PositionPoint, double power, double dTime_s,osg::Vec3d* matchVel)
{
	if (	_isnan(TrajectoryPoint[0]) ||
			_isnan(TrajectoryPoint[1]) ||
			_isnan(TrajectoryPoint[2]) ||
			_isnan(PositionPoint[0]) ||
			_isnan(PositionPoint[1]) ||
			_isnan(PositionPoint[2]) ||
			_isnan(power) ||
			_isnan(dTime_s) ||
			(matchVel && (
				_isnan((*matchVel)[0]) ||
				_isnan((*matchVel)[1]) ||
				_isnan((*matchVel)[2]))))
	{
		printf("TrajectoryPoint = (%f,%f,%f)\n", TrajectoryPoint[0], TrajectoryPoint[1], TrajectoryPoint[2]);
		printf("PositionPoint = (%f,%f,%f)\n", PositionPoint[0], PositionPoint[1], PositionPoint[2]);
		if (matchVel)
			printf("matchVel = (%f,%f,%f)\n", (*matchVel)[0], (*matchVel)[1], (*matchVel)[2]);
		printf("dTime_s = %f, power = %f\n", dTime_s, power);
		ASSERT(false);
	}

	osg::Vec3d VectorOffset=TrajectoryPoint-m_ship.GetPos_m();

	osg::Vec3d RollOrientation(m_ship.m_IntendedOrientation.conj()*osg::Vec3d(0.0,0.0,0.1));
	osg::Vec3d AngularDistance=m_ship.m_IntendedOrientationPhysics.ComputeAngularDistance(VectorOffset,RollOrientation);
	//printf("\r %f %f %f          ",RAD_2_DEG(AngularDistance[0]),RAD_2_DEG(AngularDistance[1]),RAD_2_DEG(AngularDistance[2]));

	AngularDistance[2]=0;
	m_ship.SetCurrentAngularVelocity(-AngularDistance);

	//first negotiate the max speed given the power
	double MaxSpeed=m_ship.ENGAGED_MAX_SPEED;
	double ScaledSpeed=MaxSpeed;
	{
		if ((power >= 0.0) && (power <= 1.0))
		{
			//Now to compute the speed based on MAX (Note it is up to the designer to make the power smaller in tight turns
			ScaledSpeed= MaxSpeed * power;
			DEBUG_AUTO_PILOT_SPEED("\rRamora Speeds: Max=%4.1f, Power = %3.1f, Curr = %3.1f",MaxSpeed, power, m_ship.m_Physics.GetLinearVelocity().length());
		}
		else if (power>1.0)
			SetShipSpeed(MIN(power, MaxSpeed));
	}

	if (matchVel)
	{
		VectorOffset=PositionPoint-m_ship.GetPos_m();
		osg::Vec3d LocalVectorOffset(m_ship.GetAtt_quat().conj() * VectorOffset);
		osg::Vec3d LocalMatchVel(m_ship.GetAtt_quat().conj() * (*matchVel));

		osg::Vec3d ForceRestraintPositive(m_ship.GetFlightCharacteristics().MaxAccelRight*m_ship.Mass,m_ship.GetFlightCharacteristics().MaxAccelForward*m_ship.Mass,m_ship.GetFlightCharacteristics().MaxAccelUp*m_ship.Mass);
		osg::Vec3d ForceRestraintNegative(m_ship.GetFlightCharacteristics().MaxAccelLeft*m_ship.Mass,m_ship.GetFlightCharacteristics().MaxAccelReverse*m_ship.Mass,m_ship.GetFlightCharacteristics().MaxAccelDown*m_ship.Mass);
		//Note: it is possible to overflow in extreme distances, if we challenge this then I should have an overflow check in physics
		osg::Vec3d LocalVelocity=m_ship.m_Physics.GetVelocityFromDistance_Linear(LocalVectorOffset,ForceRestraintPositive,ForceRestraintNegative,dTime_s, LocalMatchVel);

		//The logic here should make use of making coordinated turns anytime the forward/reverse velocity has a greater distance than the sides or up/down.
		//Usually if the trajectory point is the same as the position point it will perform coordinated turns most of the time while the nose is pointing
		//towards its goal.  If the nose trajectory is different it may well indeed use the strafing technique more so.

		if ((fabs(LocalVelocity[0])+fabs(LocalVelocity[2]))<fabs(LocalVelocity[1]))
		{
			//This first technique only works with the forward and partial reverse thrusters (may be useful for some ships)
			//Note: Even though this controls forward and reverse thrusters, the strafe thrusters are still working implicitly to correct turn velocity

			//Now we simply use the positive forward thruster 
			if (LocalVelocity[1]>0.0)  //only forward not reverse...
				SetShipSpeed(MIN(LocalVelocity[1],ScaledSpeed));
			else
				SetShipSpeed(0);  //We do not want to contribute forces in the wrong direction!
		}
		
		else
		{  //This technique makes use of strafe thrusters.  (Currently we can do coordinated turns with this)
			//It is useful for certain situations.  One thing is for sure, it can get the ship
			//to a point more efficiently than the above method, which may be useful for an advanced tactic.
			osg::Vec3d GlobalVelocity(m_ship.GetAtt_quat() * LocalVelocity); 
			//now to cap off the velocity speeds
			for (size_t i=0;i<3;i++)
			{
				if (GlobalVelocity[i]>ScaledSpeed)
					GlobalVelocity[i]=ScaledSpeed;
				else if (GlobalVelocity[i]<-ScaledSpeed)
					GlobalVelocity[i]=-ScaledSpeed;
			}
			//Ideally GetForceFromVelocity could work with local orientation for FlightDynmic types, but for now we convert
			osg::Vec3d GlobalForce(m_ship.m_Physics.GetForceFromVelocity(GlobalVelocity,dTime_s));
			osg::Vec3d LocalForce(m_ship.GetAtt_quat().conj() * GlobalForce); //First get the local force
			//Now to fire all the thrusters given the acceleration
			m_ship.SetCurrentLinearAcceleration(LocalForce/m_ship.Mass);
		}
	}
	else
		SetShipSpeed(ScaledSpeed);
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::SetShipSpeed(double speed_mps)
{
	// Use an averager to smooth things out
	speed_mps = m_speedAverager.GetAverage(speed_mps);

	if (speed_mps > 0.0)
	{
		double speedAdj = m_ship.GetRequestedSpeed() / speed_mps;
		if ((speedAdj > 0.7) && (speedAdj < 1.4))
			return; // We are close enough for the AI
	}

	// Now we can change the requested speed
	m_ship.SetRequestedSpeed(speed_mps);
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::AI_TryFireCannons(DestroyableEntity* target)
{
#ifdef DISABLE_AI_CANNONS
	return;
#endif

	bool fireNow = false;
	if (target && (m_ship.GetWeaponsSpeed() > 0.0))
	{

		// Find the relative position to make sure it is in front of us
		osg::Vec3d targetRelPos = m_ship.GetAtt_quat().conj() * (GetTargetLeadPoint() - m_ship.GetPos_m());

		// How close do we need to be to try to fire?
		double inRangeCoeff = 0.90;

		// We only fire if in range and if the target is in front
		osg::Vec3d targetShipSize = target->GetPhysics().GetRadiusOfConcentratedMass();
		fireNow = (targetRelPos[1] > 0.0) && // In front
			((targetRelPos[1]+targetShipSize[1]) < (m_ship.GetWeaponsRange()*inRangeCoeff)) && // In range
			((targetRelPos[1]*targetRelPos[1]) > (50.0*(targetRelPos[0]*targetRelPos[0] + targetRelPos[2]*targetRelPos[2]))); // In cone angle
		// TODO: This does not adjust for the cannon placements, we are assuming forward firing
	}

	if (fireNow != m_firingCannons)
	{
		m_firingCannons = fireNow;
		m_ship.GetEventMap()->EventOnOff_Map["Ship.TryFireMainWeapon"].Fire(m_firingCannons);
	}
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::ShipDestroyed(const osg::Vec3d& collisionPt)
{
	// TODO: IS THIS RIGHT??  This only happens when there is a collision?
	// If I am destroyed from a collision, I can count the kill towards my target
	ThrustShip2* target = dynamic_cast<ThrustShip2*>(GetTarget());
	if (target)
		target->GetController()->IncrementKills();
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::OnCannonRoundHit(GG_Framework::Logic::Entity3D* otherEntity)
{
	// Only count hits on my target
	if (GetTarget() == otherEntity)
		IncrementHits();
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::SetTarget(DestroyableEntity* newTarget)
{
	if (newTarget == &m_ship)
		newTarget = NULL;

	if (TargetShip.Ship != newTarget)
	{
		// Fire a network message with the new targets index
		double targetID = newTarget ? (double)newTarget->GetNetworkID() : -1.0;
		m_ship.GetRC_Controller().FireNetwork_EventValue("SET_TARGET", targetID, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, OC_NonCriticalEntityEvents);
	}
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::NetworkSetTarget(double targetID)
{
	DestroyableEntity* newTarget = (targetID < 0.0) ? NULL :
		(dynamic_cast<DestroyableEntity*>(m_ship.GetGameClient()->FindEntity(targetID)));
	InnerSetTarget(newTarget);
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::InnerSetTarget(GG_Framework::AppReuse::DestroyableEntity* newTarget)
{
	if (newTarget == &m_ship)
		newTarget = NULL;

	if (TargetShip.Ship != newTarget)
	{
		if (TargetShip.Ship)
		{
			TargetShip.Ship->GetEventMap()->EventPt_Map["DESTROY"].Remove(*this, &AI_Base_Controller::TargetDestroyed);
			TargetShip.Ship->GetEventMap()->EventPt_Map["SHOT_DOWN"].Remove(*this, &AI_Base_Controller::TargetDestroyed);
		}
		TargetShip.Ship = newTarget;
		if (newTarget)
		{
			TargetShip.ShipID = newTarget->GetName();
			TargetShip.Ship->GetEventMap()->EventPt_Map["DESTROY"].Subscribe(ehl, *this, &AI_Base_Controller::TargetDestroyed);
			TargetShip.Ship->GetEventMap()->EventPt_Map["SHOT_DOWN"].Subscribe(ehl, *this, &AI_Base_Controller::TargetDestroyed);
		}
		else
			TargetShip.ShipID = "";
	}
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::SelectTarget(int dir)
{
	GG_Framework::Logic::GameClient* gameClient = m_ship.GetGameClient();
	int numEntities = gameClient->GetNumEntities();
	GG_Framework::Logic::Entity3D* currTarget = TargetShip.Ship;
	int currIndex = currTarget ? currTarget->GetNetworkID() : ((dir>0) ? -1 : numEntities);

	// Loop to find the next controllable entity
	int tryIndex = currIndex;
	int numEntitiesTried = 0;
	while (numEntitiesTried < numEntities)
	{
		// Look at the NEXT index
		tryIndex += dir;

		// Watch for wrapping to the beginning of the loop
		if (tryIndex == numEntities)
			tryIndex = 0;
		else if (tryIndex < 0)
			tryIndex = numEntities-1;

		// We only want to target DestroyableEntity objects that are visible
		GG_Framework::Logic::Entity3D* tryEntity = gameClient->FindEntity(tryIndex);
		if (tryEntity && tryEntity->IsShowing() && (tryEntity!=currTarget) && (&m_ship!=tryEntity))
		{
			DestroyableEntity* newTarget = dynamic_cast<DestroyableEntity*>(tryEntity);
			if (newTarget)
			{
				this->SetTarget(newTarget);
				return;
			}
		}
		++numEntitiesTried;

	}

	// We could not find any other ships to target
	SetTarget(NULL);
}
//////////////////////////////////////////////////////////////////////////

void AI_Base_Controller::TargetTgtYou()
{
	EntityMap_NetID::iterator it =  m_ship.GetGameClient()->GetEntityIteratorBegin();

	// Find the ship targeting you that is closest
	ThrustShip2* closestTargetingShip = NULL;
	double maxDist2 = 1e9;

	// Find the entity that is targeting this
	while (it != m_ship.GetGameClient()->GetEntityIteratorEnd())
	{
		GG_Framework::Logic::Entity3D* tryEntity = (*it).second;
		if (tryEntity && tryEntity->IsShowing())
		{
			ThrustShip2* ship = dynamic_cast<ThrustShip2*>(tryEntity);
			if (ship && (ship->GetController()->GetTarget() == &m_ship))
			{
				double dist2 = (m_ship.GetPos_m() - ship->GetPos_m()).length2();
				if (dist2 < maxDist2)
				{
					maxDist2 = dist2;
					closestTargetingShip = ship;
				}
			}
		}
		++it;
	}

	// We are not being targeted
	SetTarget(closestTargetingShip);
}
//////////////////////////////////////////////////////////////////////////


void AI_Base_Controller::TargetInSight()
{
	double closestHit = 1.0e6;
	double greatestCosAngle = 0.2; // Only ships in front of you 
	DestroyableEntity* closestEntity = TargetInFront(closestHit, greatestCosAngle);
	if (closestEntity)
		this->SetTarget(closestEntity);
}
//////////////////////////////////////////////////////////////////////////

DestroyableEntity* AI_Base_Controller::TargetInFront(double& closestHit, double& greatestCosAngle)
{
	// Find the entity that is forward, first create the line segment
	osg::Vec3d gunPos; osg::Quat gunAtt;
	m_ship.GetWeaponRelPosAtt(gunPos, gunAtt);
	osg::Vec3d segStart(m_ship.GetPos_m() + m_ship.GetAtt_quat()*gunPos);
	osg::Vec3d segEnd(segStart + (gunAtt*m_ship.GetAtt_quat())*osg::Vec3d(0.0, 1.0e6, 0.0));

	// We may also want to look at ships that are "close" even if we are not right on them
	osg::Vec3d ourDirectionUnitVec = (segEnd - segStart) / 1.0e6;
	DestroyableEntity* closestEntity = NULL;

	EntityMap_NetID::iterator it =  m_ship.GetGameClient()->GetEntityIteratorBegin();

	// Find the entity that is targeting this
	while (it != m_ship.GetGameClient()->GetEntityIteratorEnd())
	{
		GG_Framework::Logic::Entity3D* tryEntity = it->second;
		if (tryEntity && (tryEntity != &m_ship) && tryEntity->IsShowing())
		{
			DestroyableEntity* tgt = dynamic_cast<DestroyableEntity*>(tryEntity);
			if (tgt)
			{
				// Find the unit vector of the other ship
				osg::Vec3d otherShipDirectionUnitVec = tgt->GetPos_m() - segStart;
				otherShipDirectionUnitVec.normalize();
				double cosAngle = ourDirectionUnitVec * otherShipDirectionUnitVec;
				if (cosAngle > greatestCosAngle)
				{
					greatestCosAngle = cosAngle;
					closestEntity = tgt;
				}

				GG_Framework::UI::OSG::PickVisitor collisionDetector;
				osgUtil::IntersectVisitor::HitList localHits = collisionDetector.getHits(tgt->Get_BB_Actor(), segStart, segEnd);

				if (!localHits.empty())
				{
					size_t numHits = localHits.size();
					for (size_t i = 0; i < numHits; ++i)
					{
						double dist = (localHits[i].getWorldIntersectPoint() - segStart).length();
						if (dist < closestHit)
						{
							closestHit = dist;
							closestEntity = tgt;
							greatestCosAngle = 1.0; // No more close angles should work
						}
					}
				}
			}
		}
		++it;
	}
	return closestEntity;
}
//////////////////////////////////////////////////////////////////////////
double const c_SafeDistanceScaler=2.0;  //number of seconds to react (this must be consistent with collider and ship)

osg::Vec3d ConvertToCollisionSpace(const osg::Vec3d& pt, const osg::Vec3d& standOffDist_m, const Entity3D* collider, bool toGlobal)
{
	osg::Vec3d ret = pt;
	if (toGlobal)
	{
		// Scale to handle distances from the collider with standoff distance
		ret[0] *= standOffDist_m[0];
		ret[1] *= standOffDist_m[1];
		ret[2] *= standOffDist_m[2];

		// Rotate
		ret = collider->GetAtt_quat() * ret;

		// Offset by position and the safe distance of its velocity
		ret += collider->GetPos_m() + (collider->GetLinearVelocity()*c_SafeDistanceScaler);
	}
	else
	{
		// Offset by position and the safe distance of its velocity
		ret -= collider->GetPos_m() + (collider->GetLinearVelocity()*c_SafeDistanceScaler);

		// Rotate
		ret = collider->GetAtt_quat().inverse() * ret;

		// Scale to handle distances from the collider with standoff distance
		ret[0] /= standOffDist_m[0];
		ret[1] /= standOffDist_m[1];
		ret[2] /= standOffDist_m[2];
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////
/// \ret closest point to collider.  This can be used to determine how to avoid collision
osg::Vec3d FindDistance(const osg::Vec3d &X1,const osg::Vec3d &X2)
{
	// Use a point-line distance algorithm to see how close I come to this unit sphere (note that X0 is 0,0,0)
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	// We are finding the t value for the line equation
	// pedantic check for the points being on top of each other
	double len2 = (X2-X1).length2();
	if (len2 < 0.00001) return osg::Vec3(0.0,0.0,0.0);

	double t = -(X1*(X2-X1)) / len2;

	// If the closest point is not between X1 and X2, make sure that we check the end points
	if (t < 0.1) t = 0.1;	// Make sure we are not too close at the beginning of the line
	else if (t > 1.0) t = 1.0;	// Make sure we are not too close at the end of the line

	// So where is this closest point anyway?
	osg::Vec3d closestPt = X1 + ((X2-X1)*t);

	return closestPt;
}

bool FindAvoidance(const osg::Vec3d& X1, osg::Vec3d& X2)
{
	// Use a point-line distance algorithm to see how close I come to this unit sphere (note that X0 is 0,0,0)
	// http://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
	// We are finding the t value for the line equation
	// pedantic check for the points being on top of each other
	double len2 = (X2-X1).length2();
	if (len2 < 0.00001) return false;

	double t = -(X1*(X2-X1)) / len2;

	// If the closest point is not between X1 and X2, make sure that we check the end points
	if (t < 0.1) t = 0.1;	// Make sure we are not too close at the beginning of the line
	else if (t > 1.0) t = 1.0;	// Make sure we are not too close at the end of the line

	// So where is this closest point anyway?
	osg::Vec3d closestPt = X1 + ((X2-X1)*t);

	// Watch for the length2
	double closestPt_Len2 = closestPt.length2();

	// If farther than 1, no worries (watch for being too close to start)
	if (closestPt_Len2 > 1.0)
		return false;	

	// Watch for going RIGHT through the point
	// Otherwise, make X2 a normalized version of closestPt
	if (closestPt_Len2 < 0.0001)
	{
		X2 = X1 ^ osg::Vec3d(1,0,0);
		X2.normalize();

		// Make sure this one still works by calling recursively
		FindAvoidance(X1,X2);
	}
	else
	{
		X2 = closestPt;
		X2.normalize();
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::TryAvoidCollision(double thisMaxRad, osg::Vec3d& intendedPos, EntityMap_NetID::iterator it)
{
	bool ret = false;

	// Loop through every other entity in the scene
	while (it != m_ship.GetGameClient()->GetEntityIteratorEnd())
	{
		// Grab the other entity
		Entity3D* collider = it->second;

		// We do not care about most of the entities
		if ((&m_ship != collider) &&			// Do not check against self
			(collider->IsShowing()) &&
			(collider->Get_BB_Actor()) &&
			(collider->GetCollisionIndex() > 5) && // Bigger than a missile
			(m_ship.GetCollisionIndex() < (collider->GetCollisionIndex()+16)) && // If we are very much bigger, do not try to evade
			(!collider->ShouldIgnoreCollisions(&m_ship)) &&
			(!m_ship.ShouldIgnoreCollisions(collider)))
		{
			// SO, we MIGHT want to avoid this one
			// Find out how big it is along each axis, and then add our own radius to avoid colliding
			osg::Vec3d standOffDist_m = collider->GetDimensions() * 0.7;
			standOffDist_m[0] += thisMaxRad;
			standOffDist_m[1] += thisMaxRad;
			standOffDist_m[2] += thisMaxRad;



			// Convert the 2 endpoints of the line to local coordinates, adjusting for standoff distance
			// These adjusted coordinates are local to the collider in position and rotation and are scaled
			// based on the offset distance.  So the collision ovoid is now a sphere of radius 1.
			osg::Vec3d X1 = ConvertToCollisionSpace(m_ship.GetPos_m(), standOffDist_m, collider, false);
			//The multiply represents how many seconds ahead to take action
			osg::Vec3d VelocityLine=m_ship.GetPos_m() + (m_ship.GetLinearVelocity() * c_SafeDistanceScaler);
			osg::Vec3d X2 = ConvertToCollisionSpace(VelocityLine, standOffDist_m, collider, false);

			{
				//See if there is a possible collision case
				osg::Vec3d ClosestPoint=FindDistance(X1,X2);
				double closestPt_Len2 = ClosestPoint.length2();

				// If farther than 1, no worries (watch for being too close)
				if (closestPt_Len2 <= 1.0)
				{
					//A potential collision is found, now to determine how to avoid
					//My first idea was to consider the possibility of always swerving if the target is far, however
					//the VIP demo3 is currently broken using this on work machine.  I believe the problem is because of the 
					//3 Karkakus ships confusing the ships... will need to investigate, but hopefully this should not be typical

					bool swerve=true;

					//I'm temporarily keeping this in until I know if it is necessary to treat the target collider differently
					//so far this does not appear to be the case
					#if 0
					//It seems to stand to reason to determine if the collider is our target.
					{
						//using a lower threshold helps to avoid confusion from if it is close to a large entity, however, we may consider passing in the
						//target name here directly, to avoid this error.  This should suffice for now.
						osg::Vec3d Threshold = collider->GetDimensions() * 0.2;
						osg::Vec3d Distance=collider->GetPos_m() - intendedPos;
						if ((Distance[0]<Threshold[0]) && (Distance[1]<Threshold[1]) && (Distance[2]<Threshold[2]))
						{
							//We now know our target is the collider, and they are about 2 seconds until impact

							//For now if so we can simply lift the avoidance work and make
							//that the responsibility of the target tracking.  We may eventually want to add logic to handle that if it is determined that there are
							//other reasons why the target point is inside the collider.
							++it;
							continue;
						}
					}
					#endif
					{
						//Here is the rare case when we are inside a big ship, we simply need to find an edge to get out
						bool InsideCollier=false;
						{
							osg::Vec3d Threshold = collider->GetDimensions();
							osg::Vec3d Distance=collider->GetPos_m() - m_ship.GetPos_m();
							if ((Distance[0]<Threshold[0]) && (Distance[1]<Threshold[1]) && (Distance[2]<Threshold[2]))
								InsideCollier=true;
						}

						if (!InsideCollier)
						{
							//We now know that the ship and collider will impact in c_SafeDistanceScaler seconds.  Now to determine the current speed to collider
							//From that we can get a rough estimate of the acceleration needed to come to a full stop.  With a little tweaking on the safe distance 
							//scaler we can make this the right amount of conservative buffering.

							//TODO remove distance here once we determine that c_SafeDistanceScaler alone is fine
							//osg::Vec3d ClosePoint_global = ConvertToCollisionSpace(ClosestPoint, standOffDist_m, collider, true);
							//osg::Vec3d Distance=ClosePoint_global-m_ship.GetPos_m();
							//double dDistance=std::min(Distance.length(),c_SafeDistanceScaler);
							double dDistance=c_SafeDistanceScaler;  //this should be fine

							double Speed=(m_ship.GetLinearVelocity()-collider->GetLinearVelocity()).length();
							//Using MaxAccelReverse is somewhat oversimplified as we assume we are facing our collision, but should be ok for now
							if (Speed/dDistance < m_ship.GetFlightCharacteristics().MaxAccelReverse)
								swerve=false;

						}
						//DOUT2("%s",swerve?"swerve":"normal");  //not the most accurate, but a rough idea

					}
					if (swerve)
					{
						m_ship.SetHeadingSpeedScale(1.0);  //ensure heading is back at full capacity!
						// Watch for an avoidance offset.  If FindAvoidance returns true, it will change X2 to be on the edge 
						// of the collision sphere.
						if (FindAvoidance(X1, X2))
						{
							// Find the new intendedPos by converting BACK the modified X2 to Global coordinates
							intendedPos = ConvertToCollisionSpace(X2, standOffDist_m, collider, true);
							ret = true;

							// The new intendedPos is closer to the ship's current position than before
							// Check below us by just letting it pass to the next in the loop		
						}
					}
				}
			}
		}
		// Next in the list	
		++it;
	}
	return ret;
}
//////////////////////////////////////////////////////////////////////////

bool AI_Base_Controller::TryAvoidCollision(osg::Vec3d& intendedPos)
{
	// Bullets and missiles do not try to evade
	if (m_ship.GetCollisionIndex() < 6) return false;

	// Find my max radius using the physics class for the ship
	const osg::Vec3d myBoundOvoid(m_ship.GetDimensions() * 0.5);
	double myMaxRad_m = MAX(myBoundOvoid[0], MAX(myBoundOvoid[1], myBoundOvoid[2]));
	if (myMaxRad_m < 1.0) myMaxRad_m = 1.0;
	myMaxRad_m *= 1.5;	// A little extra distance

	// If we are not really moving far anyway
	if ((m_ship.GetPos_m()-intendedPos).length2() < (myMaxRad_m*myMaxRad_m)) return false;

	return TryAvoidCollision(myMaxRad_m, intendedPos, m_ship.GetGameClient()->GetEntityIteratorBegin());
}
//////////////////////////////////////////////////////////////////////////
