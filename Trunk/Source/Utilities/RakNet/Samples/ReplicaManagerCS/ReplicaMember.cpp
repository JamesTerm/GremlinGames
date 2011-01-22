#include "ReplicaMember.h"
#include "Monster.h"

extern bool isServer;

// This file demonstrates using an object of class Replica as a member of another object, rather than inheriting it.
// All I do here is pass all calls up to the parent.
// In this case I know the parent is of class Monster, but in a real game you'd use Entity or Object or whatever your base network class is.
ReplicaReturnResult ReplicaMember::SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp )
{
	return ((Monster*)GetParent())->SendConstruction(currentTime, playerId, outBitStream, includeTimestamp);
}
void ReplicaMember::SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId)
{
	((Monster*)GetParent())->SendDestruction( outBitStream, playerId);
}
ReplicaReturnResult ReplicaMember::ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId)
{
	return ((Monster*)GetParent())->ReceiveDestruction(inBitStream, playerId);
}
ReplicaReturnResult ReplicaMember::SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId)
{
	return ((Monster*)GetParent())->SendScopeChange(inScope, outBitStream, currentTime, playerId);
}
ReplicaReturnResult ReplicaMember::ReceiveScopeChange(RakNet::BitStream *inBitStream, PlayerID playerId)
{
	return ((Monster*)GetParent())->ReceiveScopeChange(inBitStream, playerId);
}
ReplicaReturnResult ReplicaMember::Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId)
{
	return ((Monster*)GetParent())->Serialize(sendTimestamp, outBitStream, lastSendTime, priority, reliability, currentTime, playerId);
}
ReplicaReturnResult ReplicaMember::Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId )
{
	return ((Monster*)GetParent())->Deserialize(inBitStream, timestamp, lastDeserializeTime, playerId);
}
bool ReplicaMember::IsNetworkIDAuthority(void) const
{
	return isServer;
}
bool ReplicaMember::RequiresSetParent(void) const
{
	return true;
}
