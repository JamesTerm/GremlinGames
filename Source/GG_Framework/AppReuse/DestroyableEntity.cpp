// GG_Framework.AppReuse DestroyableEntity.cpp
#include "stdafx.h"
#include "GG_Framework.AppReuse.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::AppReuse;

// DestroyableEntity will use its own TransmittedEntity
ClassFactoryT<DestroyableEntity_TransmittedEntity, TransmittedEntity> SE3D_TransmittedEntity_ClassFactory("DestroyableEntity", TransmittedEntity::ClassFactory);

// My own class factory
ClassFactoryT<DestroyableEntity, Entity3D> DestroyableEntity_ClassFactory("DestroyableEntity", Entity3D::ClassFactory);

//! Build a Transmitted entity from the LUA Script
void DestroyableEntity_TransmittedEntity::LoadFromScript(
	GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
	GG_Framework::Logic::ITransmittedEntityOwner* entityOwner)
{
	// Create an entity we will display for destruction
	TransmittedEntity* dest_te = ClassFactory.Create("DestructionProxyEntity");
	
	const char* err;
	err = script.GetFieldTable("Entity");
	ASSERT_MSG(!err, err);

	// Get the string we need to define the OSGV shown when the entity is destroyed
	std::string Destruction_OSGV;
	script.GetField("Destruction_OSGV", &Destruction_OSGV, NULL, NULL);
	ASSERT_MSG(!err, err);

	script.GetField("RESPAWN_TIME", NULL, NULL, &RESPAWN_TIME);
	ASSERT_MSG(!err, err);

	script.GetField("RESPAWN_HITPOINTS", NULL, NULL, &RESPAWN_HITPOINTS);
	ASSERT_MSG(!err, err);

	script.Pop();

	// We need to place the destruction before ourself
	dest_te->NETWORK_ID = entityOwner->AddTransmittedEntity(dest_te);
	this->DESTRUCTION_ENTITY_INDEX = dest_te->NETWORK_ID;

	// The base class inserts me into the vector and makes other assignments
	__super::LoadFromScript(script, controllingPlayerID, entityOwner);

	// All the other values for the destruction entity are the same as ours...
	*dest_te = *this;

	// Except this stuff
	dest_te->OSGV = Destruction_OSGV;
	dest_te->CPP_CLASS = "DestructionProxyEntity";
	dest_te->NETWORK_ID = this->DESTRUCTION_ENTITY_INDEX;
	dest_te->NAME = GG_Framework::Base::BuildString("%s.DESTROY", NAME.c_str());
}
//////////////////////////////////////////////////////////////////////////

//! Build a Transmitted entity from a received Packet data
//! Apply this AFTER any packet identifiers
void DestroyableEntity_TransmittedEntity::LoadFromBitStream(RakNet::BitStream& bs)
{
	__super::LoadFromBitStream(bs);

	bs.Read(DESTRUCTION_ENTITY_INDEX);
	bs.Read(RESPAWN_TIME);
	bs.Read(RESPAWN_HITPOINTS);
}
//////////////////////////////////////////////////////////////////////////

//! Write the entity into the BitStream to be sent, WITHOUT the header
void DestroyableEntity_TransmittedEntity::WriteToBitStream(RakNet::BitStream& bs)
{
	__super::WriteToBitStream(bs);

	bs.Write(DESTRUCTION_ENTITY_INDEX);
	bs.Write(RESPAWN_TIME);
	bs.Write(RESPAWN_HITPOINTS);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
DestroyableEntity::DestroyableEntity() : 
m_remHitPoints(0.0), m_respawnHitPoints(0.0), m_destProxy(NULL), m_respawnTime(-1.0), m_nextRespawn(DBL_MAX), 
m_needFirstSpawn(true), m_numDestructions(0)
{
}
//////////////////////////////////////////////////////////////////////////


// Read in the parameters for our destruction proxy entity, re-spawn, damage information, etc.
void DestroyableEntity::Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te)
{
	__super::Initialize(gs, em, te);

	// Watch for the name of the OSGV file for our collision entity
	DestroyableEntity_TransmittedEntity* sete = dynamic_cast<DestroyableEntity_TransmittedEntity*>(&te);
	ASSERT_MSG(sete, "DestroyableEntity could not cast transmitted entity to DestroyableEntity_TransmittedEntity\n");

	// Time to re-spawn and number of hit-points
	m_respawnTime = sete->RESPAWN_TIME;
	m_respawnHitPoints = sete->RESPAWN_HITPOINTS;
	m_remHitPoints = m_respawnHitPoints;

	// Find the Destruction Proxy Entity
	m_destProxy = dynamic_cast<DestructionProxyEntity*>(GetGameClient()->FindEntity(sete->DESTRUCTION_ENTITY_INDEX));
	ASSERT_MSG(m_destProxy, "Failed to find DestructionProxyEntity\n");
	m_destProxy->SetParentEntity(this);

	// Even network clients need to know about how many hit-points they have
	GetEventMap()->EventValue_Map["HIT_PT"].Subscribe(ehl, *this, &DestroyableEntity::HitPointChange);
}
//////////////////////////////////////////////////////////////////////////

double DestroyableEntity::GetDamageFromHit(Entity3D& otherEntity, const osg::Vec3d& collisionPt)
{
	IWeaponEntity* weapon = dynamic_cast<IWeaponEntity*>(&otherEntity);
	if (weapon)
		return weapon->GetHitDamage();
	else
	{
		// Base damage on purely a collision (there are things that can be tweaked here)
		int otherIndex = otherEntity.GetCollisionIndex();
		return 1.0 + otherIndex*otherIndex;
	}
}
//////////////////////////////////////////////////////////////////////////

void DestroyableEntity::OnCollision(Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s)
{
	// Call the base class to handle 
	__super::OnCollision(otherEntity, hitPt, dTime_s);

	// Work out the hit points deducted (Notional, we will use something fancier)
#ifndef DISABLE_DESTRUCTION

	// Only do this when being controlled locally
	if (IsLocallyControlled())
	{
		if (m_remHitPoints > 0.0)
		{
			m_remHitPoints -= GetDamageFromHit(otherEntity, hitPt);
			DEBUG_DESTRUCTION("DestroyableEntity::OnCollision() m_remHitPoints = %i\n", m_remHitPoints);

			// We get destroyed if we run into another ship OR lost our hit points
			if (otherEntity.GetCollisionIndex() > GetCollisionIndex())
			{
				if (m_remHitPoints > 0.0)
					m_remHitPoints = 0.0;
				DestroyEntity(false, hitPt);
			}
			else if (m_remHitPoints <= 0.0)
			{
				m_remHitPoints = 0.0;
				DestroyEntity(true, hitPt);
			}

			// Send this message to all the clients, not a big hurry
			GetRC_Controller().FireNetwork_EventValue("HIT_PT", m_remHitPoints, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, OC_NonCriticalEntityEvents);
		}
	}
#endif
}
//////////////////////////////////////////////////////////////////////////

void DestroyableEntity::HitPointChange(double newHP)
{
	m_remHitPoints = newHP;
}
//////////////////////////////////////////////////////////////////////////

// This should only be called locally
void DestroyableEntity::DestroyEntity(bool shotDown, osg::Vec3d collisionPt)
{
	ASSERT(IsLocallyControlled());
	DEBUG_DESTRUCTION("DestroyableEntity::DestroyEntity(%s)\n", shotDown ? "Shot Down" : "Collision");
	++m_numDestructions;

	// Get the destruction proxy at the right place and set its vectors
	if (m_destProxy)
	{
		DEBUG_DESTRUCTION("DestroyableEntity::DestroyEntity() Destruction Proxy Setup\n");

		// These messages should get sent out
		m_destProxy->AlignWithOrigAndShow(GetPos_m(), GetAtt_quat(), m_Physics);

		// We have to send the messages out so they can do this everywhere
		m_destProxy->GetRC_Controller().FireNetwork_EventPt(shotDown ? "SHOT_DOWN" : "DESTROY", 
			collisionPt, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);
	}

	// Fire the event so OSG can react to the explosion (will take care of hiding)
	// Do this through the network
	DEBUG_DESTRUCTION("DestroyableEntity::DestroyEntity() Firing original events\n");
	GetRC_Controller().FireNetwork_EventPt(shotDown ? "SHOT_DOWN" : "DESTROY", 
		collisionPt, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);

	// Set SPAWN to false to stop sound effects and other OSGV stuff (through the network)
	GetRC_Controller().FireNetwork_EventOnOff("SPAWN", false, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);

	// Wait for the next re-spawn by setting the m_nextRespawn to a negative value, see GameTimerUpdate()
	if (m_respawnTime >= 0.0)
		m_nextRespawn = -m_respawnTime;
}
//////////////////////////////////////////////////////////////////////////

void DestroyableEntity::Spawn()
{
	if (IsLocallyControlled())
	{
		m_remHitPoints = GetRespawnPoints();
		ResetPos();
		ShowNextFrame(true);

		GetRC_Controller().FireNetwork_EventValue("HIT_PT", m_remHitPoints, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, OC_NonCriticalEntityEvents);

		GetRC_Controller().FireNetwork_EventOnOff("SPAWN", true, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);
		GetRC_Controller().FireNetwork_Event("SPAWN", HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);
	}
}
//////////////////////////////////////////////////////////////////////////

bool DestroyableEntity::IsBeingDestroyed()
{
	return (m_remHitPoints <= 0.0);
}
//////////////////////////////////////////////////////////////////////////

void DestroyableEntity::GameTimerUpdate(double time_s)
{
	if (IsLocallyControlled())
	{
		// Having a value of 0 or less means we just barely got destroyed
		if (m_nextRespawn <= 0.0) // Remember the time to respawn
			m_nextRespawn = time_s - m_nextRespawn;

		// Has it been long enough?
		if (m_needFirstSpawn || (time_s > m_nextRespawn))
		{
			Spawn();
			// Setting to DBL_MAX means we will not 
			m_nextRespawn = DBL_MAX;
			m_needFirstSpawn = false;
		}
	}
	__super::GameTimerUpdate(time_s);
}
//////////////////////////////////////////////////////////////////////////

void DestroyableEntity::FindRelatedEntities(std::set<Entity3D*>& relatedEntities)
{
	if (m_destProxy && !relatedEntities.count(m_destProxy))
	{
		relatedEntities.insert(m_destProxy);
		m_destProxy->FindRelatedEntities(relatedEntities);
	}
	__super::FindRelatedEntities(relatedEntities);
}
//////////////////////////////////////////////////////////////////////////

/// Called as we are losing control of this entity
void DestroyableEntity::SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner)
{
	if (sameOwner)
	{
		sendControlBS.Write(m_respawnHitPoints);
		sendControlBS.Write(m_remHitPoints);
		sendControlBS.Write(m_respawnTime);
		sendControlBS.Write(m_nextRespawn);
		sendControlBS.Write(m_needFirstSpawn);
		sendControlBS.Write(m_numDestructions);
	}
	__super::SendFinalUpdate(playerThatWantsControl, sendControlBS, sameOwner);
}
//////////////////////////////////////////////////////////////////////////

/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
void DestroyableEntity::RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner)
{
	if (sameOwner)
	{
		recvControlBS.Read(m_respawnHitPoints);
		recvControlBS.Read(m_remHitPoints);
		recvControlBS.Read(m_respawnTime);
		recvControlBS.Read(m_nextRespawn);
		recvControlBS.Read(m_needFirstSpawn);
		recvControlBS.Read(m_numDestructions);
	}
	__super::RecvFinalUpdate(recvControlBS, playerThatWantsControl, msgTime_s, sameOwner);
}
//////////////////////////////////////////////////////////////////////////

bool DestroyableEntity::ShouldIgnoreCollisions(const Entity3D* otherEntity)
{	
	if (IsBeingDestroyed() && (otherEntity == m_destProxy)) return false;
	else return __super::ShouldIgnoreCollisions(otherEntity);
}
//////////////////////////////////////////////////////////////////////////



