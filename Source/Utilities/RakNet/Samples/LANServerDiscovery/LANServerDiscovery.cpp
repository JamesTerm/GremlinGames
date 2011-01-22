// ----------------------------------------------------------------------
// RakNet version 1.0
// Filename ChatExample.cpp
// Created by Rakkar Software (rakkar@rakkarsoftware.com) January 24, 2003
// Very basic chat engine example
// ----------------------------------------------------------------------
#include "PacketEnumerations.h"
#include "RakNetworkFactory.h"
#include "RakClientInterface.h"
#include "RakServerInterface.h"
#include "NetworkTypes.h"
#include "GetTime.h"
#include "BitStream.h"
#include <assert.h>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#endif

#ifdef WIN32
#define SLEEP(arg)  ( Sleep( (arg) ) )
#else
#define SLEEP(arg)  ( usleep( (arg) *1000 ) )
#endif

int main(void)
{
	// Pointers to the interfaces of our server and client.
	// Note we can easily have both in the same program
	RakClientInterface *client;
	RakServerInterface *server;
	bool b;
	char str[256];
	char serverPort[30], clientPort[30];
	RakNetTime quitTime;
	// Holds packets
	Packet* p;	

	printf("A client / server sample showing how clients can broadcast offline packets\n");
	printf("to find active servers.\n");
	printf("Difficulty: Beginner\n\n");

	printf("Instructions:\nRun one or more servers on the same port.\nRun a client and it will get pongs from those servers.\n");
	printf("Run as (s)erver or (c)lient?\n");
	gets(str);

	if (str[0]=='s' || str[0]=='S')
	{
		client=0;
		server=RakNetworkFactory::GetRakServerInterface();
		// A server
		printf("Enter the server port\n");
		gets(serverPort);
		if (serverPort[0]==0)
			strcpy(serverPort, "60001");

		printf("Starting server.\n");
		// The server has to be started to respond to pings.
		b = server->Start(2, 0, 30, atoi(serverPort));
		if (b)
			printf("Server started, waiting for connections.\n");
		else
		{ 
			printf("Server failed to start.  Terminating.\n");
			exit(1);
		}
	}
	else
	{
		client=RakNetworkFactory::GetRakClientInterface();
		server=0;

		// Get our input
		printf("Enter the client port to listen on, or 0\n");
		gets(clientPort);
		if (clientPort[0]==0)
			strcpy(clientPort, "60000");
		printf("Enter the port to ping\n");
		gets(serverPort);
		if (serverPort[0]==0)
			strcpy(serverPort, "60001");

		// Connecting the client is very simple.  0 means we don't care about
		// a connectionValidationInteger, and false for low priority threads
		// All 255's mean broadcast
		client->PingServer("255.255.255.255", atoi(serverPort), atoi(clientPort), true);

		printf("Pinging\n");
	}

	printf("How many seconds to run this sample for?\n");
	gets(str);
	if (str[0]==0)
	{
		printf("Defaulting to 5 seconds\n");
		quitTime = RakNet::GetTime() + 5000;
	}
	else
		quitTime = RakNet::GetTime() + atoi(str) * 1000;

	// Loop for input
	while (RakNet::GetTime() < quitTime)
	{
		if (server)
			p = server->Receive();
		else 
			p = client->Receive();

		if (p==0)
		{
			SLEEP(30);
			continue;
		}
		if (server)
			server->DeallocatePacket(p);
		else
		{
			if (p->data[0]==ID_PONG)
			{
				RakNetTime time;
				memcpy((char*)&time, p->data+1, sizeof(RakNetTime));
				printf("Got pong from %s with time %i\n", client->PlayerIDToDottedIP(p->playerId), RakNet::GetTime() - time);
			}
			client->DeallocatePacket(p);
		}

		SLEEP(30);
	}

	// We're done with the network
	if (server)
		RakNetworkFactory::DestroyRakServerInterface(server);
	if (client)
		RakNetworkFactory::DestroyRakClientInterface(client);

	return 0;
}
