// GG_Framework.Logic Entity3D.h
#pragma once

#include <set>

namespace GG_Framework
{
	namespace Logic
	{
		class GameClient;
		struct TransmittedEntity;
		class FRAMEWORK_LOGIC_API Entity3D
		{
		public:
			Entity3D();
			virtual ~Entity3D();

			class FRAMEWORK_LOGIC_API EventMap : public GG_Framework::UI::EventMap
			{
			public:
				EventMap(bool listOwned = false) : GG_Framework::UI::EventMap(listOwned) {}

				// The hit, the other entity, local collision point, impulse time of the collision (1 frame?)
				Event3<Entity3D&, const osg::Vec3d&, double> Collision;
			};

			//! Fires position in m, attitude (H,P,R) in degrees
			Event2<const osg::Vec3d&, const osg::Vec3d&> PosAtt_Changed;
			
			osg::Vec3d GetPos_m() const {return m_pos_m;}
			osg::Quat GetAtt_quat() const {return m_att_quat;}
			osg::Vec3d GetDirection() const {return m_att_quat*osg::Vec3d(0.0,1.0,0.0);}
			osg::Vec3d GetLinearVelocity() const {return m_Physics.GetLinearVelocity();}
			double GetMass() const {return m_Physics.GetMass();}
			Physics::FlightDynamics &GetPhysics() {return m_Physics;}

			// This is where both the vehicle entity and camera need to align to, by default we use the actual orientation
			virtual const osg::Quat &GetIntendedOrientation() {return m_att_quat;}

			// These are all helper functions working with intended and actual orientation
			osg::Vec3d GetUpDownDirection(){return GetAtt_quat()*osg::Vec3d(0,0,1);}
			osg::Vec3d GetLeftRightDirection(){return GetAtt_quat()*osg::Vec3d(1,0,0);}

			osg::Vec3d Get_IO_ForwardReverseDirection(){return GetIntendedOrientation()*osg::Vec3d(0,1,0);}
			osg::Vec3d Get_IO_UpDownDirection(){return GetIntendedOrientation()*osg::Vec3d(0,0,1);}
			osg::Vec3d Get_IO_LeftRightDirection(){return GetIntendedOrientation()*osg::Vec3d(1,0,0);}

			osg::Vec3d GetForwardReverseDirectionConj(){return GetAtt_quat().conj()*osg::Vec3d(0,1,0);}
			osg::Vec3d GetUpDownDirectionConj(){return GetAtt_quat().conj()*osg::Vec3d(0,0,1);}
			osg::Vec3d GetLeftRightDirectionConj(){return GetAtt_quat().conj()*osg::Vec3d(1,0,0);}

			osg::Vec3d Get_IO_ForwardReverseDirectionConj(){return GetIntendedOrientation().conj()*osg::Vec3d(0,1,0);}
			osg::Vec3d Get_IO_UpDownDirectionConj(){return GetIntendedOrientation().conj()*osg::Vec3d(0,0,1);}
			osg::Vec3d Get_IO_LeftRightDirectionConj(){return GetIntendedOrientation().conj()*osg::Vec3d(1,0,0);}

			// Note that this will crash of there is no BB entity with a bounding sphere
			virtual double GetBoundRadius();
			virtual double GetBoundRadius2();

			const std::string& GetName() const {return m_name;}
			const std::string GetClassName() const {return m_className;}
			unsigned GetNetworkID() const {return m_networkID;}

			GG_Framework::UI::ActorTransform* Get_BB_Actor(){return m_BB_Actor.get();}
			GG_Framework::UI::ActorTransform* Get_UI_Actor(){return m_UI_Actor.get();}

			GameClient* GetGameClient(){return m_gameClient;}
			Entity3D::EventMap* GetEventMap(){return m_eventMap;}

			///For now this distinguishes between entities that a player could control, but at this time it is too early to know if this would also be the same 
			///method to determine if this also has the same meaning as what is "observable" by a player (e.g. only to control/observe a team's entities)
			///So far this is only used as a test case for the RC Controller, so it may be more useful to be combined with the observable logic
			//  [3/30/2009 JamesK]
			virtual bool IsPlayerControllable(){return false;}
			bool IsPlayerControlled(){return (m_isPlayerControlled==1);}
			virtual void SetPlayerControlled(bool controlled);
			
			static FactoryMapT<Entity3D> ClassFactory;

			int GetCollisionIndex(){return m_collisionIndex;}
			virtual void ResetPos();

			bool IsShowing(){return m_isShowing;}

			// This will cancel all existing controls.
			// Called when position reset or hiding, May be called more than once for several reasons
			virtual void CancelAllControls(){}

			// Knowing about where this entity is controlled
			PlayerID GetControllingPlayerID() const {return m_controllingPlayerID;}
			bool IsLocallyControlled() const;

			/// When overriding, be sure to create m_RC_Controller your own way before returning
			virtual RC_Controller& GetRC_Controller();

			/// There may be other entities related to me that should all be controlled by the same PlayerID
			virtual void FindRelatedEntities(std::set<Entity3D*>& relatedEntities) {}
			virtual Entity3D* GetParentEntity(){return NULL;}

			/// Called as we are losing control of this entity
			virtual void SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner);

			/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
			virtual void RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner);

			// We may want to ignore certain bounding checks
			virtual bool ShouldIgnoreCollisions(const Entity3D* otherEntity) const {return false;}

			osg::Vec3d GetDimensions() const {return Dimensions;}

		protected:
			//Rick sorry this is here, this is the only clean way to monitor displacement...
			//  [10/2/2009 JamesK]
			virtual void TestPosAtt_Delta(const osg::Vec3d pos_m, const osg::Vec3d att,double dTime_s) {}
			// Only Super Classes can set this directly
			virtual void SetPosAtt(const osg::Vec3d pos_m, const osg::Quat att);
			///Get if we are sending or receiving
			/// \note this may be different than IsLocallyControlled() for test simulations
			bool RC_Controller_GetIsSending() {return GetRC_Controller().RC_Controller_GetIsSending();}
			//! Here is where we get one pass of the Game Loop
			virtual void GameTimerUpdate(double time_s);
			virtual void TimeChange(double dTime_s);

			//! This is called from GameClient, and the te will go away after the call
			virtual void Initialize( GameClient& gs, Entity3D::EventMap& em, TransmittedEntity& te);

			//! Some Entity3D objects will care about the Actors they get (e.g to control turrets)
			virtual void SetActor(GG_Framework::UI::ActorTransform* ap, bool bbox);

			//! Here is access to the physics entity
			Physics::FlightDynamics m_Physics;

			// We MAY want to load some things ASYNC
			virtual void OnAsyncLoad() {}
			virtual void OnAsyncLoadComplete() {}

			virtual void OnOSGV_HIDE();
			virtual bool AllowScriptHide(){return false;}

			// Use this to control how the collision detection happens.  Less than 0 ignores collisions.
			// 0 collides with anything greater than 0.
			// All others collide only with objects greater or equal.  
			int m_collisionIndex;

			// This may be called by descendant, but only as they are being initialized
			void ShowNextFrame(bool show) {m_showNextFrame = show;}
			

			///All entities will either send position updates, or receive them and be controller from them here
			RC_Controller* m_RC_Controller;
			friend RC_Controller;

			virtual void SetControllingPlayerID(const PlayerID& controllingPlayerID) {m_controllingPlayerID = controllingPlayerID;}

			// Only set my the RC_Controller
			virtual void Show(bool show);

		private:
			IEvent::HandlerList ehl;

			//! There will be one of these for the Physics RigidBodies to use
			osg::ref_ptr<GG_Framework::UI::ActorTransform>	m_BB_Actor;

			//! We may EVENTUALLY have a map of these for different viewers, but one is OK for now
			osg::ref_ptr<GG_Framework::UI::ActorTransform>	m_UI_Actor;

			void SetActor(GG_Framework::UI::ActorTransform* ap, 
				osg::ref_ptr<GG_Framework::UI::ActorTransform>& refPtr, GG_Framework::UI::ActorScene* scene);
			
			GameClient* m_gameClient;	//!< There will be only one, but NOT established in a c'tor
									//!< because we are using a class factory, so not a const ptr
			Entity3D::EventMap* m_eventMap;	//! Same

			std::string m_name;
			std::string m_className;
			unsigned m_networkID;

			osg::Vec3d m_origPos;
			osg::Vec3d m_origAtt;	// Using H,P,R Not a Quat
			osg::Vec3d m_pos_m;
			osg::Quat m_att_quat;
			bool m_isShowing, m_showNextFrame;
			double m_lastTime_s;
			int m_isPlayerControlled; //-1 for not set

			// We do not want to test for collisions if there was a collision in the previous frame.
			// A one frame difference is not good enough to gauge the line segment
			// Game scene uses this in its ProcessCollisions, 
			Entity3D* m_collidedLastFrame; // Only used for collider

			//! This Event goes to the ActorParents
			Event2<const osg::Vec3&, const osg::Quat&> PosQuat_Changed;

			friend ThreadedEntityActorLoader;
			friend GameClient;

			// Keep a list of nodes we can use for the collision points
			std::vector<osg::Node*>	m_collisionPoints;

			// We can keep the general dimensions of the entity
			osg::Vec3d Dimensions;

			// We need to know where the controller is for this entity
			PlayerID m_controllingPlayerID;
		};
	}
}
