// RakNet Client Prototype
// Jared C Jones (jcjones86@gmail.com)
// November, 2006
// Gremlin Games
// Based off RakNet Chat Sample
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakClientInterface.h"
#include "NetworkTypes.h"
#include "BitStream.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <conio.h>

unsigned char GetPacketIdentifier(Packet *p);
const int MOTD_MAXIMUM_LENGTH = 50;
const int SERVER_NAME_MAXIMUM_LENGTH = 40;
const int SCDS_MAXIMUM_NAME_LENGTH = 40;

struct EnumerationDataStruct
{
	char MOTD[MOTD_MAXIMUM_LENGTH];
	char serverName[SERVER_NAME_MAXIMUM_LENGTH];
};

struct StaticClientDataStruct
{
	char name[SCDS_MAXIMUM_NAME_LENGTH];
};

int main(void)
{
	// Create Client Instance
	RakClientInterface *client=RakNetworkFactory::GetRakClientInterface();
	// Server statistics
	RakNetStatisticsStruct *rss;
	// This was taken from the example. Have not looked into security settings. This will do for now.
	client -> InitializeSecurity(0,0);
	// Will need this if a password is going to be used
	//client->SetPassword("Something");
	// Holds packets
	Packet* currPacket;
	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;
	// Just so we can remember where the packet came from
	bool isServer;
	// Record the first client that connects to us so we can pass it to the ping function
	PlayerID clientID = UNASSIGNED_PLAYER_ID;
	// User data
	char ip[30], serverPort[30], clientPort[30];
	StaticClientDataStruct staticClientData;
	// Dealing with client
	isServer = false;

	// Ask user for server setup options
	puts("Enter the client port to listen on");
	gets(clientPort);
	if (clientPort[0] == 0)
		strcpy(clientPort, "5000");

	puts("Enter your name (up to 40 characters)");
	gets(staticClientData.name);
	if (staticClientData.name[0] == 0)
		strcpy(staticClientData.name, "Default client name");

	client -> SetStaticClientData(UNASSIGNED_PLAYER_ID, (char*)&staticClientData, sizeof(StaticClientDataStruct));

	puts("Enter IP to connect to");
	gets(ip);
	client -> AllowConnectionResponseIPMigration(false);
	// If no IP was entered, set it as the loopback address
	if (ip[0] == 0)
		strcpy(ip, "127.0.0.1");
		
	puts("Enter the port to connect to");
	gets(serverPort);
	if (serverPort[0]==0)
		strcpy(serverPort, "10000");

	bool clientConnected = client -> Connect(ip, atoi(serverPort), atoi(clientPort), 0, 30);
	
	if (clientConnected)
		puts("Attempting connection...");
	else
	{
		puts("Bad connection attempt. Terminating.");
		exit(1);
	}

	puts("'quit' to quit. 'stat' to show stats. 'ping' to ping. Type to talk.");
	char message[2048];

	// Loop for input
	while (1)
	{
		if (kbhit())
		{
			gets(message);

			if (strcmp(message, "quit") == 0)
			{
				puts("Quitting.");
				break;
			}
			if (strcmp(message, "stat") == 0)
			{
				rss = client -> GetStatistics();
				StatisticsToString(rss, message, 2);
				printf("%s", message);
				printf("Ping: %i\n", client -> GetAveragePing());
				continue;
			}
			if (strcmp(message, "ping") == 0)
			{
				if (client -> IsConnected())
					client -> PingServer();
				continue;
			}

			// Send message to server
			client -> Send(message, (int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
		}

		// Receive some packet
		currPacket = client -> Receive();
	
		// Check if a packet was received. If not, continue to next iteration in while loop.
		if (currPacket == 0)
			continue;

		// We got a packet, get the identifier
		packetIdentifier = GetPacketIdentifier(currPacket);

		// Check if this is a network message packet
		switch (packetIdentifier)
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
				// This is what is broadcast when first connecting to a server.
				if (client -> GetStaticServerData())
				{
					printf("Server name: %s\n", ((EnumerationDataStruct*)(client -> GetStaticServerData()) -> GetData()) -> serverName);
					printf("Server MOTD: %s\n", ((EnumerationDataStruct*)(client -> GetStaticServerData()) -> GetData()) -> MOTD);
				}				
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
			default:
				printf("%s\n", currPacket -> data);
				break;
		}
		// Clean up packet allocation
		client -> DeallocatePacket(currPacket);
	}
	// Broadcast to server that the client is disconnecting.
	client -> Disconnect(300);
	// Clean up client interface allocation
	RakNetworkFactory::DestroyRakClientInterface(client);
	// Return without error
	return 0;
}

// Copied from Multiplayer.cpp
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
