// GuardianVR.Base Mouse_MechDriver.h
#pragma once

namespace GuardianVR
{
	namespace Base
	{
		class GUARDIANVR_BASE_API Mouse_MechDriver
		{
		public:
			Mouse_MechDriver(Mech& mech,Mech_Controller *parent, unsigned avgFrames);
			void OnMouseMove(float mx, float my);
			void OnMouseBtnPress(float mx, float my, unsigned int button );
			void OnMouseBtnRelease(float mx, float my, unsigned int button );

			void DriveMech();

		private:
			// Use this handler to tie events to this manipulator
			IEvent::HandlerList ehl;

			Mech& m_mech;
			Mech_Controller * const m_Parent;
			osg::Vec2f m_lastMousePos;
			osg::Vec2f* m_mousePosHist;
			unsigned m_avgFrames;
			unsigned m_currFrame;
			unsigned int m_buttonState;
		};
	}
}