// GG_Framework.UI PerformanceSwitch.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

		/// An Action is a mapping of time from beginning to end from one pose to another
		/// Once an action starts, it will play through to the end
		class FRAMEWORK_UI_API PerformanceSwitch : public osg::Group
		{
		public:
			PerformanceSwitch(int minPerf, int maxPerf);
			void PerformanceIndexChanged(int oldVal, int newVal);
		
		private:
			IEvent::HandlerList ehl;
			int m_minPerf;
			int m_maxPerf;
			bool m_showing;
		};
	}
}