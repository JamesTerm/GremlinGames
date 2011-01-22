// GG_Framework.UI NamedEffect.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		class INamedEffectClassFactory;
		class FRAMEWORK_UI_API INamedEffect : public IEffect
		{
		public:
			INamedEffect(ActorScene* actorScene, osg::Node& parent) : IEffect(actorScene), m_parent(&parent), m_classFactory(NULL) {}
			void Delete();

			// This is still pure virtual
			virtual void LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt) = 0;
			const char* GetName(){return m_name.c_str();}
			osg::Node* GetParentNode(){return m_parent.get();}

		protected:
			// Be sure to make your destructor protected
			virtual ~INamedEffect(){}

			// Always call this base class version
			virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);

		private:
			osg::ref_ptr<osg::Node> m_parent;
			std::string m_name;
			INamedEffectClassFactory* m_classFactory;
			friend INamedEffectClassFactory;
		};


		class INamedEffectClassFactory
		{
		public:
			INamedEffectClassFactory(){s_list.push_back(this);}
			virtual ~INamedEffectClassFactory(){s_list.remove(this);}
			static INamedEffect* CreateEffect(
				ActorScene* actorScene, GG_Framework::UI::EventMap& localMap, 
				osg::Node& parent, const char* lineFromFile);

		protected:
			virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent) = 0;
			virtual void DeleteEffect(INamedEffect* effect) = 0;

		private:
			static std::list<INamedEffectClassFactory*> s_list;
			friend INamedEffect;
		};
	}
}