// GG_Framework.UI OsgFile_ParticleEffect.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API OsgFile_ParticleEffect : public INamedEffect
		{
		public:
			OsgFile_ParticleEffect(ActorScene* actorScene, osg::Node& parent) : 
			  INamedEffect(actorScene, parent), m_addedAlready(false) {}
			virtual void LaunchEffect(
				double launchTime_s, bool start, double intensity, const osg::Vec3d& pt);

		protected:
			virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);

			class OsgFile_ParticleEffectClassFactory : public INamedEffectClassFactory
			{
			protected:
				virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
				{
					if (!stricmp(name, "OsgFile_ParticleEffect")) return new OsgFile_ParticleEffect(actorScene, parent);
					else return NULL;
				}
				virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<OsgFile_ParticleEffect*>(effect);}
			};


		private:
			static OsgFile_ParticleEffectClassFactory s_classFactory;
			osg::ref_ptr<osg::Node> m_osgFileNode;
			bool m_addedAlready;
		};

	}
}