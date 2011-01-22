// GG_Framework.Logic UI_GameClient.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.h"
#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Logic;
using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

UI_GameClient::UI_GameClient(GG_Framework::Logic::Network::IClient& client, 
						   GG_Framework::Logic::Network::SynchronizedTimer& logic_timer, 
						   GG_Framework::Base::Timer& osg_timer, 
						   const char* contentDirLW) : 
m_syncLogicTimer(logic_timer), m_controlledEntity(NULL), GameClient(client, logic_timer, contentDirLW)
{
	if (GG_Framework::Base::TEST_USE_SIMPLE_MODELS)
		UI_ActorScene = new BB_ActorScene(osg_timer, contentDirLW);
	else
		UI_ActorScene = new GG_Framework::UI::ActorScene(osg_timer, contentDirLW, true);
}
//////////////////////////////////////////////////////////////////////////

UI_GameClient::~UI_GameClient()
{
	delete UI_ActorScene;
}

bool UI_GameClient::LoadPredictedLoadTimes()
{
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadPredictedLoadTimes() WAITING\n");
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
			if (packetID == ID_PredictedLoadTimes)
			{
				DEBUG_PLAYER_LOADING("UI_GameClient::LoadPredictedLoadTimes() RECV-Loading\n");
				// Create a bitstream to read in all of the times into a vector
				RakNet::BitStream bs(packet->data, packet->length, false);
				unsigned char packetID;
				bs.Read(packetID);
				ASSERT(packetID == (unsigned char)ID_PredictedLoadTimes);
				unsigned numPredTimes;
				bs.Read(numPredTimes);	// Reads the number of times
				std::vector<float> predLoadTimes;
				float thisPredTime;
				for (unsigned i = 0; i < numPredTimes; ++i)
				{
					bs.Read(thisPredTime);
					predLoadTimes.push_back(thisPredTime);
				}

				// Send the list of times to the loader
				GG_Framework::UI::OSG::LoadStatus::Instance.SetPredictedLoadTimes(predLoadTimes);

				ret = true;
				stillLoading = false;
			}
			else if (packetID == ID_ConnectPermissionDenied)
			{
				ret = false;
				stillLoading = false;
			}
			else if (packetID == ID_StandardEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This should not happen here because we are blocked by the server until we say we are done
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
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadPredictedLoadTimes() COMPLETE\n");
	return ret;
}
//////////////////////////////////////////////////////////////////////////

bool UI_GameClient::LoadInitialGameData()
{
	// Connect to the Server
	LoadStatusUpdate.Fire("Connecting to the Server...");
	while(!m_client.IsConnected())
		GG_Framework::Base::ThreadSleep(500);

	// Load the predicted load times, so we can have a good remaining time estimate
	if (!LoadPredictedLoadTimes())
		return false;

	// Load the Ornamental Data, then let the Base Class Load the Real Data
	LoadStatusUpdate.Fire("Loading Game Data...");
	if (!LoadOrnamentalOSGV())
		return false;

	// This will load other player's entities
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadInitialGameData() calling base\n");
	if (!GameClient::LoadInitialGameData())
		return false;

	// Wait for our own entity
	if (!LoadOwnEntity())
		return false;

	// All is well
	return true;
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::NotifyServerReady()
{
	// All is loaded ok, let the Server know
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadInitialGameData() Sending ID_PlayerCompletedLoading\n");
	RakNet::BitStream lastBS;
	lastBS.Write((unsigned char)ID_PlayerCompletedLoading);
	m_client.Send(&lastBS, HIGH_PRIORITY, RELIABLE_ORDERED, OC_GameLoad);

	// Finally wait for the Epoch message
	GetSyncTimerEpoch();
}
//////////////////////////////////////////////////////////////////////////

bool UI_GameClient::LoadOwnEntity()
{
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadOwnEntity() WAITING\n");
	// Sit and spin until we get the last message
	bool stillLoading = true;
	bool ret = true;
	while (stillLoading)	//! \todo We should be more careful here about disconnects and such
	{
		//! \todo We also need to be more careful about the kinds of packets received
		Packet* packet = m_client.AsPacketReceiver()->Receive();
		if (packet)
		{
			int packetID = Network::GetPacketIdentifier(packet->data);
			if (packetID == ID_PlayersEntity)
			{
				DEBUG_PLAYER_LOADING("UI_GameClient::LoadOwnEntity() RECV-Loading\n");
				RakNet::BitStream bs(packet->data, packet->length, false);
				unsigned char packetID;
				bs.Read(packetID);
				ASSERT(packetID == (unsigned char)ID_PlayersEntity);
				PlayerID myPlayerID;	// This should be my own ID
				bs.Read(myPlayerID);	// Not that we need it yet
				unsigned numNewEntities;
				bs.Read(numNewEntities);	// There may be more than one
				for (unsigned index = 0; index < numNewEntities; ++index)
				{
					// Read in this TE
					TransmittedEntity* te = TransmittedEntity::CreateFromBitStream(bs);

					// This will create all of the Actors, including the UI Actor
					// We do not want to call m_controlledEntity->SetPlayerControlled(true) yet, because all
					// is not ready.  That will be called from UI_GameClient::StartUI()
					// m_controlledEntity will always point to the last one, which is ours
					m_controlledEntity = CreateNewEntity(*te, true);

					// We can destroy this newly created TE, we no longer need it
					TransmittedEntity::ClassFactory.Destroy(te->CPP_CLASS, te);
				}

				// All done
				stillLoading = false;
			}
			else if (packetID == ID_ConnectPermissionDenied)
			{
				ret = false;
				stillLoading = false;
			}
			else if (packetID == ID_StandardEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This should not happen here because we are blocked by the server until we say we are done
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
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadOwnEntity() COMPLETE\n");
	return ret;
}
//////////////////////////////////////////////////////////////////////////

// This does not happen on the AI_Client because that timer is already tied to the server
bool UI_GameClient::GetSyncTimerEpoch()
{
	DEBUG_PLAYER_LOADING("UI_GameClient::GetSyncTimerEpoch() WAITING\n");
	// Sit and spin until we get the last message
	bool stillLoading = true;
	bool ret = true;
	while (stillLoading)	//! \todo We should be more careful here about disconnects and such
	{
		//! \todo We also need to be more careful about the kinds of packets received
		Packet* packet = m_client.AsPacketReceiver()->Receive();
		if (packet)
		{
			int packetID = Network::GetPacketIdentifier(packet->data);
			if (packetID == ID_SetEpoch)
			{
				DEBUG_PLAYER_LOADING("UI_GameClient::GetSyncTimerEpoch() RECV\n");
				RakNet::BitStream timeBS(packet->data+1, sizeof(RakNetTime)+sizeof(unsigned char), false);
				RakNetTime epoch;
				timeBS.Read(epoch);
				m_syncLogicTimer.SetEpoch(epoch);
				stillLoading = false;
			}
			else if (packetID == ID_ConnectPermissionDenied)
			{
				ret = false;
				stillLoading = false;
			}
			else if (packetID == ID_StandardEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This MIGHT happen here while we are waiting for the timer epoch
			}
			else if (packetID == ID_SpecialEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This MIGHT happen here while we are waiting for the timer epoch
			}
			else
			{
				// For other messages, we want to pass them to the events
				// This might happen here, particularly when loading a new player while still waiting for this one
				FireNetworkPacketEvents(packet);
			}
			m_client.AsPacketReceiver()->DeallocatePacket(packet);
		}
		// sleep only if this is a Mock ID (on a thread)
		if (GetMyPlayerID() == Network::MOCK_SINGLE_PLAYER_ID)
			GG_Framework::Base::ThreadSleep(1);
	}
	DEBUG_PLAYER_LOADING("UI_GameClient::GetSyncTimerEpoch() COMPLETE\n");
	return ret;
}
//////////////////////////////////////////////////////////////////////////

bool UI_GameClient::LoadOrnamentalOSGV()
{
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadOrnamentalOSGV() WAITING\n");
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
			if (packetID == ID_Environment_OSGV_File)
			{
				DEBUG_PLAYER_LOADING("UI_GameClient::LoadOrnamentalOSGV() RECV-Loading\n");
				char* file = ((char*)packet->data)+1;
				if (file[0] == NULL)
					stillLoading = false;
				else
				{
					// Do not load the ornamental stuff when not using simple models
					if (!GG_Framework::Base::TEST_USE_SIMPLE_MODELS)
					{
						Entity3D::EventMap* localEventMap = new Entity3D::EventMap(true);
						MapList.push_back(localEventMap);
						UI_ActorScene->AddActorFile(*localEventMap, file);
					}
				}
			}
			else if (packetID == ID_ConnectPermissionDenied)
			{
				ret = false;
				stillLoading = false;
			}
			else if (packetID == ID_StandardEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This should not happen here
			}
			else if (packetID == ID_SpecialEntityUpdate)
			{
				// We do nothing with Entity Updates
				// This should not happen here
			}
			else
			{
				// For other messages, we want to pass them to the events
				// This should not happen here, but some do pass through
				FireNetworkPacketEvents(packet);
			}
			m_client.AsPacketReceiver()->DeallocatePacket(packet);
		}
		// sleep only if this is a Mock ID (on a thread)
		if (GetMyPlayerID() == Network::MOCK_SINGLE_PLAYER_ID)
			GG_Framework::Base::ThreadSleep(1);
	}
	DEBUG_PLAYER_LOADING("UI_GameClient::LoadOrnamentalOSGV() COMPLETE\n");
	return ret;
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::SetControlledEntity(Entity3D* newEntity)
{
	if (newEntity != m_controlledEntity)
	{
		Entity3D* oldEntity = m_controlledEntity;
		if (m_controlledEntity)
			m_controlledEntity->SetPlayerControlled(false);
		m_controlledEntity = newEntity;
		if (m_controlledEntity)
			m_controlledEntity->SetPlayerControlled(true);
		ControlledEntityChanged.Fire(oldEntity, newEntity);
	}
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::RequestControlEntity(Entity3D* tryEntity)
{//printf("UI_GameClient::RequestControlEntity()\n");
	ASSERT(tryEntity);

	RakNet::BitStream reqControlBS;
	reqControlBS.Write((unsigned char)ID_RequestControlEntity);
	unsigned char rebroadcastInfo = Network::CreateRebroadcastInstructions(HIGH_PRIORITY, RELIABLE);
	reqControlBS.Write(rebroadcastInfo);
	reqControlBS.Write((unsigned char)OC_CriticalEntityUpdates);

	// We only really need this to go to the AI Client (or more specifically, the CURRENT owner of the entity,
	// but I do not have an easy way to do that right now.

	// Here is the index of the primary entity we want to pilot, and my PlayerID
	reqControlBS.Write(tryEntity->GetNetworkID());
	reqControlBS.Write(GetMyPlayerID());

	// Send it
	m_client.Send(&reqControlBS, HIGH_PRIORITY, RELIABLE, OC_CriticalEntityUpdates);
}
//////////////////////////////////////////////////////////////////////////

void UI_GameClient::StartUI()
{
	// We want to call the function that fires all events, but it checks to see if the new entity is different
	Entity3D* newEntity = m_controlledEntity;
	m_controlledEntity = NULL;
	SetControlledEntity(newEntity);
}
//////////////////////////////////////////////////////////////////////////
