// GG_Framework.UI DebugOut_PDCB.h
#pragma once

// These simplify the use of the static DebugOut_PDCB text values
#define DOUT1(x,...) GG_Framework::UI::DebugOut_PDCB::TEXT1 = BuildString(x,__VA_ARGS__)
#define DOUT2(x,...) GG_Framework::UI::DebugOut_PDCB::TEXT2 = BuildString(x,__VA_ARGS__)
#define DOUT3(x,...) GG_Framework::UI::DebugOut_PDCB::TEXT3 = BuildString(x,__VA_ARGS__)
#define DOUT4(x,...) GG_Framework::UI::DebugOut_PDCB::TEXT4 = BuildString(x,__VA_ARGS__)
#define DOUT5(x,...) GG_Framework::UI::DebugOut_PDCB::TEXT5 = BuildString(x,__VA_ARGS__)

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API DebugOut_PDCB : public Text_PDCB
		{
		public:
			DebugOut_PDCB(Window& window) : Text_PDCB() {}

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

			virtual std::string GetText() const;
			virtual osg::Vec2 GetPosition(double winWidth, double winHeight) const {return osg::Vec2(10.0, winHeight - 20.0);}
			virtual void operator () (const osg::Camera &cam ) const;

			// To populate this with a formatted string, Use
			// GG_Framework::UI::DebugOut_PDCB::TEXT1 = GG_Framework::Base::BuildString(format, ...);
			static std::string TEXT1;
			static std::string TEXT2;
			static std::string TEXT3;
			static std::string TEXT4;
			static std::string TEXT5;
		};
	}
}