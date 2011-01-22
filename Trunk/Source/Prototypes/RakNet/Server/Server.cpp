// RakNet Server Prototype
// Jared C Jones (jcjones86@gmail.com)
// November, 2006
// Gremlin Games
// Based off RakNet Chat Sample
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakServerInterface.h"
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
	// Create server instance
	RakServerInterface *server = RakNetworkFactory::GetRakServerInterface();
	// Server statistics
	RakNetStatisticsStruct *rss;
	// This was taken from the example. Have not looked into security settings. This will do for now.
	server -> InitializeSecurity(0,0);
	// Number of addresses the server will connect on
	int i = server -> GetNumberOfAddresses();
	// Use this to set a password for the server
	//server -> SetPassword("Something");
	// Holds packets
	Packet* currPacket;
	// GetPacketIdentifier returns this
	unsigned char packetIdentifier;
	// Record the first client that connects to us so we can pass it to the ping function
	PlayerID clientID = UNASSIGNED_PLAYER_ID;
	// Port number
	char portstring[30];
	// Packet enumeration data
	EnumerationDataStruct enumerationDataStruct;

	// Ask user for server setup options
	puts("Enter the server port to listen on");
	gets(portstring);
	if (portstring[0]==0)
		strcpy(portstring, "10000");

	puts("Enter the server name  (up to 40 characters)");
	gets(enumerationDataStruct.serverName);
	if (enumerationDataStruct.serverName[0]==0)
		strcpy(enumerationDataStruct.serverName, "Default server name");

	puts("Enter the server message of the day  (up to 50 characters)");
	gets(enumerationDataStruct.MOTD);
	if (enumerationDataStruct.MOTD[0]==0)
		strcpy(enumerationDataStruct.MOTD, "Default MOTD");

	// Set server statistics using the defined struct
	server -> SetStaticServerData((char*)&enumerationDataStruct, sizeof(EnumerationDataStruct));

	puts("Starting server.");
	// Boolean value tells us whether or not the server was able to start successfully
	bool serverStarted = server -> Start(32, 0, 30, atoi(portstring));
	if (serverStarted)
		puts("Server started, waiting for connections.");
	else
	{ 
		puts("Server failed to start.  Terminating.");
		exit(1);
	}

	// Will be used for any commands or chat lines to send
	char message[2048];

	// Loop indefinately for input
	while (1)
	{
		if (kbhit())
		{
			gets(message);

			// This command will break the loop
			if (strcmp(message, "quit") == 0)
			{
				puts("Quitting.");
				break;
			}
			// Print server stats
			if (strcmp(message, "stat") == 0)
			{
				// Gets stats for first connected client
				rss = server -> GetStatistics(server->GetPlayerIDFromIndex(0));
				// Converts stats to a readable string
				StatisticsToString(rss, message, 2);
				printf("%s", message);
				// Prints ping of first connected client
				printf("Ping %i\n", server -> GetAveragePing(server -> GetPlayerIDFromIndex(0)));
				continue;
			}
			if (strcmp(message, "ping") == 0)
			{
				if (server -> GetConnectedPlayers() > 0)
					server -> PingPlayer(clientID);
				continue;
			}

			char messageToClients[1024]; // String to send to clients
			messageToClients[0] = 0;
			strcpy(messageToClients, "Server: ");
			strcat(messageToClients, message);
			server -> Send(messageToClients, (const int) strlen(messageToClients) + 1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true);
		}

		// Get a packet from either the server or the client
		currPacket = server -> Receive();
		
		// Check if packet has been received
		if (currPacket == 0)
			continue;

		// We got a packet, get the identifier
		packetIdentifier = GetPacketIdentifier(currPacket);

		// Check if this is a network message packet
		switch (packetIdentifier)
		{
			case ID_DISCONNECTION_NOTIFICATION:
				printf("Client disconnected.\n");
				break;
			case ID_NEW_INCOMING_CONNECTION:
				printf("Client connected.\n");
				clientID = currPacket -> playerId;
				break;
			case ID_RECEIVED_STATIC_DATA:
				if (server -> GetStaticClientData(currPacket -> playerId))
				{
					printf("Received static data from a client.\n");
					printf("%s has connected\n", ((StaticClientDataStruct*)(server -> GetStaticClientData(currPacket -> playerId)) -> GetData()) -> name);
				}
				break;
			case ID_CONNECTION_LOST:
				printf("Client disconnected abnormally.\n");
				break;
			default:
				// Prefix data with client name
				printf("%s: %s\n", ((StaticClientDataStruct*)(server -> GetStaticClientData(currPacket -> playerId)) -> GetData()) -> name, currPacket -> data);
				// Relay message to all clients
				sprintf(message, "%s: %s", ((StaticClientDataStruct*)(server -> GetStaticClientData(currPacket -> playerId)) -> GetData()) -> name, currPacket -> data);
				server -> Send(message, (const int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0, currPacket -> playerId, true);
				break;
		}
		// Need to clean up allocation for packet
		server -> DeallocatePacket(currPacket);
	}
	server -> Disconnect(300);
	// Clean up server
	RakNetworkFactory::DestroyRakServerInterface(server);
	// Exit without error
	return 0;
}

// Copied from Multiplayer.cpp
// If the first byte is ID_TIMESTAMP, then we want the 5th byte
// Otherwise we want the 1st byte
unsigned char GetPacketIdentifier(Packet *p)
{
	if (p == 0)
		return 255;
	if ((unsigned char)p -> data[0] == ID_TIMESTAMP)
	{
		assert(p -> length > sizeof(unsigned char) + sizeof(unsigned long));
		return (unsigned char) p -> data[sizeof(unsigned char) + sizeof(unsigned long)];
	}
	else
		return (unsigned char) p -> data[0];
}