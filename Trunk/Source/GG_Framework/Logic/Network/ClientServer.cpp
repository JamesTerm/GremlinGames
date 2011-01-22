// ClientServer.cpp
#include "StdAfx.h"
#include "GG_Framework.Logic.Network.h"
using namespace GG_Framework::Base;
using namespace GG_Framework::Logic::Network;

const unsigned char* GG_Framework::Logic::Network::GetPacketIdentifierPtr(const unsigned char* data)
{
	if ((unsigned char)data[0] == ID_TIMESTAMP)
	{
		return &(data[sizeof(unsigned char) + sizeof(unsigned long)]);
	}
	else
		return &(data[0]);
}
//////////////////////////////////////////////////////////////////////////

unsigned char GG_Framework::Logic::Network::GetPacketIdentifier(const unsigned char* data)
{
	return GetPacketIdentifierPtr(data)[0];
}
//////////////////////////////////////////////////////////////////////////

unsigned char GG_Framework::Logic::Network::CreateRebroadcastInstructions(PacketPriority priority, PacketReliability reliability)
{
	return (priority << 4) + reliability;
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::Logic::Network::FindRebroadcastInstructions(unsigned char info, PacketPriority& priority, PacketReliability& reliability)
{
	priority = (PacketPriority)((unsigned)info >> 4);
	reliability = (PacketReliability)((unsigned)info & 0x0f);
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::Logic::Network::DebugOutputPacket(Packet& packet)
{
	int thisDataType = GetPacketIdentifier(packet.data);
	DebugOutput("Packet id[%i]: %s\n", thisDataType, PacketDescriber::GetPacketDescription(packet).c_str());
}
//////////////////////////////////////////////////////////////////////////

GG_Framework::Logic::Network::PacketDescriber* GG_Framework::Logic::Network::PacketDescriber::s_inst = NULL;

// This is the singleton unless overridden
GG_Framework::Logic::Network::PacketDescriber PD;

FRAMEWORK_LOGIC_NETWORK_API void GG_Framework::Logic::Network::WriteString(RakNet::BitStream& bs, const std::string& s)
{
	short len = (short) s.length()+1;
	bs.Write(len);
	bs.Write(s.c_str(), len);
}
//////////////////////////////////////////////////////////////////////////

FRAMEWORK_LOGIC_NETWORK_API void GG_Framework::Logic::Network::ReadString(RakNet::BitStream& bs, std::string& s)
{
	short len;
	bs.Read(len);
	ASSERT((len > 0) && (len < 512));	//!< Just a check
	char* buff = new char[len];
	bs.Read(buff, len);
	s = buff;
	delete[] buff;
}
//////////////////////////////////////////////////////////////////////////

FRAMEWORK_LOGIC_NETWORK_API std::string GG_Framework::Logic::Network::PlayID2Str(const PlayerID& pID)
{
	static OpenThreads::Mutex s_mutex;
	RefMutexWrapper mw(s_mutex);
	return std::string(pID.ToString());
}
//////////////////////////////////////////////////////////////////////////

Server::Server(unsigned maxPlayers, unsigned portNumber) 
	: m_rakServer(RakNetworkFactory::GetRakServerInterface())
{
	m_rakServer->SetMTUSize(1400);
	m_rakServer->Start(maxPlayers, 0, 0, portNumber);
	m_rakServer->StartOccasionalPing();
}
//////////////////////////////////////////////////////////////////////////

bool Server::IsActive(){return m_rakServer->IsActive();}
unsigned Server::GetNumConnections()
{
	if (IsActive())
		return m_rakServer->GetConnectedPlayers();
	else return 0;
}
//////////////////////////////////////////////////////////////////////////

Packet* Server::Receive()
{
	Packet* ret = m_rakServer->Receive();
	if (ret)
	{
		// Watch for connections and disconnections
		switch(GetPacketIdentifier(ret->data))
		{
		case ID_NEW_INCOMING_CONNECTION:
			ClientConnect_Event.Fire(this, ret->playerId);
			break;

		case ID_CONNECTION_LOST:
			ClientDisconnect_Event.Fire(this, ret->playerId, false);
			break;
			
		case ID_DISCONNECTION_NOTIFICATION:
			ClientDisconnect_Event.Fire(this, ret->playerId, true);
			break;
		}

		DebugOutput("Server::Receive(): ");
		DebugOutputPacket(*ret);
	}
	return ret;
}
void Server::DeallocatePacket( Packet *packet ){m_rakServer->DeallocatePacket(packet);}
//////////////////////////////////////////////////////////////////////////

bool Server::Send( 
	const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, 
	PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers )
{
	DebugOutput("Server::Send(data): Packet Type(%i)\n", (int)GetPacketIdentifier((const unsigned char*)data));
	if (broadcast && onlyToPeers && onlyToPeers->size())
	{
		// Loop through all peers and send them directly
		bool ret = true;
		int numPeers = m_rakServer->GetAllowedPlayers();
		for (int i = 0; i < numPeers; ++i)
		{
			PlayerID pID = m_rakServer->GetPlayerIDFromIndex(i);
			if ((pID != playerId) && (m_rakServer->IsActivePlayerID(pID)) && (onlyToPeers->count(pID)))
				ret &= m_rakServer->Send(data, length, priority, reliability, orderingChannel, pID, false);
		}
		return ret;
	}
	else
		return m_rakServer->Send(data, length, priority, reliability, orderingChannel, playerId, broadcast);
}
//////////////////////////////////////////////////////////////////////////

bool Server::Send( 
	RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, 
	PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers )
{
	DebugOutput("Server::Send(bitStream): Packet Type(%i)\n", (int)GetPacketIdentifier(bitStream->GetData()));
	if (broadcast && onlyToPeers && onlyToPeers->size())
	{
		// Loop through all peers and send them directly
		bool ret = true;
		int numPeers = m_rakServer->GetAllowedPlayers();
		for (int i = 0; i < numPeers; ++i)
		{
			PlayerID pID = m_rakServer->GetPlayerIDFromIndex(i);
			if ((pID != playerId) && (m_rakServer->IsActivePlayerID(pID)) && (!onlyToPeers->count(pID)))
				ret &= m_rakServer->Send(bitStream, priority, reliability, orderingChannel, pID, false);
		}
		return ret;
	}
	else
		return m_rakServer->Send(bitStream, priority, reliability, orderingChannel, playerId, broadcast);
}
//////////////////////////////////////////////////////////////////////////

Server::~Server()
{
	m_rakServer->Disconnect(100);
	RakNetworkFactory::DestroyRakServerInterface(m_rakServer);
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

Client::Client(
	const char* serverURL, unsigned serverPort, unsigned clientPort) :
	m_rakClient(RakNetworkFactory::GetRakClientInterface())
{
	m_rakClient->SetMTUSize(1400);
	if (m_rakClient->Connect(serverURL, serverPort, clientPort, 0, 0))
	{
		printf("RakNet Client Starting Connect OK\n");
		m_rakClient->StartOccasionalPing();
		// ThreadSleep(100);	// Wait a bit to make sure the server gets the first message
		// Send the first message and make sure the server gets it
		// char msg = ID_ClientConnecting;
		// Send(&msg, 1, HIGH_PRIORITY, RELIABLE, 0);
	}
	else
		printf("RakNet Client Connect FAILED\n");
}
//////////////////////////////////////////////////////////////////////////

bool Client::IsConnected(){return m_rakClient->IsConnected();}
Packet* Client::Receive()
{
	Packet* ret = m_rakClient->Receive();
	if (ret)
	{
		DebugOutput("Client[%s]::Receive(): ", GG_Framework::Logic::Network::PlayID2Str(m_rakClient->GetPlayerID()).c_str());
		DebugOutputPacket(*ret);
	}
	return ret;
}
void Client::DeallocatePacket( Packet *packet ){m_rakClient->DeallocatePacket(packet);}
//////////////////////////////////////////////////////////////////////////


bool Client::Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	DebugOutput("Client[%s]::Send(data): Packet Type(%i)\n", GG_Framework::Logic::Network::PlayID2Str(m_rakClient->GetPlayerID()).c_str(), (int)GetPacketIdentifier((const unsigned char*)data));
	return m_rakClient->Send(data, length, priority, reliability, orderingChannel);
}
bool Client::Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel )
{
	DebugOutput("Client[%s]::Send(bitStream): Packet Type(%i)\n", GG_Framework::Logic::Network::PlayID2Str(m_rakClient->GetPlayerID()).c_str(), (int)GetPacketIdentifier(bitStream->GetData()));
	return m_rakClient->Send(bitStream, priority, reliability, orderingChannel);
}
//////////////////////////////////////////////////////////////////////////

Client::~Client()
{
	// Send one last message and make sure the server gets it
	// char msg = ID_ClientDisConnecting;
	// m_rakClient->Send(&msg, 1, HIGH_PRIORITY, RELIABLE, 0);
	// ThreadSleep(100);	// Wait a bit to make sure the server gets the LAST message
	m_rakClient->Disconnect(100);
	RakNetworkFactory::DestroyRakClientInterface(m_rakClient);
}
//////////////////////////////////////////////////////////////////////////

