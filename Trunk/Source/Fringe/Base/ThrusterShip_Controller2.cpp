#include "stdafx.h"
#include "Fringe.Base.h"

#undef __WhatJamesNeeds__
#undef __EnableTestKeys__

using namespace Fringe::Base;
using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;

  /***************************************************************************************************************/
 /*												UI_Controller													*/
/***************************************************************************************************************/

void UI_Controller::Init_AutoPilotControls()
{
	// Listen to keyboard events for switching targets
	GG_Framework::UI::KeyboardMouse_CB& kbm = GG_Framework::UI::MainWindow::GetMainWindow()->GetKeyboard_Mouse();

	kbm.AddKeyBindingR(false, "TARGET_PREV", '-');
	kbm.AddKeyBindingR(false, "TARGET_NEXT", '=');
	kbm.AddKeyBindingR(false, "TARGET_TGT_YOU", osgGA::GUIEventAdapter::KEY_BackSpace);
	kbm.AddKeyBindingR(false, "MATCH_TARGET_SPEED", 'q');

	// Bindings for each of the Target Banks
	kbm.AddKeyBindingR(false, "TARGET_INSIGHT", '0');
	kbm.AddKeyBindingR(false, "TARGET_1", '1');
	kbm.AddKeyBindingR(false, "TARGET_2", '2');
	kbm.AddKeyBindingR(false, "TARGET_3", '3');
	kbm.AddKeyBindingR(false, "TARGET_4", '4');
	kbm.AddKeyBindingR(false, "TARGET_5", '5');
	kbm.AddKeyBindingR(false, "TARGET_6", '6');
	kbm.AddKeyBindingR(false, "TARGET_7", '7');
	kbm.AddKeyBindingR(false, "TARGET_8", '8');
	kbm.AddKeyBindingR(false, "TARGET_9", '9');

	kbm.AddKeyBindingR(false, "SET_TARGET_1", GG_Framework::Base::Key('1', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_2", GG_Framework::Base::Key('2', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_3", GG_Framework::Base::Key('3', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_4", GG_Framework::Base::Key('4', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_5", GG_Framework::Base::Key('5', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_6", GG_Framework::Base::Key('6', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_7", GG_Framework::Base::Key('7', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_8", GG_Framework::Base::Key('8', osgGA::GUIEventAdapter::MODKEY_ALT));
	kbm.AddKeyBindingR(false, "SET_TARGET_9", GG_Framework::Base::Key('9', osgGA::GUIEventAdapter::MODKEY_ALT));
}


//! TODO: Use the script to grab the head position to provide the HUD
UI_Controller::UI_Controller(AI_Base_Controller *base_controller) : 
m_HUD_UI(new HUD_PDCB(osg::Vec3(0.0, 4.0, 0.5))), m_Base(NULL), m_chasePlaneCamManip(NULL),
m_mouseDriver(NULL), m_SlideButtonToggle(false), m_POV_rad_s(osg::Vec3d(0.0,0.0,0.0)),m_POV_keyboard_rad_s(osg::Vec3d(0.0,0.0,0.0)),
m_isControlled(false),m_UseMousePOV(true),m_autoPilot(true),
m_enableAutoLevelWhenPiloting(false),m_CruiseSpeed(0.0),m_hud_connected(false),m_Test1(false),m_Test2(false),
m_targetLeadRetShowing(false), m_targetInRangeLeadRetShowing(false), m_Ship_UseHeadingSpeed(true)
{
	Set_AI_Base_Controller(base_controller); //set up ship (even if we don't have one)
	m_LastSliderTime[0]=m_LastSliderTime[1]=0.0;
	m_Ship_UseRollSpeed=true;  //Chances are this is not every going to be used by the mouse... so lets give this a good default

	// Hard code these key bindings at first
	KeyboardMouse_CB &kbm = MainWindow::GetMainWindow()->GetKeyboard_Mouse();	
	JoyStick_Binder &joy = MainWindow::GetMainWindow()->GetJoystick();
	joy.AddJoy_Button_Default(0,"Ship.TryFireMainWeapon");
	joy.AddJoy_Button_Default(1,"Missile.Launch");
	// We can now use double-tap to fire the afterburners (for when we have them)
	kbm.AddKeyBindingR(true, "RequestAfterburner", GG_Framework::Base::Key('w', GG_Framework::Base::Key::DBL));
	kbm.AddKeyBindingR(true, "Thrust", GG_Framework::Base::Key('w'));
	joy.AddJoy_Button_Default(2,"Thrust");
	kbm.AddKeyBindingR(true, "Brake", 's');
	joy.AddJoy_Button_Default(3,"Brake");
	kbm.AddKeyBindingR(false, "Stop", 'x');
	joy.AddJoy_Analog_Default(JoyStick_Binder::eSlider1,"Joystick_SetCurrentSpeed");
	
	//I would like to keep this macro case to easily populate my defaults
#ifdef __WhatJamesNeeds__
	kbm.AddKeyBindingR(true, "Turn_R", 'd');
	kbm.AddKeyBindingR(true, "Turn_L", 'a');
	kbm.AddKeyBindingR(true, "Roll_CW", 'e');
	kbm.AddKeyBindingR(true, "Roll_CCW", 'q');
	kbm.AddKeyBindingR(true, "Pitch_Up", 'f');
	kbm.AddKeyBindingR(true, "Pitch_Dn", 'r');

	//for testing
	kbm.AddKeyBindingR(true, "Test1", 'n');
	kbm.AddKeyBindingR(true, "Test2", 'm');
#else
	//These are not assigned by default but can configured to use via xml preferences
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",true,1.0,0.01,true);
	joy.AddJoy_Button_Default(7,"Roll_CW");
	joy.AddJoy_Button_Default(8,"Roll_CCW");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Analog_Pitch",true,1.0,0.01,true);

	kbm.AddKeyBindingR(true, "StrafeRight", 'd');
	kbm.AddKeyBindingR(true, "StrafeLeft", 'a');
#endif

	kbm.AddKeyBindingR(false, "UserResetPos", ' ');
	kbm.AddKeyBindingR(false, "Slide", 'g');
	joy.AddJoy_Button_Default(6,"Slide",false);
	kbm.AddKeyBindingR(false, "AutoLevel", 't');

	kbm.AddKeyBindingR(true, "StrafeUp", osgGA::GUIEventAdapter::KEY_Up);
	kbm.AddKeyBindingR(true, "StrafeDown", osgGA::GUIEventAdapter::KEY_Down);
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Axis,"Analog_StrafeDown");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eSlider0,"Analog_Slider_StrafeDown",true);
	kbm.AddKeyBindingR(true, "StrafeLeft", osgGA::GUIEventAdapter::KEY_Left);
	kbm.AddKeyBindingR(true, "StrafeRight", osgGA::GUIEventAdapter::KEY_Right);
	joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Rot,"Analog_StrafeRight");
	kbm.AddKeyBindingR(false, "ShowHUD", osgGA::GUIEventAdapter::KEY_F4);

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
	joy.AddJoy_Analog_Default(JoyStick_Binder::ePOV_0,"POV_Joystick");

	Init_AutoPilotControls();
}

void UI_Controller::Flush_AI_BaseResources()
{
	//disconnect hud
	ConnectHUD_Elements(false);
	m_tgtDirLineTransform=NULL;  //this will trigger the next build hud
	m_HUD_UI->Reset();
	if (m_mouseDriver)
	{
		delete m_mouseDriver;
		m_mouseDriver=NULL;
	}
	if (m_chasePlaneCamManip)
	{
		delete m_chasePlaneCamManip;
		m_chasePlaneCamManip=NULL;
	}
	m_ship=NULL; //we don't own this
}

UI_Controller::~UI_Controller()
{
	Flush_AI_BaseResources();
}

void UI_Controller::Set_AI_Base_Controller(AI_Base_Controller *controller)
{
	//destroy all resources associated with the previous ship
	if (m_Base)
	{
		GG_Framework::Logic::Entity3D::EventMap* em = m_ship->GetEventMap();
		em->EventOnOff_Map["RequestAfterburner"].Remove(*this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Remove(*this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Remove(*this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Remove(*this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Remove(*this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Remove(*this, &UI_Controller::Turn_L);
		em->EventOnOff_Map["Roll_CW"].Remove(*this, &UI_Controller::Roll_CW);
		em->EventOnOff_Map["Roll_CCW"].Remove(*this, &UI_Controller::Roll_CCW);
		em->EventOnOff_Map["Pitch_Up"].Remove(*this, &UI_Controller::Pitch_Up);
		em->EventOnOff_Map["Pitch_Dn"].Remove(*this, &UI_Controller::Pitch_Dn);
		em->Event_Map["UserResetPos"].Remove(*this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Remove(*this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Remove(*this, &UI_Controller::ToggleSlide);
		em->Event_Map["AutoLevel"].Remove(*this, &UI_Controller::ToggleAutoLevel);
		em->EventOnOff_Map["StrafeUp"].Remove(*this, &UI_Controller::StrafeUp);
		em->EventOnOff_Map["StrafeDown"].Remove(*this, &UI_Controller::StrafeDown);
		em->EventOnOff_Map["StrafeLeft"].Remove(*this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Remove(*this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["POVMouse"].Remove(*this, &UI_Controller::ToggleMousePOV);
		em->EventOnOff_Map["POV_Down"].Remove(*this, &UI_Controller::POV_Down);
		em->EventOnOff_Map["POV_DL"].Remove(*this, &UI_Controller::POV_DL);
		em->EventOnOff_Map["POV_DR"].Remove(*this, &UI_Controller::POV_DR);
		em->EventOnOff_Map["POV_Up"].Remove(*this, &UI_Controller::POV_Up);
		em->EventOnOff_Map["POV_UL"].Remove(*this, &UI_Controller::POV_UL);
		em->EventOnOff_Map["POV_UR"].Remove(*this, &UI_Controller::POV_UR);
		em->EventOnOff_Map["POV_Left"].Remove(*this, &UI_Controller::POV_Left);
		em->EventOnOff_Map["POV_Right"].Remove(*this, &UI_Controller::POV_Right);
		em->EventOnOff_Map["SPAWN"].Remove(*this, &UI_Controller::OnSpawn);
		em->EventOnOff_Map["Test1"].Remove(*this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Remove(*this, &UI_Controller::Test2);
		em->Event_Map["ShowHUD"].Remove(*m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);
		em->EventValue_Map["BLACKOUT"].Remove(*this, &UI_Controller::BlackoutHandler);
		em->EventValue_Map["Analog_Turn"].Remove(*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_Pitch"].Remove(*this, &UI_Controller::JoyStick_Ship_Pitch);
		em->EventValue_Map["Analog_StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_StrafeDown"].Remove(*this, &UI_Controller::StrafeDown);
		em->EventValue_Map["Analog_Slider_StrafeDown"].Remove(*this, &UI_Controller::Slider_StrafeDown);
		em->EventValue_Map["Analog_Slider_Accel"].Remove(*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["POV_Joystick"].Remove(*this, &UI_Controller::Joystick_POV);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed);

		Flush_AI_BaseResources();
	}
	m_Base=controller;
	if (m_Base)
	{
		m_ship=&m_Base->m_ship;
		m_chasePlaneCamManip=new GG_Framework::AppReuse::ChasePlane_CamManipulator(*m_ship);
		m_mouseDriver=new Mouse_ShipDriver(*m_ship,this, 3);
		//Try to set this ship to our preference
		if (m_ship->GetEnableAutoLevel()!=m_enableAutoLevelWhenPiloting)
			ToggleAutoLevel();
		GG_Framework::Logic::Entity3D::EventMap* em = m_ship->GetEventMap();

		em->EventOnOff_Map["RequestAfterburner"].Subscribe(ehl, *this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Subscribe(ehl, *this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Subscribe(ehl, *this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Subscribe(ehl, *this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &UI_Controller::Turn_L);
		em->EventOnOff_Map["Roll_CW"].Subscribe(ehl, *this, &UI_Controller::Roll_CW);
		em->EventOnOff_Map["Roll_CCW"].Subscribe(ehl, *this, &UI_Controller::Roll_CCW);
		em->EventOnOff_Map["Pitch_Up"].Subscribe(ehl, *this, &UI_Controller::Pitch_Up);
		em->EventOnOff_Map["Pitch_Dn"].Subscribe(ehl, *this, &UI_Controller::Pitch_Dn);
		em->Event_Map["UserResetPos"].Subscribe(ehl, *this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Subscribe(ehl, *this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Subscribe(ehl, *this, &UI_Controller::ToggleSlide);
		em->Event_Map["AutoLevel"].Subscribe(ehl, *this, &UI_Controller::ToggleAutoLevel);
		em->EventOnOff_Map["StrafeUp"].Subscribe(ehl, *this, &UI_Controller::StrafeUp);
		em->EventOnOff_Map["StrafeDown"].Subscribe(ehl, *this, &UI_Controller::StrafeDown);
		em->EventOnOff_Map["StrafeLeft"].Subscribe(ehl, *this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Subscribe(ehl, *this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Subscribe(ehl, *this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["POVMouse"].Subscribe(ehl, *this, &UI_Controller::ToggleMousePOV);
		em->EventOnOff_Map["POV_Down"].Subscribe(ehl, *this, &UI_Controller::POV_Down);
		em->EventOnOff_Map["POV_DL"].Subscribe(ehl, *this, &UI_Controller::POV_DL);
		em->EventOnOff_Map["POV_DR"].Subscribe(ehl, *this, &UI_Controller::POV_DR);
		em->EventOnOff_Map["POV_Up"].Subscribe(ehl, *this, &UI_Controller::POV_Up);
		em->EventOnOff_Map["POV_UL"].Subscribe(ehl, *this, &UI_Controller::POV_UL);
		em->EventOnOff_Map["POV_UR"].Subscribe(ehl, *this, &UI_Controller::POV_UR);
		em->EventOnOff_Map["POV_Left"].Subscribe(ehl, *this, &UI_Controller::POV_Left);
		em->EventOnOff_Map["POV_Right"].Subscribe(ehl, *this, &UI_Controller::POV_Right);
		em->EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &UI_Controller::OnSpawn);

		em->EventOnOff_Map["Test1"].Subscribe(ehl, *this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Subscribe(ehl, *this, &UI_Controller::Test2);

		// And a button for the Speed PDCB
		em->Event_Map["ShowHUD"].Subscribe(
			m_HUD_UI->ehl, *m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);

		// Listen for blackout
		em->EventValue_Map["BLACKOUT"].Subscribe(ehl, *this, &UI_Controller::BlackoutHandler);

		em->EventValue_Map["Analog_Turn"].Subscribe(ehl,*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_Pitch"].Subscribe(ehl,*this, &UI_Controller::JoyStick_Ship_Pitch);
		em->EventValue_Map["Analog_StrafeRight"].Subscribe(ehl,*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_StrafeDown"].Subscribe(ehl,*this, &UI_Controller::StrafeDown);
		em->EventValue_Map["Analog_Slider_StrafeDown"].Subscribe(ehl,*this, &UI_Controller::Slider_StrafeDown);
		em->EventValue_Map["Analog_Slider_Accel"].Subscribe(ehl,*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["POV_Joystick"].Subscribe(ehl,*this, &UI_Controller::Joystick_POV);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed);

		// Tell the HUD the name of this ship
		m_HUD_UI->m_addnText = m_ship->GetName();
	}
}

void UI_Controller::Test1(bool on)
{
	m_Test1=on;
}
void UI_Controller::Test2(bool on)
{
	m_Test2=on;
}

void UI_Controller::Ship_AfterBurner_Thrust(bool on)	
{		
	// Touching the Afterburner always places us back in SImFLight mode
	m_ship->SetSimFlightMode(true);

	// Set the current requested speed for the ship based on whether we are turning afterburner on or off
	if (on)
		m_ship->FireAfterburner();
	else
	{
		// Set the requested speed to our current speed or at the max engaged speed (Afterburner Brake)
		osg::Vec3d LocalVelocity(m_ship->GetAtt_quat().conj() * m_ship->GetPhysics().GetLinearVelocity());
		double currSpeed = LocalVelocity[1];
		m_ship->SetRequestedSpeed(MIN(currSpeed, m_ship->GetEngaged_Max_Speed()));
	}
}

void UI_Controller::Ship_Thrust(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = m_ship->ACCEL;
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Brake(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = -m_ship->BRAKE;
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Thrust(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = m_ship->ACCEL*Intensity;
}

void UI_Controller::Ship_Brake(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = -m_ship->BRAKE*Intensity;
}

void UI_Controller::Ship_StrafeUp(double Intensity)		
{
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[2]=  m_ship->STRAFE*Intensity;	
}
void UI_Controller::Ship_StrafeDown(double Intensity)	
{		
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[2]=  -m_ship->STRAFE*Intensity;	
}
void UI_Controller::Ship_StrafeLeft(double Intensity)	
{		
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  -m_ship->STRAFE*Intensity;	
}
void UI_Controller::Ship_StrafeRight(double Intensity)	
{
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  m_ship->STRAFE*Intensity;	
}

void UI_Controller::Ship_Turn(double dir,bool UseHeadingSpeed) 
{
	if (fabs(dir)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_UseHeadingSpeed=UseHeadingSpeed;
	m_Ship_JoyMouse_rotVel_rad_s[0]=(UseHeadingSpeed?dir*m_ship->dHeading:dir)*m_ship->GetFlightCharacteristics().Camera_Restraint;
}
void UI_Controller::Ship_Pitch(double dir,bool UsePitchSpeed) 
{
	if (fabs(dir)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_UsePitchSpeed=UsePitchSpeed;
	m_Ship_JoyMouse_rotVel_rad_s[1]=(UsePitchSpeed?dir*m_ship->dPitch:dir)*m_ship->GetFlightCharacteristics().Camera_Restraint;
}
void UI_Controller::Ship_Roll(double dir,bool UseRollSpeed) 
{
	if (fabs(dir)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_UseRollSpeed=UseRollSpeed;
	m_Ship_JoyMouse_rotVel_rad_s[2]=(UseRollSpeed?dir*-m_ship->dRoll:dir)*m_ship->GetFlightCharacteristics().Camera_Restraint;
}

void UI_Controller::Ship_Turn(Directions dir)
{
	m_Ship_Keyboard_rotVel_rad_s[0]=(double)dir*m_ship->dHeading*m_ship->GetFlightCharacteristics().Camera_Restraint;
	m_Ship_UseHeadingSpeed=true;
}

void UI_Controller::Ship_Pitch(Directions dir)
{
	m_Ship_Keyboard_rotVel_rad_s[1]=(double)dir*m_ship->dPitch*m_ship->GetFlightCharacteristics().Camera_Restraint;
	m_Ship_UsePitchSpeed=true;
}

void UI_Controller::Ship_Roll(Directions dir)
{
	m_Ship_Keyboard_rotVel_rad_s[2]=(double)dir*-m_ship->dRoll*m_ship->GetFlightCharacteristics().Camera_Restraint;
	m_Ship_UseRollSpeed=true;
}

void UI_Controller::CancelAllControls()
{
	// Turns off all controls that might be on
	AfterBurner_Thrust(false);
	Thrust(false);
	Brake(false);
	Turn_L(false);
	Turn_R(false);
	Roll_CW(false);
	Roll_CCW(false);
	Pitch_Up(false);
	Pitch_Dn(false);
	StrafeUp(false);
	StrafeDown(false);
	StrafeLeft(false);
	StrafeRight(false);
	ToggleMousePOV(false);
	POV_Down(false);
	POV_DL(false);
	POV_DR(false);
	POV_Up(false);
	POV_UL(false);
	POV_UR(false);
	POV_Left(false);
	POV_Right(false);
	Test1(false);
	Test2(false);
	TryFireMainWeapon(false);
}

void UI_Controller::TryToggleAutoPilot()
{
	SetAutoPilot(!GetAutoPilot());
}

bool UI_Controller::SetAutoPilot(bool autoPilot)
{
	// Note that Autopilot MUST be on if the ship is not being controlled
	m_autoPilot = autoPilot || !m_isControlled || !m_Base->GetCanUserPilot();

	// When in autopilot, always use mouse POV
	ToggleMousePOV(m_autoPilot);

	// We always want auto-level on when in auto-pilot, otherwise set it back to what the user wants
	m_ship->SetEnableAutoLevel(autoPilot || m_enableAutoLevelWhenPiloting);

	// If we are in auto-pilot, we MUST set simulated flight mode if we have an auto-pilot route
	if (m_autoPilot)
	{
		bool hasAutoPilotRoute = m_Base->HasAutoPilotRoute();
		m_ship->SetStabilizeRotation(hasAutoPilotRoute);
		m_ship->SetSimFlightMode(hasAutoPilotRoute);
	}
	else
	{
		m_ship->SetStabilizeRotation(true);
		m_ship->SetSimFlightMode(true);
	}

	// When turning on or OFF the auto pilot, stop firing and other actions
	m_ship->CancelAllControls();

	return m_autoPilot;
}

void UI_Controller::ToggleMousePOV(bool on) 
{
	// Note that MousePOV MUST stay on when in auto pilot mode
	m_UseMousePOV= on || m_autoPilot;

	// disable snapback while in POV mouse mode (Only if explicitly called)
	m_chasePlaneCamManip->SetPerformPOVSnapBack(!m_UseMousePOV);
} 

void UI_Controller::UserResetPos()
{
	if (AreControlsDisabled())
		return;
	m_ship->ResetPos();
}

void UI_Controller::ResetPos()
{
	m_Ship_Keyboard_rotVel_rad_s =	m_Ship_JoyMouse_rotVel_rad_s = m_Ship_Keyboard_currAccel = m_Ship_JoyMouse_currAccel =	osg::Vec3d(0,0,0);

	m_chasePlaneCamManip->Reset();
	m_HUD_UI->Reset();
	m_Base->ResetPos();
}

void UI_Controller::BlackoutHandler(double bl)
{
	// Turn STRAFE on the ship to pull out of the blackout
	if ((bl != 0.0) && (!m_autoPilot))
		m_ship->SetSimFlightMode(false);
}

void UI_Controller::POV_Turn(double dir,bool UseKeyboard)
{
	if ((!UseKeyboard)&&(fabs(dir)<0.001))  //Weed out empty Joystick calls that have no effect
		return;
	if (!UseKeyboard)
		m_POV_rad_s[0]=dir;
	else
		m_POV_keyboard_rad_s[0]=dir*M_PI;
}
void UI_Controller::POV_Pitch(double dir,bool UseKeyboard)
{
	if ((!UseKeyboard)&&(fabs(dir)<0.001))  //Weed out empty Joystick calls that have no effect
		return;
	if (!UseKeyboard)
		m_POV_rad_s[1]=dir;
	else
		m_POV_keyboard_rad_s[1]=dir*M_PI;
}

void UI_Controller::ToggleAutoLevel() 
{
	if (AreControlsDisabled()) return;
	m_enableAutoLevelWhenPiloting = !m_ship->GetEnableAutoLevel();
	m_ship->SetEnableAutoLevel(m_enableAutoLevelWhenPiloting);
}

void UI_Controller::Mouse_Turn(double dir)
{
	if (!m_UseMousePOV)
		Ship_Turn(dir,false);
	else
		m_chasePlaneCamManip->POV_Turn(dir,true);
}
void UI_Controller::Mouse_Pitch(double dir)
{
	if (!m_UseMousePOV)
		Ship_Pitch(dir,false);
	else
		m_chasePlaneCamManip->POV_Pitch(dir,true);
}
void UI_Controller::Mouse_Roll(double dir)
{
	// We do not have a POV "Roll" yet, perhaps fly away from the camera?
	if (!m_UseMousePOV)
		Ship_Roll(dir, false);
}

#define __SwapUpDownAndThrustBreak__


//Note: XTerminator filter = 0.01, and is squared have been transferred to default settings
void UI_Controller::JoyStick_Ship_Turn(double dir)
{
	if (AreControlsDisabled())
		return;

	Ship_Turn(dir);
}

//Note: XTerminator filter = 0.01, and is squared have been transferred to default settings
void UI_Controller::JoyStick_Ship_Pitch(double dir)
{
	if (AreControlsDisabled())
		return;

	Ship_Pitch(dir);
}

//Note: FilterSize 0.2 for XTerminator, but should not be a default setting
void UI_Controller::Slider_StrafeDown(double Intensity)
{
	if (AreControlsDisabled())
		return;

	if (Intensity>0.0)
		Ship_StrafeUp(Intensity);
	else if (Intensity<0.0)
		Ship_StrafeDown(-Intensity);
}

void UI_Controller::Slider_Accel(double Intensity)
{
	if (AreControlsDisabled())
		return;

	if (Intensity>0.0)
		Ship_Thrust(Intensity);
	else if (Intensity<0.0)
		Ship_Brake(-Intensity);
}

void UI_Controller::Joystick_POV(double Degrees)
{
	//Do the POV first, we CAN do THAT in Auto Pilot Mode
	const double TurnSpeed_rad_s=M_PI;

	//We put the typical case first (save the amount of branching)
	if (Degrees!=-1)
	{
		//so breaking down the index
		//0 = up
		//1 = up right
		//2 = right
		//3 = down right
		//4 = down
		//5 = down left
		//6 = left
		//7 = left up
		size_t index=(size_t)(Degrees/45.0);
		double YawTable[8]={0.0,-TurnSpeed_rad_s,-TurnSpeed_rad_s,-TurnSpeed_rad_s,0.0,TurnSpeed_rad_s,TurnSpeed_rad_s,TurnSpeed_rad_s};
		double PitchTable[8]={TurnSpeed_rad_s,TurnSpeed_rad_s,0.0,-TurnSpeed_rad_s,-TurnSpeed_rad_s,-TurnSpeed_rad_s,0.0,TurnSpeed_rad_s};
		POV_Turn(YawTable[index],false);
		POV_Pitch(PitchTable[index],false);
	}
}

void UI_Controller::Joystick_SetCurrentSpeed(double Speed)
{
	if (m_Ship_Keyboard_currAccel[1]==0.0)
	{
		if (m_ship->GetAlterTrajectory())
		{
			if ((fabs(Speed-m_LastSliderTime[1])>0.05)||(Speed==0))
			{
				double SpeedToUse=m_ship->GetIsAfterBurnerOn()?m_ship->GetMaxSpeed():m_ship->GetEngaged_Max_Speed();
				//This works but I really did not like the feel of it
				double SpeedCalibrated=((Speed/2.0)+0.5)*SpeedToUse;
				m_LastSliderTime[1]=Speed;
				if (SpeedCalibrated!=m_CruiseSpeed)
				{
					m_ship->SetRequestedSpeed(SpeedCalibrated);
					m_CruiseSpeed=SpeedCalibrated;
				}
			}
		}
		else
			Ship_Thrust(0.0);
	}
}

osg::Geometry* UI_Controller::MakeVelLine(osg::Vec3 vel)
{
	// Create the vertices and geometry and get them together
	osg::Vec3Array* velocityVerts = new osg::Vec3Array;
	velocityVerts->push_back(osg::Vec3(0,0,0));
	velocityVerts->push_back(vel);
	osg::Geometry* velGeom = new osg::Geometry;
	velGeom->setVertexArray(velocityVerts);

	// We will make the line all yellow
	osg::Vec4Array* yellow = new osg::Vec4Array;
	yellow->push_back(osg::Vec4(1.0f,1.0f,0.0f,1.0f));
	velGeom->setColorArray(yellow);
	velGeom->setColorBinding(osg::Geometry::BIND_OVERALL);

	// No need to worry about normals directions
	osg::Vec3Array* normals = new osg::Vec3Array;
	normals->push_back(osg::Vec3(0.0f,-1.0f,0.0f));
	velGeom->setNormalArray(normals);
	velGeom->setNormalBinding(osg::Geometry::BIND_OVERALL);

	// We want to draw this geometry as a line
	velGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));
	return velGeom;
}

bool UI_Controller::AreControlsDisabled()
{
	return (m_autoPilot || !m_ship->IsShowing());
}

void UI_Controller::UpdateController(double dTime_s)
{
	if (AreControlsDisabled())
		m_Base->UpdateController_Base(dTime_s);

	if (m_isControlled)
	{
		// Update Mouse Controller (This is ONLY allowed to update the POV in auto pilot)
		m_mouseDriver->DriveShip();
		
		// Update POV
		m_chasePlaneCamManip->POV_Turn(m_POV_rad_s[0]+m_POV_keyboard_rad_s[0]);
		m_chasePlaneCamManip->POV_Pitch(m_POV_rad_s[1]+m_POV_keyboard_rad_s[1]);
		
		//reset now that I have used them
		m_POV_rad_s[0]=m_POV_rad_s[1]=0.0;

		//Now for the ship
		if (!AreControlsDisabled())
		{
			// Normally we pass the the ship the addition of the keyboard and mouse accel
			osg::Vec3d shipAccel = m_Ship_Keyboard_currAccel+m_Ship_JoyMouse_currAccel;

			// apply various input sources to current acceleration
			m_ship->SetCurrentLinearAcceleration(shipAccel); 
			
			//flush the JoyMouse current acceleration vec3 since it works on an additive nature
			m_Ship_JoyMouse_currAccel=osg::Vec3d(0.0,0.0,0.0);
		
		
			//add all the various input types to the main rotation velocity
			osg::Vec3d rotVel=(m_Ship_Keyboard_rotVel_rad_s+m_Ship_JoyMouse_rotVel_rad_s);
			//We may have same strange undesired flicker effect if the mouse and keyboard turns are used simultaneously! So if the keyboard is used, then
			//the mouse will get ignored
			m_ship->m_LockShipHeadingToOrientation=m_Ship_UseHeadingSpeed||m_Ship_UsePitchSpeed;
			m_ship->m_LockShipRollToOrientation=m_Ship_UseRollSpeed;

			if ((m_Ship_Keyboard_rotVel_rad_s[0]!=0) || (m_Ship_Keyboard_rotVel_rad_s[1]!=0))
			{
				m_ship->m_LockShipHeadingToOrientation=true;
				rotVel[0]=m_Ship_Keyboard_rotVel_rad_s[0]; 
				rotVel[1]=m_Ship_Keyboard_rotVel_rad_s[1]; 
				m_Ship_UseHeadingSpeed=true;
				m_Ship_UsePitchSpeed=true;
			}

			if (m_Ship_Keyboard_rotVel_rad_s[2]!=0)
			{
				m_ship->m_LockShipRollToOrientation=true;
				rotVel[2]=m_Ship_Keyboard_rotVel_rad_s[2];
				m_Ship_UseRollSpeed=true;
			}

			m_ship->SetCurrentAngularVelocity(rotVel);

			//flush the JoyMouse rotation vec3 since it works on an additive nature
			m_Ship_JoyMouse_rotVel_rad_s=osg::Vec3d(0.0,0.0,0.0);
			m_Ship_JoyMouse_currAccel=osg::Vec3d(0.0,0.0,0.0);

			// If we do go back to the AI, make sure it's reactions  reset
			m_Base->ResetAI_Reaction();
		}
		else
		{
			// From Rick:  James, what does this do? Should we be setting member variables of another class directly?
			m_ship->m_LockShipHeadingToOrientation=false;
			m_ship->m_LockShipRollToOrientation=false;
		}
	}
}

// enum eThrustState { TS_AfterBurner_Brake=0, TS_Brake, TS_Coast, TS_Thrust, TS_AfterBurner, TS_NotVisible };
const char* ThrustStateNames[] = {
	"Ship.AfterBurner_Brake", "Ship.Brake", "Ship.Coast", "Ship.Thrust", "Ship.AfterBurner", "Ship.NotVisible"
};

void UI_Controller::UpdateUI(double dTime_s)
{
	//No work to do if there is no ship to control
	if (!m_Base)
		return;
	// Positions:
	//[0]  +Right -Left
	//[1]  +Forward -Reverse
	//[2]  +Up -Down
#ifdef __EnableTestKeys__
	if (m_Test1)
		m_ship->m_Physics.ApplyFractionalForce(osg::Vec3d(0,0,m_ship->Mass),osg::Vec3d(0,10,0),dTime_s);

	else if (m_Test2)
		m_ship->m_Physics.ApplyFractionalForce(osg::Vec3d(0,0,-m_ship->Mass),osg::Vec3d(0,10,0),dTime_s);
#endif
	{
		#if 1
		osg::Vec3d pos=m_ship->GetPos_m();
		DOUT1("%f %f %f",pos[0],pos[1],pos[2]);
		#endif
		#if 0
		{
			GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
			osg::Vec3 eye,center,up;
			mainWin.GetMainCamera()->GetCameraMatrix().getLookAt(eye,center,up);
			//DOUT2("%f %f %f",eye[0],eye[1],eye[2]);
			DOUT2("%f %f %f",eye[0]-pos[0],eye[1]-pos[1],eye[2]-pos[2]);
		}
		#endif
		//DebugOut_PDCB::TEXT2 = BuildString("%s", ThrustStateNames[m_ship->GetThrustState()]);
	}
	// These items are always updated
	m_HUD_UI->SetLocked(!m_Base->GetCanUserPilot());
	m_HUD_UI->m_flightMode = GetAutoPilot() ? "AUTO PILOT" : (m_ship->GetAlterTrajectory() ? "Sim. Flt." : "SLIDE");
	m_HUD_UI->Update(*m_ship, dTime_s);

	bool useHUD = m_isControlled && m_Base->GetCanUserPilot();
	ConnectHUD_Elements(useHUD);
	if (useHUD)
	{
		// We want to add some nice lines to represent the ship's current velocity and reticals
		if (m_tgtDirLineGeode->getNumDrawables()!=0)
			m_tgtDirLineGeode->removeDrawables(0);

		// Get the guns local position and attitude
		osg::Vec3d gunPos;
		osg::Quat  gunAtt;
		m_ship->GetWeaponRelPosAtt(gunPos, gunAtt);

		// Find how far away the reticals SHOULD be
		double reticalDistance = 100000.0;
		double greatestCosAngle = 1.0; // Only ships in front of you 
		m_Base->TargetInFront(reticalDistance, greatestCosAngle);
		GG_Framework::AppReuse::DestroyableEntity* target = m_Base->GetTarget();
		if (target)
		{
			osg::Vec3d tgtLeadOffset(m_Base->GetTargetLeadPoint() - m_ship->GetPos_m());
			osg::Quat tgtLeadAtt; tgtLeadAtt.makeRotate(osg::Vec3d(0,1,0), tgtLeadOffset);
			double tgtLeadDist = tgtLeadOffset.length();
			if (tgtLeadDist < reticalDistance)
				reticalDistance = tgtLeadDist;

			// Point the yellow "arrow" right at the target
			osg::Vec3d cntrArrowPos = m_ship->GetPos_m() + 
				(m_ship->GetAtt_quat()*gunPos) + 
				((gunAtt*m_ship->GetAtt_quat())*osg::Vec3d(0.0, reticalDistance, 0.0));
			osg::Vec3d targetDir = target->GetPos_m() - cntrArrowPos;
			double dist = targetDir.normalize();
			targetDir *= MIN(dist, 0.5*dist);
			m_tgtDirLineGeode->addDrawable(MakeVelLine(targetDir));
			m_tgtDirLineTransform->setPosition(cntrArrowPos);

			// Adjust how far away our center reticle is away based on our lead position,
			// Adjusted for the cannon position on the ship
			if (m_ship->GetWeaponsSpeed() > 0.0)
			{				
				// We want to show the proper retical (in range or not
				bool inRange = (tgtLeadDist < m_ship->GetWeaponsRange());
				osg::PositionAttitudeTransform* goodRet = inRange ? m_targetInRangeLeadRetical.get() : m_targetLeadRetical.get();
				osg::PositionAttitudeTransform* badRet = !inRange ? m_targetInRangeLeadRetical.get() : m_targetLeadRetical.get();
				bool* goodRetShow = inRange ? &m_targetInRangeLeadRetShowing : &m_targetLeadRetShowing;
				bool* badRetShow = !inRange ? &m_targetInRangeLeadRetShowing : &m_targetLeadRetShowing;

				goodRet->setScale(osg::Vec3d(tgtLeadDist,tgtLeadDist,tgtLeadDist));
				goodRet->setPosition(m_ship->GetPos_m());
				goodRet->setAttitude(tgtLeadAtt);

				// Show and hide the proper ones
				if (!*goodRetShow)
				{
					m_ship->Get_UI_Actor()->getParent(0)->addChild(goodRet);
					*goodRetShow = true;
				}
				if (*badRetShow)
				{
					m_ship->Get_UI_Actor()->getParent(0)->removeChild(badRet);
					*badRetShow = false;
				}
			}
			else
			{
				if (m_targetLeadRetShowing)
				{
					m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetLeadRetical.get());
					m_targetLeadRetShowing = false;
				}
				if (m_targetInRangeLeadRetShowing)
				{
					m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetInRangeLeadRetical.get());
					m_targetInRangeLeadRetShowing = false;
				}
			}
		}
		else
		{
			// No targets, do not show either retical
			if (m_targetLeadRetShowing)
			{
				m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetLeadRetical.get());
				m_targetLeadRetShowing = false;
			}

			if (m_targetInRangeLeadRetShowing)
			{
				m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetInRangeLeadRetical.get());
				m_targetInRangeLeadRetShowing = false;
			}
		}
		m_shipSiteRetical->setPosition(gunPos);
		m_shipSiteRetical->setAttitude(gunAtt);
		m_shipSiteRetical->setScale(osg::Vec3d(reticalDistance,reticalDistance,reticalDistance));
	}
}

//////////////////////////////////////////////////////////////////////////

void UI_Controller::HookUpUI(bool ui)
{
	m_isControlled = ui;
	GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
	if (m_isControlled)
	{
		// Start with the mouse centered in the screen and turn off the cursor
		mainWin.PositionPointer(0.0f,0.0f);
		mainWin.UseCursor(false);

		// Provide the new camera manipulator
		mainWin.GetMainCamera()->SetCameraManipulator(m_chasePlaneCamManip);
		mainWin.GetKeyboard_Mouse().SetControlledEventMap(m_ship->GetEventMap());
		mainWin.GetJoystick().SetControlledEventMap(m_ship->GetEventMap());
	}

	if (m_ship)
	{
		// Set Auto Pilot to true if there is control
		SetAutoPilot(!m_isControlled || !m_Base->GetCanUserPilot());
	}
}
//////////////////////////////////////////////////////////////////////////

void UI_Controller::OnSpawn(bool on)
{
	// Set Auto Pilot to true if there is control.
	// This is commented out for now so we can leave a ship in auto-pilot and it will return to auto-pilot after spawn
	// SetAutoPilot(!m_isControlled || !m_Base->GetCanUserPilot());

	// Reset the POV
	m_chasePlaneCamManip->Reset();
}
//////////////////////////////////////////////////////////////////////////

void UI_Controller::ConnectHUD_Elements(bool connect)
{
	if (connect == m_hud_connected) return;

	m_hud_connected = connect;
	GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
	if (connect)
	{
		// Build the HUD the first time (I am not sure I like this here)
		if (!m_tgtDirLineTransform.valid())
			BuildHUD();

		// Provide some text for the speed
		mainWin.GetMainCamera()->addPostDrawCallback(*m_HUD_UI.get());

		// Connect UI elements to the scene graph
		m_ship->Get_UI_Actor()->getParent(0)->addChild(m_tgtDirLineTransform.get());
		m_ship->Get_UI_Actor()->addChild(m_shipSiteRetical.get());
	}
	else
	{
		// We do not need to get rid of the camera manipulator, since it is being replaced

		// Remove the speed text
		mainWin.GetMainCamera()->removePostDrawCallback(*m_HUD_UI.get());

		// Remove the other HUD items from the scene graph
		m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_tgtDirLineTransform.get());

		if (m_targetLeadRetShowing)
		{
			m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetLeadRetical.get());
			m_targetLeadRetShowing = false;
		}
		if (m_targetInRangeLeadRetShowing)
		{
			m_ship->Get_UI_Actor()->getParent(0)->removeChild(m_targetInRangeLeadRetical.get());
			m_targetInRangeLeadRetShowing = false;
		}
		m_ship->Get_UI_Actor()->removeChild(m_shipSiteRetical.get());
	}
}
//////////////////////////////////////////////////////////////////////////

void UI_Controller::BuildHUD()
{
	// The geode is actually the Node that holds the drawable
	m_tgtDirLineGeode = new osg::Geode;
	m_tgtDirLineTransform = new osg::PositionAttitudeTransform;
	m_tgtDirLineTransform->addChild(m_tgtDirLineGeode.get());

	// The lead retical for the target
	m_targetLeadRetical = new osg::PositionAttitudeTransform;
	m_targetLeadRetical->addChild(m_ship->GetGameClient()->Get_UI_ActorScene()->ReadActorFile(*m_ship->GetEventMap(), 
		m_Base->LEAD_RET_OSGV.c_str()));
	GG_Framework::UI::OSG::SetNodeToDrawOnTop(m_targetLeadRetical.get());

	// The in range lead retical for the target
	m_targetInRangeLeadRetical = new osg::PositionAttitudeTransform;
	m_targetInRangeLeadRetical->addChild(m_ship->GetGameClient()->Get_UI_ActorScene()->ReadActorFile(*m_ship->GetEventMap(), 
		m_Base->INRANGE_LEAD_RET_OSGV.c_str()));
	GG_Framework::UI::OSG::SetNodeToDrawOnTop(m_targetInRangeLeadRetical.get());

	m_shipSiteRetical = m_ship->GetGameClient()->Get_UI_ActorScene()->ReadActorFile(*m_ship->GetEventMap(), 
		m_Base->FWD_RET_OSGV.c_str());
	GG_Framework::UI::OSG::SetNodeToDrawOnTop(m_shipSiteRetical.get());
}
//////////////////////////////////////////////////////////////////////////




