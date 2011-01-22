// A class demonstrating using a derived instance of Replica as a member object.
// All Replica class functions are here and are called from replica

class ReplicaMember;
#include "NetworkTypes.h"
#include "PacketPriority.h"
#include "ReplicaEnums.h"

namespace RakNet
{
	class BitStream;
};

class Monster
{
public:
	Monster();
	~Monster();

	// These functions are all from the Replica interface in ReplicaManager.h
	// They are user defined pure virtual implementations that perform processing that the ReplicaManager system requests.
	// In this case, I'm implementing them in the member variable ReplicaMember and calling up to this class so I don't have to inherit from Replica
	ReplicaReturnResult SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp );
	void SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId);
	ReplicaReturnResult ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId);
	ReplicaReturnResult SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId);
	ReplicaReturnResult ReceiveScopeChange(RakNet::BitStream *inBitStream, PlayerID playerId);
	ReplicaReturnResult Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId);
	ReplicaReturnResult Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId );

	// This interface comes from Replica<-NetworkIDGenerator.  Means should we create object ids on this system?  Return true for a server, or for peer to peer.
	bool IsNetworkIDAuthority(void) const;

	// The other way to use the replication system is to include that class as a member variable and call SetParent
	ReplicaMember *replica;

	// Game specific data
	int position;
	int health;
};
