// MockClientServer.cpp
#include "StdAfx.h"
#include "GG_Framework.Logic.Network.h"
using namespace GG_Framework::Base;

Packet *AllocPacket(unsigned dataSize, unsigned char *data, const PlayerID& playerID)
{
	Packet *p =new Packet;
	p->data=data;
	p->length=dataSize;
	p->bitSize=dataSize*8;
	p->deleteData=true;
	p->playerIndex = UNASSIGNED_PLAYER_INDEX;
	p->playerId = playerID;
	return p;
}

using namespace GG_Framework::Logic::Network;

MockPeer::MockPeer() : m_otherPeer(NULL) {}
MockPeer::~MockPeer()
{
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);
	if (m_otherPeer)
	{
		m_otherPeer->m_otherPeer = NULL;
		m_otherPeer = NULL;
	}
	while (!m_msgQueue.empty())
	{
		DeallocatePacket(m_msgQueue.front());
		m_msgQueue.pop();
	}
}
//////////////////////////////////////////////////////////////////////////

void MockPeer::DeallocatePacket( Packet *packet )
{
	if ( packet == 0 )
		return;
	if (packet->deleteData)
		delete[] packet->data;
	delete(packet);
}
//////////////////////////////////////////////////////////////////////////

Packet* MockPeer::Receive()
{
	Packet* ret = NULL;
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);

	if (!m_msgQueue.empty())
	{
		ret = m_msgQueue.front();
		m_msgQueue.pop();
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////

bool MockPeer::AddToPeerQueue(const char *data, const int length, const PlayerID& playerID)
{
	if (!data) return false;
	if (length <= 0) return false;
	if (!m_otherPeer) return false;

	// Push on the OTHER Queue
	return m_otherPeer->AddToSelfQueueFromOtherPeer(data, length, playerID);
}
//////////////////////////////////////////////////////////////////////////

bool MockPeer::AddToSelfQueueFromOtherPeer(const char *data, const int length, const PlayerID& playerID)
{
	// Parameters are validated in the sending peer ABOVE

	// Allocate a copy on this side
	unsigned char* newData = new unsigned char[length]; //!< Deleted in DeallocatePacket()
	memcpy((char*)newData, data, length);

	// Watch for thread safety
	GG_Framework::Base::RefMutexWrapper rmw(m_mutex);

	// Push on our QUEUE
	m_msgQueue.push(AllocPacket(length, newData, playerID));
	return true;
}
//////////////////////////////////////////////////////////////////////////

bool MockPeer::AddToPeerQueue(RakNet::BitStream * bitStream, const PlayerID& playerID)
{
	if (!bitStream) return false;
	if (!m_otherPeer) return false;
	return AddToPeerQueue((const char*)bitStream->GetData(), bitStream->GetNumberOfBytesUsed(), playerID);
}
//////////////////////////////////////////////////////////////////////////

MockClient::MockClient(MockServer& server, PlayerID playerID) : m_playerID(playerID)
{
	m_otherPeer = &server;
	server.m_otherPeer = server.m_client = this;
}
//////////////////////////////////////////////////////////////////////////

bool MockServer::Send( 
	const char *data, const int length, PacketPriority priority, 
	PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers)
{
	if (!m_client)
		return false;
	else if (ShouldSendToClient(playerId, broadcast, onlyToPeers))
		return AddToPeerQueue(data, length, playerId);
	else return true;	// No need to send, but return true to say all was well
}
//////////////////////////////////////////////////////////////////////////

bool MockServer::Send( 
	RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, 
	char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers )
{
	if (!m_client)
		return false;
	else if (ShouldSendToClient(playerId, broadcast, onlyToPeers))
		return AddToPeerQueue(bitStream, playerId);
	else return true;	// No need to send, but return true to say all was well
}
//////////////////////////////////////////////////////////////////////////

bool MockServer::ShouldSendToClient(const PlayerID& playerId, bool broadcast, std::set<PlayerID>* onlyToPeers)
{
	PlayerID myPID = m_client->GetMyPlayerID();
	if (m_client && (broadcast ^ (playerId==myPID)))
	{
		// Watch for the list
		return onlyToPeers ? (onlyToPeers->count(myPID)!=0) : true;
	}
	return false;
}
//////////////////////////////////////////////////////////////////////////

bool MockClient::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	return AddToPeerQueue(data, length, m_playerID);
}
//////////////////////////////////////////////////////////////////////////

bool MockClient::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	return AddToPeerQueue(bitStream, m_playerID);
}
//////////////////////////////////////////////////////////////////////////

std::string MockClient::GetMockDescription() const
{
	return PlayID2Str(m_playerID);
}
//////////////////////////////////////////////////////////////////////////