// GG_Framework.Logic Entity3D.h
#pragma once

namespace GG_Framework
{
	namespace Logic
	{

		enum EnityUpdateMSGS
		{
			ENTITY_PLAYER_CONTROLLED,
			ENTITY_NONPLAYER_CONTROLLED,
			ENTITY_POSATT_UPDATE,
			ENTITY_COLLISION,
			ENTITY_ExternalForce,
			ENTITY_Event,
			ENTITY_EventOnOff,
			ENTITY_EventValue,
			ENTITY_EventPt,
			ENTITY_EventPtOnOff,
			ENTITY_EventPtValue,

			ENTITY_Last_RC_Msg,
		};

class Entity3D;
class GameClient;
struct TransmittedEntity;

class FRAMEWORK_LOGIC_API RC_Controller
{
	public:
		RC_Controller(Entity3D *Parent);
		virtual ~RC_Controller();

		//! This is called from GameClient, and the te will go away after the call
		void Initialize( GameClient& gs, TransmittedEntity& te);

		///Get if we are sending or receiving
		/// \note this may be different than IsLocallyControlled() for test simulations
		bool RC_Controller_GetIsSending() const;
		
		///This is where and when we send or receive
		virtual void TimeChange_pre(double dTime_s);
		
		///The sender needs to measure before and after the physics time change is applied
		virtual void TimeChange_post(double dTime_s);

		/// Called by the parent when it is being controlled by a player
		virtual void ParentPlayerControlled(bool c);

		/// Called from the server with a network message for my entity
		virtual void SpecialEntityMessageFromServer(unsigned char msg, double msgTime_s, RakNet::BitStream& entityBS);

		/// Some entities do not want to deal with physics ever
		void SetRecvIgnorePhysics(bool ignorePhysics);
		bool GetIgnorePhysics() {return m_ignorePhysics;}

		/// One last update, sent as the entity is changing control
		virtual void SendFinalUpdate(RakNet::BitStream& sendControlBS, bool sameOwner);
		virtual void RecvFinalUpdate(RakNet::BitStream& recvControlBS, double msgTime_s, bool sameOwner);

		// Call these methods to fire events back along the entity AND send messages along the network
		void FireNetwork_Collision		(Entity3D& otherEntity, const osg::Vec3d& localCollPt, double impulseTime);
		void FireNetwork_ExternalForce	( const osg::Vec3d &force, const osg::Vec3d &point,double FrameDuration );
		void FireNetwork_Event			(std::string msgID, PacketPriority priority, PacketReliability reliability, char orderingChannel);
		void FireNetwork_EventOnOff		(std::string msgID, bool onOff, PacketPriority priority, PacketReliability reliability, char orderingChannel);
		void FireNetwork_EventValue		(std::string msgID, double val, PacketPriority priority, PacketReliability reliability, char orderingChannel);
		void FireNetwork_EventPt		(std::string msgID, const osg::Vec3d& pt, PacketPriority priority, PacketReliability reliability, char orderingChannel);
		void FireNetwork_EventPtOnOff	(std::string msgID, const osg::Vec3d& pt, bool onOff, PacketPriority priority, PacketReliability reliability, char orderingChannel);
		void FireNetwork_EventPtValue	(std::string msgID, const osg::Vec3d& pt, double val, PacketPriority priority, PacketReliability reliability, char orderingChannel);

		/// Whoever shows the entity the first time is responsible for populating this
		struct FRAMEWORK_LOGIC_API PosAttUpdate
		{
			PosAttUpdate() : DeltaTime_s(0.0) {}

			// TODO: James, you want to add some stuff in here right?
			osg::Vec3d POS;
			osg::Quat ATT;

			osg::Vec3d LinearVelocity;		///< This represents global orientation
			osg::Vec3d AngularVelocity;		///< All angle variables are in radians!

			osg::Vec3d LinearAcceleration;
			osg::Vec3d AngularAcceleration;

			void ReadFromBS(RakNet::BitStream& bs);
			void WriteToBS(RakNet::BitStream& bs);
			double DeltaTime_s;
		};

		/// Get the last update that was sent (can be used to pass in to a ShowParent from a different entity)
		const PosAttUpdate& GetLastUpdate() const {return m_NetworkPosAtt;}

		/// Controllers Call this function to show an Entity, placing it at a new pos/att, so the messages can be sent everywhere
		virtual void ShowParent(bool showing);

		/// Override these functions to work with STANDARD updates (those sent every frame)
		virtual bool WantsToSendEntityUpdate(){return m_wantsEntityUpdate;}
		virtual void PopulateStandardEntityUpdate(RakNet::BitStream& entityUpdateBS);
		virtual void ReadStandardEntityUpdate(RakNet::BitStream& entityUpdateBS, double msgTime_s);

		void ResetLastPosAtt();

	protected:
		Entity3D * const m_pParent;
		bool m_wantsEntityUpdate, m_ignorePhysics;

		// These are all messages received off the network for a packet
		void RecvNetwork_Collision		(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_ExternalForce	(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_Event			(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_EventOnOff		(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_EventValue		(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_EventPt		(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_EventPtOnOff	(RakNet::BitStream& bs, double msgTime_s);
		void RecvNetwork_EventPtValue	(RakNet::BitStream& bs, double msgTime_s);

		PosAttUpdate m_NetworkPosAtt, m_PredictedPosAtt;
		osg::Vec3d m_RadialArmDefault; //cache the radius of concentrated mass square, which will allow us to apply torque in a r = 1 case

		//For testing purposes only
		struct SimulateInfo
		{
			PosAttUpdate posatt;
			double lastUpdateTime;
		} m_PreviousSimulatedValue;
		static SimulateInfo s_SimulatedTransfer;
		double m_lastTransmittedUpdateTime_s;
		bool m_resetVis;
		friend Entity3D;
};

	} //end namespace Logic
} //end namespace GG_Framework