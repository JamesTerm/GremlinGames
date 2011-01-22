// ServerManager.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.Network.h"

using namespace GG_Framework::Logic::Network;

ServerManager::ServerManager(IServer* server1, IServer* server2) : m_server1(server1), m_server2(server2)
{
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::ProcessMessages()
{
	if (m_server1->IsActive())
		ProcessMessages(m_server1->AsPacketReceiver());

	if (m_server2->IsActive())
		ProcessMessages(m_server2->AsPacketReceiver());
}
//////////////////////////////////////////////////////////////////////////

bool ServerManager::ShouldRebroadcastMessage(Packet* packet,
	PacketPriority& priority, PacketReliability& reliability, char& orderingChannel, std::set<PlayerID>*& ignorePlayers)
{
	priority = HIGH_PRIORITY;
	reliability = RELIABLE;
	orderingChannel = 0;

	// Do not pass on messages that are RakNet internal
	return (GetPacketIdentifier(packet->data) >= ID_USER_PACKET_ENUM);
}
//////////////////////////////////////////////////////////////////////////

void ServerManager::ProcessMessages(IPacketReceiver* fromRecv)
{
	Packet* packet;
	while(packet = fromRecv->Receive())
	{
		PacketPriority priority;
		PacketReliability reliability;
		char orderingChannel;
		std::set<PlayerID>* onlyToPeers = NULL;
		if (ShouldRebroadcastMessage(packet, priority, reliability, orderingChannel, onlyToPeers))
		{
			if (m_server1->IsActive())
				m_server1->Send((const char*)packet->data, packet->length, priority, reliability, orderingChannel, packet->playerId, true, onlyToPeers);
			if (m_server2->IsActive())
				m_server2->Send((const char*)packet->data, packet->length, priority, reliability, orderingChannel, packet->playerId, true, onlyToPeers);
		}
		fromRecv->DeallocatePacket(packet);
	}
}
//////////////////////////////////////////////////////////////////////////


