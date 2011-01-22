#include "Multiplayer.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "BitStream.h"
#include <stdlib.h> // For atoi
#include <cstring> // For strlen
#include <conio.h>
#include "RakNetStatistics.h"
#include "GetTime.h"
#include "PacketEnumerations.h"
#include "MTUSize.h"
#include <stdio.h>

#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h" // Developers of a certain platform will know what to do here.
#elif defined(_WIN32)
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#endif

bool quit;
bool sentPacket=false;

#define BIG_PACKET_SIZE 1000000

RakPeerInterface *peer1, *peer2;
char *text;

template <class InterfaceType>
class BigPacketMultiplayer : public Multiplayer<InterfaceType>
{
	void ProcessUnhandledPacket( Packet *packet, unsigned char packetIdentifier, InterfaceType *interfaceType )
	{
		if (peer2)
		{
			RakNetStatisticsStruct *rss=peer2->GetStatistics(peer2->GetPlayerIDFromIndex(0));
			StatisticsToString(rss, text, 2);
			printf("%s", text);
		}
		if (peer1)
		{
			RakNetStatisticsStruct *rss=peer1->GetStatistics(peer1->GetPlayerIDFromIndex(0));
			StatisticsToString(rss, text, 2);
			printf("%s", text);
		}

		if (packet->data[0]==255)
		{
			bool dataValid=true;
			for (int i=1; i < BIG_PACKET_SIZE; i++)
			{
                if (packet->data[i]!=i%256)
				{
					dataValid=false;
					break;
				}
			}

			if (dataValid)
				printf("Test succeeded. %i bytes.\n", packet->length);
			else
				printf("Test failed. %i bytes.\n", packet->length);
		}
		else
			printf("Unknown packet %i: Test failed. %i bytes.\n", packet->data[0], packet->length);

		quit=true;
	}
};

int main(void)
{
	peer1=peer2=0;
	
	BigPacketMultiplayer<RakPeerInterface> peerMP1;
	Multiplayer<RakPeerInterface> peerMP2;

	text= new char [BIG_PACKET_SIZE];
	quit=false;
	RakNetTime nextStatTime = RakNet::GetTime() + 1000;
	char ch;

	printf("This is a test I use to test the packet splitting capabilities of RakNet\n");
	printf("All it does is send a large block of data to the feedback loop\n");
	printf("Difficulty: Beginner\n\n");

	printf("Hit 's' to run as sender, 'r' to run as receiver, space to run local.\n");
	ch=getch();
	if (ch=='r')
	{
		peer1=RakNetworkFactory::GetRakPeerInterface();
		printf("Working as receiver\n");
	}
	else if (ch=='s')
	{
		peer2=RakNetworkFactory::GetRakPeerInterface();
		printf("Working as sender\n");
		printf("Enter remote IP: ");
		gets(text);
		if (text[0]==0)
			strcpy(text, "127.0.0.1");
	}
	else
	{
		peer1=RakNetworkFactory::GetRakPeerInterface();
		peer2=RakNetworkFactory::GetRakPeerInterface();;
		strcpy(text, "127.0.0.1");
	}
	if (peer1)
	{
		peer1->SetMaximumIncomingConnections(1);
		peer1->Initialize(1, 60000, 0);
		peer1->SetMTUSize(1492);
		peer1->SetSplitMessageProgressInterval(100); // Get ID_DOWNLOAD_PROGRESS notifications
	}
	if (peer2)
	{
		peer2->Initialize(1, 60001, 0);
		peer2->Connect(text, 60000, 0, 0);
		peer2->SetMTUSize(1492);
	}
	Sleep(500);

	// Always apply the network simulator on two systems, never just one, with half the values on each.
	// Otherwise the flow control gets confused.
	//if (peer1)
	 // peer1->ApplyNetworkSimulator(128000, 0, 0);
	//if (peer2)
	//	peer2->ApplyNetworkSimulator(128000, 0, 0);

	RakNetTime start,stop;

	start=RakNet::GetTime();
	while (!quit)
	{
		if (peer2)
		{
			peer2->DeallocatePacket(peer2->Receive());

			if (sentPacket==false && peer2->GetPlayerIDFromIndex(0)!=UNASSIGNED_PLAYER_ID)
			{
				sentPacket=true;
				for (int i=0; i < BIG_PACKET_SIZE; i++)
					text[i]=i%256;
				text[0]=(char)255; // So it doesn't register as an internal ID
				peer2->Send(text, BIG_PACKET_SIZE, HIGH_PRIORITY, RELIABLE_ORDERED, 0, peer2->GetPlayerIDFromIndex(0), false);
				// Keep the stat from updating until the messages move to the thread or it quits right away
				nextStatTime=RakNet::GetTime()+1000;
			}
		}
		if (peer1)
		{
			Packet *packet = peer1->Receive();
			while (packet)
			{
				if (packet->data[0]==ID_DOWNLOAD_PROGRESS)
				{
					RakNet::BitStream progressBS(packet->data, packet->length, false);
					progressBS.IgnoreBits(8); // ID_DOWNLOAD_PROGRESS
					unsigned int progress;
					unsigned int total;
					unsigned int partLength;
					char data[MAXIMUM_MTU_SIZE];
					progressBS.Read(progress);
					progressBS.Read(total);
					progressBS.Read(partLength);
					progressBS.Read(data, partLength);

					printf("Download progress: msgID=%i Progress %i/%i Partsize=%i\n",
						(unsigned char) data[0],
						progress,
						total,
						partLength);
				}
				else if (packet->data[0]>=ID_USER_PACKET_ENUM)
				{
					if (packet->data[0]==255)
					{
						bool dataValid=true;
						for (int i=1; i < BIG_PACKET_SIZE; i++)
						{
							if (packet->data[i]!=i%256)
							{
								dataValid=false;
								break;
							}
						}

						if (dataValid)
							printf("Test succeeded. %i bytes.\n", packet->length);
						else
							printf("Test failed. %i bytes.\n", packet->length);
					}
					else
						printf("Unknown packet %i: Test failed. %i bytes.\n", packet->data[0], packet->length);

					quit=true;
				}

				peer1->DeallocatePacket(packet);
				packet = peer1->Receive();
			}
		}

		if (RakNet::GetTime() > nextStatTime)
		{
			nextStatTime=RakNet::GetTime()+1000;
			RakNetStatisticsStruct *rssSender,*rssReceiver;
			if (peer2)
			{
				rssSender=peer2->GetStatistics(peer2->GetPlayerIDFromIndex(0));
				if (rssSender)
				{
					printf("Sender: %i waiting. %i waiting for ack. Got %i acks. KBPS=%.1f. \n", rssSender->messageSendBuffer[HIGH_PRIORITY], rssSender->messagesOnResendQueue,rssSender->acknowlegementsReceived, rssSender->bitsPerSecond/1000);
					if (peer1==0)
						printf("\n");
					if (sentPacket && rssSender->messageSendBuffer[HIGH_PRIORITY]==0 && rssSender->messagesOnResendQueue==0 && peer1==0)
					{
						RakNetStatisticsStruct *rss=peer2->GetStatistics(peer2->GetPlayerIDFromIndex(0));
						StatisticsToString(rss, text, 2);
						printf("%s", text);
						quit=true;
					}
				}
			}
			if (peer1)
			{
				rssReceiver=peer1->GetStatistics(peer1->GetPlayerIDFromIndex(0));
				if (rssReceiver)
					printf("Receiver: %i acks waiting.\n", rssReceiver->acknowlegementsPending);
			}
		}

#ifdef _WIN32
		Sleep(100);
#else
		usleep(100 * 1000);
#endif
	}
	stop=RakNet::GetTime();
	double seconds = (double)(stop-start)/1000.0;
	printf("%i bytes per second transfered. Press enter to quit\n", (int)((double)(BIG_PACKET_SIZE) / seconds )) ;
	gets(text);

	delete []text;
	RakNetworkFactory::DestroyRakPeerInterface(peer1);
	RakNetworkFactory::DestroyRakPeerInterface(peer2);

	return 0;
}
