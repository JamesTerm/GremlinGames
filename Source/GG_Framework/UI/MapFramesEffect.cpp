// GG_Framework.UI MapFramesEffect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::Base;

namespace GG_Framework
{
	namespace UI
	{
		MapFramesEffect::TestKeyBinding::Container MapFramesEffect::TestKeyBinding::s_Container;

		MapFramesEffect::MapFramesEffect(ActorScene* actorScene, osg::Node& parent) : 
			m_minEventVal(0.0), m_maxEventVal(1.0), m_minTime(0), m_maxTime(FRAME_2_TIME(1)), m_currTime(m_minTime),
				INamedEffect(actorScene, parent) 
			{
				// The parent should ALWAYS be an actor, so we can subscribe to their DefaultTime event
				Actor* actor = dynamic_cast<Actor*>(&parent);
				ASSERT(actor);
				actor->DefaultTime.Subscribe(ehl, *this, &MapFramesEffect::UpdateTime);
			}

		void MapFramesEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
		{
			INamedEffect::ReadParameters(localMap, remLineFromFile);
			sscanf(remLineFromFile, "(%lf,%lf,%lf,%lf)", &m_minEventVal, &m_maxEventVal, &m_minTime, &m_maxTime);
			m_minTime = FRAME_2_TIME(m_minTime);
			m_maxTime = FRAME_2_TIME(m_maxTime);
			m_currTime = m_minTime;
			remLineFromFile = NULL;

			// Watch for stupid values
			double dEventVal = m_maxEventVal-m_minEventVal;
			ASSERT_MSG ((dEventVal < -0.00001) || (dEventVal > 0.00001), "MapFramesEffect event times are the same");
			ASSERT_MSG ((m_maxTime >= m_minTime), "MapFramesEffect max frame less than min frame");
		}

		void MapFramesEffect::LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt)
		{
			// We ONLY care about intensity
			double t = (intensity - m_minEventVal) / (m_maxEventVal-m_minEventVal);
			m_currTime = m_minTime + (t * (m_maxTime-m_minTime));

			// CLAMP the values
			if (m_currTime < m_minTime) m_currTime = m_minTime;
			else if (m_currTime > m_maxTime) m_currTime = m_maxTime;
		}

		MapFramesEffect::TestKeyBinding::TestKeyBinding(
			char key, GG_Framework::UI::EventMap& eventMap, GG_Framework::Base::Timer* timer, 
			const char* eventName, double minRange, double maxRange) : 
		m_timer(timer), m_startingEffectTime(-1.0), m_eventMap(eventMap), m_minRange(minRange), m_maxRange(maxRange), m_valEvent(NULL)
		{
			// We only do the rest if there is not already a KB
			std::string testEventName = BuildString("TestKB_%s", eventName);
			if (MainWindow::GetMainWindow()->GetKeyboard_Mouse().AddKeyBinding(key, testEventName, true, false))
			{
				// Listen to the player pressing that key
				eventMap.EventOnOff_Map[testEventName].Subscribe(ehl, *this, &MapFramesEffect::TestKeyBinding::OnKeyPress);
				// We need to listen to the timer event
				m_timer->CurrTimeChanged.Subscribe(ehl, *this, &MapFramesEffect::TestKeyBinding::OnTimer);

				// We will be firing this event repetitively
				m_valEvent = &(eventMap.EventValue_Map[eventName]);
			}
		}

		void MapFramesEffect::TestKeyBinding::OnTimer(double time_s)
		{
			if (m_startingEffectTime > 0.0)
			{
				// How long has it been since we pressed the button
				double dTime = time_s - m_startingEffectTime;

				// Modulate this time to eventually find the appropriate range value
				static const double VARY_TIME = 2.0;
				static const double HOLD_TIME = 1.0;
				dTime = fmod(dTime, (2.0*(VARY_TIME+HOLD_TIME)));

				// Go forward and hold, then go backwards and hold
				double val = 0.0;
				if (dTime < VARY_TIME)
					val = dTime/VARY_TIME;
				else if (dTime < (VARY_TIME+HOLD_TIME))
					val = 1.0;
				else if (dTime < (VARY_TIME+HOLD_TIME+VARY_TIME))
				{
					// Oscillate backwards
					dTime = (VARY_TIME+HOLD_TIME+VARY_TIME)-dTime;
					val = dTime/VARY_TIME;
				}
				else val = 0.0;	// Pausing on the other end

				// Linear interpolation between min and max values
				val = m_minRange + (m_maxRange-m_minRange)*val;

				// Fire the event
				m_valEvent->Fire(val);
			}
		}

		void MapFramesEffect::TestKeyBinding::OnKeyPress(bool on)
		{
			// We know we are playing due to the starting effect time
			m_startingEffectTime = on ? m_timer->GetCurrTime_s() : -1.0;
		}


	}
}