// GG_Framework.Logic MockClientServer.h
#pragma once

#include <queue>

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			class MockClient;
			class FRAMEWORK_LOGIC_NETWORK_API MockPeer : public IPacketReceiver
			{
			public:
				MockPeer();
				virtual ~MockPeer();
				virtual Packet* Receive();
				virtual void DeallocatePacket( Packet *packet );
				virtual std::string GetMockDescription() const = 0;

			protected:
				bool AddToPeerQueue(const char *data, const int length, const PlayerID& playerID);
				bool AddToPeerQueue(RakNet::BitStream * bitStream, const PlayerID& playerID);
				MockPeer* m_otherPeer;

			private:
				bool AddToSelfQueueFromOtherPeer(const char *data, const int length, const PlayerID& playerID);
				std::queue<Packet*> m_msgQueue;
				OpenThreads::Mutex m_mutex;
			};
			//////////////////////////////////////////////////////////////////////////

			class FRAMEWORK_LOGIC_NETWORK_API MockServer : public IServer, public MockPeer
			{
			public:
				MockServer(std::string debugDesc) : m_debugDesc(debugDesc) {}
				virtual IPacketReceiver* AsPacketReceiver(){return this;}

				virtual bool IsActive(){return (m_otherPeer!=NULL);}
				virtual unsigned GetNumConnections(){return (IsActive()) ? 1 : 0;}
				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers );
				virtual bool Send( RakNet::BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, PlayerID playerId, bool broadcast, std::set<PlayerID>* onlyToPeers );
				virtual std::string GetMockDescription() const {return m_debugDesc;}
				MockClient* GetMockClient(){return m_client;}
				friend MockClient;

			private:
				bool ShouldSendToClient(const PlayerID& playerId, bool broadcast, std::set<PlayerID>* onlyToPeers);
				MockClient* m_client;
				std::string m_debugDesc;
			};
			//////////////////////////////////////////////////////////////////////////

			/// A special PlayerID to use for the AI_CLient
			const PlayerID MOCK_AI_PLAYER_ID =
			{
				0xFFFFFFFE, 0xFFFF
			};

			/// A special PlayerID to use for the AI_CLient
			const PlayerID MOCK_SINGLE_PLAYER_ID =
			{
				0xFFFFFFFD, 0xFFFF
			};
			//////////////////////////////////////////////////////////////////////////

			class FRAMEWORK_LOGIC_NETWORK_API MockClient : public IClient, public MockPeer
			{
			public:
				//! For playerID, use MOCK_AI_PLAYER_ID or MOCK_SINGLE_PLAYER_ID
				MockClient(MockServer& server, PlayerID playerID);
				virtual IPacketReceiver* AsPacketReceiver(){return this;}

				virtual bool IsConnected(){return (m_otherPeer!=NULL);}
				virtual bool Send( const char *data, const int length, PacketPriority priority, PacketReliability reliability, char orderingChannel );
				virtual bool Send( RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel );
				virtual std::string GetMockDescription() const;
				virtual PlayerID GetMyPlayerID() const {return m_playerID;}

			private:
				PlayerID m_playerID;
			};
			//////////////////////////////////////////////////////////////////////////
		}
	}
}