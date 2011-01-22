// GG_Framework.UI Action.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

#include "..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

Action::Action(GG_Framework::UI::EventMap& eventMap, ActorScene& actorScene, const char* lineFromFile)
{
	char name[64];
	char startPose[64];
	char endPose[64];
	if (sscanf(lineFromFile, "@ %63s %63s %i %63s %i", name, startPose, &m_startFrame, endPose, &m_endFrame) != 5)
	{
		throw std::exception("Error in Action Line");
	}
	m_name = name;
	m_startPose = startPose;
	m_endPose = endPose;

	// Look for Delayed Events
	const char* lookAfter = lineFromFile;
	const char* firstBrace;
	while (firstBrace = strchr(lookAfter, '['))
	{
		char modeStr[8];
		char eventName[64];
		int startFrame, endFrame;
		const char* endBrace = strchr(firstBrace, ']');
		const char* hyphen = strchr(firstBrace, '-');
		if (hyphen && (hyphen < endBrace))
		{
			sscanf(firstBrace, "[%s %i-%i %s]", modeStr, &startFrame, &endFrame, eventName);
		}
		else
		{
			sscanf(firstBrace, "[%s %i %s]", modeStr, &startFrame, eventName);
			endFrame = startFrame;
		}
		// Strip the ']' off the eventName if it is there
		{
			char* endBrace = strchr(eventName, ']');
			if (endBrace) *endBrace = 0;
		}
		if (!strcmp(eventName, "HIDE"))
		{
			DEBUG_DESTRUCTION("Action::Action found delayed HIDE %i frames from %s.\n", startFrame, m_name.c_str());
		}
		m_delayedEvents.push_back(DelayedEvent(eventMap, actorScene, eventName, startFrame, endFrame, (IEffect::Mode)(toupper(modeStr[0]))));
		lookAfter = firstBrace+1;
	}
}
//////////////////////////////////////////////////////////////////////////

void Action::LaunchDelayedEvents(double launchTime)
{
	int launchFrame = TIME_2_FRAME(launchTime);
	std::list<Action::DelayedEvent>::iterator pos;
	for (pos = m_delayedEvents.begin(); pos != m_delayedEvents.end(); ++pos)
		(*pos).Launch(launchFrame);
}
//////////////////////////////////////////////////////////////////////////

double Action::GetActionTime_s(double offsetFromStart)
{
	double ret = FRAME_2_TIME(m_startFrame);
	if (m_endFrame > m_startFrame)
		ret = FRAME_2_TIME(m_startFrame) + offsetFromStart;
	else if (m_startFrame > m_endFrame)
		ret = FRAME_2_TIME(m_endFrame) - offsetFromStart;
	return ret;
}
//////////////////////////////////////////////////////////////////////////

Action::DelayedEvent::DelayedEvent(GG_Framework::UI::EventMap& eventMap, 
	ActorScene& actorScene, const char* eventName, int startFramesAfterLaunch, int endFramesAfterLaunch, IEffect::Mode mode) :
m_eventName(eventName), m_mode(mode), m_launchFrame(-1), m_started(false),
	m_startFramesAfterLaunch(startFramesAfterLaunch), m_endFramesAfterLaunch(endFramesAfterLaunch),
	m_actorScene(&actorScene), m_eventMap(&eventMap)
{}
//////////////////////////////////////////////////////////////////////////

void Action::DelayedEvent::Launch(int launchFrame)
{
	m_started = false;
	if (m_launchFrame < 0) // Make sure we are only subscribing once
		m_actorScene->GetTimer()->CurrTimeChanged.Subscribe(ehl, *this, &Action::DelayedEvent::GlobalTimeChangedCallback);
	m_launchFrame = launchFrame;
}
//////////////////////////////////////////////////////////////////////////

void Action::DelayedEvent::GlobalTimeChangedCallback(double newTime_s)
{
	int currFrame = TIME_2_FRAME(newTime_s);
	if (currFrame >= (m_launchFrame+m_startFramesAfterLaunch))
	{
		if (m_started && (currFrame >= (m_launchFrame+m_endFramesAfterLaunch)))
		{
			if (m_mode == IEffect::TOGGLE)
			{
				m_eventMap->Event_Map[m_eventName].Fire();
			}
			else
			{
				m_eventMap->EventOnOff_Map[m_eventName].Fire(false);
			}
			m_started = false;
			m_actorScene->GetTimer()->CurrTimeChanged.Remove(*this, &Action::DelayedEvent::GlobalTimeChangedCallback);
			m_launchFrame = -1;
		}
		else if (!m_started)
		{
			if (m_mode == IEffect::ON)
			{
				if (m_eventName == "HIDE")
				{
					DEBUG_DESTRUCTION("Action::DelayedEvent::GlobalTimeChangedCallback() Firing HIDE.\n");
				}
				m_eventMap->Event_Map[m_eventName].Fire();
				m_actorScene->GetTimer()->CurrTimeChanged.Remove(*this, &Action::DelayedEvent::GlobalTimeChangedCallback);
				m_launchFrame = -1;
			}
			else if (m_mode == IEffect::TOGGLE)
			{
				m_started = true;
				m_eventMap->Event_Map[m_eventName].Fire();
			}
			else
			{
				m_started = true;
				m_eventMap->EventOnOff_Map[m_eventName].Fire(true);
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////