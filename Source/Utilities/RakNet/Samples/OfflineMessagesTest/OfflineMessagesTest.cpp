#include "Multiplayer.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "BitStream.h"
#include <stdlib.h> // For atoi
#include <cstring> // For strlen

#ifdef _WIN32
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#endif

int nextTest;


RakPeerInterface *peer1=RakNetworkFactory::GetRakPeerInterface();
RakPeerInterface *peer2=RakNetworkFactory::GetRakPeerInterface();

template <class InterfaceType>
class OfflineMessageTestMultiplayer : public Multiplayer<InterfaceType>
{
	void ReceiveAdvertisedSystem( Packet *packet, InterfaceType *interfaceType )
	{
		if (packet->length>1)
			printf("Got Advertise system with data: %s\n", packet->data+1);
		else
			printf("Got Advertise system with no data\n");

		peer2->Ping("127.0.0.1", 60001, false);
		nextTest++;
	}

	void ReceivePong( Packet *packet, InterfaceType *interfaceType )
	{
		// Peer or client. Response from a ping for an unconnected system.
		RakNetTime time, dataLength;
		memcpy( ( char* ) & time, packet->data + sizeof( unsigned char ), sizeof( unsigned int ) );
		dataLength = packet->length - sizeof( unsigned char ) - sizeof( unsigned int );
		printf( "ID_PONG from PlayerID:%u:%u on %p.\nPing is %i\nData is %i bytes long.\n", packet->playerId.binaryAddress, packet->playerId.port, interfaceType, time, dataLength );

		if ( dataLength > 0 )
			printf( "Data is %s\n", packet->data + sizeof( unsigned char ) + sizeof( unsigned int ) );

		nextTest++;
		// ProcessUnhandledPacket(packet, ID_PONG,interfaceType);
	}
};

int main(void)
{
	Multiplayer<RakPeerInterface> peerMP1;
	OfflineMessageTestMultiplayer<RakPeerInterface> peerMP2;
	char text[1024];
	bool sentPacket=false;
	nextTest=0;

	printf("This project tests the advertise system and offline ping messages.\n");
	printf("Difficulty: Beginner\n\n");

	peer1->SetMaximumIncomingConnections(1);
	peer1->Initialize(1, 60001, 0);
	peer2->Initialize(1, 60002, 0);
	peer1->SetOfflinePingResponse("Offline Ping Data", (int)strlen("Offline Ping Data")+1);

	printf("Systems started.  Waiting for advertise system packet\n");

	// Wait for connection to complete
#ifdef _WIN32
	Sleep(300);
#else
	usleep(300 * 1000);
#endif

	peer1->AdvertiseSystem("127.0.0.1", 60002,"hello world", (int)strlen("hello world")+1);

	while (nextTest!=2)
	{
		peerMP1.ProcessPackets(peer1);
		peerMP2.ProcessPackets(peer2);

#ifdef _WIN32
		Sleep(30);
#else
		usleep(30 * 1000);
#endif
	}

	printf("Test complete. Press enter to quit\n");
	gets(text);

	RakNetworkFactory::DestroyRakPeerInterface(peer1);
	RakNetworkFactory::DestroyRakPeerInterface(peer2);

	return 0;
}
