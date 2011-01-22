// GG_Framework.Logic ServerManager.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;

// Keep a single PacketDescriber around, this becomes the singleton
GG_Framework::Logic::PacketDescriber PD;

ServerManager::ServerManager(GG_Framework::Logic::Scripting::Script* script, GG_Framework::Logic::Network::SynchronizedTimer& aiTimer,
							 Network::IServer* playerServer, Network::MockServer* aiServer, AI_GameClient& aiGameClient) : 
	Network::ServerManager(playerServer, aiServer), AI_Timer(aiTimer),
		m_script(script), m_gameStarted(false), m_AI_GameClient(aiGameClient)
{
	ASSERT(m_script);

	// Read in the predicted load times if they are there
	const char* err = m_script->GetGlobalTable("LOAD_TIMES");
	double predTime;

	// Loop until we do not get a good number, remember LUA arrays are 1 based.
	while (!err && !m_script->GetIndex(m_predLoadTimes.size()+1, NULL, NULL, &predTime))
		m_predLoadTimes.push_back(predTime);

	// Start our vector&map with these 2
	AddPlayerID(UNASSIGNED_PLAYER_ID); // 0
	AddPlayerID(Network::MOCK_AI_PLAYER_ID); // 1

	// This makes us exit cleanly when asked to cancel from the application
	setCancelModeAsynchronous();
}
//////////////////////////////////////////////////////////////////////////

ServerManager::~ServerManager()
{
	// Delete all of the TransmittedEntities
	std::vector<TransmittedEntity*>::iterator tePos;
	for (tePos = m_entities.begin(); tePos != m_entities.end(); ++tePos)
	{
		TransmittedEntity* te = (*tePos);
		if (te)
			TransmittedEntity::ClassFactory.Destroy(te->CPP_CLASS, te);
	}
}
//////////////////////////////////////////////////////////////////////////

unsigned ServerManager::AddPlayerID(PlayerID playerID)
{
	// Watch for being already in the vector&map
	ASSERT(m_clientMap[playerID] == 0);

	// Add to the Map and the Vector so they match
	unsigned newPos = m_clientVector.size();
	m_clientMap[playerID] = newPos;
	m_clientVector.push_back(playerID);
	return newPos;
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::RemovePlayerID(PlayerID playerID)
{
	// Watch for NOT already being in the vector&map
	unsigned oldIndex = m_clientMap[playerID];
	// ASSERT(oldIndex != 0); (It might not have fully connected, so do not assert (yet)
							// Until we better define when the add REALLY happens

	// Set the map to point to the 0 index and the vector to point at UNASSIGNED_PLAYER_ID
	m_clientVector[oldIndex] = UNASSIGNED_PLAYER_ID;
	m_clientMap[playerID] = 0;

	// TODO: When the player leaves, what do we do with a ship he controls?
	// We can either assign it back to the AI, -OR- Really we should get rid of the entity all together
	// We would have to send messages around to get rid of it, and all of its sub-items.
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::Initialize()
{
	// Initialize all of Initial Entities from the LUA Script
	ReadInitialEntities();

	// Connect the AI First
	Network::MockServer* aiServer = dynamic_cast<Network::MockServer*>(GetServer2());
	ASSERT(aiServer);
	ASSERT(aiServer->GetMockClient());
	PeerConnected(aiServer, aiServer->GetMockClient()->GetMyPlayerID());

	// This should always succeed, If it does not, just bounce out
	ASSERT(m_AI_GameClient.LoadInitialGameData());
	printf("AI Client Load Successful\n");
	
	// We can now start broadcasting to the AI Player
	DEBUG_PLAYER_LOADING("Allowing broadcasts to %s\n", GG_Framework::Logic::Network::PlayID2Str(Network::MOCK_AI_PLAYER_ID).c_str());
	m_connectedPeers.insert(Network::MOCK_AI_PLAYER_ID);

	// See of the Player Server is a Network(multi-player) or Mock(Single-Player) Server
	Network::Server* playerServer = dynamic_cast<Network::Server*>(GetServer1());
	if (playerServer)
	{
		// Subscribe to Players being Added and Removed
		playerServer->ClientConnect_Event.Subscribe(ehl, *this, &ServerManager::PeerConnected);
		playerServer->ClientDisconnect_Event.Subscribe(ehl, *this, &ServerManager::PeerDisconnected);
		printf("Waiting for Players to Join...\n");
	}
	else
	{
		// Connect the Single Attached Player
		Network::MockServer* mockServer = dynamic_cast<Network::MockServer*>(GetServer1());
		ASSERT(mockServer);
		ASSERT(mockServer->GetMockClient());
		PeerConnected(mockServer, mockServer->GetMockClient()->GetMyPlayerID());
	}
}
//////////////////////////////////////////////////////////////////////////

bool ServerManager::SendEnvironmentFile(const char* fileToSend, Network::IServer* server, PlayerID playerID)
{
	unsigned len = (unsigned)strlen(fileToSend);
	char* buff = new char[len+2];
	buff[0] = (char)ID_Environment_OSGV_File;
	strcpy(buff+1, fileToSend);
	server->Send(buff, len+2, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL);
	delete[] buff;
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool ServerManager::SendEnvironmentFiles(Network::IServer* server, PlayerID playerID)
{
	// Load the environment variables from the script, (e.g. Planet System Script).
	bool ret = true;
	const char* err = m_script->GetGlobalTable("ENVIRONMENT");
	ASSERT_MSG(!err, GG_Framework::Base::BuildString("Error Reading \"%s\" from LUA Script: %s", m_script->NameMap["ENVIRONMENT"].c_str(), err).c_str());

	{
		unsigned index = 1;
		while (true)
		{
			std::string fileToLoad;
			if (m_script->GetIndex(index, &fileToLoad) == NULL)
			{
				if (!SendEnvironmentFile(fileToLoad.c_str(), server, playerID))
				{
					ret = false;
					break;
				}
				++index;
			}
			else
				break;
		}
		// Pop the Table
		m_script->Pop();
	}

	// Send the last message with no string, so the Client Knows that no more are coming
	SendEnvironmentFile("", server, playerID);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

bool ServerManager::SendPredictedLoadTimes(Network::IServer* server, PlayerID playerID)
{
	DEBUG_PLAYER_LOADING("ServerManager::SendPredictedLoadTimes(%s)\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());

	// Create a BitStream to hold the Entity
	RakNet::BitStream bs;
	bs.Write((unsigned char)ID_PredictedLoadTimes);
	unsigned numLoadTimes = m_predLoadTimes.size();
	bs.Write(numLoadTimes);
	for (unsigned i = 0; i < numLoadTimes; ++i)
		bs.Write(m_predLoadTimes[i]);

	// Send it to the new Client
	return (server->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL));
}
//////////////////////////////////////////////////////////////////////////

// This is also where server specific messages can be processed
bool ServerManager::ShouldRebroadcastMessage(Packet* packet,
		PacketPriority& priority, PacketReliability& reliability, char& orderingChannel, std::set<PlayerID>*& ignorePlayers)
{
	// If we DO broadcast, ignore these
	ignorePlayers = &m_connectedPeers;

	const unsigned char* ID = Network::GetPacketIdentifierPtr(packet->data);
	if (*ID == ID_PlayerCompletedLoading)
	{
		DEBUG_PLAYER_LOADING("ID_PlayerCompletedLoading %s\n", GG_Framework::Logic::Network::PlayID2Str(packet->playerId).c_str());
		// We know this message only comes from real players, already connected
		// The first server is the player server
		PeerFinishedConnect(packet->playerId);

		// This is not broadcast to everyone
		return false;
	}
	else if (*ID == ID_CompletedLoadingOtherPlayerEntity)
	{
		RakNet::BitStream bs(packet->data, packet->length, false);
		unsigned char packetID;
		bs.Read(packetID);
		ASSERT(packetID == (unsigned char)ID_CompletedLoadingOtherPlayerEntity);
		PlayerID playerID;
		bs.Read(playerID);

		DEBUG_PLAYER_LOADING("%s Completed Loading %s\n", GG_Framework::Logic::Network::PlayID2Str(packet->playerId).c_str(), GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());
		PeerFinishedConnect(playerID);

		// This is not broadcast to everyone
		return false;
	}
	else if (*ID == ID_StandardEntityUpdate)
	{
		// We want to send along different ordering channels based on the ID of this player
		int playerIndex = GetClientIndex(packet->playerId);
		orderingChannel = (char)((playerIndex % 30) + 2);
		priority = MEDIUM_PRIORITY;
		reliability = UNRELIABLE_SEQUENCED;

		// We ARE rebroadcasting
		return true;
	}
	else if ((*ID >= ID_ClientMessages) && (*ID <= ID_LastClientMessage))
	{
		// Read from the packet information how to handle rebroadcasting
		unsigned char rebroadcastInfo = ID[1];
		GG_Framework::Logic::Network::FindRebroadcastInstructions(rebroadcastInfo, priority, reliability);
		orderingChannel = ID[2];

		// We ARE rebroadcasting
		return true;
	}

	return Network::ServerManager::ShouldRebroadcastMessage(packet,
		priority, reliability, orderingChannel, ignorePlayers);
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::ReadInitialEntities()
{
	const char* err = m_script->GetGlobalTable("EXISTING_ENTITIES");
	if (err)	// Not an error really, there are just not any existing entities.
		DebugOutput("Error Reading \"%s\" from LUA Script: %s", m_script->NameMap["EXISTING_ENTITIES"].c_str(), err);
	else
	{
		unsigned index = 1;
		while (true)
		{
			if (m_script->GetIndexTable(index) == NULL)
			{
				// Create the new Entity from the LUA script
				// The AI controls all of these initial things
				// This function will also add the entity (and all entities it may create) to my list
				TransmittedEntity* newEntity = TransmittedEntity::CreateFromServerScript(*m_script, m_AI_GameClient.GetMyPlayerID(), this);

				// Pop the Table for THIS Entity
				m_script->Pop();
				++index;
			}
			else
				break;
		}

		// Pop the Table for all EXISTING_ENTITIES
		m_script->Pop();
	}
}
//////////////////////////////////////////////////////////////////////////

unsigned ServerManager::AddTransmittedEntity(TransmittedEntity* newEntity)
{
	// Must be thread safe, wrap in a mutex
	GG_Framework::Base::RefMutexWrapper mr(m_entitiesV_mutex);
	m_entities.push_back(newEntity);
	return m_entities.size()-1;	// The index of the entity I just added
}
//////////////////////////////////////////////////////////////////////////

bool ServerManager::SendExistingEntities(Network::IServer* server, PlayerID playerID)
{
	// Loop through the vector of all of the existing entities
	std::vector<TransmittedEntity*>::iterator tePos;
	for (tePos = m_entities.begin(); tePos != m_entities.end(); ++tePos)
	{
		TransmittedEntity* newEntity = *tePos;
		if (newEntity)
		{
			// DEBUG_PLAYER_LOADING("ServerManager::SendExistingEntities(%s) send %s\n", 
			//	(playerID==Network::MOCK_AI_PLAYER_ID)?"AI":"UI", newEntity->NAME.c_str());

			// Create a BitStream to hold the Entity
			RakNet::BitStream bs;
			bs.Write((unsigned char)ID_ExistingEntity);
			newEntity->WriteToBitStream(bs);

			// Send it to the new Client
			server->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL);
			// DEBUG_PLAYER_LOADING("ServerManager::SendExistingEntities(%s) send ID_ExistingEntity\n", (playerID==Network::MOCK_AI_PLAYER_ID)?"AI":"UI");
		}
	}

	// Send the last message with no string, so the Client Knows that no more are coming
	RakNet::BitStream lastBS;
	lastBS.Write((unsigned char)ID_ExistingEntity);
	server->Send(&lastBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL);
	DEBUG_PLAYER_LOADING("ServerManager::SendExistingEntities(%s) send *last* ID_ExistingEntity\n", (playerID==Network::MOCK_AI_PLAYER_ID)?"AI":"UI");
	
	return true;
}
//////////////////////////////////////////////////////////////////////////

class PlayerEntity_Handler
{
public:
	PlayerEntity_Handler(PlayerID controllerPlayerID, ITransmittedEntityOwner* entityOwner) : 
	  m_transEntity(NULL), m_controllerPlayerID(controllerPlayerID), m_entityOwner(entityOwner) {}

	TransmittedEntity* LoadPlayerEntities(Scripting::Script* script, int playerIndex)
	{
		event.Subscribe(ehl, *this, &PlayerEntity_Handler::Callback);
		// The player index is off by one from the player number in the scripts.  It should start at 1.
		const char* err = script->CallFunction("GetNewPlayerEntity", "i>t", playerIndex-1, &event);
		if (err) printf("Failed call to \"GetNewPlayerEntity\": %s\n", err);
		DEBUG_PLAYER_LOADING("Loaded Player[%i] = %s\n", playerIndex, m_transEntity->NAME.c_str());
		TransmittedEntity* ret = m_transEntity;
		m_transEntity = NULL;	// We no longer own it
		return ret;
	}
private:
	IEvent::HandlerList ehl;
	TransmittedEntity* m_transEntity;
	Event1<Scripting::Script*> event;
	PlayerID m_controllerPlayerID;
	ITransmittedEntityOwner* m_entityOwner;

	void Callback(Scripting::Script* script)
	{
		m_transEntity = TransmittedEntity::CreateFromServerScript(*script, m_controllerPlayerID, m_entityOwner);
	}
};
//////////////////////////////////////////////////////////////////////////

void ServerManager::CreatePlayersEntities(PlayerID playerID, std::vector<TransmittedEntity*>& newEntities)
{
	// Must be thread safe, wrap in a mutex
	GG_Framework::Base::RefMutexWrapper mr(m_entitiesV_mutex);
	
	// Remember how many there are in the list already
	unsigned origNumEntities = m_entities.size();

	// Create the new entities
	PlayerEntity_Handler handler(playerID, this);
	int newPlayerIndex = (int)AddPlayerID(playerID);
	handler.LoadPlayerEntities(m_script, newPlayerIndex);

	// Copy over any newly created ones
	for (unsigned index = origNumEntities; index < m_entities.size(); ++index)
		newEntities.push_back(m_entities[index]);
}
//////////////////////////////////////////////////////////////////////////

void SendFailedMessage(Network::IServer* server, PlayerID playerID)
{
	RakNet::BitStream kickOutBS;
	kickOutBS.Write((unsigned char)ID_ConnectPermissionDenied);
	server->Send(&kickOutBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL);
}

void ServerManager::PeerConnected(Network::IServer* server, PlayerID playerID)
{
	// Tell the console about the connection
	printf("%s Peer Connected: %s\n", (playerID==Network::MOCK_AI_PLAYER_ID) ? "AI" : "UI", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());

	// Try to send players that are not AI the environment variables
	if (playerID != Network::MOCK_AI_PLAYER_ID)
	{
		// The player will want to know the predicted load time
		if (!SendPredictedLoadTimes(server, playerID))
		{
			SendFailedMessage(server, playerID);
			return;
		}

		// Only UI care about environments (things that are only visible, no collisions or anything else)
		if (!SendEnvironmentFiles(server, playerID))
		{
			SendFailedMessage(server, playerID);
			return;
		}
	}

	// Here is where we send the other entities in the scene
	SendExistingEntities(server, playerID);

	// Try to get a new Entity for the new Player to control (If NOT the AI Client)
	if (playerID != Network::MOCK_AI_PLAYER_ID)
	{
		// Make a vector of player entities that will be created
		std::vector<TransmittedEntity*> newEntities;
		CreatePlayersEntities(playerID, newEntities);
		if (!newEntities.size())
		{
			SendFailedMessage(server, playerID);
			return;
		}

		// If there was a player's ship, send it to the new player and other players
		// Keep a count of all the messages we send out here
		unsigned waitCount = 0;

		// And finally send all of the player's entities
		DEBUG_PLAYER_LOADING("ServerManager::PeerConnected(%s %s) sending player's entity\n", (playerID==Network::MOCK_AI_PLAYER_ID)?"AI":"UI", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());
		RakNet::BitStream lastBS;
		lastBS.Write((unsigned char)ID_PlayersEntity);
		lastBS.Write(playerID);
		lastBS.Write(newEntities.size());
		for (unsigned index = 0; index < newEntities.size(); ++index)
			newEntities[index]->WriteToBitStream(lastBS);

		// Send it to each of the connected players (including this one)
		for (unsigned i = 0; i < (unsigned)m_clientVector.size(); ++i)
		{
			PlayerID clientToSend = m_clientVector[i];
			DEBUG_PLAYER_LOADING("\tto %s\n", GG_Framework::Logic::Network::PlayID2Str(clientToSend).c_str());
			if (clientToSend ==  Network::MOCK_AI_PLAYER_ID)
			{
				GetServer2()->Send(&lastBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, clientToSend, false, NULL);
				++waitCount;
			}
			else if (clientToSend != UNASSIGNED_PLAYER_ID)
			{
				server->Send(&lastBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, clientToSend, false, NULL);
				++waitCount;
			}
		}
		DEBUG_PLAYER_LOADING("\tWaitCount=%i\n",waitCount);
		m_clientWaitCount[playerID]=waitCount;
	}
}
//////////////////////////////////////////////////////////////////////////

//! Called when the player returns the message saying it is done loading and ready to go
//! Starts our own timer if this is the first player, then sends the epoch to the player
void ServerManager::PeerFinishedConnect(PlayerID playerID)
{
	// The AI_Timer is not set until the first player is done loading
	if (playerID != Network::MOCK_AI_PLAYER_ID)
	{
		// Are we waiting for anyone else to say they are complete with the connection?
		// We know we are done when the count falls back to 0
		m_clientWaitCount[playerID]-=1;
		DEBUG_PLAYER_LOADING("ServerManager::PeerFinishedConnect(%s) Wait Count = %i\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str(), m_clientWaitCount[playerID]);
		if (m_clientWaitCount[playerID]==0)
		{
			// We can actually start our own timer now.  This happens by setting my epoch
			// To the current local time on this machine.
			if (!m_gameStarted)
			{
				m_gameStarted = true;
				AI_Timer.SetEpoch(RakNet::GetTime());
			}

			// The LAST message to the Client sets its EPOCH so it can get started
			{
				DEBUG_PLAYER_LOADING("ServerManager::PeerFinishedConnect(%s) sending EPOCH\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());
				RakNet::BitStream epochBS;
				epochBS.Write((unsigned char)ID_TIMESTAMP);
				epochBS.Write(AI_Timer.GetEpoch());	//! Sending a time-stamped message in RakNet
													//! Automatically converts the time to the local
													//! time on THAT machine.
				epochBS.Write((unsigned char)ID_SetEpoch);
				GetServer1()->Send(&epochBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, false, NULL);
			}

			// And to every other Client to let them know they can add the new player's ship
			// to their own scenes
			{
				DEBUG_PLAYER_LOADING("ServerManager::PeerFinishedConnect(%s) sending ATTACH\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());
				RakNet::BitStream finalShipLoad;
				finalShipLoad.Write((unsigned char)ID_AttachPlayerEntity);
				finalShipLoad.Write(playerID);

				// We can send these along to all peers, even the ones in the middle of connecting because
				// we are sending it on the same ordering channel, so it will come AFTER the other game load messages
				GetServer1()->Send(&finalShipLoad, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, true, NULL);
				GetServer2()->Send(&finalShipLoad, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad, playerID, true, NULL);
			}

			// We can now start broadcasting to this player
			DEBUG_PLAYER_LOADING("Allowing broadcasts to %s\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str());
			m_connectedPeers.insert(playerID);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::PeerDisconnected(Network::IServer* playerServer, PlayerID playerID, bool normalDisconnect)
{
	// Tell the console about the connection
	printf("Peer Disconnected: %s: Normal Disconnect = %s\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str(), normalDisconnect ? "true" : "false");
	DebugOutput("******** ServerManager::PeerDisconnected(%s, %s)\n", GG_Framework::Logic::Network::PlayID2Str(playerID).c_str(), 
		normalDisconnect ? "true" : "false");

	// Tell the AI to force disconnect the player to clean them up
	if (!normalDisconnect)
		m_AI_GameClient.ForceDisconnect(playerID);
	
	RemovePlayerID(playerID);
	m_connectedPeers.erase(playerID);
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::tryRun()
{
	// Sets up the events and connections,
	// Also loads the AI Scene Data and places the Single Player Data on the Server
	Initialize();

	// This will loop until the last player exits
	while(!m_gameStarted || 
		(GetServer1()->IsActive() && GetServer1()->GetNumConnections()))
	{
		// Pass the Messages Around
		ProcessMessages();

		// The Timer firing is what keeps the AI game moving
		if (m_gameStarted)
		{
			// Fire the Timer to keep the AI Game Running
			AI_Timer.FireTimer();
			ThreadSleep(20);	// sleep for a bit to throttle the Server and AI
		}
		else
		{// Epoch has not happened yet, but the AI_GameClient may be connecting to Players
			m_AI_GameClient.LookForFirstPlayerEntity();
			ThreadSleep(100);
		}
	}

	// Write out the log file if needed
	AI_Timer.Logger.WriteLog();
}
//////////////////////////////////////////////////////////////////////////