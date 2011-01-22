// RakWrapper.cpp
// December 2, 2006
// Jared C Jones
// Gremlin Games

#include "RakWrapper.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <conio.h>

const int SERVER_PORT = 1023;
const int CLIENT_PORT = 1024;

// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
unsigned char GetPacketIdentifier(Packet *p)
{
	if (p == 0)
		return 255;

	if ((unsigned char) p -> data[0] == ID_TIMESTAMP)
	{
		assert(p -> length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p -> data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p -> data[0];
}

//---------------------------
// BEGIN RAKWRAPPER
//---------------------------

RakWrapperNS::RakWrapper::RakWrapper() : m_currPacket(NULL)
{
}

RakWrapperNS::RakWrapper::~RakWrapper() {
}

RakWrapperNS::RakWrapper::PacketPropertiesStruct RakWrapperNS::RakWrapper::GetNextPacketType()
{
	ReleaseCurrentPacket();
	PacketPropertiesStruct pps(0,0);
	m_currPacket = ReceivePacket();
	if (m_currPacket)
	{
		char packetID = GetPacketIdentifier(m_currPacket);
		switch (packetID)
		{
			case ID_DISCONNECTION_NOTIFICATION:
				printf("Client disconnected from server.\n");
				break;
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("A client has disconnected from the server.\n");
				break;
			case ID_REMOTE_CONNECTION_LOST:
				printf("A client has lost connection with the server.\n");
				break;
			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("New client connection established.\n");
				break;
			case ID_REMOTE_EXISTING_CONNECTION:
				printf("Existing client connection found.\n");
				break;
			case ID_RECEIVED_STATIC_DATA:
				printf("Received static data from server.\n");			
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
				printf("Connection attempt failed\n");
				break;
			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("Server is full.\n");
				break;
			case ID_INVALID_PASSWORD:
				printf("Password is incorrect.\n");
				break;
			case ID_CONNECTION_LOST:
				printf("Connection with server lost. This is likely a problem with the server.\n");
				break;
			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("You are now connected to the server.\n");
				break;
			case ID_USER_PACKET_ENUM:
				printf("*** Received Data Message ***.\n");
				memcpy(&pps, m_currPacket->data+1, sizeof(pps));
				break;
			default:
				printf("Other Message? [%i].\n", (int)packetID);
				break;
		}
		if (!pps.length)
			ReleaseCurrentPacket();
	}
	return pps;
}

//---------------------------
// END RAKWRAPPER
//---------------------------



//---------------------------
// BEGIN RAKCLIENT
//---------------------------

RakWrapperNS::RakClient::RakClient() {
	// Create RakClient Interface
	m_client = RakNetworkFactory::GetRakClientInterface();
	m_client->StopOccasionalPing();
}

RakWrapperNS::RakClient::~RakClient() 
{
	ReleaseCurrentPacket();
	// Broadcast to server that the client is disconnecting.
	m_client -> Disconnect(300);
	// Clean up client interface allocation
	RakNetworkFactory::DestroyRakClientInterface(m_client);
}

void RakWrapperNS::RakClient::Connect(const char* playerName, const char* serverIP) 
{
	if (!serverIP || (serverIP[0] < '1') || (serverIP[0] > '9'))
		serverIP = "127.0.0.1";	// Set to loop back

	// Implement Connect For Client
	m_client -> InitializeSecurity(0,0);

	// User data
	StaticClientDataStruct staticClientData;
	strcpy(staticClientData.name, playerName);

	// Dealing with client
	m_client -> SetStaticClientData(UNASSIGNED_PLAYER_ID, (char*)&staticClientData, sizeof(StaticClientDataStruct));

	bool clientConnected = m_client -> Connect(serverIP, SERVER_PORT, CLIENT_PORT, 0, 30);
	
	if (clientConnected)
		puts("Attempting connection...");
	else
		puts("Bad connection attempt.");
}

bool RakWrapperNS::RakClient::Send( const char *data, const int length)
{
	return m_client->Send(data, length, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
}

void RakWrapperNS::RakClient::ReleaseCurrentPacket()
{
	if (m_currPacket)
	{
		m_client->DeallocatePacket(m_currPacket);
		m_currPacket = NULL;
	}
}

Packet* RakWrapperNS::RakClient::ReceivePacket()
{
	return m_client->Receive();
}

//---------------------------
// END RAKCLIENT
//---------------------------



//---------------------------
// BEGIN RAKSERVER
//---------------------------

RakWrapperNS::RakServer::RakServer() {
	// Create RakServer Interface
	m_server = RakNetworkFactory::GetRakServerInterface();
}

RakWrapperNS::RakServer::~RakServer() 
{
	// Destruct RakServer Interface
	ReleaseCurrentPacket();
	// Broadcast to clients that the server is disconnecting.
	m_server -> Disconnect(300);
	// Clean up server interface allocation
	RakNetworkFactory::DestroyRakServerInterface(m_server);
}

void RakWrapperNS::RakServer::Connect(const char* serverName) 
{
	// Security settings. Haven't really checked this out. Assuming the default will be okay for now.
	m_server -> InitializeSecurity(0,0);

	// Packet enumeration data
	EnumerationDataStruct enumerationDataStruct;
	strcpy(enumerationDataStruct.serverName, serverName);
	// Set server statistics using the defined struct
	m_server->SetStaticServerData((char*)&enumerationDataStruct, sizeof(EnumerationDataStruct));
	// Boolean value tells us whether or not the server was able to start successfully
	bool serverStarted = m_server -> Start(32, 0, 30, SERVER_PORT);
	if (serverStarted)
		puts("Server started, waiting for connections.");
	else
		puts("Server failed to start.");
}

bool RakWrapperNS::RakServer::Send( const char *data, const int length)
{
	// Broadcasts to everyone
	return m_server->Send(data, length, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true);
}

void RakWrapperNS::RakServer::ReleaseCurrentPacket()
{
	if (m_currPacket)
	{
		m_server->DeallocatePacket(m_currPacket);
		m_currPacket = NULL;
	}
}

Packet* RakWrapperNS::RakServer::ReceivePacket()
{
	return m_server->Receive();
}


//---------------------------
// END RAKSERVER
//---------------------------
