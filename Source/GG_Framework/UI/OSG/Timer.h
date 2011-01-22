// Timer.h
#pragma once
#include <osg/Timer>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
			
			class FRAMEWORK_UI_OSG_API OSG_Timer : public GG_Framework::Base::Timer
			{
			public:
				OSG_Timer(std::string logFileName);
				virtual ~OSG_Timer();
				virtual double FireTimer();

				virtual double ConvertFromNetTime(unsigned int netTime_ms) {return (double)netTime_ms / 1000.0;}
				virtual unsigned int ConvertToNetTime(double time_s) {return (unsigned int)(time_s * 1000.0);}

			private:
				osg::Timer m_OSG_timer;
				osg::Timer_t m_lastTimerTick;
			};
		}
	}
}