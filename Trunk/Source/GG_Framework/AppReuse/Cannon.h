// Cannon.h
#pragma once

#include <set>

namespace GG_Framework
{
	namespace AppReuse
	{

		class FRAMEWORK_APPREUSE_API ICannonOwner
		{
		public:
			virtual GG_Framework::Logic::Entity3D& ICO_GetSelfEntity3D() = 0;
			virtual const osg::Vec3d* GetGunTarget() = 0;

			// Called from one of my cannon rounds when it hits
			Event1<GG_Framework::Logic::Entity3D*> CannonRoundHit;
		};

		class IWeaponEntity
		{
		public:
			virtual double GetHitDamage() = 0;
		};

		class CannonRound_RC_Controller;
		class FRAMEWORK_APPREUSE_API CannonRound : public GG_Framework::Logic::Entity3D, public IWeaponEntity
		{
		public:
			CannonRound() : m_roundLife_s(1.5), m_fireTime_s(-1.0), 
				m_parent(NULL), m_lastTime_s(-1.0), m_hitDamage(1.0) 
				{}
			~CannonRound();
			void CannonInit(double roundLife_s, ICannonOwner* parent, double hitDamage);

			void Fire(double hitDamage, const osg::Vec3d& pos_m, const osg::Quat& att_q, const osg::Vec3d& vel, const osg::Vec3d& accel, double time_s);

			virtual void GameTimerUpdate(double time_s);
			virtual void TimeChange(double dTime_s);

			void OnCollision(Entity3D& otherEntity, const osg::Vec3d& myHitPt, double dTime_s);
			virtual bool ShouldIgnoreCollisions(const Entity3D* otherEntity) const;

			// Expand the bound radius to make it easier to hit
			virtual double GetBoundRadius(){return 1.0;}
			virtual double GetBoundRadius2(){return 1.0;}

			virtual double GetHitDamage(){return m_hitDamage;}

			/// This override creates a CannonRound_RC_Controller
			virtual GG_Framework::Logic::RC_Controller& GetRC_Controller();

			/// Called from my RC Controller
			void RC_Fire(double hitDamage);

			virtual Entity3D* GetParentEntity(){return &(m_parent->ICO_GetSelfEntity3D());}
		
		private:
			double m_lastTime_s;
			double m_roundLife_s;
			double m_hitDamage;
			ICannonOwner* m_parent;
			double m_fireTime_s;
			osg::Vec3d m_tumbleAccel;
			IEvent::HandlerList ehl;

			CannonRound_RC_Controller* m_CR_RC_Controller;
		};
		//////////////////////////////////////////////////////////////////////////

		enum CannonRdUpdateMSGS
		{
			CannonRd_Fire = GG_Framework::Logic::ENTITY_Last_RC_Msg + 1,
		};

		class FRAMEWORK_APPREUSE_API CannonRound_RC_Controller : public GG_Framework::Logic::RC_Controller
		{
		public:
			CannonRound_RC_Controller(CannonRound* parent) : 
			  m_cannonRoundParent(parent), GG_Framework::Logic::RC_Controller(parent) {}

			/// Called from the server with a network message for my entity
			/// This version watches for being fired
			virtual void SpecialEntityMessageFromServer(unsigned char msg, double msgTime_s, RakNet::BitStream& entityBS);

			/// Called from CannonRound, sends the network message
			void FireNetwork_FireCR(float hitDamage, const osg::Vec3d& pos_m, const osg::Quat& att_q, const osg::Vec3d& vel, const osg::Vec3d& accel);
			void RecvNetwork_FireCR(RakNet::BitStream& bs, double msgTime_s);

		private:
			CannonRound* m_cannonRoundParent;
		};
		//////////////////////////////////////////////////////////////////////////

		//! USe this as a part of a Transmitted Entity
		struct FRAMEWORK_APPREUSE_API CannonDesc
		{
			CannonDesc();

			// We WILL connect the cannons to a NULL in the BB scene
			// But for now we will just fire forward from the center
			std::string Placement_NULL_Name;

			// A vector of all of our rounds
			std::vector<unsigned> Round_Magazine_NetID;

			// The name of the event that triggers the cannons to be fired (While)
			std::string Fire_EventName;

			double Round_Speed;  // m/s
			double Round_Life; // s
			double Round_Accuracy; // possible m after one second
			double Round_Damage; // Subtraction from hit points

			double Firing_Rate; // per second
			double Firing_Offset;		// amount of time from the time the button is fired to when the first 
											// round fires.  This is good for chaining multiple cannons or having them
											// be offset a little from each other

			double HEAT_UP_TIME;
			double COOL_DOWN_TIME;
			double RESTART_LEVEL;
			double Adjust_Angle_Deg; // How far from center the cannon will auto-adjust to hit the target, default 1.2 deg

			void LoadFromScript(std::string ownerName, int cannonIndex,
				GG_Framework::Logic::Scripting::Script& script, PlayerID controllingPlayerID, 
				GG_Framework::Logic::ITransmittedEntityOwner* entityOwner);

			void LoadFromBitStream(RakNet::BitStream& bs);
			void WriteToBitStream(RakNet::BitStream& bs);
		};

		class FRAMEWORK_APPREUSE_API Cannon
		{
		public:
			Cannon(unsigned cannonIndex, ICannonOwner& cannonOwner, CannonDesc& desc);

			virtual void OnAsyncLoadComplete();

			void OnFireCannons(bool start);
			void TimeChanged(double time_s);

			enum TempStatus {
				CANNON_Overheated = 0,
				CANNON_Warning,
				CANNON_Nominal,
				CANNON_Cold,
			};

			//! tempLevel ranges from 0 to 1
			void GetCannonTemp(TempStatus& tempStatus, double& tempLevel, double& restartLevel) const
			{tempStatus = m_tempStatus; tempLevel = m_tempLevel; restartLevel = m_desc.RESTART_LEVEL;}

			static double ASSIST;

			// Include my cannon rounds in the list of related entities
			virtual void FindRelatedEntities(std::set<GG_Framework::Logic::Entity3D*>& relatedEntities);

			const osg::Vec3d& GetAbsPos() const {return m_absPos_m;}
			const osg::Vec3d& GetRelPos() const {return m_relPos_m;}
			osg::Quat  GetAbsAtt() const {return m_absAtt_q;}
			osg::Quat  GetRelAtt() const {return m_relAtt_q;}

		private:
			CannonDesc m_desc;
			ICannonOwner& m_cannonOwner;
			IEvent::HandlerList ehl;

			std::vector<CannonRound*> m_magazine;
			unsigned m_currMagIndex;

			void FindPosition();
			void FireRound(CannonRound* round, double time_s);

			// Use -2 for not firing, -1 for not fired the first round yet
			double m_lastFireTime_s;
			double m_timeBetweenRounds_s;
			double m_cosAdjAngle;

			// Keep track of cannon temperatures
			TempStatus m_tempStatus;
			double m_tempLevel;
			double m_lastTime_s;
			bool m_isFiring;

			osg::Vec3d m_relPos_m;
			osg::Quat m_relAtt_q;
			osg::Vec3d m_absPos_m;
			osg::Quat m_absAtt_q;

			osg::ref_ptr<osg::Node> m_cannonPositionNode;
		};
		//////////////////////////////////////////////////////////////////////////
	}
}
