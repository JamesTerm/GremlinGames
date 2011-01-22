// GG_Framework.UI Effect.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

void IEffect::EventCallbackPtDnUp(const osg::Vec3d& pt, bool pressed)
{
	bool start = pressed;
	m_pressed = start && GetLooping();

	// Get the time the event was fired.  This might be an alternate event time
	double effectLaunchTime = m_subscribedEventMap->AlternateEventTime ? 
		*(m_subscribedEventMap->AlternateEventTime) : m_actorScene->GetOsgTimer()->GetCurrTime_s();
		
	LaunchEffect(effectLaunchTime, start, GetDefaultIntensity(), pt);
}
//////////////////////////////////////////////////////////////////////////

void IEffect::EventCallbackPtIntensity(const osg::Vec3d& pt, double intensity)
{
	bool start = !(m_pressed&&GetLooping()&&(m_mode != LOOP));
	m_pressed = start && GetLooping();

	// Get the time the event was fired.  This might be an alternate event time
	double effectLaunchTime = m_subscribedEventMap->AlternateEventTime ? 
		*(m_subscribedEventMap->AlternateEventTime) : m_actorScene->GetOsgTimer()->GetCurrTime_s();

	LaunchEffect(effectLaunchTime, start, intensity, pt);
}
//////////////////////////////////////////////////////////////////////////

void IEffect::EventCallbackDnUp(bool pressed)
{
	osg::Vec3d defPt = GetDefaultPoint();
	EventCallbackPtDnUp(defPt, pressed);
}
//////////////////////////////////////////////////////////////////////////

void IEffect::EventCallbackIntensity(double intensity)
{
	osg::Vec3d defPt = GetDefaultPoint();
	EventCallbackPtIntensity(defPt, intensity);
}
//////////////////////////////////////////////////////////////////////////

void IEffect::EventCallbackDn()
{
	osg::Vec3d defPt = GetDefaultPoint();
	EventCallbackPtIntensity(defPt, GetDefaultIntensity());
}
//////////////////////////////////////////////////////////////////////////

void IEffect::EventCallbackPt(const osg::Vec3d& pt)
{
	EventCallbackPtIntensity(pt, GetDefaultIntensity());
}
//////////////////////////////////////////////////////////////////////////

void IEffect::Initialize(GG_Framework::UI::EventMap& localMap, const char* lineFromFile)
{
	ReadParameters(localMap, lineFromFile);
}
//////////////////////////////////////////////////////////////////////////

void IEffect::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	// We need to remember this to get the network offsets when events are fired
	m_subscribedEventMap = &localMap;

	char s[2][32];
	if (sscanf(remLineFromFile, "%31s %31s", s[0], s[1]) != 2)
		throw std::exception("Error in Effect Line");

	// Find the mode we are working in
	m_mode = (IEffect::Mode)(toupper(s[0][0]));

	if ((!stricmp(s[1], "START")) && (m_mode != ON))
		m_mode = TOGGLE;	// The START is only for On and Toggle loops

	if (m_mode==WHILE) // While
	{
		localMap.EventOnOff_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackDnUp);
		localMap.EventPtOnOff_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackPtDnUp);
	}
	else
	{
		localMap.Event_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackDn);
		localMap.EventValue_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackIntensity);
		localMap.EventPt_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackPt);
		localMap.EventPtValue_Map[s[1]].Subscribe(ehl, *this, &IEffect::EventCallbackPtIntensity);
	}

	m_constant = false;
	if ((m_mode == TOGGLE) || (m_mode == WHILE))
	{
		if ((!stricmp(s[1], "START")) || (!stricmp(s[1], "SPAWN")))
			m_constant = true;
	}

	remLineFromFile += strlen(s[0]) + strlen(s[1]) + 2;
}
//////////////////////////////////////////////////////////////////////////
