// GG_Framework.UI MapFramesEffect.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		// This effect listens to an event and sets a frame based on the value of the event
		// All frames are mapped from minimum to maximum values, linearly interpolated, and clamped.
		// See See MapFramesEffect.txt for design details.
		class FRAMEWORK_UI_API MapFramesEffect : public INamedEffect
		{
		public:
			MapFramesEffect(ActorScene* actorScene, osg::Node& parent);
			virtual void LaunchEffect(
				double launchTime_s, bool start, double intensity, const osg::Vec3d& pt);

			// This class is responsible for listening to a keyboard event and firing the appropriate value event
			class TestKeyBinding
			{
			public:
				static void Add(char key, GG_Framework::UI::EventMap& localMap, GG_Framework::Base::Timer* timer, 
					const char* eventName, double minRange, double maxRange) {
					// Deleted in the Container class below
					s_Container.Add(new TestKeyBinding(key, localMap, timer, eventName, minRange, maxRange));
				}

			private:
				TestKeyBinding(
					char key, GG_Framework::UI::EventMap& eventMap, GG_Framework::Base::Timer* timer, 
					const char* eventName, double minRange, double maxRange);

				void OnTimer(double time_s);
				IEvent::HandlerList ehl;

				double m_minRange, m_maxRange;
				GG_Framework::UI::EventMap& m_eventMap;

				// Set while the Key is pressed, is -1 normally
				double m_startingEffectTime;
				GG_Framework::Base::Timer* m_timer;

				// When the user presses the appropriate key
				void OnKeyPress(bool on);

				// The event we fire regularly
				Event1<double>* m_valEvent;

				// this single instance is simply responsible for owning all instances of TestKeyBinding so it can clean them up
				class Container
				{
				private:
					std::vector<TestKeyBinding*> m_bindings;
				public:
					Container(){}
					void Add(TestKeyBinding* kb){m_bindings.push_back(kb);}
					~Container()
					{
						for (unsigned i = 0; i < m_bindings.size(); ++i)
							delete m_bindings[i];
					}
				};
				static Container s_Container;

			};

		protected:
			virtual void ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile);

			class MapFramesEffectClassFactory : public INamedEffectClassFactory
			{
			protected:
				virtual INamedEffect* CreateEffect(ActorScene* actorScene, const char* name, osg::Node& parent)
				{
					if (!stricmp(name, "MapFramesEffect")) return new MapFramesEffect(actorScene, parent);
					else return NULL;
				}
				virtual void DeleteEffect(INamedEffect* effect){delete dynamic_cast<MapFramesEffect*>(effect);}
			};

			void UpdateTime(double& time){time = m_currTime;}

		private:
			double m_minEventVal, m_maxEventVal;
			double m_minTime, m_maxTime;
			double m_currTime;

			// Listen for adjustments to all sound scales from the actor
			IEvent::HandlerList ehl;
			static MapFramesEffectClassFactory s_classFactory;
		};

	}
}