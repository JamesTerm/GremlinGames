// GG_Framework.Logic ServerManager.h
#include <vector>
#include <map>
#include <set>

namespace GG_Framework
{
	namespace Logic
	{
		/// The Logic::ServerManager
		/// Not only handles passing communications between the AI and the Player Clients,
		/// But it also handles working with the LUA script, and getting all of the players
		/// initialized at start-up, including the AI, but it also has the main timer
		/// used across the entire game, which has an epoch (the system time that corresponds to 0 game time).
		/// This class represents functionality common to single and multi-player systems.
		class FRAMEWORK_LOGIC_API ServerManager : 
			public Network::ServerManager, public GG_Framework::Base::ThreadedClass, public ITransmittedEntityOwner
		{
		public:
			//! The playerServer will be a Network::Server for multi-player games OR
			//! a MockServer for Single Player Games
			ServerManager(GG_Framework::Logic::Scripting::Script* script, GG_Framework::Logic::Network::SynchronizedTimer& aiTimer,
				Network::IServer* playerServer, Network::MockServer* aiServer, AI_GameClient& aiGameClient);
			virtual ~ServerManager();

			//! Call this one time after the constructor.  Make sure any Mock Clients are
			//! Connected to the Mock Servers, including the AI one and the Player one if
			//! In a Single Player Game.
			void Initialize();

			//! Keep track of connected Player ID's, The AI is always at 1.  Bad values are at 0
			//! The numbers stay consistent, even if players remove themselves (out of order)
			//! These are the indices the LUA script uses.  Does not assign it until 
			//! after the Player has connected and has an Entity?
			//! 0 is UNASSIGNED_PLAYER_ID, 1 is Network::MOCK_AI_PLAYER_ID
			unsigned GetClientIndex(PlayerID id){return m_clientMap[id];}
			PlayerID GetClientID(unsigned index){return (index < m_clientVector.size())? m_clientVector[index]:UNASSIGNED_PLAYER_ID;}

			// ITransmittedEntityOwner
			// MUST be Thread Safe
			virtual unsigned AddTransmittedEntity(TransmittedEntity* newEntity);

			GG_Framework::Logic::Network::SynchronizedTimer&	AI_Timer;		//!< Keep a separate timer for JUST the AI part of the scene

		protected:
			//! This function is called when any Peer connects.  It is a callback function
			//! for multi-player games, or it is called directly from Initialize() in
			//! single player games.  It is also called from Initialize() for the AI_Client.
			virtual void PeerConnected(Network::IServer* server, PlayerID playerID);

			//! This function is only called when a player in a multi-player game disconnects,
			//! Either from a normal disconnect, or from a time-out.
			virtual void PeerDisconnected(Network::IServer* playerServer, PlayerID playerID, bool normalDisconnect);

			//! Called when the player returns the message saying it is done loading and ready to go
			virtual void PeerFinishedConnect(PlayerID playerID);

			//! This message will help us know how to process the in-coming message
			//! Return false for messages intended for the Server, true for messages to be re-broadcast
			//! If returning true, set ignorePlayers to some other value than NULL for a set of other players to NOT send to
			virtual bool ShouldRebroadcastMessage(Packet* packet,
				PacketPriority& priority, PacketReliability& reliability, char& orderingChannel, std::set<PlayerID>*& ignorePlayers);

			// This is the part that is really run, surrounded by try/catch blocks
			void tryRun();

			std::set<PlayerID> m_connectedPeers;	//!< Peers that are NOT in this list have not fully connected, so we need to NOT include them in broadcasts

		private:
			//! This is called when a peer is connected (other than the AI) to provide
			//! the environment visuals and load times
			bool SendPredictedLoadTimes(Network::IServer* server, PlayerID playerID);
			bool SendEnvironmentFiles(Network::IServer* server, PlayerID playerID);
			bool SendEnvironmentFile(const char* fileToSend, Network::IServer* server, PlayerID playerID);

			//! Asks the LUA SCript for the Player's Entity to fly, 
			//! Provides the list of newly added entities, is thread safe
			//! If there was a problem, newEntities will not have anything added to it
			void CreatePlayersEntities(PlayerID playerID, std::vector<TransmittedEntity*>& newEntities);

			//! Called on any Peer connection to get all of the existing entities in the game
			bool SendExistingEntities(Network::IServer* server, PlayerID playerID);

			IEvent::HandlerList ehl;
			GG_Framework::Logic::Scripting::Script* const m_script;

			// The AI Game Data
			bool m_gameStarted;
			AI_GameClient& m_AI_GameClient;

			// The Map and vector of player ID's
			// 0 is UNASSIGNED_PLAYER_ID
			// 1 is Network::MOCK_AI_PLAYER_ID
			std::vector<PlayerID> m_clientVector;
			std::map<PlayerID, unsigned> m_clientMap;
			unsigned AddPlayerID(PlayerID playerID);
			void RemovePlayerID(PlayerID playerID);

			// Keep a list of predicted load times read from the LUA script
			std::vector<float> m_predLoadTimes;

			// This map is used to track when all existing clients (and this one)
			// have returned from loading the new player's ship.  When the counter
			// drops to 0, we can send the EPOCH to the new player and send a message
			// to the other players to tell them to place the new object in scene.
			std::map<PlayerID, unsigned> m_clientWaitCount;

			// Here is a vector of all of the existing entities.  They are read only one time from the LUA script
			// The index into this vector represents the NETWORK_ID of the TransmittedEntity and thus the Entity3D created
			std::vector<TransmittedEntity*> m_entities;
			void ReadInitialEntities();
			OpenThreads::Mutex m_entitiesV_mutex;

			// Be able to throttle the AI
			GG_Framework::Base::ThrottleFrame m_ThrottleFrames;
		};

	}
}