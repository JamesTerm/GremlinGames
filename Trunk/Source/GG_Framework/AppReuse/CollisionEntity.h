// GG_Framework.AppReuse CollisionEntity.h
#pragma once

namespace GG_Framework
{
	namespace AppReuse
	{
		//! A CollisionEntity is any entity that responds to collisions in a scene.  This base class performs the
		//! physics interactions by applying external forces and firing collision events for OSGV.  This base class
		//! never gets blown up.
		class FRAMEWORK_APPREUSE_API CollisionEntity : public GG_Framework::Logic::Entity3D
		{
		protected:
			virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);

			// Watch for being hit
			virtual void OnCollision(Entity3D& otherEntity, const osg::Vec3d& hitPt, double dTime_s);

		private:
			struct CannonRoundHitOccurance
			{
				CannonRoundHitOccurance(double t, const osg::Vec3d& hp) : time(t), hitPt(hp) {}
				double time;
				osg::Vec3d hitPt;
			};
			std::list<CannonRoundHitOccurance> m_cannonHits;

			void TryDisplayCannonHit(osg::Vec3d hitPt);
			IEvent::HandlerList ehl;
		};
	}
}