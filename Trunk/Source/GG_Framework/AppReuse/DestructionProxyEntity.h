// DestructionProxyEntity.h
#pragma once

namespace GG_Framework
{
	namespace AppReuse
	{
		//! This class represents an entity in the scene that is exploding.  A DestroyableEntity
		//! Creates one of these and does not show it until the DestroyableEntity starts to be destroyed
		//! At that time, it hides itself and shows this.  This Entity3D has some basic collision,
		//! but there are no notions of hit points, so this Entity3D cannot be blown up.
		//! This created and loaded asynchronously as a part of the DestroyableEntity
		class DestructionProxyEntity : public CollisionEntity
		{
		public:
			DestructionProxyEntity() : m_parentEntity(NULL) {}
			virtual ~DestructionProxyEntity() { }
			
			// Initialize with the name of an OSGV file
			virtual void Initialize(GG_Framework::Logic::GameClient& gs, GG_Framework::Logic::Entity3D::EventMap& em, GG_Framework::Logic::TransmittedEntity& te);

			// Call this function to place this at the proper pos, att, and velocities of the original
			void AlignWithOrigAndShow(osg::Vec3d origCurrPos_m, osg::Quat origAtt, const GG_Framework::Logic::Physics::PhysicsEntity& origPhysics);

			void SetParentEntity(Entity3D* parentEntity){m_parentEntity = parentEntity;}
			virtual Entity3D* GetParentEntity(){return m_parentEntity;}

		protected:
			virtual bool AllowScriptHide(){return true;}
			Entity3D* m_parentEntity;
		};
	}
}

