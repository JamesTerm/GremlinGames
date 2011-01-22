// GG_Framework.Logic SynchronizedTimer.h
#pragma once
#include <NetworkTypes.h>

namespace GG_Framework
{
	namespace Logic
	{
		namespace Network
		{
			//! This is a special Timer object that uses RakNet to synchronize its 
			//! time with another PEER using an EPOCH for the current time.
			class FRAMEWORK_LOGIC_NETWORK_API SynchronizedTimer : public GG_Framework::Base::Timer
			{
			public:
				SynchronizedTimer(std::string logFileName) : GG_Framework::Base::Timer(logFileName), m_epoch(0) {}
				virtual ~SynchronizedTimer() {}

				//! Returns the time increment since the last time Fired, or since epoch was set
				virtual double FireTimer();

				virtual double ConvertFromNetTime(unsigned int netTime_ms);
				virtual unsigned int ConvertToNetTime(double time_s);

				//! Set the Epoch the first time.  It can be called more than once.  Will NOT
				//! Fire the time changed event, but it WILL set the last time and the current time.
				void SetEpoch(RakNetTime epoch);
				RakNetTime GetEpoch(){ASSERT(m_epoch!=0); return m_epoch;}

				virtual double GetSynchronizedActualTime();

			private:
				RakNetTime m_epoch;
			};
		}
	}
}