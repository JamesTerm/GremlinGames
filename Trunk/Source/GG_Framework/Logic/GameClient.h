// GG_Framework.Logic GameClient.h
#pragma once
#include <string>
#include <map>

class GG_Framework::Logic::Entity3D;
typedef std::map<unsigned, GG_Framework::Logic::Entity3D*, std::greater<unsigned> > EntityMap_NetID;
typedef std::map<std::string, GG_Framework::Logic::Entity3D*, std::greater<std::string> > EntityMap_Name;

namespace GG_Framework
{
	namespace Logic
	{
		struct TransmittedEntity;

		//! This version of the ActorScene ONLY builds BB versions of the Scene
		//! For now it returns the regular kind of of ActorTransform, but we will
		//! Eventually override the file reader to tweak with the LWS file.
		class BB_ActorScene : public GG_Framework::UI::ActorScene
		{
		public:
			BB_ActorScene(GG_Framework::Base::Timer& timer, const char* contentDirLW) : 
			  GG_Framework::UI::ActorScene(timer, contentDirLW, true) {}


			virtual bool IsVisibleScene(){return false;}
			virtual GG_Framework::UI::ActorTransform* 
			  ReadFromSceneFile(GG_Framework::UI::EventMap& localEventMap, const char* fn);
		};
		//////////////////////////////////////////////////////////////////////////
		//! For each game client running (one for each player and one for the UI), there is an instance of GameClient.
		//! For players it is the UI_GameClient and for the AI it is the AI_GameClient (i.e. server's game scene).
		//! This class is the main class for handling all input from the network, and owns all of the entities, processes collisions, etc.  
		//! It does a lot (too much probably)
		//! GameClient contains a BB_ActorScene, used of course for collision detection and all logic
		//! So the server runs the AI game scene
		//! On the single player version, there is an AI_GameClient and a UI_GameClient running on separate threads;.
		//! talking to each other with mocked network connections so they are modeled just like a real scenario, where there would be a server and client 
		//! running as separate apps.
		//! One easy way to know if you are in a UI or AI scene is to call Get_UI_ActorScene(), which would return NULL for the AI_GameClient
		//! This class is responsible for maintaining the Scene.
		class FRAMEWORK_LOGIC_API GameClient
		{
		public:
			GameClient(
				GG_Framework::Logic::Network::IClient& client, 
				GG_Framework::Base::Timer& logic_timer, const char* contentDirLW);
			virtual ~GameClient();

			//! This might be called on a separate thread
			virtual bool LoadInitialGameData();
			Event1<const char*> LoadStatusUpdate;	//! Use this event to see about changes

			//! This list will be used for all internal events
			//! The START event will be fired by the Runner
			EventMapList MapList;

			GG_Framework::Base::Timer& GetLogicTimer(){return m_logicTimer;}

			// Some Event Maps for working with incoming packets
			Event1<Packet&> IncomingPacketEvent;
			std::map<int, Event1<Packet&> >	IncomingPacketEventMap;

			virtual BB_ActorScene* Get_BB_ActorScene(){return &m_actorSceneBB;}
			virtual GG_Framework::UI::ActorScene* Get_UI_ActorScene(){return NULL;}

			// Find an entity with an NAME or NETWORK_ID
			Entity3D* FindEntity(std::string name);
			Entity3D* FindEntity(unsigned networkID);
			unsigned GetNumEntities(){return m_entityMap_NetID.size();}

			// Get an iterator to my map so I can enumerate through every element
			EntityMap_NetID::iterator GetEntityIteratorBegin(){return m_entityMap_NetID.begin();}
			EntityMap_NetID::iterator GetEntityIteratorEnd(){return m_entityMap_NetID.end();}
		
			Entity3D* CreateNewEntity(TransmittedEntity& te, bool buildActors);

			// This Client NAME is unique in the game
			PlayerID GetMyPlayerID() const {return m_client.GetMyPlayerID();}

			// Utility Functions for helping us post updates for an entity.  Create the entityUpdateBS, then call this
			// Use -1 for time as a default to get the current timer
			void CreateEntityUpdateBitStream(RakNet::BitStream& entityUpdateBS, 
				PacketPriority priority, PacketReliability reliability, char orderingChannel,
				unsigned entityNetID, unsigned char msgID, double time_s=-1.0);
			
			// Then populate it with stuff ...
			// Then call this to send the message along
			void PostEntityUpdateToServer(RakNet::BitStream& entityUpdateBS);


		protected:
			//! We will reuse an updater that is sent to the BB scene
			osgUtil::UpdateVisitor m_updateBB;
			osg::ref_ptr<osg::FrameStamp> m_frameStampBB;

			//! Use this to send messages back along the network
			GG_Framework::Logic::Network::IClient& m_client;

			//! Called when a new entity is sent (new Player came in after we did)
			//! Based on ID_PlayersEntity
			void NewPlayersEntity(Packet& packet);
			void AttachNewPlayersEntity(Packet& packet);

			//! Called when an entity packet arrives, fires event handlers in RC_Controller 
			void ReceiveSpecialEntityUpdatePacket(Packet& packet);

			//! Called when working with standard updates that happen regularly and are UNRELIABLE_SEQUENCED
			void SendStandardEntityUpdates(double time_s);
			void ReceiveStandardEntityUpdates(Packet& packet);

			void ChangeEntityController(Packet& packet);
			void SendChangeEntityController(Entity3D* tryEntity, const PlayerID& playerThatWantsControl);

			//! Here is where we get one pass of the Game Loop
			virtual void GameTimerUpdate(double time_s);

			//! If you process a message manually, send it through here to fire all of the events
			virtual void FireNetworkPacketEvents( Packet* packet );

		private:
			//! To listen to all events
			IEvent::HandlerList ehl;

			// Keep track of some profiling
			GG_Framework::Base::ProfilingLogger CollLogger;
			GG_Framework::Base::ProfilingLogger SendNetLogger;
			GG_Framework::Base::ProfilingLogger RecvNetLogger;

			BB_ActorScene m_actorSceneBB;
			GG_Framework::Base::Timer& m_logicTimer;

			// This map of Entities that we maintain
			EntityMap_Name m_entityMap_Name;
			EntityMap_NetID m_entityMap_NetID;
			friend Entity3D;

			// We may want to stutter entity updates
			unsigned char m_standardUpdatePause, m_standardUpdateCounter;

			/// Collision Detection
			double m_lastTimeUpdate;
			virtual void ProcessCollisions(double dTime_s);
			bool ProcessCollision(Entity3D* collider, Entity3D* bigShip, double dTime_s);

			void CheckOnAsyncLoaders();
			bool CheckOnAsyncLoaders(PlayerID playerID, unsigned startIndex);
			void CleanAsyncLoaders(PlayerID playerID);
			std::vector<ThreadedEntityActorLoader*> m_asyncLoadersToCheck;
			std::vector<ThreadedEntityActorLoader*> m_asyncLoadersToAttach;
		};
		//////////////////////////////////////////////////////////////////////////
		
		class FRAMEWORK_LOGIC_API AI_GameClient : public GameClient
		{
		public:
			AI_GameClient(
				GG_Framework::Logic::Network::IClient& client, 
				GG_Framework::Base::Timer& timer, const char* contentDirLW);

			// While waiting for the first player, just keep doing a logic_timer update at 0
			void LookForFirstPlayerEntity(){GameTimerUpdate(0.0);}

			// If one client closes un-expectantly, we may need to take over its entities, or destroy them
			// This version resets all of the entities to be owned by AI.
			// Returns true iff at least one entity was changed (may be recursively called)
			virtual void ForceDisconnect(PlayerID playerID);

		protected:
			void ReceiveRequestControlEntity(Packet& packet);

		private:
			//! To listen to all events
			IEvent::HandlerList ehl;
		};
	}
}


