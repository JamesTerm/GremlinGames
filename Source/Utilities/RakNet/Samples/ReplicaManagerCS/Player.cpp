#include "Player.h"
#include "BitStream.h"
#include "StringTable.h"
#include "ReplicaManager.h"
#include <stdio.h>

extern bool isServer;
extern ReplicaManager replicaManager;
extern Player *player;

Player::Player()
{
	// Setup my object
	position=3;
	health=4;

	// Objects are only processed by the system after you tell the manager to replicate them.
	// Here I do it in the constructor, but in a real game you would probably do it after the object is done loading
	// This does NOT call serialize automatically - it only sends a call to create the remote object.
	if (isServer)
		replicaManager.Construct(this, false, UNASSIGNED_PLAYER_ID, true);

	// Calling
	// replicaManager.SetScope(this, true, UNASSIGNED_PLAYER_ID)
	// here would work for existing players, but not for future players.  So we call it in SendConstruction instead, which is called once for all players

	if (isServer)
	{
		// For security, as a server disable all receives except REPLICA_RECEIVE_SERIALIZE
		// I could do this manually by putting if (isServer) return; at the top of all my receive functions too.
		replicaManager.DisableReplicaInterfaces(this, REPLICA_RECEIVE_DESTRUCTION | REPLICA_RECEIVE_SCOPE_CHANGE );
	}
	else
	{
		// For convenience and for saving bandwidth, as a client disable all sends except REPLICA_SEND_SERIALIZE
		// I could do this manually by putting if (isServer==false) return; at the top of all my send functions too.
		replicaManager.DisableReplicaInterfaces(this, REPLICA_SEND_CONSTRUCTION | REPLICA_SEND_DESTRUCTION | REPLICA_SEND_SCOPE_CHANGE );
	}
}
Player::~Player()
{
	printf("Inside ~Player\n");

	// Dereplicate the object before it is destroyed, or you will crash the next time the ReplicaManager updates
	replicaManager.Destruct(this, UNASSIGNED_PLAYER_ID, true);
	replicaManager.DereferencePointer(this);
	player=0;
}
ReplicaReturnResult Player::SendConstruction( RakNetTime currentTime, PlayerID playerId, RakNet::BitStream *outBitStream, bool *includeTimestamp )
{
	// This string was pre-registered in main with stringTable->AddString so we can send it with the string table and save bandwidth
	stringTable->EncodeString("Player", 255, outBitStream);

	// This means that this object starts in scope this player.  In a real game, you would only start in scope if an object
	// were immediately visible to that player.  This only really applies to the server in a client/server game, since clients wouldn't hide their
	// own object updates from the server.
	// We could have left this line out by calling ReplicaManager::SetDefaultScope(true); in main()
	replicaManager.SetScope(this, true, playerId, false);

	printf("Sending monster to %i:%i\n", playerId.binaryAddress, playerId.port);

	return REPLICA_PROCESSING_DONE;
}
void Player::SendDestruction(RakNet::BitStream *outBitStream, PlayerID playerId)
{
}
ReplicaReturnResult Player::ReceiveDestruction(RakNet::BitStream *inBitStream, PlayerID playerId)
{	
	delete player;

	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::SendScopeChange(bool inScope, RakNet::BitStream *outBitStream, RakNetTime currentTime, PlayerID playerId)
{
	if (inScope)
		printf("Sending scope change to true in Player\n");
	else
		printf("Sending scope change to false in Player\n");

	// Up to you to write this.  If you write nothing, the system will treat that as if you wanted to cancel the scope change
	outBitStream->Write(inScope);
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::ReceiveScopeChange(RakNet::BitStream *inBitStream, PlayerID playerId)
{
	bool inScope;
	inBitStream->Read(inScope);
	if (inScope)
		printf("Received message that scope is now true in Player\n");
	else
		printf("Received message that scope is now false in Player\n");
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::Serialize(bool *sendTimestamp, RakNet::BitStream *outBitStream, RakNetTime lastSendTime, PacketPriority *priority, PacketReliability *reliability, RakNetTime currentTime, PlayerID playerId)
{
	if (lastSendTime==0)
		printf("First call to Player::Serialize for %i:%i\n", playerId.binaryAddress, playerId.port);

	outBitStream->Write(position);
	outBitStream->Write(health);
	return REPLICA_PROCESSING_DONE;
}
ReplicaReturnResult Player::Deserialize(RakNet::BitStream *inBitStream, RakNetTime timestamp, RakNetTime lastDeserializeTime, PlayerID playerId )
{
	if (lastDeserializeTime==0)
		printf("First call to Player::Deserialize\n");
	else
		printf("Got Player::Deserialize\n");

	inBitStream->Read(position);
	inBitStream->Read(health);
	return REPLICA_PROCESSING_DONE;
}
bool Player::IsNetworkIDAuthority(void) const
{
	return isServer;
}
