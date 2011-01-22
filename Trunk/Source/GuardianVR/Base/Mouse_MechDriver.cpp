// GuardianVR.Base Mouse_MechDriver.cpp
#include "stdafx.h"
#include "GuardianVR.Base.h"

using namespace GuardianVR::Base;

Mouse_MechDriver::Mouse_MechDriver(Mech& mech,Mech_Controller *parent, unsigned avgFrames) : 
	m_mech(mech),m_Parent(parent), m_mousePosHist(NULL), m_avgFrames(avgFrames), m_currFrame(0), m_buttonState(0)
{
	// Set up the Mouse Control to drive
	GG_Framework::Logic::Entity3D::EventMap* em = m_mech.GetEventMap();
	em->KBM_Events.MouseMove.Subscribe(ehl, *this, &Mouse_MechDriver::OnMouseMove);
	em->KBM_Events.MouseBtnPress.Subscribe(ehl, *this, &Mouse_MechDriver::OnMouseBtnPress);
	em->KBM_Events.MouseBtnRelease.Subscribe(ehl, *this, &Mouse_MechDriver::OnMouseBtnRelease);

	if (m_avgFrames)
	{
		m_mousePosHist = new osg::Vec2f[m_avgFrames];
	}
}
//////////////////////////////////////////////////////////////////////////

void Mouse_MechDriver::OnMouseBtnPress(float mx, float my, unsigned int button)
{
	// Start the gun firing, this will eventually be mapped in a users pref, but it is ok for now 
	if (button == 1)
		m_mech.GetEventMap()->EventOnOff_Map["Mech.TryFireMainWeapon"].Fire(true);
	m_buttonState |= button;
	OnMouseMove(mx, my);
}
////////////////////////////////////////////////////////////////////////////////////////////

void Mouse_MechDriver::OnMouseBtnRelease(float mx, float my, unsigned int button )
{
	// Stop the gun firing on the left MMB, this will eventually be mapped in a users pref, but it is ok for now
	if (button == 1)
		m_mech.GetEventMap()->EventOnOff_Map["Mech.TryFireMainWeapon"].Fire(false);
	m_buttonState &= ~button;
	OnMouseMove(mx, my);
}
////////////////////////////////////////////////////////////////////////////////////////////

void Mouse_MechDriver::OnMouseMove(float mx, float my)
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

//! \todo  Perhaps this should be a utility function somewhere?  Along with the stuff that keeps the flags
bool IsButtonOn(unsigned btnFlags, unsigned btn)
{
	return ((btnFlags & (1<<(btn-1))) != 0);
}
//////////////////////////////////////////////////////////////////////////

void Mouse_MechDriver::DriveMech()
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
	static const float x_coeff = -0.1f;
	static const float y_coeff = -0.1f;

	// Finally Turn the Heading or Pitch (or roll if using the rt mouse button
	{
		m_Parent->Mouse_Turn(dX*x_coeff);
		m_Parent->Mouse_Pitch(dY*y_coeff);
	}
}                                                      
//////////////////////////////////////////////////////////////////////////



