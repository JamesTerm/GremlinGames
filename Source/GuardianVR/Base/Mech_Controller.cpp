#include "stdafx.h"
#include "GuardianVR.Base.h"

using namespace GuardianVR::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

//////////////////////////////////////////////////////////////////////////

  /***************************************************************************************************************/
 /*											Mech_Controller												*/
/***************************************************************************************************************/

//! TODO: Use the script to grab the head position to provide the HUD
Mech_Controller::Mech_Controller(Mech& mech, bool canUserPilot) : 
Mech_ControllerBase(mech), m_chasePlaneCamManip(m_mech), 
m_mouseDriver(m_mech,this, 3), m_POV_rad_s(osg::Vec3d(0.0,0.0,0.0)),m_POV_keyboard_rad_s(osg::Vec3d(0.0,0.0,0.0)),
m_isControlled(false),m_UseMousePOV(true),m_autoPilot(true),m_Mech_UseHeadingSpeed(true),m_Mech_UsePitchSpeed(true),
m_canUserPilot(canUserPilot)  //!< WE are NOt controlled by default, so 
{
	// Hard code these key bindings at first
	GG_Framework::UI::KeyboardMouse_CB& kbm = GG_Framework::UI::MainWindow::GetMainWindow()->Keyboard_Mouse;
	
	/* We can now use double-tap to fire the afterburners (for when we have them)
	kbm.AddKeyBindingR(true, "WalkForward", GG_Framework::Base::KeyWithFlags('w'));
	kbm.AddKeyBindingR(true, "WalkBack", 's');
	kbm.AddKeyBindingR(true, "StrafeRight", 'd');
	kbm.AddKeyBindingR(true, "StrafeLeft", 'a');

	//Multiple key assignments now work Yay!
	kbm.AddKeyBindingR(true, "WalkForward", osgGA::GUIEventAdapter::KEY_Up );
	kbm.AddKeyBindingR(true, "WalkBack", osgGA::GUIEventAdapter::KEY_Down);
	kbm.AddKeyBindingR(true, "StrafeLeft", osgGA::GUIEventAdapter::KEY_Left);
	kbm.AddKeyBindingR(true, "StrafeRight", osgGA::GUIEventAdapter::KEY_Right);
	*/

	kbm.AddKeyBindingR(false, "UserResetPos", '.');
	kbm.AddKeyBindingR(false, "ToggleAutoPilot", 'z');
	kbm.AddKeyBindingR(true, "POVMouse", 'p');
	kbm.AddKeyBindingR(true, "POV_DL", '1');
	kbm.AddKeyBindingR(true, "POV_Down", '2');
	kbm.AddKeyBindingR(true, "POV_DR", '3');
	kbm.AddKeyBindingR(true, "POV_Left", '4');
	kbm.AddKeyBindingR(true, "POV_Right", '6');
	kbm.AddKeyBindingR(true, "POV_UL", '7');
	kbm.AddKeyBindingR(true, "POV_Up", '8');
	kbm.AddKeyBindingR(true, "POV_UR", '9');


	// Then subscribe to these events
	GG_Framework::Logic::Entity3D::EventMap* em = m_mech.GetEventMap();
	/*
	em->EventOnOff_Map["WalkForward"].Subscribe(ehl, *this, &Mech_Controller::WalkForward);
	em->EventOnOff_Map["WalkBack"].Subscribe(ehl, *this, &Mech_Controller::WalkBack);
	em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &Mech_Controller::Turn_R);
	em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &Mech_Controller::Turn_L);
	em->EventOnOff_Map["StrafeLeft"].Subscribe(ehl, *this, &Mech_Controller::StrafeLeft);
	em->EventOnOff_Map["StrafeRight"].Subscribe(ehl, *this, &Mech_Controller::StrafeRight);
	*/
	
	em->Event_Map["UserResetPos"].Subscribe(ehl, *this, &Mech_Controller::UserResetPos);
	em->Event_Map["ResetPos"].Subscribe(ehl, *this, &Mech_Controller::ResetPos);
	em->Event_Map["ToggleAutoPilot"].Subscribe(ehl, *this, &Mech_Controller::TryToggleAutoPilot);
	em->EventOnOff_Map["POVMouse"].Subscribe(ehl, *this, &Mech_Controller::ToggleMousePOV);
	em->EventOnOff_Map["POV_Down"].Subscribe(ehl, *this, &Mech_Controller::POV_Down);
	em->EventOnOff_Map["POV_DL"].Subscribe(ehl, *this, &Mech_Controller::POV_DL);
	em->EventOnOff_Map["POV_DR"].Subscribe(ehl, *this, &Mech_Controller::POV_DR);
	em->EventOnOff_Map["POV_Up"].Subscribe(ehl, *this, &Mech_Controller::POV_Up);
	em->EventOnOff_Map["POV_UL"].Subscribe(ehl, *this, &Mech_Controller::POV_UL);
	em->EventOnOff_Map["POV_UR"].Subscribe(ehl, *this, &Mech_Controller::POV_UR);
	em->EventOnOff_Map["POV_Left"].Subscribe(ehl, *this, &Mech_Controller::POV_Left);
	em->EventOnOff_Map["POV_Right"].Subscribe(ehl, *this, &Mech_Controller::POV_Right);
	em->EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &Mech_Controller::OnSpawn);

	// By default, we are in autopilot
	SetAutoPilot(true);
}

void Mech_Controller::Mech_Turn(double dir,bool UseHeadingSpeed) 
{
	m_Mech_UseHeadingSpeed=UseHeadingSpeed;
	m_Mech_JoyMouse_rotVel_rad_s[0]=UseHeadingSpeed?dir*m_mech.dHeading:dir;
}
void Mech_Controller::Mech_Pitch(double dir,bool UsePitchSpeed) 
{
	m_Mech_UsePitchSpeed=UsePitchSpeed;
	m_Mech_JoyMouse_rotVel_rad_s[1]=UsePitchSpeed?dir*m_mech.dPitch:dir;
}

void Mech_Controller::Mech_Turn(Directions dir)
{
	m_Mech_Keyboard_rotVel_rad_s[0]=(double)dir*m_mech.dHeading;
	m_Mech_UseHeadingSpeed=true;
}

void Mech_Controller::CancelAllControls()
{
	// Turns off all controls that might be on
	/*
	WalkForward(false);
	WalkBack(false);
	Turn_L(false);
	Turn_R(false);
	StrafeLeft(false);
	StrafeRight(false);
	*/
	ToggleMousePOV(false);
	POV_Down(false);
	POV_DL(false);
	POV_DR(false);
	POV_Up(false);
	POV_UL(false);
	POV_UR(false);
	POV_Left(false);
	POV_Right(false);
}

void Mech_Controller::TryToggleAutoPilot()
{
	SetAutoPilot(!GetAutoPilot());
}

bool Mech_Controller::SetAutoPilot(bool autoPilot)
{
	// Note that Autopilot MUST be on if the mech is not being controlled
	m_autoPilot = autoPilot || !m_isControlled || !GetCanUserPilot();

	// When in autopilot, always use mouse POV
	ToggleMousePOV(m_autoPilot);

	// When turning on or OFF the auto pilot, stop firing and other actions
	m_mech.CancelAllControls();

	return m_autoPilot;
}

void Mech_Controller::ToggleMousePOV(bool on) 
{
	// Note that MousePOV MUST stay on when in auto pilot mode
	m_UseMousePOV= on || m_autoPilot;

	// disable snapback while in POV mouse mode (Only if explicitly called)
	m_chasePlaneCamManip.SetPerformPOVSnapBack(!m_UseMousePOV);
} 

void Mech_Controller::UserResetPos()
{
	m_mech.ResetPos();
}

void Mech_Controller::ResetPos()
{
	if (AreControlsDisabled())
		return;
	m_Mech_Keyboard_rotVel_rad_s =	m_Mech_JoyMouse_rotVel_rad_s = m_Mech_Keyboard_currAccel = m_Mech_JoyMouse_currAccel =	osg::Vec3d(0,0,0);

	m_chasePlaneCamManip.Reset();
	__super::ResetPos();
}

void Mech_Controller::POV_Turn(double dir,bool UseKeyboard)
{
	if (!UseKeyboard)
		m_POV_rad_s[0]=dir;
	else
		m_POV_keyboard_rad_s[0]=dir*M_PI;
}
void Mech_Controller::POV_Pitch(double dir,bool UseKeyboard)
{
	if (!UseKeyboard)
		m_POV_rad_s[1]=dir;
	else
		m_POV_keyboard_rad_s[1]=dir*M_PI;
}

void Mech_Controller::Mouse_Turn(double dir)
{
	if (!m_UseMousePOV)
		Mech_Turn(dir,false);
	else
		m_chasePlaneCamManip.POV_Turn(dir,true);
}
void Mech_Controller::Mouse_Pitch(double dir)
{
	if (!m_UseMousePOV)
		Mech_Pitch(dir,false);
	else
		m_chasePlaneCamManip.POV_Pitch(dir,true);
}

#define __SwapUpDownAndWalkForwardBreak__

bool Mech_Controller::AreControlsDisabled()
{
	return (m_autoPilot || !m_mech.IsShowing());
}

void Mech_Controller::UpdateController(double dTime_s)
{
	if (AreControlsDisabled())
		__super::UpdateController(dTime_s);

	if (m_isControlled)
	{
		// Update Mouse Controller (This is ONLY allowed to update the POV in auto pilot)
		m_mouseDriver.DriveMech();
		
		// Update POV
		m_chasePlaneCamManip.POV_Turn(m_POV_rad_s[0]+m_POV_keyboard_rad_s[0]);
		m_chasePlaneCamManip.POV_Pitch(m_POV_rad_s[1]+m_POV_keyboard_rad_s[1]);
		
		//reset now that I have used them
		m_POV_rad_s[0]=m_POV_rad_s[1]=0.0;

		//Now for the mech
		if (!AreControlsDisabled())
		{
			// apply various input sources to current acceleration
			// m_mech.SetCurrentLinearAcceleration(m_Mech_Keyboard_currAccel+m_Mech_JoyMouse_currAccel); 		

			// Set the angular velocity for turning
			m_mech.SetCurrentAngularVelocity(m_Mech_Keyboard_rotVel_rad_s+m_Mech_JoyMouse_rotVel_rad_s);
		}
	}
}

void Mech_Controller::UpdateUI(double dTime_s)
{
	// TODO: Update the HUD
}

//////////////////////////////////////////////////////////////////////////

bool Mech_Controller::GetCanUserPilot()
{
	return (m_canUserPilot && !m_mech.IsBeingDestroyed());
}
//////////////////////////////////////////////////////////////////////////

void Mech_Controller::OnSpawn(bool on)
{
	// Set Auto Pilot to true if there is control
	SetAutoPilot(!m_isControlled || !GetCanUserPilot());

	// Reset the POV
	m_chasePlaneCamManip.Reset();
}
//////////////////////////////////////////////////////////////////////////
/*
void Mech_Controller::ReadScriptParameters(Mech_TransmittedEntity& te)
{
	__super::ReadScriptParameters(te);
	// TODO:  Read in the Script parameters
}
//////////////////////////////////////////////////////////////////////////
*/

void Mech_Controller::HookUpUI(bool ui)
{
	m_isControlled = ui;
	GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
	if (m_isControlled)
	{
		// Start with the mouse centered in the screen and turn off the cursor
		mainWin.PositionPointer(0.0f,0.0f);
		mainWin.UseCursor(false);

		// Provide the new camera manipulator
		mainWin.GetMainCamera()->SetCameraManipulator(&m_chasePlaneCamManip);
		mainWin.Keyboard_Mouse.SetControlledEventMap(m_mech.GetEventMap());
	}

	// Set Auto Pilot to true if there is control
	SetAutoPilot(!m_isControlled || !GetCanUserPilot());
}
//////////////////////////////////////////////////////////////////////////
