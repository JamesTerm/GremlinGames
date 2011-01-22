// ClientMain.cpp
// December 6, 2006
// Rick Pingry
// Gremlin Games
#include "..\RakWrapper\RakWrapper.h"
#include "Common.h"
#include <iostream>
#include <string>
#include <conio.h>
using namespace std;

void ProcessMessage(RakWrapperNS::RakClient& client)
{
	RakWrapperNS::RakWrapper::PacketPropertiesStruct packProp = client.GetNextPacketType();

	if (!packProp.length)
		return;

	int senderID = -1;
	switch(packProp.type) 
	{
		case vector:
			{
				VectorStruct *va = new VectorStruct[packProp.length];
				client.GetNextPacket<VectorStruct>(va, packProp.length, senderID);

				cout << "Received vector packet from " << senderID << endl;
				for (int i=0; i<packProp.length; i++)
					cout << "va[" << i << "] = " << "(" << va[i].x << ", " << va[i].y << ", " << va[i].z << ")" << endl;

				delete[] va;
			}
			break;
		case integer:
			{
				int *ia = new int[packProp.length];
				client.GetNextPacket<int>(ia, packProp.length, senderID);

				cout << "Received integer array packet from " << senderID << endl;
				for (int i=0; i<packProp.length; i++)
					cout << "ia[" << i << "] = " << ia[i] << endl;

				delete[] ia;
			}
			break;
		case str:
			{
				char *ca = new char[packProp.length];
				client.GetNextPacket<char>(ca, packProp.length, senderID);

				cout << "Received character array packet from " << senderID << endl;
				cout << ca << endl;

				delete[] ca;
			}
			break;
	}
}

int main(void)
{
	char playerName[MAXIMUM_NAME_LENGTH];
	char serverIP[32];

	cout << "Starting Evolutions Client" << endl;
	cout << "==========================" << endl;

	cout << "Enter Player Name: ";
	cin >> playerName;
	cout << "Enter Server IP Address (or '0' for loop-back): ";
	cin >> serverIP;

	RakWrapperNS::RakClient client;
	client.Connect(playerName, serverIP);

	cout << "Type 'ints', 'vectors', 'quit', or any other string" << endl;

	string message;

	while (true) {
		if (kbhit())
		{
			cin >> message;

			if (message == "quit") {
				cout << "Goodbye." << endl;
				break;
			}
			else if (message == "vectors") {
				VectorStruct va[10];
				for (int i=0; i<10; i++) {
					va[i].x = i * 1.1;
					va[i].y = i * 1.2;
					va[i].z = i * 1.3;
				}
				cout << "Sending vectors Packet to Server" << endl;
				client.SendPacket<VectorStruct>(RakWrapperNS::RakWrapper::PacketPropertiesStruct(vector, 10), va);
			}
			else if (message == "ints") {
				int ia[5];
				for (int i=0; i<5; i++)
					ia[i] = i;
				cout << "Sending ints Packet to Server" << endl;
				client.SendPacket<int>(RakWrapperNS::RakWrapper::PacketPropertiesStruct(integer, 5), ia);
			}
			else {
				cout << "Sending string Packet to Server" << endl;
				client.SendPacket<char>(RakWrapperNS::RakWrapper::PacketPropertiesStruct(str, (short)message.length()+1), message.c_str());
			}
		}


		ProcessMessage(client);

		/*if (message == "stat")
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
		}*/
		// Send message to server
		//client -> Send(message, (int) strlen(message)+1, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
	}

	/*
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

	*/
	// Return without error
	return 0;
}
