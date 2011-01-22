// GG_Framework.UI.OSG Timer.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

OSG_Timer::OSG_Timer(std::string logFileName) : GG_Framework::Base::Timer(logFileName)
{
	m_lastTimerTick = m_OSG_timer.tick();
}

OSG_Timer::~OSG_Timer()
{
}

double OSG_Timer::FireTimer()
{
	osg::Timer_t thisTimerTick = m_OSG_timer.tick();
	double dTick_s = m_OSG_timer.delta_s(m_lastTimerTick,thisTimerTick);
	m_lastTimerTick = thisTimerTick;
	IncTime_s(dTick_s);
	return dTick_s;
}