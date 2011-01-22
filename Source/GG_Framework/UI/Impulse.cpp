// GG_Framework.UI Impulse.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

namespace GG_Framework
{
	namespace UI
	{

Impulse::Impulse(Actor& parent) : IEffect(parent.GetActorScene()),
	m_parent(parent), m_finalAction(NULL), m_active(false)
{
}
//////////////////////////////////////////////////////////////////////////

void Impulse::ReadParameters(GG_Framework::UI::EventMap& localMap, const char*& remLineFromFile)
{
	// Remember the line for debug output
	m_debugDesc = remLineFromFile;

	m_parent.DebugStringLine(GG_Framework::Base::BuildString("==%s(ReadParameters)==", m_debugDesc.c_str()));

	// Always call the base class first
	IEffect::ReadParameters(localMap, remLineFromFile);

	char s[8][32];
	int numActionsRead = sscanf(remLineFromFile, "%31s %31s %31s %31s %31s %31s", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]);
	for (int actNum = 0; actNum < numActionsRead; ++actNum)
	{
		Action* action = m_parent.GetAction(s[actNum]);
		ASSERT_MSG(action, GG_Framework::Base::BuildString("Could not create Action called %s.\n", s[actNum]).c_str());
		if ((numActionsRead > 1) &&
			(actNum == numActionsRead-1) && GetLooping())
		{
			m_finalAction = action;
		}
		else
		{
			m_actionList.push_back(action);
		}
	}
}
//////////////////////////////////////////////////////////////////////////

void Impulse::LaunchEffect(double launchTime_s, bool start, double intensity, const osg::Vec3d& pt )
{
	m_parent.DebugStringLine(GG_Framework::Base::BuildString("==%s(%s)==", m_debugDesc.c_str(), (start?"START":"STOP")));
	if (start)
	{
		m_parent.ClearActionQue();
		unsigned numActions = m_actionList.size();
		for (unsigned i = 0; i < numActions; ++i)
		{
			m_parent.QueAction(launchTime_s, *this, *m_actionList[i], GetLooping()&&(i==numActions-1));
		}
	}
	else if (m_finalAction && m_active)
	{
		m_parent.ClearActionQue();
		m_parent.QueAction(launchTime_s, *this, *m_finalAction, false);
		m_active = false;
	}
	m_parent.DebugOutputActionQue();
}
//////////////////////////////////////////////////////////////////////////
	}
}