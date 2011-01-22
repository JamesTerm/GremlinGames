// Fringe.Base Ship.cpp
#include "stdafx.h"
#include "Fringe.Base.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace Fringe::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;

void Ship::Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te)
{
	DEBUG_CANNON_FIRE("Ship::Initialize(...)\n");
	__super::Initialize(gs, em, te);

	ThrustShip2_TransmittedEntity* tste = dynamic_cast<ThrustShip2_TransmittedEntity*>(&te);
	if (tste)
	{
		std::vector<CannonDesc>::iterator pos;
		unsigned index = 0;
		for (pos = tste->Cannons.begin(); pos != tste->Cannons.end(); ++pos)
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

	// Listen for the main firing weapons
	em.EventOnOff_Map["Ship.TryFireMainWeapon"].Subscribe(ehl, *this, &Ship::TryFireMainWeapon);
}
//////////////////////////////////////////////////////////////////////////

void Ship::GetWeaponRelPosAtt(osg::Vec3d& pos, osg::Quat& att)
{
	pos = osg::Vec3d();
	att = osg::Quat();
	
	size_t numCannons = m_Cannons.size();
	if (numCannons > 0)
	{
		for (size_t i = 0; i < numCannons; ++i)
		{
			pos += m_Cannons[i]->GetRelPos();
			//att += m_Cannons[i]->GetRelAtt();
		}
		pos /= numCannons;
		//att /= numCannons;
	}
}
//////////////////////////////////////////////////////////////////////////

Ship::~Ship()
{
	std::vector<Cannon*>::iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
		delete (*pos);
	m_Cannons.clear();
}
//////////////////////////////////////////////////////////////////////////

void Ship::GetWeaponStatus(Cannon::TempStatus& tempStatus, double& tempLevel, double& restartLevel) const
{
	ASSERT(IsLocallyControlled());
	tempStatus = m_cannonTempStatus;
	tempLevel = m_cannonTempLevel;
	restartLevel = m_cannonRestartLevel;
}
//////////////////////////////////////////////////////////////////////////

Ship::Ship() : 
m_cannonRange(0.0), m_cannonSpeed(0.0), m_vaporTrails(false), m_accelDeriv(5), 
m_gunTargetLead(NULL), m_cannonTempStatus(Cannon::CANNON_Cold), m_cannonTempLevel(0.0), m_cannonRestartLevel(0.0)
{}


void Ship::UpdateCannonTemps()
{
	double tempLevel = 0.0;
	double restartLevel = 0.0;
	Cannon::TempStatus tempStatus = Cannon::CANNON_Cold;

	if (!IsBeingDestroyed())
	{
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

	m_cannonTempLevel = tempLevel;
	m_cannonRestartLevel = restartLevel;
	if (m_cannonTempStatus != tempStatus)
	{
		if (m_cannonTempStatus == Cannon::CANNON_Warning)
			GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Warning"].Fire(false);
		else if (m_cannonTempStatus == Cannon::CANNON_Overheated)
			GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Overheat"].Fire(false);

		m_cannonTempStatus = tempStatus;

		if (m_cannonTempStatus == Cannon::CANNON_Warning)
			GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Warning"].Fire(true);
		else if (m_cannonTempStatus == Cannon::CANNON_Overheated)
			GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Overheat"].Fire(true);
	}
}
//////////////////////////////////////////////////////////////////////////

void Ship::TimeChange(double dTime_s)
{
	// Handle the cannon temps which will fire events
	if (IsLocallyControlled())
		UpdateCannonTemps();

	__super::TimeChange(dTime_s);

	// Should we try to fire the vapor trails?
	if (m_vaporTrails != ShouldFireVaporTrails(dTime_s))
	{
		m_vaporTrails = !m_vaporTrails;
		GetEventMap()->EventOnOff_Map["VAPOR_TRAIL"].Fire(m_vaporTrails);
	}
}
//////////////////////////////////////////////////////////////////////////

void Ship::CancelAllControls()
{
	__super::CancelAllControls();
	if (IsLocallyControlled())
	{
		UpdateCannonTemps();
		TryFireMainWeapon(false);
	}
}
//////////////////////////////////////////////////////////////////////////

bool Ship::ShouldFireVaporTrails(double dTime_s)
{
	osg::Vec3d vel = GetLinearVelocity();
	osg::Quat attConj = GetAtt_quat().conj();
	osg::Vec3d accel = m_accelDeriv.GetVectorDerivative(vel, dTime_s);

	// If we are not visible, no trails at all
	if (!IsShowing())
	{
		m_accelDeriv.Reset();
		return false;
	}
	
	// If moving slower than 100 mps, there are no trails
	if (vel.length2() < 10000.0)
		return false;

	// If there is some acceleration in the direction the ship is traveling
	accel = attConj*accel;
	if (accel[1] < 0.0)
		accel[1] = 0.0;	// Only forward counts
	if (accel.length2() > 400.0)
		return true;

	// Otherwise look for the current rotation being not in line with the velocity
	osg::Vec3d diffAttack = attConj*vel;
	if ((diffAttack[0]*diffAttack[0] + diffAttack[2]*diffAttack[2]) > 50000.0)
		return true;

	// Nothing to cause th vapor trails
	return false;
}
//////////////////////////////////////////////////////////////////////////

void Ship::OnAsyncLoadComplete()
{
	__super::OnAsyncLoadComplete();

	DEBUG_CANNON_FIRE("Ship::OnAsyncLoadComplete()\n");
	// Let each cannon do its complete
	std::vector<Cannon*>::iterator pos;
	for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
		(*pos)->OnAsyncLoadComplete();
}
//////////////////////////////////////////////////////////////////////////

void Ship::TryFireMainWeapon(bool on)
{
	if (IsLocallyControlled())
	{
		std::vector<Cannon*>::iterator pos;
		for (pos = m_Cannons.begin(); pos != m_Cannons.end(); ++pos)
			(*pos)->OnFireCannons(on);
	}
}
//////////////////////////////////////////////////////////////////////////

void Ship::FindRelatedEntities(std::set<Entity3D*>& relatedEntities)
{
	// Loop through each cannon to include all of its rounds
	std::vector<GG_Framework::AppReuse::Cannon*>::iterator thisCannon = m_Cannons.begin();
	while (thisCannon != m_Cannons.end())
	{
		(*thisCannon)->FindRelatedEntities(relatedEntities);
		++thisCannon;
	}

	// Base class
	__super::FindRelatedEntities(relatedEntities);
}
//////////////////////////////////////////////////////////////////////////

void Ship::SetControllingPlayerID(const PlayerID& controllingPlayerID)
{//printf("Ship::SetControllingPlayerID\n");
	// My base class
	__super::SetControllingPlayerID(controllingPlayerID); 

	// If My GameClient is a Fringe::Base::UI_GameClient ...
	Fringe::Base::UI_GameClient* ui_client = dynamic_cast<Fringe::Base::UI_GameClient*>(GetGameClient());

	if (ui_client)
		ui_client->ShipChangedControl(this, controllingPlayerID);
}
//////////////////////////////////////////////////////////////////////////

/// Called as we are losing control of this entity
void Ship::SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner)
{
	if (sameOwner)
	{
		sendControlBS.Write(m_cannonRange);
		sendControlBS.Write(m_cannonSpeed);
	}
	__super::SendFinalUpdate(playerThatWantsControl, sendControlBS, sameOwner);

	if (m_cannonTempStatus == Cannon::CANNON_Warning)
		GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Warning"].Fire(false);
	else if (m_cannonTempStatus == Cannon::CANNON_Overheated)
		GetEventMap()->EventOnOff_Map["Ship.UI.Cannon_Overheat"].Fire(false);
	m_cannonTempStatus = Cannon::CANNON_Cold;
}
//////////////////////////////////////////////////////////////////////////

/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
void Ship::RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner)
{
	if (sameOwner)
	{
		recvControlBS.Read(m_cannonRange);
		recvControlBS.Read(m_cannonSpeed);
	}
	__super::RecvFinalUpdate(recvControlBS, playerThatWantsControl, msgTime_s, sameOwner);
}
//////////////////////////////////////////////////////////////////////////




