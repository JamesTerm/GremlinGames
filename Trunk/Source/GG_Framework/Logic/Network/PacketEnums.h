// PacketEnums.h
#pragma once
#include "PacketEnumerations.h"

//! These are for packets
enum
{
	ID_Network_First = ID_USER_PACKET_ENUM,
	ID_Network_Last
};

// These are for ordering channels
enum
{
	OC_Default = 0,
};

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			//! Create a global instance of this somewhere to help with Packets
			class FRAMEWORK_LOGIC_NETWORK_API PacketDescriber
			{
			public:
				PacketDescriber(){s_inst = this;}
				virtual ~PacketDescriber(){if (s_inst==this) s_inst=NULL;}

				// Uses the singleton
				static std::string GetPacketDescription(Packet& packet){return s_inst ? s_inst->DescribePacket(packet) : "NO PacketDescriber";}

			protected:
				virtual std::string DescribePacket(Packet& packet)
				{
					int thisDataType = GetPacketIdentifier(packet.data);

					switch (thisDataType)
					{
					case ID_NEW_INCOMING_CONNECTION:
						return GG_Framework::Base::BuildString("Client connected to server(%s).", PlayID2Str(packet.playerId).c_str());

					case ID_RECEIVED_STATIC_DATA:
						return("a bitstream containing static data, automatic from connection");

					case ID_DISCONNECTION_NOTIFICATION:
						return("Client disconnected from server.");

					case ID_REMOTE_DISCONNECTION_NOTIFICATION:
						return("A client has disconnected from the server.");

					case ID_REMOTE_CONNECTION_LOST:
						return("A client has lost connection with the server.");

					case ID_REMOTE_NEW_INCOMING_CONNECTION:
						return("New client connection established.");

					case ID_REMOTE_EXISTING_CONNECTION:
						return("Existing client connection found.");

					case ID_CONNECTION_ATTEMPT_FAILED:
						return("Connection attempt failed");

					case ID_NO_FREE_INCOMING_CONNECTIONS:
						return("Server is full.");

					case ID_INVALID_PASSWORD:
						return("Password is incorrect.");

					case ID_CONNECTION_LOST:
						return("Connection lost (timed out).");

					case ID_CONNECTION_REQUEST_ACCEPTED:
						return("You are now connected to the server.");

					default:
						return GG_Framework::Base::BuildString("UNKNOWN Type: %i", (int)thisDataType);
					}
				}
			private:
				static PacketDescriber* s_inst;
			};
		}
	}
}


