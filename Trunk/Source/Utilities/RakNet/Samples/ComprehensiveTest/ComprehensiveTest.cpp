#include "Multiplayer.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "BitStream.h"
#include <stdlib.h> // For atoi
#include <cstring> // For strlen
#include "Rand.h"
#include "RakNetStatistics.h"

#ifdef _WIN32
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#endif

//#define _VERIFY_RECIPIENTS
#define _DO_PRINTF

#define NUM_PEERS 10

void RPC1(RPCParameters *rpcParameters)
{
	unsigned short packetPort, interfacePort;
	interfacePort=rpcParameters->recipient->GetInternalID().port;
	memcpy((char*)&packetPort, rpcParameters->input, sizeof(unsigned short));
#ifdef _VERIFY_RECIPIENTS
	assert(packetPort==65535 || packetPort==interfacePort); // If this assert hits then the wrong recipient got the packet
#endif
#ifdef _DO_PRINTF
	if (rpcParameters->input)
		printf("RPC1: %s\n", rpcParameters->input);
	else
		printf("RPC1\n");
#endif
}

void RPC2(RPCParameters *rpcParameters)
{
	unsigned short packetPort, interfacePort;
	interfacePort=rpcParameters->recipient->GetInternalID().port;
	memcpy((char*)&packetPort, rpcParameters->input, sizeof(unsigned short));
#ifdef _VERIFY_RECIPIENTS
	assert(packetPort==65535 || packetPort==interfacePort); // If this assert hits then the wrong recipient got the packet
#endif
#ifdef _DO_PRINTF
	if (rpcParameters->input)
		printf("RPC1: %s\n", rpcParameters->input);
	else
		printf("RPC1\n");
#endif
}

void RPC3(RPCParameters *rpcParameters)
{
	unsigned short packetPort, interfacePort;
	interfacePort=rpcParameters->recipient->GetInternalID().port;
	memcpy((char*)&packetPort, rpcParameters->input, sizeof(unsigned short));
#ifdef _VERIFY_RECIPIENTS
	assert(packetPort==65535 || packetPort==interfacePort); // If this assert hits then the wrong recipient got the packet
#endif
#ifdef _DO_PRINTF
	if (rpcParameters->input)
		printf("RPC1: %s\n", rpcParameters->input);
	else
		printf("RPC1\n");
#endif
}

void RPC4(RPCParameters *rpcParameters)
{
	unsigned short packetPort, interfacePort;
	interfacePort=rpcParameters->recipient->GetInternalID().port;
	memcpy((char*)&packetPort, rpcParameters->input, sizeof(unsigned short));
#ifdef _VERIFY_RECIPIENTS
	assert(packetPort==65535 || packetPort==interfacePort); // If this assert hits then the wrong recipient got the packet
#endif
#ifdef _DO_PRINTF
	if (rpcParameters->input)
		printf("RPC1: %s\n", rpcParameters->input);
	else
		printf("RPC1\n");
#endif
}


template <class InterfaceType>
class ComprehensiveTestMultiplayer : public Multiplayer<InterfaceType>
{
	virtual void ProcessUnhandledPacket( Packet *packet, unsigned char packetIdentifier, InterfaceType *interfaceType )
	{
		unsigned short packetPort, interfacePort;
		memcpy((char*)&packetPort, packet->data+1, sizeof(unsigned short));
		interfacePort=interfaceType->GetInternalID().port;
#ifdef _VERIFY_RECIPIENTS
		assert(packetPort==65535 || packetPort==interfacePort); // If this assert hits then the wrong recipient got the packet
#endif
#ifdef _DO_PRINTF
		printf("%i: [%i] %s", packet->playerId.port, packet->data[0], packet->data+1);
#endif
	}
};

int main(void)
{
	ComprehensiveTestMultiplayer<RakPeerInterface> peerMultiplayers[NUM_PEERS];
	RakPeerInterface *peers[NUM_PEERS];
	int peerIndex;
	float nextAction;
	int i;

	printf("This is just a test app to run a bit of everything to test for crashes.\n");
	printf("Difficulty: Intermediate\n\n");

	char data[8096];

	int seed = 12345;
	printf("Using seed %i\n", seed);
	seedMT(seed);

	for (i=0; i < NUM_PEERS; i++)
	{
		peers[i]=RakNetworkFactory::GetRakPeerInterface();
		peers[i]->SetMaximumIncomingConnections(NUM_PEERS);
		peers[i]->Initialize(NUM_PEERS, 60000+i, 0);
		peers[i]->SetOfflinePingResponse("Offline Ping Data", (int)strlen("Offline Ping Data")+1);
		REGISTER_STATIC_RPC(peers[i], RPC1);
		REGISTER_STATIC_RPC(peers[i], RPC2);
		REGISTER_STATIC_RPC(peers[i], RPC3);
		REGISTER_STATIC_RPC(peers[i], RPC4);
	}

	for (i=0; i < NUM_PEERS; i++)
	{
		peers[i]->Connect("127.0.0.1", 60000+(randomMT()%NUM_PEERS), 0, 0);		
	}

	RakNetTime endTime = RakNet::GetTime()+600000;
	while (RakNet::GetTime()<endTime)
	{
		nextAction = frandomMT();

		if (nextAction < .04f)
		{
			// Initialize
			peerIndex=randomMT()%NUM_PEERS;
			peers[peerIndex]->Initialize(NUM_PEERS, 60000+peerIndex, randomMT()%30);
			peers[peerIndex]->Connect("127.0.0.1", 60000+randomMT() % NUM_PEERS, 0, 0);
		}
		else if (nextAction < .09f)
		{
			// Connect
			peerIndex=randomMT()%NUM_PEERS;
			peers[peerIndex]->Connect("127.0.0.1", 60000+randomMT() % NUM_PEERS, 0, 0);
		}
		else if (nextAction < .10f)
		{
			// Disconnect
			peerIndex=randomMT()%NUM_PEERS;
		//	peers[peerIndex]->Disconnect(randomMT() % 100);
		}
		else if (nextAction < .12f)
		{
			// GetConnectionList
			peerIndex=randomMT()%NUM_PEERS;
			PlayerID remoteSystems[NUM_PEERS];
			unsigned short numSystems=NUM_PEERS;
			peers[peerIndex]->GetConnectionList(remoteSystems, &numSystems);
			if (numSystems>0)
			{
#ifdef _DO_PRINTF
				printf("%i: ", 60000+numSystems);
				for (i=0; i < numSystems; i++)
				{
					printf("%i: ", remoteSystems[i].port);
				}
				printf("\n");
#endif
			}			
		}
		else if (nextAction < .14f)
		{
			// Send
			int dataLength;
			PacketPriority priority;
			PacketReliability reliability;
			unsigned char orderingChannel;
			PlayerID target;
			bool broadcast;

		//	data[0]=ID_RESERVED1+(randomMT()%10);
			data[0]=ID_RESERVED9;
			dataLength=3+(randomMT()%8000);
//			dataLength=600+(randomMT()%7000);
			priority=(PacketPriority)(randomMT()%(int)NUMBER_OF_PRIORITIES);
			reliability=(PacketReliability)(randomMT()%((int)RELIABLE_SEQUENCED+1));
			orderingChannel=randomMT()%32;
			if ((randomMT()%NUM_PEERS)==0)
				target=UNASSIGNED_PLAYER_ID;
			else
				target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
#ifdef _MSC_VER
#pragma warning( disable : 4800 ) // warning C4800: 'unsigned int' : forcing value to bool 'true' or 'false' (performance warning)
#endif
			broadcast=(bool)(randomMT()%2);
#ifdef _VERIFY_RECIPIENTS
			broadcast=false; // Temporarily in so I can check recipients
#endif

			peerIndex=randomMT()%NUM_PEERS;
			sprintf(data+3, "dataLength=%i priority=%i reliability=%i orderingChannel=%i target=%i broadcast=%i\n", dataLength, priority, reliability, orderingChannel, target.port, broadcast);
			//unsigned short localPort=60000+i;
#ifdef _VERIFY_RECIPIENTS
			memcpy((char*)data+1, (char*)&target.port, sizeof(unsigned short));
#endif
			data[dataLength-1]=0;
			peers[peerIndex]->Send(data, dataLength, priority, reliability, orderingChannel, target, broadcast);
		}
		else if (nextAction < .18f)
		{
			// RPC
			int dataLength;
			PacketPriority priority;
			PacketReliability reliability;
			unsigned char orderingChannel;
			PlayerID target;
			bool broadcast;
			char RPCName[10];

			data[0]=ID_RESERVED9+(randomMT()%10);
			dataLength=3+(randomMT()%8000);
//			dataLength=600+(randomMT()%7000);
			priority=(PacketPriority)(randomMT()%(int)NUMBER_OF_PRIORITIES);
			reliability=(PacketReliability)(randomMT()%((int)RELIABLE_SEQUENCED+1));
			orderingChannel=randomMT()%32;
			if ((randomMT()%NUM_PEERS)==0)
				target=UNASSIGNED_PLAYER_ID;
			else
				target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			broadcast=(bool)(randomMT()%2);
#ifdef _VERIFY_RECIPIENTS
			broadcast=false; // Temporarily in so I can check recipients
#endif

			peerIndex=randomMT()%NUM_PEERS;
			sprintf(data+3, "dataLength=%i priority=%i reliability=%i orderingChannel=%i target=%i broadcast=%i\n", dataLength, priority, reliability, orderingChannel, target.port, broadcast);
#ifdef _VERIFY_RECIPIENTS
			memcpy((char*)data, (char*)&target.port, sizeof(unsigned short));
#endif
			data[dataLength-1]=0;
			sprintf(RPCName, "RPC%i", (randomMT()%4)+1);
			peers[peerIndex]->RPC(RPCName, data, dataLength*8, priority, reliability, orderingChannel, target, broadcast, dataLength > 5 ? (bool)(randomMT()%10) : false, UNASSIGNED_NETWORK_ID,0);
		}
		else if (nextAction < .181f)
		{
			// CloseConnection
			PlayerID target;
			peerIndex=randomMT()%NUM_PEERS;
			target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			peers[peerIndex]->CloseConnection(target, (bool)(randomMT()%2), 0);
		}
		else if (nextAction < .20f)
		{
			// Offline Ping
			peerIndex=randomMT()%NUM_PEERS;
			peers[peerIndex]->Ping("127.0.0.1", 60000+(randomMT()%NUM_PEERS), (bool)(randomMT()%2));
		}
		else if (nextAction < .21f)
		{
			// Online Ping
			PlayerID target;
			target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			peerIndex=randomMT()%NUM_PEERS;
			peers[peerIndex]->Ping(target);
		}
		else if (nextAction < .22f)
		{
			PlayerID target, myPlayerId;
			peerIndex=randomMT()%NUM_PEERS;
			myPlayerId=peers[peerIndex]->GetInternalID();
			target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			peers[peerIndex]->SetRemoteStaticData(myPlayerId, "My player ID", (int)strlen("My player ID")+1);
			peers[peerIndex]->SetRemoteStaticData(target, "Remote player ID", (int)strlen("Remote player ID")+1);
		}
		else if (nextAction < .221f)
		{
			// GetRemoteStaticData
			PlayerID target, myPlayerId;
			RakNet::BitStream *staticData;
			peerIndex=randomMT()%NUM_PEERS;
			myPlayerId=peers[peerIndex]->GetInternalID();
			target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			if ((randomMT()%2)==0)
			{
				staticData = peers[peerIndex]->GetRemoteStaticData(target);
#ifdef _DO_PRINTF
				if (staticData && staticData->GetNumberOfBitsUsed()>0)
					printf("Target Static Data = %s\n", staticData->GetData());
#endif
			}
			else
			{
				staticData = peers[peerIndex]->GetRemoteStaticData(myPlayerId);
#ifdef _DO_PRINTF
				if (staticData && staticData->GetNumberOfBitsUsed()>0)
					printf("Local Static Data = %s\n", staticData->GetData());
#endif
			}			
		}
		else if (nextAction < .24f)
		{
			// SetCompileFrequencyTable
			peerIndex=randomMT()%NUM_PEERS;
			peers[peerIndex]->SetCompileFrequencyTable(randomMT()%2);
		}
		else if (nextAction < .25f)
		{
			// GetStatistics
			PlayerID target, myPlayerId;
			RakNetStatisticsStruct *rss;
			myPlayerId=peers[peerIndex]->GetInternalID();
			target=peers[peerIndex]->GetPlayerIDFromIndex(randomMT()%NUM_PEERS);
			peerIndex=randomMT()%NUM_PEERS;
			rss=peers[peerIndex]->GetStatistics(myPlayerId);
			if (rss)
			{
				StatisticsToString(rss, data, 0);
#ifdef _DO_PRINTF
				printf("Statistics for local system %i:\n%s", myPlayerId.port, data);
#endif
			}
			
			rss=peers[peerIndex]->GetStatistics(target);
			if (rss)
			{
				StatisticsToString(rss, data, 0);
#ifdef _DO_PRINTF
				printf("Statistics for target system %i:\n%s", target.port, data);
#endif
			}			
		}

		for (i=0; i < NUM_PEERS; i++)
			peerMultiplayers[i].ProcessPackets(peers[i]);

#ifdef _WIN32
		Sleep(0);
#else
		usleep(0);
#endif
	}


	for (i=0; i < NUM_PEERS; i++)
		RakNetworkFactory::DestroyRakPeerInterface(peers[i]);

	return 0;
}
