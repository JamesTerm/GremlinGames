// Fringe.Base Mouse_ShipDriver.h
#pragma once

namespace Fringe
{
	namespace Base
	{
		class FRINGE_BASE_API Mouse_ShipDriver
		{
		public:
			Mouse_ShipDriver(ThrustShip2& ship,UI_Controller *parent, unsigned avgFrames);
			void OnMouseMove(float mx, float my);
			void OnMouseBtnPress(float mx, float my, unsigned int button );
			void OnMouseBtnRelease(float mx, float my, unsigned int button );

			void DriveShip();

		private:
			// Use this handler to tie events to this manipulator
			IEvent::HandlerList ehl;

			ThrustShip2& m_ship;
			UI_Controller * const m_ParentUI_Controller;
			osg::Vec2f m_lastMousePos;
			osg::Vec2f* m_mousePosHist;
			unsigned m_avgFrames;
			unsigned m_currFrame;
			unsigned int m_buttonState;
		};
	}
}