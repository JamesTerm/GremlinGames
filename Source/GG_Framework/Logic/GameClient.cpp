// GG_Framework.Logic GameClient.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

// The rest of these work from the top down
#undef IGNORE_ALL_COLLISIONS1
#undef IGNORE_ALL_COLLISIONS2
#undef IGNORE_ALL_COLLISIONS3
#undef IGNORE_EXTERNAL_FORCES
#undef IGNORE_COLLISION_EVENTS

using namespace GG_Framework::Logic;
using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

#define __SHOW_LAG_TEST__

GameClient::GameClient(
		  GG_Framework::Logic::Network::IClient& client, 
		  GG_Framework::Base::Timer& logic_timer, const char* contentDirLW) : 
m_client(client), m_actorSceneBB(logic_timer, contentDirLW), m_logicTimer(logic_timer), m_lastTimeUpdate(-1.0),
m_standardUpdatePause(1), CollLogger("Collision"), SendNetLogger("SendNet"), RecvNetLogger("RecvNet")
{
	m_standardUpdateCounter = m_standardUpdatePause;	// Makes it so it draws the first frame
	m_logicTimer.CurrTimeChanged.Subscribe(ehl, *this, &GameClient::GameTimerUpdate);
	IncomingPacketEventMap[ID_PlayersEntity].Subscribe(ehl, *this, &GameClient::NewPlayersEntity);
	IncomingPacketEventMap[ID_AttachPlayerEntity].Subscribe(ehl, *this, &GameClient::AttachNewPlayersEntity);
	IncomingPacketEventMap[ID_SpecialEntityUpdate].Subscribe(ehl, *this, &GameClient::ReceiveSpecialEntityUpdatePacket);
	IncomingPacketEventMap[ID_StandardEntityUpdate].Subscribe(ehl, *this, &GameClient::ReceiveStandardEntityUpdates);
	IncomingPacketEventMap[ID_ChangeEntityController].Subscribe(ehl, *this, &GameClient::ChangeEntityController);

	// Track some profiling times
	CollLogger.SetLogger(m_logicTimer.Logger);
	SendNetLogger.SetLogger(m_logicTimer.Logger);
	RecvNetLogger.SetLogger(m_logicTimer.Logger);

	m_frameStampBB = new osg::FrameStamp;
	m_updateBB.setFrameStamp( m_frameStampBB.get() );
}
//////////////////////////////////////////////////////////////////////////

GameClient::~GameClient()
{
	// Clean up the Entity Map
	EntityMap_Name::iterator pos;
	for (pos = m_entityMap_Name.begin(); pos != m_entityMap_Name.end(); ++pos)
	{
		Entity3D* entity = pos->second;
		Entity3D::ClassFactory.Destroy(entity->GetClassName(), entity);
		pos->second = NULL;
	}
	m_entityMap_Name.clear();
	m_entityMap_NetID.clear();	// Just copies of the same things

	for (unsigned i = 0; i < m_asyncLoadersToCheck.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToCheck[i];
		if (loader)
		{
			// Should be done, or we would not have gotten a message back
			loader->cancel();
			delete loader->entity3D;
			delete loader;
			m_asyncLoadersToCheck[i]=NULL;
		}
	}

	for (unsigned i = 0; i < m_asyncLoadersToAttach.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToAttach[i];
		if (loader)
		{
			// Should be done, or we would not have gotten a message back
			loader->cancel();
			delete loader->entity3D;
			delete loader;
			m_asyncLoadersToAttach[i]=NULL;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void GameClient::GameTimerUpdate(double time_s)
{
	// Get all the packets sent from the server so far
	RecvNetLogger.Start();
	while (true)
	{
		Packet* packet = m_client.AsPacketReceiver()->Receive();
		if (packet)
		{
			FireNetworkPacketEvents(packet);
			m_client.AsPacketReceiver()->DeallocatePacket(packet);
		}
		else
			break;
	}
	RecvNetLogger.End();

	CheckOnAsyncLoaders();
	if (m_lastTimeUpdate > 0.0)
	{
#ifdef __SHOW_LAG_TEST__
		if (Get_UI_ActorScene())
		{
			double frameTime = time_s - m_lastTimeUpdate;
			static double MINFRAME = frameTime;
			static double MINFRAME_T = time_s;	
			if ((MINFRAME > frameTime) || (time_s-MINFRAME_T > 5.0))
			{
				MINFRAME = frameTime;
				MINFRAME_T = time_s;
			}
			static double MAXFRAME = frameTime;
			static double MAXFRAME_T = time_s;
			if ((MAXFRAME < frameTime) || (time_s-MAXFRAME_T > 5.0))
			{
				MAXFRAME = frameTime;
				MAXFRAME_T = time_s;
			}

			static double LAST_FRAMETIME = frameTime;
			double dTime = (LAST_FRAMETIME > frameTime) ? LAST_FRAMETIME-frameTime : frameTime-LAST_FRAMETIME;
			LAST_FRAMETIME = frameTime;
			static double MAXDTIME = dTime;
			static double MAXDTIME_T = time_s;
			if ((MAXDTIME < dTime) || (time_s-MAXDTIME_T > 5.0))
			{
				MAXDTIME = dTime;
				MAXDTIME_T = time_s;
			}

			double offsetFromActTime = time_s - GetLogicTimer().GetSynchronizedActualTime();

			DOUT4("Frame Times=(%f-%f), DTIME=%f, Offset=%f", MINFRAME, MAXFRAME, MAXDTIME, offsetFromActTime);
		}
#endif
		
		m_frameStampBB->setSimulationTime(time_s);
		m_frameStampBB->setFrameNumber(TIME_2_FRAME(time_s));
		m_actorSceneBB.GetScene()->accept(m_updateBB);
	
		CollLogger.Start();
		ProcessCollisions(time_s - m_lastTimeUpdate);
		CollLogger.End();

		// Only send entity updates once the game has really started
		// We may want to pause every few frames
		SendNetLogger.Start();
		++m_standardUpdateCounter;
		if (m_standardUpdateCounter > m_standardUpdatePause)
		{
			m_standardUpdateCounter = 1;
			SendStandardEntityUpdates(time_s);
		}
		SendNetLogger.End();
	}

	m_lastTimeUpdate = time_s;
}
//////////////////////////////////////////////////////////////////////////

void GameClient::SendStandardEntityUpdates(double time_s)
{
	// We are not sending a single update for all of our entities
	RakNet::BitStream entityUpdateBS;

	// We always need the timestamp first
	entityUpdateBS.Write((unsigned char)ID_TIMESTAMP);

	// And the time, that will be converted along the network
	RakNetTime t_ms = GetLogicTimer().ConvertToNetTime(time_s);
	entityUpdateBS.Write(t_ms);

	// And the identifier that says we are sending an Entity Update
	entityUpdateBS.Write((unsigned char)ID_StandardEntityUpdate);

	// Write out the number of flags we are sending
	unsigned int numEntities = m_entityMap_NetID.size();
	entityUpdateBS.Write(numEntities);

	// Loop through all the entities writing flags, first to make sure we are defining the ones we own 
	unsigned netID;
	for (netID = 0; netID < numEntities; ++netID)
	{
		Entity3D* entity = m_entityMap_NetID[netID];
		entityUpdateBS.Write(entity ? entity->IsLocallyControlled() : false);
	}

	// Then, only for the ones we own, write the showing state
	for (netID = 0; netID < numEntities; ++netID)
	{
		Entity3D* entity = m_entityMap_NetID[netID];
		if (entity && entity->IsLocallyControlled())
		{
			entityUpdateBS.Write(entity->IsShowing());
		}
	}

	// Some entities may say there is nothing to update
	bool sending = false;
	for (netID = 0; netID < numEntities; ++netID)
	{
		Entity3D* entity = m_entityMap_NetID[netID];
		if (entity && entity->IsLocallyControlled() && entity->IsShowing())
		{
			sending = true;
			entityUpdateBS.Write(entity->GetRC_Controller().WantsToSendEntityUpdate());
		}
	}

	if (!sending)
		return;	// Nothing is sending, no need to continue (unless something gotten hidden and there is nothine else :O)

	// Let the RC_Controller send their updates
	for (netID = 0; netID < numEntities; ++netID)
	{
		Entity3D* entity = m_entityMap_NetID[netID];
		if (entity && entity->IsLocallyControlled() && entity->IsShowing() && entity->GetRC_Controller().WantsToSendEntityUpdate())
			entity->GetRC_Controller().PopulateStandardEntityUpdate(entityUpdateBS);
	}

	// Send the message, using the appropriate priority, reliability, and ordering channel
	m_client.Send(&entityUpdateBS, HIGH_PRIORITY, UNRELIABLE_SEQUENCED, OC_PosAttUpdates);
}
//////////////////////////////////////////////////////////////////////////

void GameClient::CreateEntityUpdateBitStream
	(RakNet::BitStream& entityUpdateBS, 
	PacketPriority priority, PacketReliability reliability, char orderingChannel,
	unsigned entityNetID, unsigned char msgID, double time_s)
{
	// We always need the timestamp first
	entityUpdateBS.Write((unsigned char)ID_TIMESTAMP);

	// And the time, that will be converted along the network
	if (time_s < 0.0) time_s = GetLogicTimer().GetCurrTime_s();
	RakNetTime t_ms = GetLogicTimer().ConvertToNetTime(time_s);
	entityUpdateBS.Write(t_ms);

	// And the identifier that says we are sending an Entity Update
	entityUpdateBS.Write((unsigned char)ID_SpecialEntityUpdate);
	
	// And information about how to re-broadcast for the server
	unsigned char rebroadcastInfo = Network::CreateRebroadcastInstructions(priority, reliability);
	entityUpdateBS.Write(rebroadcastInfo);
	entityUpdateBS.Write(orderingChannel);

	// And the ID of the entity we are talking to
	entityUpdateBS.Write(entityNetID);

	// And the message we want to push on
	entityUpdateBS.Write(msgID);
}
int REBROADCAST_INFO_POS = sizeof(unsigned char)	// TIMESTAMP
						+ sizeof(RakNetTime)		// TIME
						+ sizeof(unsigned char);	// MSG ID
//////////////////////////////////////////////////////////////////////////

void GameClient::PostEntityUpdateToServer(RakNet::BitStream& entityUpdateBS)
{
	// Find the rebroadcast info and ordering
	unsigned char* data = entityUpdateBS.GetData();
	unsigned char rebroadcastInfo = data[REBROADCAST_INFO_POS];
	PacketPriority priority; PacketReliability reliability;
	Network::FindRebroadcastInstructions(rebroadcastInfo, priority, reliability);
	char orderingChannel = data[REBROADCAST_INFO_POS+1];
	m_client.Send(&entityUpdateBS, priority, reliability, orderingChannel);
}
//////////////////////////////////////////////////////////////////////////

void GameClient::ReceiveStandardEntityUpdates(Packet& packet)
{
	// Create the BitStream to pass to everyone, starting with one byte into the packet, since we know the first byte is ID_TIMESTAMP
	RakNet::BitStream entityUpdateBS(packet.data+sizeof(unsigned char), packet.length-sizeof(unsigned char), false);

	// Read the adjusted timer and convert it to seconds
	RakNetTime t_ms;
	entityUpdateBS.Read(t_ms);
	double msgTime_s = GetLogicTimer().ConvertFromNetTime(t_ms);
	double currTime_s = GetLogicTimer().GetCurrTime_s();
	if (msgTime_s > currTime_s)
		msgTime_s = currTime_s;

#ifdef __SHOW_LAG_TEST__
	if (Get_UI_ActorScene())
	{	
		double lagTime_s = currTime_s - msgTime_s;
		static double MINLAG = 1e6;
		static double MINLAG_T = 0.0;
		if ((MINLAG > lagTime_s) || (currTime_s-MINLAG_T > 5.0))
		{
			MINLAG = lagTime_s;
			MINLAG_T = currTime_s;
		}
		static double MAXLAG = 0.0;
		static double MAXLAG_T = 0.0;
		if ((MAXLAG < lagTime_s) || (currTime_s-MAXLAG_T > 5.0))
		{
			MAXLAG = lagTime_s;
			MAXLAG_T = currTime_s;
		}

		DOUT5("CurrTime = %f, MsgTime = %f, lag=(%f-%f)", currTime_s, msgTime_s, MINLAG, MAXLAG);
	}
#endif

	// Read off the message, should be ID_StandardEntityUpdate
	unsigned char msg;
	entityUpdateBS.Read(msg);
	ASSERT(msg==ID_StandardEntityUpdate);

	// Read through the total number of entities (hopefully this matches our own)
	unsigned int numEntities;
	entityUpdateBS.Read(numEntities);
	if (numEntities == 0) return;	// Just to make sure there are some entities to work with
	char* ReadFlags = new char[numEntities];

	// Work through which entities this player owns (we could check for cheating here)
	unsigned netID;
	for (netID = 0; netID < numEntities; ++netID)
	{
		bool owned; entityUpdateBS.Read(owned);
		ReadFlags[netID] = owned ? 1 : 0;
	}

	// Check for the visible flag
	for (netID = 0; netID < numEntities; ++netID)
	{
		if (ReadFlags[netID])
		{
			bool vis; entityUpdateBS.Read(vis);
			Entity3D* entity = m_entityMap_NetID[netID];
			if (entity)
				entity->GetRC_Controller().ShowParent(vis);
			ReadFlags[netID] = vis ? 1 : 0;
		}
	}

	// Check to see if the controller wants to send a message
	for (netID = 0; netID < numEntities; ++netID)
	{
		if (ReadFlags[netID])
		{
			bool update; entityUpdateBS.Read(update);
			ReadFlags[netID] = update ? 1 : 0;
		}
	}

	// Finally we make the update happen
	for (netID = 0; netID < numEntities; ++netID)
	{
		if (ReadFlags[netID])
		{
			Entity3D* entity = m_entityMap_NetID[netID];
			if (!entity)
				return; // This MIGHT fail while a player is loading, just abort until the next message
			entity->GetRC_Controller().ReadStandardEntityUpdate(entityUpdateBS, msgTime_s);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void GameClient::ReceiveSpecialEntityUpdatePacket(Packet& packet)
{
	// Create the BitStream to pass to everyone
	RakNet::BitStream entityBS(packet.data+sizeof(unsigned char), packet.length-sizeof(unsigned char), false);

	// Read the adjusted timer and convert it to seconds
	RakNetTime t_ms;
	entityBS.Read(t_ms);
	double msgTime_s = GetLogicTimer().ConvertFromNetTime(t_ms);

	// Read off the message, should be ID_SpecialEntityUpdate
	unsigned char msg;
	entityBS.Read(msg);
	ASSERT(msg==ID_SpecialEntityUpdate);

	// We do not need rebroadcast info or ordering channel, it is for the server
	unsigned char rebroadcastInfo;
	entityBS.Read(rebroadcastInfo);
	unsigned char orderingChannel;
	entityBS.Read(orderingChannel);

	// Read in the netID of the entity we want
	unsigned entityNetID;
	entityBS.Read(entityNetID);

	// And the ID of the msg
	entityBS.Read(msg);

	// Tell the Entity's RC_Controller about the rest of the message
	Entity3D* entity = FindEntity(entityNetID);
	ASSERT(entity);
	entity->GetRC_Controller().SpecialEntityMessageFromServer(msg, msgTime_s, entityBS);
}
//////////////////////////////////////////////////////////////////////////

void GameClient::NewPlayersEntity(Packet& packet)
{
	RakNet::BitStream bs(packet.data, packet.length, false);
	unsigned char packetID;
	bs.Read(packetID);
	ASSERT(packetID == (unsigned char)ID_PlayersEntity);
	PlayerID playerID;
	bs.Read(playerID);

	// Read in each of the entities created for this new player
	unsigned numNewEntities;
	bs.Read(numNewEntities);
	for (unsigned index = 0; index < numNewEntities; ++index)
	{
		TransmittedEntity* te = TransmittedEntity::CreateFromBitStream(bs);
		// DEBUG_PLAYER_LOADING("GameClient::NewPlayersEntity() recv %s\n", te->NAME.c_str());
		Entity3D* newEntity = CreateNewEntity(*te, false);
		ThreadedEntityActorLoader* loader = new ThreadedEntityActorLoader(
			Get_UI_ActorScene(), Get_BB_ActorScene(), newEntity, te->OSGV);
		loader->asynchPlayerID = playerID;

#if 0
		// Run synchronously if we have not fully started the scene, but asynchronously if the scene is running
		// We determine this by the time on the timer
		if (GetTimer().GetCurrTime_s() > 0.0)
			loader->start();
		else
#endif	// GAH!!!! It will not work multi-threaded!  :(
			loader->run();

		m_asyncLoadersToCheck.push_back(loader);
		TransmittedEntity::ClassFactory.Destroy(te->CPP_CLASS, te);
	}
}
//////////////////////////////////////////////////////////////////////////

// Look for all of the loaders that are done, and see if all of them for a specific PlayerID are done
void GameClient::CheckOnAsyncLoaders()
{
	for (unsigned i = 0; i < m_asyncLoadersToCheck.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToCheck[i];
		if (loader && (loader->UpdateState()==1))
		{
			if (CheckOnAsyncLoaders(loader->asynchPlayerID, i+1))
				CleanAsyncLoaders(loader->asynchPlayerID);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

// Checks all of the async loaders on or after startIndex, returns true iff they are ALL done
bool GameClient::CheckOnAsyncLoaders(PlayerID playerID, unsigned startIndex)
{
	for (unsigned i = startIndex; i < m_asyncLoadersToCheck.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToCheck[i];
		if (loader && (loader->UpdateState()!=1) && (loader->asynchPlayerID == playerID))
			return false;	// There are still some waiting
	}
	// All must be done
	return true;
}
//////////////////////////////////////////////////////////////////////////

void GameClient::CleanAsyncLoaders(PlayerID playerID)
{
	for (unsigned i = 0; i < m_asyncLoadersToCheck.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToCheck[i];
		if (loader && (loader->asynchPlayerID == playerID))
		{
			// Push it on the other vector to wait to attach
			m_asyncLoadersToAttach.push_back(loader);
			m_asyncLoadersToCheck[i] = NULL;	// No longer on the list to check
		}
	}

	// Send the message back that we are done loading
	DEBUG_PLAYER_LOADING("GameClient::CleanAsyncLoaders(%s) ID_CompletedLoadingOtherPlayerEntity\n", Network::PlayID2Str(playerID).c_str());
	RakNet::BitStream bs;
	bs.Write((unsigned char)ID_CompletedLoadingOtherPlayerEntity);
	bs.Write(playerID);
	m_client.Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad);
}
//////////////////////////////////////////////////////////////////////////

void GameClient::AttachNewPlayersEntity(Packet& packet)
{
	// Get the playerID that we can attach
	RakNet::BitStream bs(packet.data, packet.length, false);
	unsigned char packetID;
	bs.Read(packetID);
	ASSERT(packetID == (unsigned char)ID_AttachPlayerEntity);
	PlayerID playerID;
	bs.Read(playerID);
	DEBUG_PLAYER_LOADING("GameClient::AttachNewPlayersEntity(%s)\n", Network::PlayID2Str(playerID).c_str());

	// Look through all entities ready to attach
	for (unsigned i = 0; i < m_asyncLoadersToAttach.size(); ++i)
	{
		ThreadedEntityActorLoader* loader = m_asyncLoadersToAttach[i];
		if (loader && (loader->asynchPlayerID==playerID))
		{
			// Should be done, or we would not have gotten a message back
			ASSERT(loader->UpdateState()==2);
			Entity3D* entity = loader->Complete();
			ASSERT(entity);
			ASSERT(!entity->GetName().empty());
			ASSERT_MSG((m_entityMap_Name[entity->GetName()] == NULL) || (m_entityMap_Name[entity->GetName()] == entity), 
				BuildString("AttachNewPlayersEntity DUP %s=%x", entity->GetName().c_str(), m_entityMap_Name[entity->GetName()]).c_str());
			m_entityMap_Name[entity->GetName()] = entity;
			// printf("AttachNewPlayersEntity Adding to m_entityMap_Name: %s\n", entity->GetName().c_str());
			m_entityMap_NetID[entity->GetNetworkID()] = entity;
			delete loader;
			m_asyncLoadersToAttach[i]=NULL;
		}
	}
}
//////////////////////////////////////////////////////////////////////////

Entity3D* GameClient::CreateNewEntity(TransmittedEntity& te, bool buildActors)
{
	Entity3D* newEntity = Entity3D::ClassFactory.Create(te.CPP_CLASS);
	if (!newEntity)
		printf("**** ERROR Creating Entity3D of type %s", te.CPP_CLASS.c_str());
	else
	{
		Entity3D::EventMap* newEm = new Entity3D::EventMap(true);
		MapList.push_back(newEm);
		newEntity->Initialize(*this, *newEm, te);
		if (buildActors)
		{
			ThreadedEntityActorLoader loader(
				Get_UI_ActorScene(), Get_BB_ActorScene(), newEntity, te.OSGV);
			loader.run();
			loader.Complete();
		}
		ASSERT(newEntity);
		ASSERT(!te.NAME.empty());
		ASSERT_MSG(m_entityMap_Name[te.NAME] == NULL, 
			BuildString("CreateNewEntity DUP %s=%x", te.NAME.c_str(), m_entityMap_Name[te.NAME]).c_str());
		m_entityMap_Name[te.NAME] = newEntity;
		// printf("Adding to m_entityMap_Name: %s\n", te.NAME.c_str());
		m_entityMap_NetID[te.NETWORK_ID] = newEntity;
	}
	return newEntity;
}
//////////////////////////////////////////////////////////////////////////

bool GameClient::LoadInitialGameData()
{
	DEBUG_PLAYER_LOADING("GameClient::LoadInitialGameData() WAITING\n");
	// Load the Initial Game Data
	bool stillLoading = true;
	bool ret = true;
	while (stillLoading)	//! \todo We should be more careful here about disconnects and such
	{
		//! \todo We also need to be more careful about the kinds of packets received
		Packet* packet = m_client.AsPacketReceiver()->Receive();
		if (packet)
		{
			int packetID = Network::GetPacketIdentifier(packet->data);
			if (packetID == ID_ExistingEntity)
			{
				if (packet->length < 2)
				{
					DEBUG_PLAYER_LOADING("GameClient::LoadInitialGameData() recv *LAST* ID_ExistingEntity\n");
					break;	//! There are no more entities being sent
				}
				RakNet::BitStream bs(packet->data, packet->length, false);
				unsigned char packetID;
				bs.Read(packetID);
				ASSERT(packetID == (unsigned char)ID_ExistingEntity);
				TransmittedEntity* te = TransmittedEntity::CreateFromBitStream(bs);
				// DEBUG_PLAYER_LOADING("GameClient::LoadInitialGameData() recv %s\n", te->NAME.c_str());
				CreateNewEntity(*te, true);
				TransmittedEntity::ClassFactory.Destroy(te->CPP_CLASS, te);
			}
			else if (packetID == ID_ConnectPermissionDenied)
			{
				ret = false;
				stillLoading = false;
			}
			else if (packetID == ID_SpecialEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This should not happen here because we are blocked by the server until we say we are done
			}
			else
			{
				// For other messages, we want to pass them to the events
				// This should not happen here because we are blocked by the server until we say we are done
				FireNetworkPacketEvents(packet);
			}
			m_client.AsPacketReceiver()->DeallocatePacket(packet);
		}
		// sleep only if this is a Mock ID (on a thread)
		if (GetMyPlayerID() == Network::MOCK_SINGLE_PLAYER_ID)
			GG_Framework::Base::ThreadSleep(1);
	}
	DEBUG_PLAYER_LOADING("GameClient::LoadInitialGameData() COMPLETE\n");
	return ret;
}
//////////////////////////////////////////////////////////////////////////

Entity3D* GameClient::FindEntity(std::string name)
{
	EntityMap_Name::const_iterator i = m_entityMap_Name.find(name);

	if (i == m_entityMap_Name.end())
		return NULL;
	else
		return i->second;
}
//////////////////////////////////////////////////////////////////////////

Entity3D* GameClient::FindEntity(unsigned networkID)
{
	EntityMap_NetID::const_iterator i = m_entityMap_NetID.find(networkID);

	if (i == m_entityMap_NetID.end())
		return NULL;
	else
		return i->second;
}
//////////////////////////////////////////////////////////////////////////

// This is some complex and critical logic I would like to test independently
bool ShouldCollide(	int C_collIndex, int C_netID, bool C_localCtrl, bool C_playerControl, 
				   int B_collIndex, int B_netID, bool B_localCtrl, bool B_playerControl)
{
	// Make sure the collider is big enough (already checked in imp, but good here to make sure in testing)
	if (C_collIndex < 0)
		return false;

	// Do not try to hit bullets
	if (B_collIndex < 1)
		return false;

	// Watch for collision indexes (not this collision anyway, will be handled when we try it the other way)
	if (C_collIndex > B_collIndex)
		return false;
	if ((C_collIndex == B_collIndex) && (C_netID >= B_netID))
		return false;

	// We have already handled size issues
	
	// If I control the (smaller) collider as an AI, we can do it here
	if (C_localCtrl && C_playerControl)
		return true;

	// If I control the big ship, we can do it here, as long as the collider is not player controlled (by someone else)
	if (B_localCtrl && B_playerControl && !C_playerControl) // Watch for player controlling collider, do it there
		return true;

	// If the collider is not locally controlled, do it where the (smaller) collider is
	if (!C_localCtrl)
		return false;

	// If the either ship is controlled by a player, but not here, do it there
	if ((!C_localCtrl && C_playerControl) || (!B_localCtrl && B_playerControl))
		return false;

	// I can do it here
	return true;
}

#ifdef _DEBUG
// Unit Testing the logic for 
class ShouldCollide_Tester
{
	struct ShouldCollide_Tester_Entity
	{
		ShouldCollide_Tester_Entity(int ci, int id, int cc, bool pc) :
			CollIndex(ci), NetID(id), Control_Client(cc), PlayerControlled(pc) {}

		int CollIndex;
		int NetID;
		int Control_Client;
		bool PlayerControlled;
	};

	// ASSERTS That Only ONE Client does the check and returns the client, or -1 if none of them.  Checks both directions
	int FindCollidingClient(const ShouldCollide_Tester_Entity& e1, ShouldCollide_Tester_Entity& e2, int numClients)
	{
		int ret = -1;
		for (int client = 0; client < numClients; ++client)
		{
			if (ShouldCollide(	e1.CollIndex, e1.NetID, e1.Control_Client==client, e1.PlayerControlled,
								e2.CollIndex, e2.NetID, e2.Control_Client==client, e2.PlayerControlled))
			{
				ASSERT(ret==-1);
				ret=client;
			}
			if (ShouldCollide(	e2.CollIndex, e2.NetID, e2.Control_Client==client, e2.PlayerControlled,
								e1.CollIndex, e1.NetID, e1.Control_Client==client, e1.PlayerControlled))
			{
				ASSERT(ret==-1);
				ret=client;
			}
		}
		return ret;
	}
public:
	ShouldCollide_Tester()
	{
		// Use this to cound entities
		int netID = 0;

		// The First Big Ship has a bullet and a missile out there, Then its collision entity, then itself
			ShouldCollide_Tester_Entity AI_Ramora_Bullet	(0, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Ramora_Missile	(1, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Ramora_Dest		(4, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Ramora			(4, netID++, 0, false);

		// Player 1 has a Vexhall
			ShouldCollide_Tester_Entity AI_Vexhall_Bullet	(0, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Vexhall_Missile	(1, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Vexhall_Dest		(2, netID++, 0, false);
			ShouldCollide_Tester_Entity AI_Vexhall			(2, netID++, 0, false);

		// Player 1 has a Karackus
			ShouldCollide_Tester_Entity UI_Karackus_Bullet	(0, netID++, 1, false);
			ShouldCollide_Tester_Entity UI_Karackus_Missile	(1, netID++, 1, false);
			ShouldCollide_Tester_Entity UI_Karackus_Dest	(5, netID++, 1, false);
			ShouldCollide_Tester_Entity UI_Karackus			(5, netID++, 1, true);

		// Player 2 has a Q33
			ShouldCollide_Tester_Entity UI_Q33_Bullet		(0, netID++, 2, false);
			ShouldCollide_Tester_Entity UI_Q33_Missile		(1, netID++, 2, false);
			ShouldCollide_Tester_Entity UI_Q33_Dest			(2, netID++, 2, false);
			ShouldCollide_Tester_Entity UI_Q33				(2, netID++, 2, true);

		// Make sure the collisions are ok
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Ramora_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Ramora_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Ramora_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Ramora, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Vexhall_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Vexhall_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Karackus_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Karackus_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Q33_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Q33_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Bullet, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Ramora_Missile, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Ramora_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Ramora, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Vexhall_Bullet, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Vexhall_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Karackus_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Q33_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Missile, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Ramora_Dest, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Ramora, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Vexhall_Bullet, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Vexhall_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Karackus_Missile, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Q33_Dest, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora_Dest, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Ramora, AI_Ramora, 3) == -1);
		ASSERT (FindCollidingClient(AI_Ramora, AI_Vexhall_Bullet, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora, AI_Vexhall_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Karackus_Missile, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Q33_Dest, 3) == 2);
		ASSERT (FindCollidingClient(AI_Ramora, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, AI_Vexhall_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, AI_Vexhall_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Karackus_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Karackus_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Q33_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Q33_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Bullet, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Vexhall_Missile, AI_Vexhall_Missile, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, AI_Vexhall_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Karackus_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Q33_Missile, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Missile, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Vexhall_Dest, AI_Vexhall_Dest, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, AI_Vexhall, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Karackus_Missile, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall_Dest, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(AI_Vexhall, AI_Vexhall, 3) == -1);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Karackus_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Karackus_Missile, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Karackus_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Q33_Dest, 3) == 0);
		ASSERT (FindCollidingClient(AI_Vexhall, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Karackus_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Karackus_Missile, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Karackus_Dest, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Q33_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Q33_Missile, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Q33_Dest, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Bullet, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Karackus_Missile, 3) == -1);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Karackus_Dest, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Q33_Missile, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Q33_Dest, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Missile, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Karackus_Dest, 3) == -1);
		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Karackus, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Q33_Bullet, 3) == 2);
		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Q33_Dest, 3) == 2);
		ASSERT (FindCollidingClient(UI_Karackus_Dest, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Karackus, UI_Karackus, 3) == -1);
		ASSERT (FindCollidingClient(UI_Karackus, UI_Q33_Bullet, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus, UI_Q33_Missile, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus, UI_Q33_Dest, 3) == 1);
		ASSERT (FindCollidingClient(UI_Karackus, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Q33_Bullet, UI_Q33_Bullet, 3) == -1);
		ASSERT (FindCollidingClient(UI_Q33_Bullet, UI_Q33_Missile, 3) == 2);
		ASSERT (FindCollidingClient(UI_Q33_Bullet, UI_Q33_Dest, 3) == 2);
		ASSERT (FindCollidingClient(UI_Q33_Bullet, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Q33_Missile, UI_Q33_Missile, 3) == -1);
		ASSERT (FindCollidingClient(UI_Q33_Missile, UI_Q33_Dest, 3) == 2);
		ASSERT (FindCollidingClient(UI_Q33_Missile, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Q33_Dest, UI_Q33_Dest, 3) == -1);
		ASSERT (FindCollidingClient(UI_Q33_Dest, UI_Q33, 3) == 2);

		ASSERT (FindCollidingClient(UI_Q33, UI_Q33, 3) == -1);
				
	}
};
ShouldCollide_Tester sc_Tester;

#endif

void GameClient::ProcessCollisions(double dTime_s)
{
#ifdef IGNORE_ALL_COLLISIONS1
	return;
#endif

	if (TEST_IGNORE_COLLISIONS)
		return;

	// No collision tests if there was no change in time
	if (dTime_s <= 0.0)
		return;

	// Then loop through each one looking for collisions
	{
		EntityMap_Name::iterator colliderPos;
		for (colliderPos = m_entityMap_Name.begin(); colliderPos != m_entityMap_Name.end(); ++colliderPos)
		{
			Entity3D* collider = (*colliderPos).second;
			
			// Ignore colliders that are not showing, or have a collision index of less than 0, or are not locally controlled
			if (collider->IsShowing() && (collider->GetCollisionIndex() > -1))
			{
				Entity3D* collided = NULL;
				EntityMap_Name::iterator bigShipPos;
				for (bigShipPos = m_entityMap_Name.begin(); (bigShipPos != m_entityMap_Name.end()) && !collided; ++bigShipPos)
				{
					// This checks all the obvious stuff
					Entity3D* bigShip = (*bigShipPos).second;
					if (	(collider->m_collidedLastFrame != bigShip) &&
							(collider != bigShip) &&
							bigShip->IsShowing() &&
							(bigShip->Get_BB_Actor() != null) &&
							!collider->ShouldIgnoreCollisions(bigShip) &&
							!bigShip->ShouldIgnoreCollisions(collider))
					{
						if (ShouldCollide(collider->GetCollisionIndex(), collider->GetNetworkID(), collider->IsLocallyControlled(), collider->IsPlayerControlled(),
										  bigShip->GetCollisionIndex(), bigShip->GetNetworkID(), bigShip->IsLocallyControlled(), bigShip->IsPlayerControlled()))
						{
							if (ProcessCollision(collider, bigShip, dTime_s))
							{
								collided = bigShip;
							}
						}
					}
				}
				
				// Remember for the next frame
				collider->m_collidedLastFrame = collided;
			}
			else
				collider->m_collidedLastFrame = NULL;
		}
	}
}
//////////////////////////////////////////////////////////////////////////
#undef __UseCollisionVersion1__

#ifdef __UseCollisionVersion1__
typedef osgUtil::Hit hit_type;
#else
typedef osgUtil::LineSegmentIntersector::Intersection hit_type;
#endif

void ApplyExternalForces(hit_type& bigShipHit, Entity3D& bigShip, Entity3D& collider, osg::Vec3d Collider_PointOfEntry, double dTime_s)
{
#ifdef IGNORE_EXTERNAL_FORCES
	return;
#endif

	//from en.wikipedia.org/wiki/Elastic_collision
	//To solve an equation involving two colliding bodies in two-dimensions, the overall velocity of each body must be split into two perpendicular 
	//velocities: one tangent to the common normal surfaces of the colliding bodies at the point of contact, the other along the line of collision. 
	//Since the collision only imparts force along the line of collision, the velocities that are tangent to the point of collision do not change. 
	//The velocities along the line of collision can then be used in the same equations as a one-dimensional collision. The final velocities can then
	//be calculated from the two new component velocities and will depend on the point of collision

	//Check out www.engineershandbook.com/Tables/frictioncoefficients.htm
	const double KineticFrictionScalar=0.0; 
	// To keep things simple for now, lets just use no friction
	// 0.42; //steel (hard) against steel (hard) under sliding on the table of this link

	//First break apart the big ship's velocity and apply to collider
	osg::Vec3d SurfaceNormal=bigShipHit.getWorldIntersectNormal();
	osg::Vec3d BigShip_LineOfCollisionVelocity;
	{
		double Magnitude = fabs( bigShip.GetLinearVelocity() * SurfaceNormal);
		BigShip_LineOfCollisionVelocity = SurfaceNormal * Magnitude;
	}
	//TODO apply friction to this
	osg::Vec3d BigShip_Tangential_Velocity=bigShip.GetLinearVelocity()-BigShip_LineOfCollisionVelocity;


	//Now to break apart the collider's velocity and apply to the big ship
	SurfaceNormal=-SurfaceNormal;  //Now points inside the big ship
	osg::Vec3d Collider_LineOfCollisionVelocity;
	{
		double Magnitude=fabs( collider.GetLinearVelocity()*SurfaceNormal);
		Collider_LineOfCollisionVelocity= SurfaceNormal*Magnitude;
	}
	//TODO apply friction to this
	osg::Vec3d Collider_Tagential_Velocity=collider.GetLinearVelocity()-Collider_LineOfCollisionVelocity;

	//Now we can use the line of collision velocity as a one-dimension head-one elastic collision
	osg::Vec3d BigShipFinalVelocity=bigShip.GetPhysics().GetVelocityFromCollision(BigShip_LineOfCollisionVelocity,collider.GetMass(),Collider_LineOfCollisionVelocity);
	osg::Vec3d ColliderFinalVelocity=collider.GetPhysics().GetVelocityFromCollision(Collider_LineOfCollisionVelocity,bigShip.GetMass(),BigShip_LineOfCollisionVelocity);

	//Now we can complete the final velocities by adding the tangential velocities!
	//This is why I have combined both into one callback
	ColliderFinalVelocity+=Collider_Tagential_Velocity;
	BigShipFinalVelocity+=BigShip_Tangential_Velocity;

	// The collision point is local and needs to be rotated globally
	Collider_PointOfEntry = collider.GetAtt_quat() * Collider_PointOfEntry;
	osg::Vec3d BigShip_PointOfEntry= (bigShipHit.getWorldIntersectPoint() - bigShip.GetPos_m()); //keep it in global orientation

	// Now we can apply the force
	osg::Vec3d colliderForce = collider.GetPhysics().GetForceFromVelocity(ColliderFinalVelocity,dTime_s);
	osg::Vec3d bigShipForce = bigShip.GetPhysics().GetForceFromVelocity(BigShipFinalVelocity,dTime_s);
	collider.GetRC_Controller().FireNetwork_ExternalForce(colliderForce,Collider_PointOfEntry,dTime_s);
	bigShip.GetRC_Controller().FireNetwork_ExternalForce(bigShipForce,BigShip_PointOfEntry,dTime_s);
}
//////////////////////////////////////////////////////////////////////////


struct CollisionSegment
{
	osg::Vec3d Start;
	osg::Vec3d End;
	double Dist;
	bool operator < (const CollisionSegment& rhs) {return Dist < rhs.Dist;}
};

bool GameClient::ProcessCollision(Entity3D* collider, Entity3D* bigShip, double dTime_s)
{
	struct Internal
	{
		bool ProcessCollisionSegment_List_v1(std::list<CollisionSegment> &collisionSegmentList,Entity3D* collider, Entity3D* bigShip, double dTime_s)
		{
			osg::Vec3d colliderCurrPos(collider->GetPos_m());
			osg::Quat colliderCurrAtt(collider->GetAtt_quat());

			// Loop through the line segments (ordered) to find the first one that intersects
			std::list<CollisionSegment>::iterator colliderPos;
			osgUtil::IntersectVisitor::HitList localHits;
			for (colliderPos = collisionSegmentList.begin(); colliderPos != collisionSegmentList.end(); ++colliderPos)
			{
				CollisionSegment& cs(*colliderPos);
				GG_Framework::UI::OSG::LineSegmentIntersectVisitor collisionDetector;
				localHits = collisionDetector.getIntersections(bigShip->Get_BB_Actor(), cs.Start, cs.End);
				if (!localHits.empty())
				{
#ifdef DEBUG_COLLISION_DETECTION
					if (collider->Get_UI_Actor())
					{
						DEBUG_COLLISION_DETECTION("GameClient::ProcessCollision(%s, %s) ** COLLIDE\n", collider->GetName().c_str(), bigShip->GetName().c_str());
						DEBUG_COLLISION_DETECTION("\tStart(%4.1f, %4.1f, %4.1f), End(%4.1f, %4.1f, %4.1f)\n", 
							cs.Start[0], cs.Start[1], cs.Start[2], cs.End[0], cs.End[1], cs.End[2]);
						osg::Vec3d actorPos = bigShip->GetPos_m();
						double radius = bigShip->Get_BB_Actor()->getBound().radius();
						DEBUG_COLLISION_DETECTION("\tCenter(%4.1f, %4.1f, %4.1f), radius = %4.1f\n", 
							actorPos[0], actorPos[1], actorPos[2], radius);
					}
#endif // DEBUG_COLLISION_DETECTION

					// WE had a collision!  YEAH!!!   Here is the position on the big ship
					osgUtil::Hit& hit = *(localHits.begin());

					// Place the collider back where the collision happened
					osg::Vec3d worldIntersect(hit.getWorldIntersectPoint());
					osg::Vec3d newColliderPos = colliderCurrPos + (worldIntersect - cs.End);

					GG_Framework::Base::ReleaseDebugFile dbf;
					dbf.MESSAGE.append("ProcessCollisionSegment_List_v1 \n");
					collider->SetPosAtt(newColliderPos, colliderCurrAtt);

					// Remember where the collider hit, make it local to the collider
					osg::Vec3d localColliderPos = 
						colliderCurrAtt.conj() * (worldIntersect - newColliderPos);

					#ifdef __UseCollisionVersion1__
					// Apply the external forces
					ApplyExternalForces(hit, *bigShip, *collider, localColliderPos, dTime_s);
					#endif

					// Work out the big ship collision point
					osg::Vec3d bigShipHitPt = bigShip->GetAtt_quat().conj() * (hit.getWorldIntersectPoint() - bigShip->GetPos_m());

#ifndef IGNORE_COLLISION_EVENTS
					// Fire some events
					collider->GetRC_Controller().FireNetwork_Collision(*bigShip, localColliderPos, dTime_s);
					bigShip->GetRC_Controller().FireNetwork_Collision(*collider, bigShipHitPt, dTime_s);
#endif // IGNORE_COLLISION_EVENTS

					// No need to search farther
					return true;
				}
			}
			return false;
		}

		bool ProcessCollisionSegment_List_v2(std::list<CollisionSegment> &collisionSegmentList,Entity3D* collider, Entity3D* bigShip, double dTime_s)
		{
			osg::Vec3d colliderCurrPos(collider->GetPos_m());
			osg::Quat colliderCurrAtt(collider->GetAtt_quat());

			// Loop through the line segments (ordered) to find the first one that intersects
			std::list<CollisionSegment>::iterator colliderPos;
			for (colliderPos = collisionSegmentList.begin(); colliderPos != collisionSegmentList.end(); ++colliderPos)
			{
				CollisionSegment& cs(*colliderPos);
				osg::ref_ptr<osgUtil::LineSegmentIntersector> collisionDetector=new osgUtil::LineSegmentIntersector( cs.Start, cs.End);
				osgUtil::IntersectionVisitor iv(collisionDetector.get());
				bigShip->Get_BB_Actor()->accept(iv);
				if (!collisionDetector->containsIntersections()) continue;

				osgUtil::LineSegmentIntersector::Intersections &localHits = collisionDetector->getIntersections();
				if (!localHits.empty())
				{
#ifdef DEBUG_COLLISION_DETECTION
					if (collider->Get_UI_Actor())
					{
						DEBUG_COLLISION_DETECTION("GameClient::ProcessCollision(%s, %s) ** COLLIDE\n", collider->GetName().c_str(), bigShip->GetName().c_str());
						DEBUG_COLLISION_DETECTION("\tStart(%4.1f, %4.1f, %4.1f), End(%4.1f, %4.1f, %4.1f)\n", 
							cs.Start[0], cs.Start[1], cs.Start[2], cs.End[0], cs.End[1], cs.End[2]);
						osg::Vec3d actorPos = bigShip->GetPos_m();
						double radius = bigShip->Get_BB_Actor()->getBound().radius();
						DEBUG_COLLISION_DETECTION("\tCenter(%4.1f, %4.1f, %4.1f), radius = %4.1f\n", 
							actorPos[0], actorPos[1], actorPos[2], radius);
					}
#endif // DEBUG_COLLISION_DETECTION

					// WE had a collision!  YEAH!!!   Here is the position on the big ship
					osgUtil::LineSegmentIntersector::Intersection& hit = *(localHits.begin());

					// Place the collider back where the collision happened
					osg::Vec3d worldIntersect(hit.getWorldIntersectPoint());
					osg::Vec3d newColliderPos = colliderCurrPos + (worldIntersect - cs.End);

					GG_Framework::Base::ReleaseDebugFile dbf;
					dbf.MESSAGE.append("ProcessCollisionSegment_List_v2 \n");
					collider->SetPosAtt(newColliderPos, colliderCurrAtt);

					// Remember where the collider hit, make it local to the collider
					osg::Vec3d localColliderPos = 
						colliderCurrAtt.conj() * (worldIntersect - newColliderPos);

					#ifndef __UseCollisionVersion1__
					ApplyExternalForces(hit, *bigShip, *collider, localColliderPos, dTime_s);
					#endif

					// Work out the big ship collision point
					osg::Vec3d bigShipHitPt = bigShip->GetAtt_quat().conj() * (hit.getWorldIntersectPoint() - bigShip->GetPos_m());

#ifndef IGNORE_COLLISION_EVENTS
					// Fire some events
					collider->GetRC_Controller().FireNetwork_Collision(*bigShip, localColliderPos, dTime_s);
					bigShip->GetRC_Controller().FireNetwork_Collision(*collider, bigShipHitPt, dTime_s);
#endif // IGNORE_COLLISION_EVENTS

					// No need to search farther
					return true;
				}
			}
			return false;
		}

	} Internal;


#ifdef IGNORE_ALL_COLLISIONS2
	return false;
#endif

	// ASSERT. if we have gotten this far, we know we SHOULD process this collision and
	// all aspects of the collision are in order (See GameClient::ProcessCollisions())

	// Find the current and previous positions of both entities
	osg::Vec3d colliderCurrPos(collider->GetPos_m());
	osg::Vec3d colliderLinearVel(collider->GetPhysics().GetLinearVelocity());
	osg::Vec3d colliderPrevPos(colliderCurrPos - (colliderLinearVel * dTime_s));
	osg::Vec3d bigShipCurrPos(bigShip->GetPos_m());
	osg::Vec3d bigShipLinearVel(bigShip->GetPhysics().GetLinearVelocity());
	osg::Vec3d bigShipPrevPos(bigShipCurrPos - (bigShipLinearVel * dTime_s));

	// Do a quick bounding sphere check 
	// (I wonder if there is something tighter than this)
	double checkRadius2 = 
		collider->GetBoundRadius2() + bigShip->GetBoundRadius2() + 
		(colliderCurrPos-colliderPrevPos).length2() + (bigShipCurrPos - bigShipPrevPos).length2();
	if ((bigShipCurrPos - colliderCurrPos).length2() > checkRadius2)
		return false;

	// Find the current and previous attitudes of the collider
	osg::Quat colliderCurrAtt(collider->GetAtt_quat());
	osg::Quat colliderPrevAtt;
	{
		osg::Vec3d rot = collider->GetPhysics().GetAngularVelocity() * -dTime_s;
		colliderPrevAtt = FromLW_Rot_Radians(rot[0],rot[1],rot[2]) * colliderCurrAtt;
	}

	// We only need the difference in attitude from the big ship (to account for its rotation
	osg::Quat bigShipDiffAtt;
	{
		osg::Vec3d rot = bigShip->GetPhysics().GetAngularVelocity() * -dTime_s;
		bigShipDiffAtt = FromLW_Rot_Radians(rot[0],rot[1],rot[2]);
	}
	

	// Pre-Process the list of collision line segments, we may want to sort them
	std::list<CollisionSegment> collisionSegmentList;


	// Loop through all items in the collision list to prepare the line segments
	if (collider->m_collisionPoints.empty())
	{
		// There is only one collision point, and that is the center of the collider
		CollisionSegment cs;
		cs.End = colliderCurrPos;
		cs.Start = colliderPrevPos;

		// Offset the start based on the recent motions of the big ship
		{
			// Offset cs.Start based on the previous bigShip pos, also makes it relative to curr big ship pos
			cs.Start -= bigShipPrevPos;

			// Rotate the start point around based on the rotation of the big ship
			cs.Start = bigShipDiffAtt * cs.Start;

			// Make it global again by adding back the big ship global position
			cs.Start += bigShipCurrPos;
		}

		// We do not care about the sort
		cs.Dist = 0.0;

		collisionSegmentList.push_back(cs);
	}
	else
	{
		// Remember the relative velocities of the two ships to help with the sort
		osg::Vec3d relVel(colliderLinearVel - bigShipLinearVel);

		// There are several potential collision points, find them all and sort them
		for (unsigned i = 0; i < collider->m_collisionPoints.size(); ++i)
		{
			CollisionSegment cs;
			osg::Vec3d localPos = GG_Framework::UI::OSG::GetNodePosition(
				collider->m_collisionPoints[i], collider->Get_BB_Actor(), collider->Get_BB_Actor());
			cs.End = (colliderCurrAtt*localPos) + colliderCurrPos;
			cs.Start = (colliderPrevAtt*localPos) + colliderPrevPos;

			// Offset the start based on the recent motions of the big ship
			{
				// Offset cs.Start based on the previous bigShip pos, also makes it relative to curr big ship pos
				cs.Start -= bigShipPrevPos;

				// Rotate the start point around based on the rotation of the big ship
				cs.Start = bigShipDiffAtt * cs.Start;

				// Make it global again by adding back the big ship global position
				cs.Start += bigShipCurrPos;
			}

			// Base the sort on the direction of the points relative to the velocities
			cs.Dist = (cs.End - colliderCurrPos) * relVel;

			collisionSegmentList.push_back(cs);
		}

		// Sort them all based on the distance
		collisionSegmentList.sort();
	}
	bool ret=false;
#ifndef IGNORE_ALL_COLLISIONS3
	#ifdef __UseCollisionVersion1__
	ret=Internal.ProcessCollisionSegment_List_v1(collisionSegmentList,collider, bigShip, dTime_s);
	#else
	ret=Internal.ProcessCollisionSegment_List_v2(collisionSegmentList,collider, bigShip, dTime_s);
	#endif
#endif // IGNORE_ALL_COLLISIONS3

	// There were no collisions
	return ret;
}
//////////////////////////////////////////////////////////////////////////

void GameClient::FireNetworkPacketEvents( Packet* packet )
{
	// Here is where we fire the various events for the various packets
	IncomingPacketEvent.Fire(*packet);
	int packetID = Network::GetPacketIdentifier(packet->data);
	IncomingPacketEventMap[packetID].Fire(*packet);
}
//////////////////////////////////////////////////////////////////////////

void GameClient::ChangeEntityController(Packet& packet)
{//printf("GameClient::ChangeEntityController\n");
	// Create the BitStream to pass to everyone, starting with one byte into the packet, since we know the first byte is ID_TIMESTAMP
	RakNet::BitStream recvControlBS(packet.data+sizeof(unsigned char), packet.length-sizeof(unsigned char), false);

	// Read the adjusted timer and convert it to seconds
	RakNetTime t_ms;
	recvControlBS.Read(t_ms);
	double msgTime_s = GetLogicTimer().ConvertFromNetTime(t_ms);
	double currTime_s = GetLogicTimer().GetCurrTime_s();
	if (msgTime_s > currTime_s)
		msgTime_s = currTime_s;

	// Read off the message, should be ID_ChangeEntityController
	unsigned char msg;
	recvControlBS.Read(msg);
	ASSERT(msg==ID_ChangeEntityController);

	// We do not need rebroadcast info or ordering channel, it is for the server
	unsigned char rebroadcastInfo;
	recvControlBS.Read(rebroadcastInfo);
	unsigned char orderingChannel;
	recvControlBS.Read(orderingChannel);

	// Read in the PlayerID that wants it
	PlayerID playerThatWantsControl; recvControlBS.Read(playerThatWantsControl);
	bool sameOwner;  recvControlBS.Read(sameOwner);

	// How many elements are changing?
	unsigned numElements; recvControlBS.Read(numElements);

	// Loop through each one
	for (unsigned i = 0; i < numElements; ++i)
	{
		unsigned netID; recvControlBS.Read(netID);
		Entity3D* thisEntity = FindEntity(netID);
		ASSERT(thisEntity);	// Not sure what to do on a failure here
		thisEntity->RecvFinalUpdate(recvControlBS, playerThatWantsControl, msgTime_s, sameOwner);
	}
}
//////////////////////////////////////////////////////////////////////////

void GameClient::SendChangeEntityController(Entity3D* tryEntity, const PlayerID& playerThatWantsControl)
{//printf("GameClient::SendChangeEntityController\n");
	// Make sure this is a real entity that I am controlling
	if (tryEntity)
	{
		// We are going to broadcast another message to everyone to switch control
		RakNet::BitStream sendControlBS;

		// We always need the timestamp first
		sendControlBS.Write((unsigned char)ID_TIMESTAMP);

		// And the time, that will be converted along the network
		RakNetTime t_ms = GetLogicTimer().ConvertToNetTime(GetLogicTimer().GetCurrTime_s());
		sendControlBS.Write(t_ms);

		// Write out what we are sending and info about the ServerManager re-sending it
		sendControlBS.Write((unsigned char)ID_ChangeEntityController);
		unsigned char rebroadcastInfo = Network::CreateRebroadcastInstructions(HIGH_PRIORITY, RELIABLE);
		sendControlBS.Write(rebroadcastInfo);
		sendControlBS.Write((unsigned char)OC_CriticalEntityUpdates);

		// Send the playerID with the new PlayerID
		sendControlBS.Write(playerThatWantsControl);

		// AM I sending the final updates (only if I am the owner)
		bool sameOwner = (tryEntity->GetControllingPlayerID() == GetMyPlayerID());
		sendControlBS.Write(sameOwner);

		// Find all of the related entities
		std::set<Entity3D*> relatedEntities;
		relatedEntities.insert(tryEntity);
		tryEntity->FindRelatedEntities(relatedEntities);
		//printf("Found %i related entities\n", relatedEntities.size());

		// Write out the number of entities
		sendControlBS.Write((unsigned)relatedEntities.size());

		// We want to get all of the final information from this entity onto the stream and tell the entity its new controller
		std::set<Entity3D*>::iterator thisEntity = relatedEntities.begin();
		while (thisEntity != relatedEntities.end())
		{
			// Wait to do the tried entity last
			if (*thisEntity != tryEntity)
			{
				sendControlBS.Write((*thisEntity)->GetNetworkID());
				(*thisEntity)->SendFinalUpdate(playerThatWantsControl, sendControlBS, sameOwner);
			}
			thisEntity++;
		}

		// And finally the one we are trying to pilot
		sendControlBS.Write(tryEntity->GetNetworkID());
		tryEntity->SendFinalUpdate(playerThatWantsControl, sendControlBS, sameOwner);

		// Send it
		//printf("Posting ID_ChangeEntityController\n");
		m_client.Send(&sendControlBS, HIGH_PRIORITY, RELIABLE, OC_CriticalEntityUpdates);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

AI_GameClient::AI_GameClient(
			  GG_Framework::Logic::Network::IClient& client, 
			  GG_Framework::Base::Timer& timer, const char* contentDirLW) : 
GameClient(client, timer, contentDirLW)
{
	IncomingPacketEventMap[ID_RequestControlEntity].Subscribe(ehl, *this, &AI_GameClient::ReceiveRequestControlEntity);
}

void AI_GameClient::ReceiveRequestControlEntity(Packet& packet)
{//printf("GameClient::ReceiveRequestControlEntity\n");
	RakNet::BitStream recvBS(packet.data, packet.length, false);
	unsigned char packetID;
	recvBS.Read(packetID);
	ASSERT(packetID == (unsigned char)ID_RequestControlEntity);

	// We do not need rebroadcast info or ordering channel, it is for the server
	unsigned char rebroadcastInfo;
	recvBS.Read(rebroadcastInfo);
	unsigned char orderingChannel;
	recvBS.Read(orderingChannel);

	// Read in the Entity that wants to be changed
	unsigned netID; recvBS.Read(netID);
	Entity3D* tryEntity = FindEntity(netID);

	// Read in the PlayerID that wants it
	PlayerID playerThatWantsControl; recvBS.Read(playerThatWantsControl);

	// Send the new message to allow
	SendChangeEntityController(tryEntity, playerThatWantsControl);
}
//////////////////////////////////////////////////////////////////////////

void AI_GameClient::ForceDisconnect(PlayerID playerID)
{
	// Loop through all of entities to find ones that are owned by this player ID
	for (unsigned netID = 0; netID < GetNumEntities(); ++netID)
	{
		Entity3D* entity = FindEntity(netID);
		if (entity->GetControllingPlayerID() == playerID)
		{
			// Find the top level parent
			Entity3D* parentEntity = entity->GetParentEntity();
			while (parentEntity)
			{
				entity = parentEntity;
				parentEntity = entity->GetParentEntity();
			}

			// Force a disconnect
			SendChangeEntityController(entity, GetMyPlayerID());
		}
	}
}
//////////////////////////////////////////////////////////////////////////