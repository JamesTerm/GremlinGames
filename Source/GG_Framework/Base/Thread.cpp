#include "stdafx.h"
#include "GG_Framework.Base.h"

bool GG_Framework::Base::ThreadedClass::ERROR_STATE = false;

void GG_Framework::Base::ThreadSleep(unsigned sleepMS)
{
	Sleep(sleepMS);
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::Base::ThrottleFrame::SetThrottleFPS(double FPS)
{
	m_throttleFPS=FPS;
}
void GG_Framework::Base::ThrottleFrame::SetMinSleep(int MinSleep) 
{
	m_MinSleep=MinSleep;
}

void GG_Framework::Base::ThrottleFrame::Init(double FPS,int MinSleep)
{
	SetThrottleFPS(FPS);
	SetMinSleep(MinSleep);
	m_ThrottleTimer.setStartTick();
}
//////////////////////////////////////////////////////////////////////////

double GG_Framework::Base::ThrottleFrame::operator()()
{
	// How much time has actually elapsed since the last update?
	double TimeElapsed_s=m_ThrottleTimer.time_s();

	// Nothing to do if there is no throttle anyway
	if (m_throttleFPS > 0.0)
	{
		// How much time in total do we need to wait?
		double waitTime_s= (1.0/m_throttleFPS);
		int SleepTime_ms=(waitTime_s>TimeElapsed_s) ? max((int)((waitTime_s-TimeElapsed_s)*1000.0),m_MinSleep) : m_MinSleep;

		//Note: NEVER use sleep 0!  A sleep 0 is only used for critical synchronization as it does a busy wait... That means it eats cycles instead of
		//giving them back... it is better to not sleep at all instead of using 0.
		//  [3/7/2010 JamesK]
		if (SleepTime_ms)
			GG_Framework::Base::ThreadSleep(SleepTime_ms);
	}

	// This is effectively the START of the NEXT frame.  The sleep is at the END of this frame, AFTER we get the effective time elapsed (not including the sleep)
	m_ThrottleTimer.setStartTick();
	return TimeElapsed_s; 
}
//////////////////////////////////////////////////////////////////////////

void GG_Framework::Base::ThreadedClass::run()
{
	try
	{
		tryRun();
	}

	catch (std::exception & exc)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << exc.what() << std::endl;
		ERROR_STATE = true;
	}
	catch (const char* msg)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
		ERROR_STATE = true;
	}
	catch (const std::string& msg)
	{
		std::cout << "*** UNRECOVERABLE ERROR: " << msg << std::endl;
		ERROR_STATE = true;
	}
	catch (...)
	{
		std::cout << "*** UNRECOVERABLE ERROR: Unknown Error Type" << std::endl;
		ERROR_STATE = true;
	}
}
//////////////////////////////////////////////////////////////////////////