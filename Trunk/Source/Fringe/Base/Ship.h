//! Fringe.Base	Ship.h
#pragma once

#include <set>

namespace Fringe
{
	namespace Base
	{
		//! A Ship is a DestroyableEntity that can also have cannons
		class FRINGE_BASE_API Ship : public GG_Framework::AppReuse::DestroyableEntity, public GG_Framework::AppReuse::ICannonOwner
		{
		public:
			Ship();
			virtual ~Ship();
			osg::Vec3d GetVelocity_m_s() const {return m_Physics.GetLinearVelocity();}
			double GetWeaponsRange() const {return m_cannonRange;}
			double GetWeaponsSpeed() const {return m_cannonSpeed;}
			void GetWeaponRelPosAtt(osg::Vec3d& pos, osg::Quat& att);

			virtual void TimeChange(double dTime_s);
			virtual void TryFireMainWeapon(bool on);

			const osg::Vec3d* GetGunTarget(){return m_gunTargetLead;}
			void SetGunTarget(const osg::Vec3d* target){m_gunTargetLead = target;}

			void GetWeaponStatus(GG_Framework::AppReuse::Cannon::TempStatus& tempStatus, double& tempLevel, double& restartLevel) const;

			// ICannonOwner
			virtual GG_Framework::Logic::Entity3D& ICO_GetSelfEntity3D() {return *this;}

			// Include my cannon rounds in the list of related entities
			virtual void FindRelatedEntities(std::set<Entity3D*>& relatedEntities);

			// If I am gaining control of a ship, I want to FLY it!
			virtual void SetControllingPlayerID(const PlayerID& controllingPlayerID);

			/// Called as we are losing control of this entity
			virtual void SendFinalUpdate(const PlayerID& playerThatWantsControl, RakNet::BitStream& sendControlBS, bool sameOwner);

			/// Someone is getting new control.  All clients get this message to assign the new controller and get last update
			virtual void RecvFinalUpdate(RakNet::BitStream& recvControlBS, const PlayerID& playerThatWantsControl, double msgTime_s, bool sameOwner);


		protected:
			virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);

			virtual bool ShouldFireVaporTrails(double dTime_s);
			virtual void CancelAllControls();

			const osg::Vec3d* m_gunTargetLead;

			std::vector<GG_Framework::AppReuse::Cannon*> m_Cannons;
			double m_cannonRange;
			double m_cannonSpeed;
			bool m_vaporTrails;
			GG_Framework::UI::OSG::VectorDerivativeOverTimeAverager m_accelDeriv;

			// Load the rounds for the cannon async
			virtual void OnAsyncLoadComplete();

		private:
			GG_Framework::AppReuse::Cannon::TempStatus m_cannonTempStatus;
			double m_cannonTempLevel;
			double m_cannonRestartLevel;
			void UpdateCannonTemps();
			IEvent::HandlerList ehl;
		};
	}
}