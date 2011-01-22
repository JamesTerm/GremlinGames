// GG_Framework.UI Framerate_PDCB.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API Framerate_PDCB : public Text_PDCB
		{
		public:
			Framerate_PDCB(Window& window) : Text_PDCB(), m_window(window) {}

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

			virtual std::string GetText() const {return GG_Framework::Base::BuildString("%3.1f (%3.1f) fps [%i]", m_window.GetFrameRate(), m_window.GetAverageFramerate(), m_window.GetPerformanceIndex());}
			virtual osg::Vec2 GetPosition(double winWidth, double winHeight) const {return osg::Vec2(winWidth - 230.0, winHeight - 20.0);}

		private:
			bool m_enabled;
			Window& m_window;
			GG_Framework::UI::OSG::Text m_text;
		};
	}
}