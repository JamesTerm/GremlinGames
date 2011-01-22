#include "Replica.h"

// An implementation of Replica that passes all calls up to a parent class
// Used if I don't want my game class to inherit from Replica
class ReplicaMember : public Replica
{
	virtual ReplicaReturnResult SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp );
	virtual void SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId);
	virtual ReplicaReturnResult ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId);
	virtual ReplicaReturnResult SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId);
	virtual ReplicaReturnResult ReceiveScopeChange(RakNet::BitStream *inBitStream, PlayerID playerId);
	virtual ReplicaReturnResult Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId);
	virtual ReplicaReturnResult Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId );
	virtual bool IsNetworkIDAuthority(void) const;
	// Safety check so the user does not forget to call SetParent
	virtual bool RequiresSetParent(void) const;
};
