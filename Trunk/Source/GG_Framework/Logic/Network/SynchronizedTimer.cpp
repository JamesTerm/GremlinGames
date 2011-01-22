// GG_Framework.Logic SynchronizedTimer.cpp
#include "stdafx.h"
#include "GG_Framework.Logic.Network.h"

using namespace GG_Framework::Logic::Network;

//! Returns the time increment since the last time Fired, or since epoch was set
double SynchronizedTimer::FireTimer()
{
	double newTime = GetSynchronizedActualTime();
	double ret = newTime - GetCurrTime_s();
	SetCurrTime_s(newTime);
	return ret;
}
//////////////////////////////////////////////////////////////////////////

//! Set the Epoch the first time.  It can be called more than once.  Will NOT
//! Fire the time changed event, but it WILL set the current time
void SynchronizedTimer::SetEpoch(RakNetTime epoch)
{
	if (m_epoch == 0)
	{
		m_epoch = epoch;
	}
	else if (epoch != m_epoch)
	{
		double dTime_s = (double)(m_epoch-epoch) / 1000.0;
		IncTime_s(dTime_s);
		m_epoch = epoch;
	}
}
//////////////////////////////////////////////////////////////////////////

double SynchronizedTimer::ConvertFromNetTime(unsigned int netTime_ms)
{
	ASSERT(m_epoch != 0);
	return (double)(netTime_ms - m_epoch) / 1000.0;
}
//////////////////////////////////////////////////////////////////////////

unsigned int SynchronizedTimer::ConvertToNetTime(double time_s)
{
	ASSERT(m_epoch != 0);
	return (unsigned int)(time_s * 1000.0) + m_epoch;
}
//////////////////////////////////////////////////////////////////////////

double SynchronizedTimer::GetSynchronizedActualTime()
{
	return ConvertFromNetTime(RakNet::GetTime());
}
//////////////////////////////////////////////////////////////////////////