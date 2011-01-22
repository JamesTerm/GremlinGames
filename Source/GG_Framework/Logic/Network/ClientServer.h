// ClientServer.h
#pragma once

#include "RakServerInterface.h"
#include "RakClientInterface.h"

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			static const unsigned DEFAULT_SERVER_PORT = 2000;

			//! An actual RAKNET Server
			class FRAMEWORK_LOGIC_NETWORK_API Server : public IServer, public IPacketReceiver
			{
			public:
				Server(unsigned maxPlayers, unsigned portNumber = DEFAULT_SERVER_PORT);
				virtual ~Server();
				virtual IPacketReceiver* AsPacketReceiver(){return this;}

				virtual bool IsActive();
				virtual unsigned GetNumConnections();
				virtual Packet* Receive();
				virtual void DeallocatePacket( Packet *packet );

				Event2<IServer*, PlayerID> ClientConnect_Event;
				Event3<IServer*, PlayerID, bool> ClientDisconnect_Event;	
						//!< 3rd parameter is true for a normal disconnect and
						//!< false if we lost a server from a time-out.

				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers );
				virtual bool Send( RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers );

				RakServerInterface* GetRakServer(){return m_rakServer;}

			private:
				RakServerInterface* m_rakServer;
			};
			//////////////////////////////////////////////////////////////////////////


			static const unsigned DEFAULT_CLIENT_PORT = 2100;
			static const char* LOOPBACK_URL = "127.0.0.1";

			//! An Actual RAKNET Client
			class FRAMEWORK_LOGIC_NETWORK_API Client : public IClient, public IPacketReceiver
			{
			public:
				Client(
					const char* serverURL = LOOPBACK_URL, 
					unsigned serverPort = DEFAULT_SERVER_PORT,
					unsigned clientPort = DEFAULT_CLIENT_PORT);
				virtual ~Client();
				virtual IPacketReceiver* AsPacketReceiver(){return this;}

				virtual bool IsConnected();
				virtual Packet* Receive();
				virtual void DeallocatePacket( Packet *packet );

				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel );
				virtual bool Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel );

				RakClientInterface* GetClientInterface(){return m_rakClient;}

				virtual PlayerID GetMyPlayerID() const {return m_rakClient->GetPlayerID();}

			private:
				RakClientInterface* m_rakClient;
			};
			//////////////////////////////////////////////////////////////////////////
		}
	}
}