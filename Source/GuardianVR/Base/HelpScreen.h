// GuardianVR.Base HelpScreen.h
#pragma once

#ifndef __Use_OSG_Svn__
#include <Producer\Camera>
#else
#include <osg\Camera>
#endif

namespace GuardianVR
{
	namespace Base
	{
		//! The HelpScreen will only be used for the VIP investors demo short-term.  Ultimately, 
		//! We will use nicer tools to display the key-bindings, but for now, the VIP demo will
		//! show this hard-coded help screen to know what buttons to push.
		class GUARDIANVR_BASE_API HelpScreen
			: public osg::Camera::DrawCallback
		{
		public:
			// Uses the head position relative to the center of the ship to calculate G forces
			HelpScreen();

			void ToggleEnabled(){m_enabled = !m_enabled;}
			bool IsEnabled() const {return m_enabled;}
			void Enable(bool enable){m_enabled = enable;}

			virtual void operator () (const osg::Camera &cam ) const;

			// Use this handler to tie events to this callback to toggle
			IEvent::HandlerList ehl;

		private:
			bool m_enabled;
			mutable GG_Framework::UI::OSG::Text m_text;

			void OnMainWindowEscape(GG_Framework::UI::MainWindow* mainWindow, bool& okToQuit);
		};
	}
}
