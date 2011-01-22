// GG_Framework.AppReuse DestroyableEntity.h
#pragma once

namespace GG_Framework
{
	namespace AppReuse
	{
		//! We will need this Transmitted Entity to know about the Destruction proxy
		struct FRAMEWORK_APPREUSE_API DestroyableEntity_TransmittedEntity : public GG_Framework::Logic::TransmittedEntity
		{
			//! The name of the OSGV file for the DestructedProxyEntity
			int DESTRUCTION_ENTITY_INDEX;
			double RESPAWN_TIME;
			double RESPAWN_HITPOINTS;

			//! Write the entity into the BitStream to be sent, WITHOUT the header
			virtual void WriteToBitStream(RakNet::BitStream& bs);

			//! Build a Transmitted entity from the LUA Script
			//! This assumes that there is a Table on the top of the LUA stack
			virtual void LoadFromScript(
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
				GG_Framework::Logic::ITransmittedEntityOwner* entityOwner);

			//! Build a Transmitted entity from a received Packet data
			//! Apply this AFTER any packet identifiers
			virtual void LoadFromBitStream(RakNet::BitStream& bs);
		};


		//! A DestroyableEntity is an Entity3D that can be destroyed (blown up).  It handles collision detection and 
		//! hit points and maintains a DestructionProxyEntity that is shown in the place of this object while
		//! being blown up.  It also waits for its own re-spawn time.
		class FRAMEWORK_APPREUSE_API DestroyableEntity : public CollisionEntity
		{
		public:
			DestroyableEntity();

			bool IsBeingDestroyed();

			virtual void Spawn();

			double GetRespawnPoints() const {return m_respawnHitPoints;} // (1 + m_Physics.GetMass() / 1400.0);}
			double GetRemainingHitPoints() const {return ((m_remHitPoints < 0.0) ? 0.0 : m_remHitPoints);}
			unsigned GetNumDestructions() const {return m_numDestructions;}

			/// Called as we are losing control of this entity
			virtual void SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner);

			/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
			virtual void RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner);

			/// Ignore collisions as I am being destroyed, so I do not crash into my destruction proxy while being destroyed
			virtual bool ShouldIgnoreCollisions(const Entity3D* otherEntity);


		protected:
			virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);

			// Watch for being hit to handle hit points and call DestroyEntity()
			virtual void OnCollision(Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s);

			// How many hit points of damage does the hit cause?  (may depend on the hit position
			virtual double GetDamageFromHit(Entity3D& otherEntity, const osg::Vec3d& collisionPt);

			// Called by the OnCollision that finishes the ship off
			virtual void DestroyEntity(bool shotDown, osg::Vec3d collisionPt);

			// Load the destruction entity asyc
			virtual bool AllowScriptHide(){return IsBeingDestroyed();}

			virtual void GameTimerUpdate(double time_s);

			// May be called from the RC side when hit points change
			void HitPointChange(double newHP);

			// Include my proxy in the list of related entities
			virtual void FindRelatedEntities(std::set<Entity3D*>& relatedEntities);

		private:
			// Temporary hit points.  We will eventually want something fancier
			double m_respawnHitPoints;
			double m_remHitPoints;
			double m_respawnTime;
			double m_nextRespawn;
			bool m_needFirstSpawn;
			unsigned m_numDestructions;
			DestructionProxyEntity* m_destProxy;
			IEvent::HandlerList ehl;
		};
	}
}