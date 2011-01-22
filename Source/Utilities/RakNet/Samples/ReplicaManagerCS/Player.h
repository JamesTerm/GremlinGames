#include "NetworkTypes.h"
#include "PacketPriority.h"
#include "ReplicaEnums.h"
#include "Replica.h"

namespace RakNet
{
	class BitStream;
};

// One way to use the replication system is derive from the Replica class
class Player : public Replica
{
public:
	Player();
	~Player();

	// These functions are all from the Replica interface in ReplicaManager.h
	// They are user defined pure virtual implementations that perform processing that the ReplicaManager system requests.
	ReplicaReturnResult SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp );
	void SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId);
	ReplicaReturnResult ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId);
	ReplicaReturnResult SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId);
	ReplicaReturnResult ReceiveScopeChange(RakNet::BitStream *inBitStream, PlayerID playerId);
	ReplicaReturnResult Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId);
	ReplicaReturnResult Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId );

	// This interface comes from Replica<-NetworkIDGenerator.  Means should we create object ids on this system?  Return true for a server, or for peer to peer.
	bool IsNetworkIDAuthority(void) const;

	// Game specific data
	int position;
	int health;
};
