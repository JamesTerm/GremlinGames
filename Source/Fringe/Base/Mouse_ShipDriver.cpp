// Fringe.Base Mouse_ShipDriver.cpp
#include "stdafx.h"
#include "Fringe.Base.h"

using namespace Fringe::Base;

Mouse_ShipDriver::Mouse_ShipDriver(ThrustShip2& ship,UI_Controller *parent, unsigned avgFrames) : 
	m_ship(ship),m_ParentUI_Controller(parent), m_mousePosHist(NULL), m_avgFrames(avgFrames), m_currFrame(0), m_mouseRoll(false)
{
	GG_Framework::UI::KeyboardMouse_CB &kbm = GG_Framework::UI::MainWindow::GetMainWindow()->GetKeyboard_Mouse();	
	kbm.AddKeyBindingR(true, "Ship.MouseRoll", osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);

	// Set up the Mouse Control to drive
	GG_Framework::Logic::Entity3D::EventMap* em = m_ship.GetEventMap();
	em->KBM_Events.MouseMove.Subscribe(ehl, *this, &Mouse_ShipDriver::OnMouseMove);
	em->EventOnOff_Map["Ship.MouseRoll"].Subscribe(ehl, *this, &Mouse_ShipDriver::OnMouseRoll);

	if (m_avgFrames)
	{
		m_mousePosHist = new osg::Vec2f[m_avgFrames];
	}
}
//////////////////////////////////////////////////////////////////////////

void Mouse_ShipDriver::OnMouseMove(float mx, float my)
{
	//Rick keep this here so I can debug code... thanks
	//Ideally when the window loses focus we should release control of the mouse
	// return;

	// Watch for the window being too small.  We will also need the width and height
	int x,y;
	unsigned w,h;
	GG_Framework::UI::MainWindow::GetMainWindow()->GetWindowRectangle(x,y,w,h);
	if ((w < 2) || (h < 2))
		return;

	// We want to use the physical distance in pixels to normalize the distance the mouse moves
	// and to avoid the re-centering effect that gives a small amount of pixel deviation
	float pixelX, pixelY;
	if (!GG_Framework::UI::MainWindow::GetMainWindow()->ComputePixelCoords(mx, my, pixelX, pixelY))
		return;

	// We are assuming that the mouse was set to its 0,0 position (center screen) Before every call
	float dX = pixelX - (float)x - ((float)w/2.0f);
	float dY = pixelY - (float)y - ((float)h/2.0f);

	// Remember this for the next time, if larger than what we have
	if ((dX > 1.5f) || (dX < -1.5f))  // Watch for very small amounts of mouse drift
	{
		if (fabs(dX) > fabs(m_lastMousePos[0]))
			m_lastMousePos[0] = dX;
	}
	if ((dY > 1.5f) || (dY < -1.5f))  // Watch for very small amounts of mouse drift
	{
		if (fabs(dY) > fabs(m_lastMousePos[1]))
			m_lastMousePos[1] = dY;
	}

	// Reset back to 0,0 for next time
	GG_Framework::UI::MainWindow::GetMainWindow()->PositionPointer(0.0f,0.0f);
}
//////////////////////////////////////////////////////////////////////////

void Mouse_ShipDriver::DriveShip()
{
	float dX = 0.0f;
	float dY = 0.0f;
	if (m_mousePosHist)
	{
		// Store from this frame
		if (m_currFrame >= m_avgFrames)
			m_currFrame = 0;
		m_mousePosHist[m_currFrame] = m_lastMousePos;
		++m_currFrame;

		// Get the average
		for (unsigned i=0; i < m_avgFrames; ++i)
		{
			dX += m_mousePosHist[i][0];
			dY += m_mousePosHist[i][1];
		}
		dX /= (float)m_avgFrames;
		dY /= (float)m_avgFrames;
	}
	else
	{
		// We are not using averages, just use this value
		dX = m_lastMousePos[0];
		dY = m_lastMousePos[1];
	}
	// Reset for next time
	m_lastMousePos = osg::Vec2f(0.0f, 0.0f);

	// Will be used eventually for sensitivity and mouse flip, store in script, etc.
	static const float x_coeff = -0.004f;
	static const float y_coeff = -0.004f;
	static const float roll_coeff = 0.01f;

	// Finally Turn the Heading or Pitch (or roll if using the rt mouse button
	if (m_mouseRoll)
	{
		// Use this roll versus the scroll wheel
		float dR = dY - dX;

		//! \todo JAMES: here is the funky roll
		if (dR != 0.0f)
			m_ParentUI_Controller->Mouse_Roll(dR*roll_coeff);
	}
	else
	{
		if (dX != 0.0f)
			m_ParentUI_Controller->Mouse_Turn(dX*x_coeff);
		if (dY != 0.0f)
			m_ParentUI_Controller->Mouse_Pitch(dY*y_coeff);
	}
}                                                      
//////////////////////////////////////////////////////////////////////////



