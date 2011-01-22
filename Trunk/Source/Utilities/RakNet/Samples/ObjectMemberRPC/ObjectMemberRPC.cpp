#include "RakNetworkFactory.h"
#include "RakPeerInterface.h"
#include <stdlib.h>
#include <cstdio>
#include <cstring>
#include "NetworkIDGenerator.h"
#include "GetTime.h"
#ifdef _WIN32
#include <windows.h> // Sleep
#else
#include <unistd.h> // usleep
#include <cstdio>
#endif

// Quick and dirty overload to mean that this instance of the program can generate network IDs.  It works here in this sample because there is only
// one system anyway.  In a real game you would return true or false depending on whether the game was running in server mode or client mode.
// In peer to peer you would have one peer generate IDs, and that peer returns true only as an authority while other peers return true only as a recipient.
class ServerNetworkIDGenerator : public NetworkIDGenerator
{
	virtual bool IsNetworkIDAuthority(void) const {return true;}
};

#ifdef __GNUC__
#define __cdecl
#endif

// Classes that use class member RPCs must derive from NetworkIDGenerator
class Apple : public ServerNetworkIDGenerator
{
public:
	Apple()
	{
	}
	// RPC member Functions MUST be marked __cdecl!
	virtual void __cdecl func1(RPCParameters *rpcParms)
	{
		if (rpcParms->input)
			printf("Base Apple func1: %s\n", rpcParms->input);
		else
			printf("Base Apple func1\n");
	}

	// RPC member Functions MUST be marked __cdecl!
	virtual void __cdecl func1(char *blah)
	{
		printf("Func1.  Does not match function signature and should never be called.\n");
	}

	// RPC member Functions MUST be marked __cdecl!
	virtual void __cdecl func2(RPCParameters *rpcParms)
	{
		if (rpcParms->input)
			printf("Base Apple func2: %s\n", rpcParms->input);
		else
			printf("Base Apple func2\n");
	}

	// RPC member Functions MUST be marked __cdecl!
	virtual void __cdecl func3(RPCParameters *rpcParms)
	{
		if (rpcParms->input)
			printf("Base Apple func3: %s\n", rpcParms->input);
		else
			printf("Base Apple func3\n");
	}
};

class GrannySmith : public Apple
{
	void __cdecl func1(RPCParameters *rpcParms)
	{
		printf("Derived GrannySmith func1: %s\n", rpcParms->input);
	}
};

void main(void)
{
	RakNetTime time;
	RakPeerInterface *rakPeer1 = RakNetworkFactory::GetRakPeerInterface();
	RakPeerInterface *rakPeer2 = RakNetworkFactory::GetRakPeerInterface();

	printf("This project shows how to call member functions of your application object.\n");
	printf("instances.\n");
	printf("Difficulty: Advanced\n\n");

	rakPeer1->Initialize(2,60000,0);
	rakPeer2->Initialize(2,60001,0);
	rakPeer2->SetMaximumIncomingConnections(2);
	rakPeer1->Connect("127.0.0.1", 60001, 0, 0);

	// This means on rakPeer2, let Apple::func1 (and 2 and 3) be called.  You call this on the system that processes the RPC, not on the system that calls it.
	REGISTER_CLASS_MEMBER_RPC(rakPeer2, Apple, func1) 
	REGISTER_CLASS_MEMBER_RPC(rakPeer2, Apple, func2) 
	REGISTER_CLASS_MEMBER_RPC(rakPeer2, Apple, func3)

	// Wait for the connection to complete
#ifdef _WIN32
	Sleep( 250 );
#else
	usleep( 250 * 1000 );
#endif

	Apple *apple;
	apple = new GrannySmith;
	time = RakNet::GetTime();

	// CLASS_MEMBER_ID means make a string out of the two parameters.  This is used as a unique identifier for the function, which matches the unique identifier made by the macro REGISTER_CLASS_MEMBER_RPC with the 2nd and 3rd parameter.
	printf("Calling func1 of Apple base class with test string 1.\n");
	rakPeer1->RPC(CLASS_MEMBER_ID(Apple, func1), "test string 1", (int)(strlen("test string 1")+1)*8, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true, false, apple->GetNetworkID(), 0);
	printf("Calling func2 of Apple base class with test string 2.\n");
	rakPeer1->RPC(CLASS_MEMBER_ID(Apple, func2), "test string 2", (int)(strlen("test string 2")+1)*8, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true, false, apple->GetNetworkID(), 0);
	printf("Calling func3 of Apple base class with no test string.\n");
	rakPeer1->RPC(CLASS_MEMBER_ID(Apple, func3), 0,0, HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_PLAYER_ID, true, false, apple->GetNetworkID(), 0);

	while (RakNet::GetTime() < time + 5000)
	{
		// Just ignore the test data.  It's necessary to call Receive since RPCs run in the user thread.
		rakPeer1->DeallocatePacket(rakPeer1->Receive());
		rakPeer2->DeallocatePacket(rakPeer2->Receive());
		Sleep(30);
	}

	char str[256];
	printf("Sample complete.  Press enter to quit.");
	gets(str);

	RakNetworkFactory::DestroyRakPeerInterface(rakPeer1);
	RakNetworkFactory::DestroyRakPeerInterface(rakPeer2);
}