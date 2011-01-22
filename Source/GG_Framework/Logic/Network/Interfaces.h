// GG_Framework.RakNet.Interfaces.h
#pragma once
#include "NetworkTypes.h"
#include "PacketPriority.h"
#include <set>

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			class FRAMEWORK_LOGIC_NETWORK_API IPacketReceiver
			{
			public:
				virtual Packet* Receive() = 0;
				virtual void DeallocatePacket( Packet *packet )=0;
			};
			//////////////////////////////////////////////////////////////////////////

			class FRAMEWORK_LOGIC_NETWORK_API IServer
			{
			public:
				virtual bool IsActive() = 0;
				virtual unsigned GetNumConnections() = 0;
				virtual IPacketReceiver* AsPacketReceiver() = 0;

				/// /pre The server must be active.
				/// Send the data stream of length \a length to whichever \a playerId you specify.
				/// \param[in] data The data to send
				/// \param[in] length The length, in bytes, of \a data
				/// \param[in] priority See PacketPriority
				/// \param[in] reliability See PacketReliabilty
				/// \param[in] orderingChannel The ordering channel to use, from 0 to 31.  Ordered or sequenced packets sent on the channel arrive ordered or sequence in relation to each other.  See the manual for more details on this.
				/// \param[in] playerId Who to send to.  Specify UNASSIGNED_PLAYER_ID to designate all connected systems.
				/// \param[in] broadcast Whether to send to everyone or not.  If true, then the meaning of \a playerId changes to mean who NOT to send to.
				/// \return Returns false on failure, true on success	
				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers )=0;

				/// /pre The server must be active.
				/// Send the data stream of length \a length to whichever \a playerId you specify.
				/// \param[in] bitStream The bitstream to send.
				/// \param[in] priority See PacketPriority
				/// \param[in] reliability See PacketReliabilty
				/// \param[in] orderingChannel The ordering channel to use, from 0 to 31.  Ordered or sequenced packets sent on the channel arrive ordered or sequence in relation to each other.  See the manual for more details on this.
				/// \param[in] playerId Who to send to.  Specify UNASSIGNED_PLAYER_ID to designate all connected systems.
				/// \param[in] broadcast Whether to send to everyone or not.  If true, then the meaning of \a playerId changes to mean who NOT to send to.
				/// \return Returns false on failure, true on success	
				virtual bool Send( RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers)=0;
			};
			//////////////////////////////////////////////////////////////////////////

			class FRAMEWORK_LOGIC_NETWORK_API IClient
			{
			public:
				virtual bool IsConnected() = 0;
				virtual IPacketReceiver* AsPacketReceiver() = 0;

				/// Sends a block of data to the specified system that you are connected to.
				/// This function only works while the connected (Use the Connect function).
				/// \param[in] data The block of data to send
				/// \param[in] length The size in bytes of the data to send
				/// \param[in] priority What priority level to send on.
				/// \param[in] reliability How reliability to send this data
				/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.- Packets are only ordered relative to other packets on the same stream
				/// \return False if we are not connected to the specified recipient.  True otherwise
				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel )=0;

				/// Sends a block of data to the specified system that you are connected to.
				/// This function only works while the connected (Use the Connect function).
				/// \param[in] bitStream The bitstream to send
				/// \param[in] priority What priority level to send on.
				/// \param[in] reliability How reliability to send this data
				/// \param[in] orderingChannel When using ordered or sequenced packets, what channel to order these on.- Packets are only ordered relative to other packets on the same stream
				/// \return False if we are not connected to the specified recipient.  True otherwise
				virtual bool Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel )=0;


				virtual PlayerID GetMyPlayerID() const = 0;
			};
			//////////////////////////////////////////////////////////////////////////
		}
	}
}