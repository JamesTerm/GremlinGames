// GG_Framework.UI Text_PDCB.h
#pragma once

#include <osg\Camera>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API Text_PDCB : public osg::Camera::DrawCallback
		{
		public:
			Text_PDCB() : m_enabled(false) {}
			void ToggleEnabled(){m_enabled = !m_enabled;}
			virtual bool IsEnabled() const {return m_enabled;}
			void Enable(bool enable){m_enabled = enable;}

			virtual void operator () (const osg::Camera &cam ) const;
			
			//! The text that will be displayed when enabled
			virtual std::string GetText() const = 0;

			//! The position of the lower left of the string, winHeight is top of the window, winWidth is right edge
			//! It appears that each character is about 10 units.
			virtual osg::Vec2 GetPosition(double winWidth, double winHeight) const = 0;

			//! The default text color is yellow
			virtual osg::Vec4 GetTextColor() const {return osg::Vec4(1,1,0,1);}

		protected:
			bool m_enabled;
			mutable GG_Framework::UI::OSG::Text m_text;
		};
	}
}