// GG_Framework.UI.OSG LoadStatus.h
#pragma once

#include <vector>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{

			/// A singleton of this class used to report when different loading states are complete.
			/// See \ref Load_Status_txt.
			class FRAMEWORK_UI_OSG_API LoadStatus
			{
			private:
				std::vector<float> m_predictedLoadTimes;
				std::vector<float> m_actualLoadTimes;
				osg::Timer_t m_initialTimer;
				osg::Timer m_OSG_timer;
				OpenThreads::Mutex m_mutex;
				bool m_complete;

			public:
				static LoadStatus Instance;
				LoadStatus() : m_complete(false) {m_initialTimer = m_OSG_timer.tick();}

				/// This function is called several times during the load process
				void TaskComplete();

				/// This function is called only one time, when the load is REALLY complete
				/// Writes the completion times to a file
				void LoadComplete();

				/// Called once from the application, sets the predicted times until the load is complete
				/// Also resets the timer
				void SetPredictedLoadTimes(std::vector<float>& predLoad);

				/// This event is called with elapsed time and remaining time.  Remaining time is -1 if no predicted
				/// times are available.  It is provided with 0.0 when the LoadComplete happens.
				/// It will NOT be fired after that last time when LoadComplete is called.
				/// This event is protected with a mutex so that it is thread safe, but the handlers should be
				/// also because the event may be fired from various threads
				Event2<float, float> LoadStatusUpdate;
			};
		}
	}
}