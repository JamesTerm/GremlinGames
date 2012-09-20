#include "stdafx.h"
#include "AI_Tester.h"
#include "Debug.h"

#undef __EnableTestKeys__

using namespace AI_Tester;
//using namespace GG_Framework::Logic;
using namespace GG_Framework::Base;
using namespace GG_Framework::UI;
using namespace osg;
bool g_UseMouse=false;


namespace Scripting=GG_Framework::Logic::Scripting;

  /***************************************************************************************************************/
 /*												Mouse_ShipDriver												*/
/***************************************************************************************************************/

Mouse_ShipDriver::Mouse_ShipDriver(Ship_2D& ship,UI_Controller *parent, unsigned avgFrames) : 
	m_ship(ship),m_ParentUI_Controller(parent), m_mousePosHist(NULL), m_avgFrames(avgFrames), m_currFrame(0), m_mouseRoll(false)
{
	//GG_Framework::UI::KeyboardMouse_CB &kbm = GG_Framework::UI::MainWindow::GetMainWindow()->GetKeyboard_Mouse();	
	//kbm.AddKeyBindingR(true, "Ship.MouseRoll", osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON);

	// Set up the Mouse Control to drive
	Entity2D::EventMap* em = m_ship.GetEventMap();
	em->KBM_Events.MouseMove.Subscribe(ehl, *this, &Mouse_ShipDriver::OnMouseMove);
	//em->EventOnOff_Map["Ship.MouseRoll"].Subscribe(ehl, *this, &Mouse_ShipDriver::OnMouseRoll);

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
	if (!g_UseMouse)
	 return;

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
	static const float x_coeff = 0.004f;
	static const float y_coeff = -0.004f;
	static const float roll_coeff = 0.01f;

	// Finally Turn the Heading or Pitch (or roll if using the rt mouse button
	if (m_mouseRoll)
	{
		// Use this roll versus the scroll wheel
		float dR = dY - dX;

		//! \todo JAMES: here is the funky roll
		//if (dR != 0.0f)
		//	m_ParentUI_Controller->Mouse_Roll(dR*roll_coeff);
	}
	else
	{
		if (dX != 0.0f)
			m_ParentUI_Controller->Mouse_Turn(dX*x_coeff);
		//if (dY != 0.0f)
		//	m_ParentUI_Controller->Mouse_Pitch(dY*y_coeff);
	}
}                                                      
//////////////////////////////////////////////////////////////////////////




  /***************************************************************************************************************/
 /*												UI_Controller													*/
/***************************************************************************************************************/

void UI_Controller::Init_AutoPilotControls()
{
	//Do not care about this
#if 0
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
#endif
}


//! TODO: Use the script to grab the head position to provide the HUD
UI_Controller::UI_Controller(AI_Base_Controller *base_controller,bool AddJoystickDefaults) : 
	/*m_HUD_UI(new HUD_PDCB(osg::Vec3(0.0, 4.0, 0.5))), */
	m_Base(NULL),m_mouseDriver(NULL),m_SlideButtonToggle(false),m_isControlled(false),m_ShipKeyVelocity(0.0),m_CruiseSpeed(0.0),m_LeftVelocity(0.0),m_RightVelocity(0.0),
	m_autoPilot(true),m_enableAutoLevelWhenPiloting(false),m_Test1(false),m_Test2(false),m_Ship_UseHeadingSpeed(true)
{
	ResetPos();
	Set_AI_Base_Controller(base_controller); //set up ship (even if we don't have one)
	m_LastSliderTime[0]=m_LastSliderTime[1]=0.0;

	// Hard code these key bindings at first
	KeyboardMouse_CB &kbm = MainWindow::GetMainWindow()->GetKeyboard_Mouse();	
	JoyStick_Binder &joy = MainWindow::GetMainWindow()->GetJoystick();
	kbm.AddKeyBindingR(true, "Ship.TryFireMainWeapon", osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON);
	//disabled until it works
	//kbm.AddKeyBindingR(true, "RequestAfterburner", GG_Framework::Base::Key('w', GG_Framework::Base::Key::DBL));
	kbm.AddKeyBindingR(true, "Thrust", GG_Framework::Base::Key('w'));
	kbm.AddKeyBindingR(true, "Brake", 's');
	kbm.AddKeyBindingR(false, "Stop", 'x');
	kbm.AddKeyBindingR(true, "Turn_R", 'd');
	kbm.AddKeyBindingR(true, "Turn_L", 'a');
	kbm.AddKeyBindingR(false, "UseMouse", '/');
	//I would like to keep this macro case to easily populate my defaults
	#if 0
	//for testing
	kbm.AddKeyBindingR(true, "Test1", 'n');
	kbm.AddKeyBindingR(true, "Test2", 'm');
	#endif
	kbm.AddKeyBindingR(true, "StrafeRight", 'e');
	kbm.AddKeyBindingR(true, "StrafeLeft", 'q');
	kbm.AddKeyBindingR(false, "UserResetPos", ' ');
	kbm.AddKeyBindingR(false, "Slide", 'g');
	kbm.AddKeyBindingR(true, "StrafeLeft", osgGA::GUIEventAdapter::KEY_Left);
	kbm.AddKeyBindingR(true, "StrafeRight", osgGA::GUIEventAdapter::KEY_Right);
	kbm.AddKeyBindingR(false, "ToggleAutoPilot", 'z');
	//kbm.AddKeyBindingR(false, "ShowHUD", osgGA::GUIEventAdapter::KEY_F4);

	if (AddJoystickDefaults)
	{
		joy.AddJoy_Button_Default(0,"Ship.TryFireMainWeapon");
		joy.AddJoy_Button_Default(1,"Missile.Launch");
		// We can now use double-tap to fire the afterburners (for when we have them)
		joy.AddJoy_Button_Default(2,"Thrust");
		joy.AddJoy_Button_Default(3,"Brake");
		joy.AddJoy_Analog_Default(JoyStick_Binder::eSlider1,"Joystick_SetCurrentSpeed");
		//These are not assigned by default but can configured to use via xml preferences
		joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",false,1.0,0.01,true);
		//hmmm could use this to thrust
		//joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Analog_Pitch",true,1.0,0.01,true);
		joy.AddJoy_Button_Default(6,"Slide",false);
		joy.AddJoy_Analog_Default(JoyStick_Binder::eZ_Rot,"Analog_StrafeRight");
	}

	Init_AutoPilotControls();
}


const char *UI_Controller::ExtractControllerElementProperties(Controller_Element_Properties &Element,const char *Eventname,Scripting::Script& script)
{
	const char *err=NULL;
	err = script.GetFieldTable(Eventname);
	if (!err)
	{
		Element.Event=Eventname;
		std::string sType;
		err = script.GetField("type",&sType,NULL,NULL);
		ASSERT_MSG(!err, err);
		
		if (strcmp(sType.c_str(),"joystick_analog")==0)
		{
			Element.Type=Controller_Element_Properties::eJoystickAnalog;
			JoyStick_Binder::JoyAxis_enum JoyAxis;
			double dJoyAxis;
			err = script.GetField("key", NULL, NULL,&dJoyAxis);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration
			JoyAxis=(JoyStick_Binder::JoyAxis_enum)((int)dJoyAxis);
			bool IsFlipped;
			err = script.GetField("is_flipped", NULL, &IsFlipped,NULL);
			ASSERT_MSG(!err, err);
			double Multiplier;
			err = script.GetField("multiplier", NULL, NULL,&Multiplier);
			ASSERT_MSG(!err, err);
			double FilterRange;
			err = script.GetField("filter", NULL, NULL,&FilterRange);
			ASSERT_MSG(!err, err);
			double CurveIntensity;
			err = script.GetField("curve_intensity", NULL, NULL, &CurveIntensity);
			ASSERT_MSG(!err, err);

			Controller_Element_Properties::ElementTypeSpecific::AnalogSpecifics_rw &set=Element.Specifics.Analog;
			set.JoyAxis=JoyAxis;
			set.IsFlipped=IsFlipped;
			set.Multiplier=Multiplier;
			set.FilterRange=FilterRange;
			set.CurveIntensity=CurveIntensity;
			//joy.AddJoy_Analog_Default(JoyAxis,Eventname,IsFlipped,Multiplier,FilterRange,IsSquared,ProductName.c_str());
		}
		else if (strcmp(sType.c_str(),"joystick_button")==0)
		{
			Element.Type=Controller_Element_Properties::eJoystickButton;
			size_t WhichButton;
			double dWhichButton;
			err = script.GetField("key", NULL, NULL,&dWhichButton);
			ASSERT_MSG(!err, err);
			//cast to int first, and then to the enumeration; The -1 allows for cardinal types (good since we can use numbers written on button)
			WhichButton=(JoyStick_Binder::JoyAxis_enum)((int)dWhichButton-1);
			bool useOnOff;
			err = script.GetField("on_off", NULL, &useOnOff,NULL);
			ASSERT_MSG(!err, err);
			bool dbl_click=false;
			err = script.GetField("dbl", NULL, &dbl_click,NULL); //This one can be blank
			err=NULL;  //don't return an error (assert for rest)

			Controller_Element_Properties::ElementTypeSpecific::ButtonSpecifics_rw &set=Element.Specifics.Button;
			set.WhichButton=WhichButton;
			set.useOnOff=useOnOff;
			set.dbl_click=dbl_click;
			//joy.AddJoy_Button_Default( WhichButton,Eventname,useOnOff,dbl_click,ProductName.c_str());
		}
		else assert(false);
		script.Pop();
	}
	return err;
}

void UI_Controller::Flush_AI_BaseResources()
{
	#if 0
	//disconnect hud
	ConnectHUD_Elements(false);
	m_tgtDirLineTransform=NULL;  //this will trigger the next build hud
	m_HUD_UI->Reset();
	#endif
	if (m_mouseDriver)
	{
		delete m_mouseDriver;
		m_mouseDriver=NULL;
	}
	m_ship=NULL; //we don't own this
}

UI_Controller::~UI_Controller()
{
	Set_AI_Base_Controller(NULL); //this will unbind the events and flush the AI resources
}

GG_Framework::UI::JoyStick_Binder &UI_Controller::GetJoyStickBinder()
{
	return MainWindow::GetMainWindow()->GetJoystick();
}

void UI_Controller::Set_AI_Base_Controller(AI_Base_Controller *controller)
{
	//destroy all resources associated with the previous ship
	if (m_Base)
	{
		Entity2D::EventMap* em = m_ship->GetEventMap();
		//disabled until it works
		//em->EventOnOff_Map["RequestAfterburner"].Remove(*this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Remove(*this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Remove(*this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Remove(*this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Remove(*this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Remove(*this, &UI_Controller::Turn_L);
		em->Event_Map["UserResetPos"].Remove(*this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Remove(*this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Remove(*this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["SlideHold"].Remove(*this, &UI_Controller::SlideHold);
		em->EventOnOff_Map["StrafeLeft"].Remove(*this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Remove(*this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Remove(*this, &UI_Controller::OnSpawn);
		em->Event_Map["UseMouse"].Remove(*this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Remove(*this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Remove(*this, &UI_Controller::Test2);
		//em->Event_Map["ShowHUD"].Remove(*m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);
		em->EventValue_Map["BLACKOUT"].Remove(*this, &UI_Controller::BlackoutHandler);
		em->EventValue_Map["Analog_Turn"].Remove(*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Remove(*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed_2);
		em->EventValue_Map["Joystick_SetLeftVelocity"].Remove(*this, &UI_Controller::Joystick_SetLeftVelocity);
		em->EventValue_Map["Joystick_SetRightVelocity"].Remove(*this, &UI_Controller::Joystick_SetRightVelocity);
		m_ship->BindAdditionalEventControls(false);
		Flush_AI_BaseResources();
	}
	m_Base=controller;
	if (m_Base)
	{
		m_ship=&m_Base->m_ship;
		m_mouseDriver=new Mouse_ShipDriver(*m_ship,this, 3);
		Entity2D::EventMap* em = m_ship->GetEventMap();
		//disabled until it works
		//em->EventOnOff_Map["RequestAfterburner"].Subscribe(ehl, *this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Subscribe(ehl, *this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Subscribe(ehl, *this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Subscribe(ehl, *this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &UI_Controller::Turn_L);
		em->Event_Map["UserResetPos"].Subscribe(ehl, *this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Subscribe(ehl, *this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Subscribe(ehl, *this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["SlideHold"].Subscribe(ehl, *this, &UI_Controller::SlideHold);
		em->EventOnOff_Map["StrafeLeft"].Subscribe(ehl, *this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Subscribe(ehl, *this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Subscribe(ehl, *this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &UI_Controller::OnSpawn);

		em->Event_Map["UseMouse"].Subscribe(ehl, *this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Subscribe(ehl, *this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Subscribe(ehl, *this, &UI_Controller::Test2);

		// And a button for the Speed PDCB
		//em->Event_Map["ShowHUD"].Subscribe(m_HUD_UI->ehl, *m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);

		// Listen for blackout
		em->EventValue_Map["BLACKOUT"].Subscribe(ehl, *this, &UI_Controller::BlackoutHandler);

		em->EventValue_Map["Analog_Turn"].Subscribe(ehl,*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Subscribe(ehl,*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Subscribe(ehl,*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed_2);
		em->EventValue_Map["Joystick_SetLeftVelocity"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetLeftVelocity);
		em->EventValue_Map["Joystick_SetRightVelocity"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetRightVelocity);

		// Tell the HUD the name of this ship
		//m_HUD_UI->m_addnText = m_ship->GetName();

		m_ship->BindAdditionalEventControls(true);
		m_ship->BindAdditionalUIControls(true,&GetJoyStickBinder());
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
void UI_Controller::UseMouse()
{
	g_UseMouse=!g_UseMouse;
}

void UI_Controller::Ship_AfterBurner_Thrust(bool on)	
{	
	//Note this will happen implicitly
	// Touching the Afterburner always places us back in SImFLight mode
	//m_ship->SetSimFlightMode(true);

	// Set the current requested speed for the ship based on whether we are turning afterburner on or off
	if (on)
		m_ship->FireAfterburner();
	else
	{
		// Set the requested speed to our current speed or at the max engaged speed (Afterburner Brake)
		//Vec2d LocalVelocity(m_ship->GetAtt_quat().conj() * m_ship->GetPhysics().GetLinearVelocity());
		Vec2d LocalVelocity=GlobalToLocal(m_ship->GetAtt_r(),m_ship->GetPhysics().GetLinearVelocity());
		double currSpeed = LocalVelocity[1];
		m_ship->SetRequestedVelocity(MIN(currSpeed, m_ship->GetEngaged_Max_Speed()));
	}
}

void UI_Controller::Ship_Thrust(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = m_ship->GetAccelSpeed();
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Brake(bool on)
{	
	if (on)
		m_Ship_Keyboard_currAccel[1] = -m_ship->GetBrakeSpeed();
	else
		m_Ship_Keyboard_currAccel[1] = 0.0;
}

void UI_Controller::Ship_Thrust(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = m_ship->GetAccelSpeed()*Intensity;
}

void UI_Controller::Ship_Brake(double Intensity)
{	
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[1] = -m_ship->GetBrakeSpeed()*Intensity;
}

void UI_Controller::Ship_StrafeLeft(double Intensity)	
{		
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  -m_ship->GetStrafeSpeed()*Intensity;	
}
void UI_Controller::Ship_StrafeRight(double Intensity)	
{
	if (fabs(Intensity)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_JoyMouse_currAccel[0]=  m_ship->GetStrafeSpeed()*Intensity;	
}

void UI_Controller::Ship_Turn(double dir,bool UseHeadingSpeed) 
{
	if (fabs(dir)<0.001)  //Weed out empty Joystick calls that have no effect
		return;
	m_Ship_UseHeadingSpeed=UseHeadingSpeed;
	m_Ship_JoyMouse_rotAcc_rad_s=(UseHeadingSpeed?dir*m_ship->GetHeadingSpeed():dir)*m_ship->GetCameraRestraintScaler();
}

void UI_Controller::Ship_Turn(Directions dir)
{
	m_Ship_Keyboard_rotAcc_rad_s=(double)dir*m_ship->GetHeadingSpeed()*m_ship->GetCameraRestraintScaler();
	m_Ship_UseHeadingSpeed=true;
}


void UI_Controller::CancelAllControls()
{
	// Turns off all controls that might be on
	AfterBurner_Thrust(false);
	Thrust(false);
	Brake(false);
	Turn_L(false);
	Turn_R(false);
	StrafeLeft(false);
	StrafeRight(false);
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
	//ToggleMousePOV(m_autoPilot);

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

void UI_Controller::UserResetPos()
{
	if (AreControlsDisabled())
		return;
	//TODO see if this call is safe... it was put here as m_Ship_JoyMouse_rotAcc_rad_s would get stuck causing additional movement
	//(when testing swerve robot)... if this causes problems the UI Ship may call it
	ResetPos();
	m_ship->ResetPos();
}

void UI_Controller::ResetPos()
{
	m_Ship_Keyboard_rotAcc_rad_s =	m_Ship_JoyMouse_rotAcc_rad_s = m_ShipKeyVelocity = 0.0;
	m_Ship_Keyboard_currAccel = m_Ship_JoyMouse_currAccel =	Vec2d(0,0);

	//m_HUD_UI->Reset();
	//TODO see where ResetPos is called within the game, as m_Base does not have a check
	if (m_Base)
		m_Base->ResetPos();
}

void UI_Controller::BlackoutHandler(double bl)
{
	// Turn STRAFE on the ship to pull out of the blackout
	if ((bl != 0.0) && (!m_autoPilot))
		m_ship->SetSimFlightMode(false);
}

void UI_Controller::Mouse_Turn(double dir)
{
	Ship_Turn(dir,false);
}

//Note: XTerminator filter = 0.01, and is squared have been transferred to default settings
void UI_Controller::JoyStick_Ship_Turn(double dir)
{
	if (AreControlsDisabled())
		return;

	Ship_Turn(dir);
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

void UI_Controller::Joystick_SetCurrentSpeed(double Speed)
{
	if ((m_Ship_Keyboard_currAccel[1]==0.0)&&(!AreControlsDisabled()))
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
					//m_ship->SetRequestedVelocity(SpeedCalibrated);
					m_CruiseSpeed=SpeedCalibrated;
				}
			}
		}
		else
			m_Ship_JoyMouse_currAccel[1]=Speed;
	}
}

void UI_Controller::Joystick_SetCurrentSpeed_2(double Speed)
{
	if ((m_Ship_Keyboard_currAccel[1]==0.0)&&(!AreControlsDisabled()))
	{
		if (m_ship->GetAlterTrajectory())
		{
			//Avoid jitter for slider controls by testing the tolerance of change
			if ((fabs(Speed-m_LastSliderTime[1])>0.05)||(Speed==0))
			{
				double SpeedToUse=m_ship->GetIsAfterBurnerOn()?m_ship->GetMaxSpeed():m_ship->GetEngaged_Max_Speed();
				double SpeedCalibrated=Speed*SpeedToUse;
				m_LastSliderTime[1]=Speed;
				if (SpeedCalibrated!=m_CruiseSpeed)
				{
					//m_ship->SetRequestedVelocity(SpeedCalibrated);
					m_CruiseSpeed=SpeedCalibrated;
				}
			}
		}
		else
			m_Ship_JoyMouse_currAccel[1]=Speed;
	}
}

void UI_Controller::Joystick_SetLeftVelocity(double Velocity)
{
	if (!AreControlsDisabled())
		m_LeftVelocity=Velocity;
	else
		m_LeftVelocity=0.0;
}

void UI_Controller::Joystick_SetRightVelocity(double Velocity)
{
	if (!AreControlsDisabled())
		m_RightVelocity=Velocity;
	else
		m_RightVelocity=0.0;
}

#if 0
osg::Geometry* UI_Controller::MakeVelLine(osg::Vec2 vel)
{
	// Create the vertices and geometry and get them together
	osg::Vec2Array* velocityVerts = new osg::Vec2Array;
	velocityVerts->push_back(osg::Vec2(0,0,0));
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
#endif

bool UI_Controller::AreControlsDisabled()
{
	//return (m_autoPilot || !m_ship->IsShowing());
	return (m_autoPilot );
}

void UI_Controller::UpdateController(double dTime_s)
{
	if (AreControlsDisabled())
	{
		if (m_Base->m_Goal)
			m_Base->m_Goal->Process(dTime_s);
	}

	if (m_isControlled)
	{
		// Update Mouse Controller (This is ONLY allowed to update the POV in auto pilot)
		m_mouseDriver->DriveShip();

		//Now for the ship
		if (!AreControlsDisabled())
		{
			//factor in the tank steering velocities
			double AuxillerySpeed=0.0;
			{
				if (m_ship->GetAlterTrajectory())
					AuxillerySpeed=((m_LeftVelocity + m_RightVelocity) * 0.5) * m_ship->GetEngaged_Max_Speed();
				else
				{
					//Haha this is absolutely silly driving tank steering in slide mode, but it works
					m_Ship_JoyMouse_currAccel[1]+=((m_LeftVelocity + m_RightVelocity) * 0.5) * m_ship->GetAccelSpeed();
				}
				const double difference=(m_LeftVelocity + -m_RightVelocity);
				const double omega = (fabs(difference)>0.05)? difference * 0.5 : 0;
				m_Ship_JoyMouse_rotAcc_rad_s+=omega*m_ship->GetHeadingSpeed();
				//DOUT4("%f %f %f",m_LeftVelocity,m_RightVelocity,difference);
			}
			// Normally we pass the the ship the addition of the keyboard and mouse accel
			Vec2d shipAccel = m_Ship_Keyboard_currAccel+m_Ship_JoyMouse_currAccel;

			// apply various input sources to current acceleration
			if (m_ship->GetAlterTrajectory())
			{
				m_ShipKeyVelocity+=(shipAccel[1]*dTime_s);
				m_ship->SetRequestedVelocity(Vec2d(shipAccel[0],m_CruiseSpeed+AuxillerySpeed+m_ShipKeyVelocity)); //this will check implicitly for which mode to use
			}
			else
				m_ship->SetCurrentLinearAcceleration(shipAccel); 

			
			//flush the JoyMouse current acceleration vec2 since it works on an additive nature
			m_Ship_JoyMouse_currAccel=Vec2d(0.0,0.0);
		
		
			//add all the various input types to the main rotation velocity
			double rotAcc=(m_Ship_Keyboard_rotAcc_rad_s+m_Ship_JoyMouse_rotAcc_rad_s);
			//We may have same strange undesired flicker effect if the mouse and keyboard turns are used simultaneously! So if the keyboard is used, then
			//the mouse will get ignored
			bool LockShipHeadingToOrientation=m_Ship_UseHeadingSpeed;

			if (m_Ship_Keyboard_rotAcc_rad_s!=0) 
			{
				LockShipHeadingToOrientation=true;
				rotAcc=m_Ship_Keyboard_rotAcc_rad_s;
				m_Ship_UseHeadingSpeed=true;
			}

			m_ship->SetCurrentAngularAcceleration(rotAcc,LockShipHeadingToOrientation);

			//flush the JoyMouse rotation acceleration since it works on an additive nature
			m_Ship_JoyMouse_rotAcc_rad_s=0.0;
			m_Ship_JoyMouse_currAccel=Vec2d(0.0,0.0);
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
		Vec2d pos=m_ship->GetPos_m();
		DOUT(1,"x=%.2f y=%.2f r=%.2f",Meters2Feet(pos[0]),Meters2Feet(pos[1]),RAD_2_DEG(m_ship->GetAtt_r()));
		Vec2d Velocity=m_ship->GetLinearVelocity_ToDisplay();
		DOUT(3,"Vel[0]=%.2f Vel[1]=%.2f Rot=%.2f mode=%s",Meters2Feet(Velocity[0]),Meters2Feet(Velocity[1]),m_ship->GetAngularVelocity_ToDisplay(),m_ship->GetAlterTrajectory()?"Sim":"Slide");
		#endif
		#if 0
		Vec2d pos=m_ship->GetPos_m();
		DOUT1("x=%f y=%f r=%f",pos[0],pos[1],RAD_2_DEG(m_ship->GetAtt_r()));
		DOUT3("Speed=%f mode=%s",m_ship->GetPhysics().GetLinearVelocity().length(),m_ship->GetAlterTrajectory()?"Sim":"Slide");
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
}




