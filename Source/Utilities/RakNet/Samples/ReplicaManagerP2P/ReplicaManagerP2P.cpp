// In this test, you connect peers in an arbitrary topology.
// Each peer can create/destroy one object and can increase an integer in that object
// Each object is owned by one player
// If the owner disconnects, that object is deleted
// This is a more advanced sample of ReplicaManager.  Understand ReplicaManagerCS before looking at this one.

#include "NetworkTypes.h"
#include "StringTable.h"
#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include "BitStream.h"
#include "PacketEnumerations.h"
#include "ReplicaManager.h"
#include "Replica.h"

#ifdef _COMPATIBILITY_1
#include "Compatibility1Includes.h" // Developers of a certain platform will know what to do here.
#elif defined(_WIN32)
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#endif

#define MAX_PEERS 8

ReplicaManager replicaManager;
RakPeerInterface *rakPeer;

class TestObject : public Replica
{
public:
	TestObject()
	{
		testInteger=0;
		owner=UNASSIGNED_PLAYER_ID;
	}

	~TestObject()
	{
		replicaManager.DereferencePointer(this);
	}

	ReplicaReturnResult SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp )
	{
		// Don't send back to the owner of an object.
		// If we didn't prevent then the object would be created on the system that just sent it to us, then back again, forever in a feedback loop.
		if (playerId==owner)
			return REPLICA_PROCESSING_DONE;

		// This string was pre-registered in main with stringTable->AddString so we can send it with the string table and save bandwidth
		stringTable->EncodeString("TestObject", 255, outBitStream);

		// Write the owner when we construct the object, so we have it right away in order to prevent feedback loops
		outBitStream->Write(owner);

		return REPLICA_PROCESSING_DONE;
	}
	void SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId)
	{
		// Optional, nothing to send here.
	}
	ReplicaReturnResult ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId)
	{
		printf("Remote object owned by %s:%i destroyed\n", rakPeer->PlayerIDToDottedIP(owner), owner.port);
		replicaManager.Destruct(this, playerId, true); // Forward the destruct message to all other systems but the sender
		delete this;
		return REPLICA_PROCESSING_DONE;
	}
	ReplicaReturnResult SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId)
	{
		outBitStream->Write(inScope);
		return REPLICA_PROCESSING_DONE;
	}
	ReplicaReturnResult ReceiveScopeChange(RakNet::BitStream *inBitStream,PlayerID playerId)
	{
		return REPLICA_PROCESSING_DONE;
	}
	ReplicaReturnResult Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId)
	{
		// Don't send back to the owner of an object.
		if (playerId==owner)
			return REPLICA_PROCESSING_DONE;

		outBitStream->Write(testInteger);
		return REPLICA_PROCESSING_DONE;
	}
	ReplicaReturnResult Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId )
	{
		inBitStream->Read(testInteger);
		printf("Test integer remotely set to %i on object owned by %s:%i\n", testInteger, rakPeer->PlayerIDToDottedIP(owner), owner.port);

		// Forward this event onto everyone but the sender
		replicaManager.SignalSerializeNeeded(this, playerId, true);

		return REPLICA_PROCESSING_DONE;
	}
	// Interface from NetworkIDGenerator.
	bool IsNetworkIDAuthority(void) const
	{
		// In peer to peer, everyone is an authority.  You must define _P2P_OBJECT_ID for this to work.
		return true;
	}

	// Just an integer I'm synchronizing for testing purposes
	int testInteger;

	// When this guy disconnects, we delete the object.
	PlayerID owner;
};

// This is a required callback I registered to get called when a remote participant calls ReplicaManager::Register on a new object.
// It's equivalent to creating our local copy of an object when a new object is created on a remote system
ReplicaReturnResult ConstructionCB(RakNet::BitStream *inBitStream, RakNetTime timestamp, NetworkID networkID, PlayerID senderId, ReplicaManager *caller)
{
	char output[255];
	TestObject *testObject;

	// I encoded all the data in inBitStream SendConstruction
	// I am using the string table to send strings, which is a way to send pre-known strings in a single byte.
	// I registered "TestObject" in main().
	// The stringTable system has the limitation that all systems must register all the same strings in the same order.
	// I could have also used stringCompressor, which would always work but is less efficient to use when we have known strings
	stringTable->DecodeString(output, 255, inBitStream);
	if (strcmp(output, "TestObject")==0)
	{
		testObject = new TestObject;
		// Construct on all other systems but the system that just sent to us.
		// This also automatically references the pointer so we don't need to call replicaManager.ReferencePointer(testObject);
		replicaManager.Construct(testObject, false, senderId, true);

		// Calling SetNetworkID is important and necessary, because all calls other than construction rely on being able to lookup an object by its network ID
		// If we didn't do this, we would never get scope, serialize, or destruction calls because the ReplicaManager wouldn't be able to find this object
		testObject->SetNetworkID(networkID);
		inBitStream->Read(testObject->owner);
		printf("New remote test object created\n");
	}
	else
	{
		// Unknown string
		assert(0);
	}
	return REPLICA_PROCESSING_DONE;
}


int main(void)
{
	char ch;
	char userInput[256];
	unsigned int numAddresses, index;
	TestObject *t;
	rakPeer = RakNetworkFactory::GetRakPeerInterface();
	TestObject *localObject=0;
	PlayerID playerId;

	// This is a global command which will cause sends of NetworkID to use the full PlayerID / localSystemId component.
	// This way NetworkIDs can be created locally.
	NetworkID::SetPeerToPeerMode(true);

	// You have to attach ReplicaManager for it to work, as it is one of the RakNet plugins
	rakPeer->AttachPlugin(&replicaManager);

	// Anytime we get a new connection, call AddParticipant() on that connection
	replicaManager.SetAutoParticipateNewConnections(true);

	// Anytime we get a new participant, automatically call Construct() for them with all known objects
	replicaManager.SetAutoConstructToNewParticipants(true);

	replicaManager.SetReceiveConstructionCB(ConstructionCB);

	// Here I use the string table class to efficiently send strings I know in advance.
	// The encoding is done in the monster and player SendConstruction functions
	// The decoding is done in the ConstructionCB callback
	// The stringTable class will also send strings that weren't registered but this just falls back
	// to the stringCompressor and wastes 1 extra bit on top of that
	stringTable->AddString("TestObject", false); // 2nd parameter of false means a static string so it's not necessary to copy it

	// By default all objects are not in scope, meaning we won't serialize the data automatically when they are constructed
	// Calling this eliminates the need to call replicaManager.SetScope(this, true, playerId); in Replica::SendConstruction.
	replicaManager.SetDefaultScope(true);

	// Just start the client, or the server
	printf("Demonstration of ReplicaManager for peer to peer\n");
	printf("The replica manager provides a framework to make it easier to synchronize\n");
	printf("object creation, destruction, and member object updates\n");
	printf("Difficulty: Advanced\n\n");
	printf("Each peer has one object\n");
	printf("Objects updates are chained e.g. A -> B -> C\n");
	printf("When a peer disconnects, its object is also destroyed\n");

	// Start RakNet
	// You can't use 0 because then other systems do not know what port to connect to.
	printf("Enter local port (do NOT use 0): ");
	gets(userInput);
	if (userInput[0]==0)
		strcpy(userInput, "60000");
	rakPeer->Initialize(MAX_PEERS,atoi(userInput),0,0);
	rakPeer->SetMaximumIncomingConnections(MAX_PEERS);
	numAddresses=rakPeer->GetNumberOfAddresses();
	printf("My IP addresses are:\n");
	for (index=0; index < numAddresses; index++)
		printf("%i. %s\n", index+1, rakPeer->GetLocalIP(index));

	printf("Commands:\n(Q)uit\n(Space) Show status\n(C)onnect to another peer\n(D)isconnect from another peer.\n(T)oggle creation/destruction of local object\n(I)ncrement counter in local object\n");
	Packet *p;
	while (1)
	{
		p = rakPeer->Receive();
		while (p)
		{
			if (p->data[0]==ID_DISCONNECTION_NOTIFICATION || p->data[0]==ID_CONNECTION_LOST)
			{
				printf("Connection lost to %s:%i\n", rakPeer->PlayerIDToDottedIP(p->playerId), p->playerId.port);

				// Delete the object owned by this player
				for (index=0; index < replicaManager.GetReplicaCount(); index++)
				{
					t = (TestObject *) replicaManager.GetReplicaAtIndex(index);
					if (t->owner==p->playerId)
					{
						printf("Deleting object owned by that system.\n");
						replicaManager.Destruct(t, UNASSIGNED_PLAYER_ID, true); // Send the destruct message to all
						delete t;
						break;
					}
				}				
			}
			else if (p->data[0]==ID_NEW_INCOMING_CONNECTION || p->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
			{
				printf("New connection from %s:%i\n", rakPeer->PlayerIDToDottedIP(p->playerId), p->playerId.port);
				
				// We need our own unique IP address to go along with the NetworkID.  Our externalPlayerID should be unique.
				// The internalPlayerID, returned by rakPeer->GetLocalIP, won't be unique if we are behind a NAT machine
				if (NetworkIDGenerator::GetExternalPlayerID()==UNASSIGNED_PLAYER_ID)
					NetworkIDGenerator::SetExternalPlayerID(rakPeer->GetExternalID(p->playerId));
			}
			else if (p->data[0]==ID_CONNECTION_ATTEMPT_FAILED)
			{
				printf("Connection attempt to %s:%i failed.\n", rakPeer->PlayerIDToDottedIP(p->playerId), p->playerId.port);
			}
			rakPeer->DeallocatePacket(p);
			p = rakPeer->Receive();
		}

		if (kbhit())
		{
			ch=getch();
			if (ch=='q' || ch=='Q')
			{
				printf("Quitting.\n");
				break;
			}
			else if (ch==' ')
			{
				printf("\nSTATUS:\n");
				if (localObject)
					printf("Local object test integer=%i\n", localObject->testInteger);
				else
					printf("Local object is not instantiated.\n");

				for (index=0; index < replicaManager.GetReplicaCount(); index++)
				{
					t = (TestObject *) replicaManager.GetReplicaAtIndex(index);
					printf("%i. Remote object owned by %s:%i with test integer=%i\n", index+1, rakPeer->PlayerIDToDottedIP(t->owner), t->owner.port, t->testInteger );
				}
				printf("\n");
			}
			else if (ch=='c' || ch=='C')
			{
				printf("Enter port to connect to: ");
				gets(userInput);
				if (userInput[0]==0)
					strcpy(userInput, "60000");
				unsigned short port;
				port=atoi(userInput);
				printf("Enter IP address to connect to: ");
				gets(userInput);	
				if (userInput[0]==0)
					strcpy(userInput, "127.0.0.1");
				rakPeer->Connect(userInput, port, 0, 0);
				printf("Attemping connection...\n");
			}
			else if (ch=='d' || ch=='D')
			{
				for (index=0; index < MAX_PEERS; index++)
				{
					playerId=rakPeer->GetPlayerIDFromIndex(index);
					if (playerId==UNASSIGNED_PLAYER_ID)
						break;
					printf("%i. %s:%i\n", index+1, rakPeer->PlayerIDToDottedIP(playerId), playerId.port);
				}
				if (index>0)
				{
					printf("Enter the number of the system to disconnect from: ");
                    gets(userInput);
					playerId=rakPeer->GetPlayerIDFromIndex(atoi(userInput)-1);
					if (playerId==UNASSIGNED_PLAYER_ID)
						printf("Invalid number\n");
					else
					{
						printf("Disconnecting from %s:%i\n", rakPeer->PlayerIDToDottedIP(playerId), playerId.port);
						rakPeer->CloseConnection(playerId, true, 0);
					}
				}	
			}
			else if (ch=='t' || ch=='T')
			{
				if (localObject==0)
				{
					if (NetworkIDGenerator::GetExternalPlayerID()==UNASSIGNED_PLAYER_ID)
					{
						printf("You cannot create objects before connecting at least once.\n");
						printf("This sample requires you do so so you know your external IP\nThis is copied to TestObject::owner.\n");
					}
					else
					{
						localObject=new TestObject;
						replicaManager.Construct(localObject, false, UNASSIGNED_PLAYER_ID, true); // Construct on all other systems
						localObject->owner=NetworkIDGenerator::GetExternalPlayerID();
						// Don't let other systems mess with our junk
						replicaManager.DisableReplicaInterfaces(localObject, REPLICA_RECEIVE_DESTRUCTION | REPLICA_RECEIVE_SERIALIZE | REPLICA_RECEIVE_SCOPE_CHANGE);
						printf("Local test object created.\n");
					}
				}
				else
				{
					replicaManager.Destruct(localObject, UNASSIGNED_PLAYER_ID, true); // Send the destruct message to all
					delete localObject;
					localObject=0;
					printf("Local test object destroyed.\n");
				}
			}
			else if (ch=='i' || ch=='I')
			{
				if (localObject)
				{
					localObject->testInteger++;
					replicaManager.SignalSerializeNeeded(localObject, UNASSIGNED_PLAYER_ID, true);
					printf("Local object test integer is now %i\n", localObject->testInteger);
				}
				else
				{
					printf("Local object is not instantiated.\n");
				}
			}
		}
#ifdef _WIN32
		Sleep(30);
#else
		usleep(30 * 1000);
#endif
	}

	rakPeer->Disconnect(100, 0);
	delete localObject;
	RakNetworkFactory::DestroyRakPeerInterface(rakPeer);

	return 1;
}
