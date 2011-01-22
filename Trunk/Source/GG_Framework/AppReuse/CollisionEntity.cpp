// GG_Framework.AppReuse CollisionEntity.cpp
#include "stdafx.h"
#include "GG_Framework.AppReuse.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::AppReuse;

// CollisionEntity will use the base TransmittedEntity
ClassFactoryT<TransmittedEntity, TransmittedEntity> CE_TransmittedEntity_ClassFactory("CollisionEntity", TransmittedEntity::ClassFactory);

// My own class factory
ClassFactoryT<CollisionEntity, Entity3D> CollisionEntity_ClassFactory("CollisionEntity", Entity3D::ClassFactory);

void CollisionEntity::Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te)
{
	__super::Initialize(gs, em, te);

	// Listen for the collision event
	em.Collision.Subscribe(ehl, *this, &CollisionEntity::OnCollision);
}
//////////////////////////////////////////////////////////////////////////

void CollisionEntity::TryDisplayCannonHit(osg::Vec3d hitPt)
{
	// If we are not visible, we do not need to play the cannon hits at all
	if (!IsShowing())
		return;

	// Cannon rounds can play on top of each other, and often are played with sound and particle effects, REALLY dragging down the system
	// We want to see if the hits happen too close to each other, and NOT send those messages.  The damage from DestroyableEntity still happens

	double minTime = 1.0;	// TODO: Get this from a user preference
	double minDistSqr = 16.0;  // TODO: Get this from a user preference.  This is squared
	double oldTime = GetGameClient()->GetTimer().GetCurrTime_s() - minTime;

	// Pop off any old items in the list
	while (!m_cannonHits.empty())
	{
		CannonRoundHitOccurance& cr = m_cannonHits.front();
		if (cr.time < oldTime)
			m_cannonHits.pop_front();
		else 
			break;
	}

	// Loop through the list to see if any of the remaining hits were too close.  They should all be close enough in time.
	std::list<CannonRoundHitOccurance>::iterator pos;
	for (pos = m_cannonHits.begin(); pos != m_cannonHits.end(); ++pos)
	{
		CannonRoundHitOccurance& cr = *pos;
		if ((cr.hitPt-hitPt).length2() < minDistSqr)
			return;	// Too close, do not try to display
	}

	// Here is a new hit to draw and add to the back of the list
	m_cannonHits.push_back(CannonRoundHitOccurance(oldTime+minTime, hitPt));
	GetEventMap()->EventPt_Map["CANNON_HIT"].Fire(hitPt);
}
//////////////////////////////////////////////////////////////////////////

void CollisionEntity::OnCollision(Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s)
{
	// We MAY be getting this from the network
	// Fire the event with the point from the collision
	// Were we hit by a cannon round or did we collide with something?
	CannonRound* cr = dynamic_cast<CannonRound*>(&otherEntity);
	if (cr)
		TryDisplayCannonHit(hitPt);
	else
		GetEventMap()->EventPt_Map["COLLIDE"].Fire(hitPt);
}
//////////////////////////////////////////////////////////////////////////