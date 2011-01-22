// Cannon.cpp
#include "stdafx.h"
#include "GG_Framework.AppReuse.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::AppReuse;
using namespace GG_Framework::Logic;

// The base Entity3D will use the base TransmittedEntity
ClassFactoryT<TransmittedEntity, TransmittedEntity> CannonRound_TransmittedEntity_ClassFactory("CannonRound", TransmittedEntity::ClassFactory);
ClassFactoryT<CannonRound, Entity3D> CannonRound_ClassFactory("CannonRound", Entity3D::ClassFactory);

CannonDesc::CannonDesc() : Placement_NULL_Name(""), Fire_EventName("Ship.FireWeapon"),
Round_Speed(5000.0), Round_Life(1.5), Round_Accuracy(100.0), Round_Damage(1.0), 
Firing_Rate(10), Firing_Offset(0.0), HEAT_UP_TIME(3.0), COOL_DOWN_TIME(10.0), RESTART_LEVEL(0.1), Adjust_Angle_Deg(1.2)
{}

void CannonDesc::LoadFromScript(std::string ownerName, int cannonIndex,
					GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
					GG_Framework::Logic::ITransmittedEntityOwner* entityOwner)
{
	const char* err;
	err = script.GetField("Placement_NULL_Name", &Placement_NULL_Name, NULL, NULL);
	ASSERT_MSG(!err, err);
	std::string Round_OSGV_Filename;
	err = script.GetField("Round_OSGV_Filename", &Round_OSGV_Filename, NULL, NULL);
	ASSERT_MSG(!err, err);
	err = script.GetField("Fire_EventName", &Fire_EventName, NULL, NULL);
	ASSERT_MSG(!err, err);

	err = script.GetField("Round_Speed", NULL, NULL, &Round_Speed);
	ASSERT_MSG(!err, err);
	err = script.GetField("Round_Life", NULL, NULL, &Round_Life);
	ASSERT_MSG(!err, err);
	err = script.GetField("Round_Accuracy", NULL, NULL, &Round_Accuracy);
	ASSERT_MSG(!err, err);
	err = script.GetField("Round_Damage", NULL, NULL, &Round_Damage);
	ASSERT_MSG(!err, err);
	double Round_Mass;
	err = script.GetField("Round_Mass", NULL, NULL, &Round_Mass);
	ASSERT_MSG(!err, err);
	err = script.GetField("Firing_Rate", NULL, NULL, &Firing_Rate);
	ASSERT_MSG(!err, err);
	err = script.GetField("Firing_Offset", NULL, NULL, &Firing_Offset);
	ASSERT_MSG(!err, err);

	script.GetField("HEAT_UP_TIME", NULL, NULL, &HEAT_UP_TIME);
	if (HEAT_UP_TIME <= 0.0)
		HEAT_UP_TIME = 3.0;
	script.GetField("COOL_DOWN_TIME", NULL, NULL, &COOL_DOWN_TIME);
	if (COOL_DOWN_TIME <= 0.0)
		COOL_DOWN_TIME = 8.0;
	script.GetField("RESTART_LEVEL", NULL, NULL, &RESTART_LEVEL);
	if ((RESTART_LEVEL < 0.0) || (RESTART_LEVEL > 1.0))
		RESTART_LEVEL = 0.3;
	script.GetField("Adjust_Angle_Deg", NULL, NULL, &Adjust_Angle_Deg);
	if ((Adjust_Angle_Deg < 0.0) || (Adjust_Angle_Deg > 90.0))
		Adjust_Angle_Deg = 1.5;

	// Create each of the rounds we need
	unsigned numRoundsReady = (unsigned)(Round_Life * Firing_Rate) + 1;
	for (unsigned i = 0; i < numRoundsReady; ++i)
	{
		TransmittedEntity* cannonRoundTE = TransmittedEntity::ClassFactory.Create("CannonRound");
		cannonRoundTE->COLLISION_INDEX = 0;
		cannonRoundTE->CONTROLLING_PLAYER_ID = controllingPlayerID;
		cannonRoundTE->CPP_CLASS = "CannonRound";
		cannonRoundTE->Mass = Round_Mass;
		cannonRoundTE->NAME = GG_Framework::Base::BuildString("%s.C%i.R%i", ownerName.c_str(), cannonIndex, i);
		DEBUG_CANNON_FIRE("ID = %s\n", cannonRoundTE->NAME.c_str());
		cannonRoundTE->OSGV = Round_OSGV_Filename;
		cannonRoundTE->NETWORK_ID = entityOwner->AddTransmittedEntity(cannonRoundTE);
		Round_Magazine_NetID.push_back(cannonRoundTE->NETWORK_ID);
	}
}
//////////////////////////////////////////////////////////////////////////

void CannonDesc::LoadFromBitStream(RakNet::BitStream& bs)
{
	GG_Framework::Logic::Network::ReadString(bs, Placement_NULL_Name);
	GG_Framework::Logic::Network::ReadString(bs, Fire_EventName);

	bs.Read(Round_Speed);
	bs.Read(Round_Life);
	bs.Read(Round_Accuracy);
	bs.Read(Round_Damage);
	bs.Read(Firing_Rate);
	bs.Read(Firing_Offset);
	bs.Read(HEAT_UP_TIME);
	bs.Read(COOL_DOWN_TIME);
	bs.Read(RESTART_LEVEL);
	bs.Read(Adjust_Angle_Deg);

	// Read in the NetID's of all of the rounds in the magazine
	unsigned numRounds;
	bs.Read(numRounds);
	for (unsigned i = 0; i  < numRounds; ++i)
	{
		unsigned netID;
		bs.Read(netID);
		Round_Magazine_NetID.push_back(netID);
	}
}
//////////////////////////////////////////////////////////////////////////

void CannonDesc::WriteToBitStream(RakNet::BitStream& bs)
{
	GG_Framework::Logic::Network::WriteString(bs, Placement_NULL_Name);
	GG_Framework::Logic::Network::WriteString(bs, Fire_EventName);
	bs.Write(Round_Speed);
	bs.Write(Round_Life);
	bs.Write(Round_Accuracy);
	bs.Write(Round_Damage);
	bs.Write(Firing_Rate);
	bs.Write(Firing_Offset);
	bs.Write(HEAT_UP_TIME);
	bs.Write(COOL_DOWN_TIME);
	bs.Write(RESTART_LEVEL);
	bs.Write(Adjust_Angle_Deg);

	// Write out the net ID's of all of the rounds
	unsigned numRounds = Round_Magazine_NetID.size();
	bs.Write(numRounds);
	for (unsigned i = 0; i < numRounds; ++i)
		bs.Write(Round_Magazine_NetID[i]);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void CannonRound::CannonInit(double roundLife_s, ICannonOwner* parent, double hitDamage)
{
	this->ShowNextFrame(false);
	m_roundLife_s = roundLife_s;
	
	// I do not want to check for collisions against my own cannonOwner
	m_parent = parent;
	m_hitDamage = hitDamage;

	// Cannon Rounds NEVER allow physics when receiving messages
	GetRC_Controller().SetRecvIgnorePhysics(true);

	// Listen for hits
	GetEventMap()->Collision.Subscribe(ehl, *this, &CannonRound::OnCollision);
}
//////////////////////////////////////////////////////////////////////////

CannonRound::~CannonRound()
{
	// Delete our controller here so it does not happen lower down
	if (m_CR_RC_Controller)
		delete m_CR_RC_Controller;
	m_CR_RC_Controller = NULL;
	m_RC_Controller = NULL;
}

bool CannonRound::ShouldIgnoreCollisions(const Entity3D* otherEntity) const
{
	// We want to ignore a collision with the cannonOwner that fired us
	return (m_parent && (&(m_parent->ICO_GetSelfEntity3D()) == otherEntity));
}
//////////////////////////////////////////////////////////////////////////

void CannonRound::Fire(double hitDamage, const osg::Vec3d& pos_m, const osg::Quat& att_q, const osg::Vec3d& vel, const osg::Vec3d& accel, double time_s)
{
	// This should not happen unless we are locally controlled
	if (!IsLocallyControlled())
		return;

	m_fireTime_s = time_s;
	m_tumbleAccel = accel;

	// Set my current position
	m_Physics.ResetVectors();
	SetPosAtt(pos_m, att_q);
	m_Physics.SetLinearVelocity(vel);

	// Tell my RC Controller all about it
	GetRC_Controller(); // Just to make sure RC Controller is created
	m_CR_RC_Controller->FireNetwork_FireCR(hitDamage, pos_m, att_q, vel, accel);
}

void CannonRound::RC_Fire(double hitDamage)
{
	// Watch for rounds that are skipping
	m_hitDamage = hitDamage;
	if (m_hitDamage == 0.0)
		m_collisionIndex = -1;
	else
		m_collisionIndex = 0;

	// We are visible now
	ShowNextFrame(true);

	// Set these after setting visible, so we get the messages
	GetRC_Controller().FireNetwork_Event("Fire", HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);
}
//////////////////////////////////////////////////////////////////////////

void CannonRound::GameTimerUpdate(double time_s)
{
	if (IsLocallyControlled())
	{
		if (m_fireTime_s > -1.0)
		{
			if ((time_s-m_fireTime_s) > m_roundLife_s)
			{
				GetRC_Controller().FireNetwork_Event("Expire", HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_CriticalEntityEvents);
				m_fireTime_s = -1.0;
				ShowNextFrame(false);
			}
		}
	}

	__super::GameTimerUpdate(time_s);
}
//////////////////////////////////////////////////////////////////////////

RC_Controller& CannonRound::GetRC_Controller()
{
	if (!m_RC_Controller)
	{
		m_CR_RC_Controller = new CannonRound_RC_Controller(this);
		m_RC_Controller = m_CR_RC_Controller;
	}
	return *m_RC_Controller;
}
//////////////////////////////////////////////////////////////////////////

void CannonRound::TimeChange(double dTime_s)
{
	// Only if we are still moving (and we are locally controlled)
	if (IsLocallyControlled() && (m_fireTime_s > -1.0))
	{
		// Apply a little force for the tumble
		m_Physics.ApplyFractionalForce((m_tumbleAccel)*m_Physics.GetMass(), dTime_s);
	}

	// Call the base class to Run physics update for displacement
	__super::TimeChange(dTime_s);
}
//////////////////////////////////////////////////////////////////////////

void CannonRound::OnCollision(Entity3D& otherEntity, const osg::Vec3d& myHitPt, double dTime_s)
{
	GetEventMap()->Event_Map["Impact"].Fire();

	if (IsLocallyControlled())
	{
		m_fireTime_s = -1.0;
		ShowNextFrame(false);

		// Fire an event along the parent to let everyone know
		m_parent->CannonRoundHit.Fire(&otherEntity);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Cannon::Cannon(unsigned cannonIndex, ICannonOwner& cannonOwner, CannonDesc& desc) 
	: m_cannonOwner(cannonOwner), m_desc(desc), m_currMagIndex(0), m_lastFireTime_s(-2.0), 
	m_timeBetweenRounds_s(1.0 / desc.Firing_Rate), m_tempStatus(CANNON_Cold), m_tempLevel(0.0), m_isFiring(false),
	m_lastTime_s(-1.0), m_cosAdjAngle(cos(DEG_2_RAD(desc.Adjust_Angle_Deg)))
{
	DEBUG_CANNON_FIRE("Cannon::Cannon(...)\n");

	// How many rounds do we need to have read?
	unsigned numRoundsReady = desc.Round_Magazine_NetID.size();
	
	for (unsigned round = 0; round < numRoundsReady; ++round)
	{
		unsigned netID = desc.Round_Magazine_NetID[round];
		CannonRound* cr = dynamic_cast<CannonRound*>(m_cannonOwner.ICO_GetSelfEntity3D().GetGameClient()->FindEntity(netID));
		ASSERT_MSG(cr, "Failed to cast new entity to CannonRound\n");
		cr->CannonInit(desc.Round_Life, &cannonOwner, desc.Round_Damage);
		m_magazine.push_back(cr);
	}

	// Listen for time changes so we can fire the rounds on time
	m_cannonOwner.ICO_GetSelfEntity3D().GetGameClient()->GetLogicTimer().CurrTimeChanged.Subscribe(ehl, *this, &Cannon::TimeChanged);
}
//////////////////////////////////////////////////////////////////////////

void Cannon::OnAsyncLoadComplete()
{
	// Find the node where the cannon resides
	if (m_cannonOwner.ICO_GetSelfEntity3D().Get_BB_Actor())
	{
		m_cannonPositionNode = GG_Framework::UI::OSG::FindChildNode(m_cannonOwner.ICO_GetSelfEntity3D().Get_BB_Actor(), m_desc.Placement_NULL_Name);
	}
}
//////////////////////////////////////////////////////////////////////////

void Cannon::OnFireCannons(bool start)
{
	DEBUG_CANNON_FIRE("Cannon::OnFireCannons(%s)\n", start ? "start" : "stop");
	if (start)
		m_lastFireTime_s = -1;
	m_lastFireTime_s = (start ? -1.0 : -2.0);
}
//////////////////////////////////////////////////////////////////////////

void Cannon::FindRelatedEntities(std::set<Entity3D*>& relatedEntities)
{
	// Insert all my rounds into this list
	std::vector<CannonRound*>::iterator currRound = m_magazine.begin();
	while (currRound != m_magazine.end())
	{
		if (*currRound)
		{
			relatedEntities.insert(*currRound);
			(*currRound)->FindRelatedEntities(relatedEntities);
		}
		++currRound;
	}
}
//////////////////////////////////////////////////////////////////////////

void Cannon::TimeChanged(double time_s)
{
	// Do nothing if not locally owned
	if (!m_cannonOwner.ICO_GetSelfEntity3D().IsLocallyControlled())
		return;

	double dTime_s = time_s - m_lastTime_s;
	m_lastTime_s = time_s;

	// Find the current global and local position
	FindPosition();

	if (m_lastFireTime_s < -1.0)
	{
		// The user does not want to TRY to fire
	}
	else if (m_lastFireTime_s == -1.0)
	{
		// The user JUST pressed the fire button, get ready to fire next frame
		// Set the time so that the cannon will fire at the appropriate time
		m_lastFireTime_s = time_s - m_timeBetweenRounds_s + m_desc.Firing_Offset;
	}
	else if ((time_s - m_lastFireTime_s) > m_timeBetweenRounds_s)
	{
		if (m_tempStatus != CANNON_Overheated)
		{
			m_lastFireTime_s = time_s;
			
			// Fire my next round
			FireRound(m_magazine[m_currMagIndex++], time_s);
			if (m_currMagIndex == m_magazine.size())
				m_currMagIndex = 0;

			// Make the next round come out at a slightly different time
			m_timeBetweenRounds_s = (1.0 / m_desc.Firing_Rate);
		}
	}

	// We are not firing, so we can cool down
	if ((m_lastFireTime_s < -1.0) || (m_tempStatus == CANNON_Overheated))
	{
		// we are NOT firing
		m_tempLevel -= dTime_s / m_desc.COOL_DOWN_TIME;
		if (m_isFiring)
		{
			m_isFiring = false;
			// Fire the actual event name for the OSG to play with it
			m_cannonOwner.ICO_GetSelfEntity3D().GetEventMap()->EventOnOff_Map[m_desc.Fire_EventName].Fire(m_isFiring);
		}
	}
	else
	{
		// We are firing
		if (!m_isFiring)
		{
			m_isFiring = true;
			// Fire the actual event name for the OSG to play with it
			m_cannonOwner.ICO_GetSelfEntity3D().GetEventMap()->EventOnOff_Map[m_desc.Fire_EventName].Fire(m_isFiring);
		}

		// And heating up, set the new temp and status
		m_tempLevel += dTime_s / m_desc.HEAT_UP_TIME;
	}

	// Watch for being below 0 and above 1 (overheated), Set the proper status
	// if overheated, must cool down to m_desc.RESTART_LEVEL
	if (m_tempLevel < 0.0)
		m_tempLevel = 0.0;
	if (m_tempLevel > 1.0)
	{
		m_tempLevel = 1.0;
		m_tempStatus = CANNON_Overheated;
	}
	else if (m_tempLevel < m_desc.RESTART_LEVEL)
		m_tempStatus = CANNON_Cold;
	else if (m_tempStatus != CANNON_Overheated)	// If it gets to be overheated, we have to wait till cool down
	{
		if (m_tempLevel < (0.5 + (m_desc.RESTART_LEVEL * 0.7)))
			m_tempStatus = CANNON_Nominal;
		else
			m_tempStatus = CANNON_Warning;
	}

	const char* tempStatusString;
	switch (m_tempStatus)
	{
	case CANNON_Cold: tempStatusString =       "COLD         "; break;
	case CANNON_Overheated: tempStatusString = "OVERHEATED   "; break;
	case CANNON_Warning: tempStatusString =    "WARNING      "; break;
	case CANNON_Nominal: tempStatusString =    "NOMINAL      "; break;
	}

	DEBUG_CANNON_OVERHEAT("\rCannon Temp = %0.3f: %s", m_tempLevel, tempStatusString);
}
//////////////////////////////////////////////////////////////////////////

void Cannon::FindPosition()
{
	osg::Vec3d ship_absPos_m(m_cannonOwner.ICO_GetSelfEntity3D().GetPos_m());
	osg::Quat ship_absAtt_q(m_cannonOwner.ICO_GetSelfEntity3D().GetAtt_quat());

	if (m_cannonPositionNode.valid())
	{
		// The position node matrix is in a LWS (which is NEVER cached, so is unique for each object), 
		// so we are not worried about a unique parent, and we can go to the top of the scene
		osg::Matrix m = GG_Framework::UI::OSG::GetNodeMatrix(m_cannonPositionNode.get(), null, m_cannonOwner.ICO_GetSelfEntity3D().Get_BB_Actor());
		m_relPos_m = m.getTrans();
		m_relAtt_q = m.getRotate();
	}

	// If a Ship's target position is close enough, we can do some auto-tracking to it
	const osg::Vec3d* targetPos = m_cannonOwner.GetGunTarget();
	if (targetPos)
	{
		// Find the position of the target relative to the cannon
		osg::Vec3d targetRelPos = (ship_absAtt_q.conj() * (*targetPos - ship_absPos_m)) - m_relPos_m;
		targetRelPos.normalize();

		// See which way the cannon is pointing now
		osg::Vec3d cannonDir = m_relAtt_q * osg::Vec3d(0,1,0);

		if ((cannonDir*targetRelPos) > m_cosAdjAngle)
			m_relAtt_q.makeRotate(cannonDir, targetRelPos);
	}

	m_absPos_m = ship_absPos_m + (ship_absAtt_q * m_relPos_m);
	m_absAtt_q = m_relAtt_q * ship_absAtt_q;
}
//////////////////////////////////////////////////////////////////////////

void Cannon::FireRound(CannonRound* round, double time_s)
{
	// Do nothing if not locally owned
	if (!m_cannonOwner.ICO_GetSelfEntity3D().IsLocallyControlled())
		return;

	// Find the velocity
	osg::Vec3d vel = m_cannonOwner.ICO_GetSelfEntity3D().GetLinearVelocity() + (m_absAtt_q * osg::Vec3d(0.0, m_desc.Round_Speed, 0.0));

	// Find a random acceleration to apply to make it offset
	osg::Vec3d accel = 
		osg::Vec3d(RAND_GEN()*RAND_GEN()-0.5, RAND_GEN()*RAND_GEN()-0.5, RAND_GEN()*RAND_GEN()-0.5) * m_desc.Round_Accuracy;

	// Calculate the appropriate damage
	double damage = m_desc.Round_Damage;

	// Fire AWAY!
	round->Fire(damage, m_absPos_m, m_absAtt_q, vel, accel, time_s);
}
//////////////////////////////////////////////////////////////////////////

void CannonRound_RC_Controller::SpecialEntityMessageFromServer(unsigned char msg, double msgTime_s, RakNet::BitStream& entityBS)
{
	if (msg == CannonRd_Fire)
	{
		// Fire the message along
		RecvNetwork_FireCR(entityBS, msgTime_s);

		// No need to pass on
		return;
	}

	// We did not catch it, let our parent class
	__super::SpecialEntityMessageFromServer(msg, msgTime_s, entityBS);
}
//////////////////////////////////////////////////////////////////////////

void CannonRound_RC_Controller::FireNetwork_FireCR
	(float hitDamage, const osg::Vec3d& pos_m, const osg::Quat& att_q, const osg::Vec3d& vel, const osg::Vec3d& accel)
{
	// Pack up this pos info into an update
	m_NetworkPosAtt.POS = pos_m;
	m_NetworkPosAtt.ATT = att_q;
	m_NetworkPosAtt.LinearAcceleration = accel;
	m_NetworkPosAtt.LinearVelocity = vel;

	// Create the message
	RakNet::BitStream bs;
	m_pParent->GetGameClient()->CreateEntityUpdateBitStream(
		bs, HIGH_PRIORITY, RELIABLE_SEQUENCED, OC_PosAttUpdates, m_pParent->GetNetworkID(), CannonRd_Fire);

	// Populate the message
	m_NetworkPosAtt.WriteToBS(bs);
	bs.Write(hitDamage);

	// Post the Message
	m_pParent->GetGameClient()->PostEntityUpdateToServer(bs);

	// Send the message back to my parent
	m_cannonRoundParent->RC_Fire(hitDamage);
}
//////////////////////////////////////////////////////////////////////////

void CannonRound_RC_Controller::RecvNetwork_FireCR(RakNet::BitStream& bs, double msgTime_s)
{
	// Read info back out of the message (much of this is like a ENTITY_VIS_SHOW/ENTITY_POSATT_UPDATE
	m_resetVis = true;
	m_lastTransmittedUpdateTime_s = msgTime_s;
	m_NetworkPosAtt.ReadFromBS(bs);
	float hitDamage;
	bs.Read(hitDamage);

	// Send the message back to my parent
	m_cannonRoundParent->RC_Fire(hitDamage);
}
//////////////////////////////////////////////////////////////////////////
