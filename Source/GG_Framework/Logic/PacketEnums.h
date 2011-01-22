// Logic PacketEnums.h
#pragma once;

//! These are for packets
enum
{
	ID_PredictedLoadTimes = ID_Network_Last+1,		//!< from server to client to let them know the predicted load times
	ID_Environment_OSGV_File,	//!< from server to client to load in a scene, along with a char*
	ID_ExistingEntity,			//!< from server to client to load an existing entity.  
	ID_PlayersEntity,			//!< from Server, tells the player about a new entity, has Player's ID
	ID_PlayerCompletedLoading,	//!< Sent back from Player saying it is ready to go

	ID_CompletedLoadingOtherPlayerEntity,	//!< Sent from each existing player back to server to say it is done loading
	ID_AttachPlayerEntity,		//!< The server returns this message to say the entity's actors can be placed in the ActorScene

	ID_SetEpoch,				//!< On Client Load, this comes AFTER a timestamp that sets the EPOCH
	ID_ConnectPermissionDenied,	//!< The Script says the Client is not allowed to connect at this time

	ID_StandardEntityUpdate,

	ID_ClientMessages,			//! For all Client Messages, post the Rebroadcast instructions right after this
	ID_RequestControlEntity,	//!< Called by a UI_GameClient that wants to control some entity, only AI will respond
	ID_ChangeEntityController,	//!< AI_GameClient broadcasts this to everyone with a list of entities that are being changed

	ID_SpecialEntityUpdate,			//!< For any messages passed to a specific entity
								//!< Be sure to Write((unsigned char)ID_TIMESTAMP) and the current time before writing this
								//!< Then include rebroadcast instructions
								//!< Then include the int for the EntityID, then the next type sent

	ID_LastClientMessage,
	ID_Logic_Last
};

// These are for ordering channels
enum
{	
	OC_GameLoad = 0,
	OC_CriticalEntityUpdates = 0,
	OC_CriticalEntityEvents = 0,
	OC_NonCriticalEntityEvents = 0,
	OC_PosAttUpdates = 1,	// When sending from the client, we always send on 1, but we will get messages from the server on the others
};

namespace GG_Framework
{
	namespace Logic
	{
		//! Create a global instance of this somewhere to help with Packets
		class FRAMEWORK_LOGIC_API PacketDescriber : public Network::PacketDescriber
		{
		protected:
			virtual std::string DescribePacket(Packet& packet)
			{
				int thisDataType = Network::GetPacketIdentifier(packet.data);

				switch (thisDataType)
				{
				case ID_PredictedLoadTimes:
					return("ID_PredictedLoadTimes");

				case ID_Environment_OSGV_File:
					return("ID_Environment_OSGV_File");

				case ID_ExistingEntity:
					return("ID_ExistingEntity");

				case ID_PlayersEntity:
					return("ID_PlayersEntity");

				case ID_PlayerCompletedLoading:
					return("ID_PlayerCompletedLoading");

				case ID_CompletedLoadingOtherPlayerEntity:
					return("ID_CompletedLoadingOtherPlayerEntity");

				case ID_AttachPlayerEntity:
					return("ID_AttachPlayerEntity");

				case ID_SetEpoch:
					return("ID_SetEpoch");

				case ID_ConnectPermissionDenied:
					return("ID_ConnectPermissionDenied");

				case ID_ClientMessages:
					return("ID_ClientMessages");

				case ID_RequestControlEntity:
					return ("ID_RequestControlEntity");

				case ID_ChangeEntityController:
					return ("ID_ChangeEntityController");

				case ID_SpecialEntityUpdate:
					return("ID_SpecialEntityUpdate");

				case ID_StandardEntityUpdate:
					return("ID_StandardEntityUpdate");

				case ID_LastClientMessage:
					return("ID_LastClientMessage");

				case ID_Logic_Last:
					return("ID_Logic_Last");

				default:
					return __super::DescribePacket(packet);
				}
			}
		};
	}
}