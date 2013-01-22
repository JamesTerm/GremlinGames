#include "../Base/Base_Includes.h"
#include <math.h>
#include <assert.h>
#include "../Base/Vec2d.h"
#include "../Base/Misc.h"
#include "../Base/Event.h"
#include "../Base/EventMap.h"
#include "../Base/Script.h"
#include "Entity_Properties.h"
#include "Physics_1D.h"
#include "Physics_2D.h"
#include "Entity2D.h"
#include "Goal.h"
#include "Ship.h"
#include "AI_Base_Controller.h"
#include "../Base/Joystick.h"
#include "../Base/JoystickBinder.h"
#include "UI_Controller.h"
#include "Debug.h"

#undef __EnableTestKeys__

using namespace Framework::Base;
using namespace Framework::UI;

namespace Scripting=Framework::Scripting;
const double Half_Pi=M_PI/2.0;


  /***************************************************************************************************************/
 /*												UI_Controller													*/
/***************************************************************************************************************/


void UI_Controller::Init_AutoPilotControls()
{
}


//! TODO: Use the script to grab the head position to provide the HUD
UI_Controller::UI_Controller(JoyStick_Binder &joy,AI_Base_Controller *base_controller) : 
	/*m_HUD_UI(new HUD_PDCB(osg::Vec3(0.0, 4.0, 0.5))), */
	m_Base(NULL),m_JoyStick_Binder(joy),m_isControlled(false),m_ShipKeyVelocity(0.0),m_SlideButtonToggle(false),m_CruiseSpeed(0.0),
	m_autoPilot(true),m_enableAutoLevelWhenPiloting(false),m_Ship_UseHeadingSpeed(true),m_Test1(false),m_Test2(false),m_IsBeingDestroyed(false),
	m_POVSetValve(false)
{
	ResetPos();
	Set_AI_Base_Controller(base_controller); //set up ship (even if we don't have one)
	m_LastSliderTime[0]=m_LastSliderTime[1]=0.0;

	//No longer putting controls here as they may change depending on what controller is used (e.g. deadzone tweaks)
	#if 0
	//Note: Only generic default keys assigned for windriver builds.  Others are moved to the specific robot
	//We may wish to move these as well, but for now assert this is generic enough for all robots, which will come in
	//handy if we want to change the driving style (e.g. arcade, tank steering, etc)
	joy.AddJoy_Analog_Default(JoyStick_Binder::eY_Axis,"Joystick_SetCurrentSpeed_2",true,1.0,0.1,false,"Joystick_1");
	joy.AddJoy_Analog_Default(JoyStick_Binder::eX_Axis,"Analog_Turn",false,1.0,0.1,true,"Joystick_1");
	#endif
	
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
	m_ship=NULL; //we don't own this
}

UI_Controller::~UI_Controller()
{
	Set_AI_Base_Controller(NULL); //this will unbind the events and flush the AI resources
}

Framework::UI::JoyStick_Binder &UI_Controller::GetJoyStickBinder()
{
	return m_JoyStick_Binder;
}

void UI_Controller::Set_AI_Base_Controller(AI_Base_Controller *controller)
{
	//destroy all resources associated with the previous ship
	if (m_Base)
	{
		Framework::Base::EventMap* em = m_ship->GetEventMap();
		//disabled until it works
		//em->EventOnOff_Map["RequestAfterburner"].Remove(*this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Remove(*this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Remove(*this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Remove(*this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Remove(*this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Remove(*this, &UI_Controller::Turn_L);
		em->Event_Map["Turn_180"].Remove(*this, &UI_Controller::Turn_180);
		em->Event_Map["UserResetPos"].Remove(*this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Remove(*this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Remove(*this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["SlideHold"].Remove(*this, &UI_Controller::SlideHold);
		em->EventOnOff_Map["StrafeLeft"].Remove(*this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Remove(*this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Remove(*this, &UI_Controller::OnSpawn);
		//em->Event_Map["UseMouse"].Remove(*this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Remove(*this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Remove(*this, &UI_Controller::Test2);
		//em->Event_Map["ShowHUD"].Remove(*m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);
		em->EventValue_Map["BLACKOUT"].Remove(*this, &UI_Controller::BlackoutHandler);

		em->EventValue_Map["POV_Turn"].Remove(*this, &UI_Controller::Ship_Turn90_POV);
		em->EventValue_Map["Analog_Turn"].Remove(*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Remove(*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Remove(*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Remove(*this, &UI_Controller::Joystick_SetCurrentSpeed_2);
		m_ship->BindAdditionalEventControls(false);
		if (!m_IsBeingDestroyed)
			m_ship->BindAdditionalUIControls(false,&GetJoyStickBinder());
		Flush_AI_BaseResources();
	}
	m_Base=controller;
	if (m_Base)
	{
		m_ship=&m_Base->m_ship;
		Framework::Base::EventMap* em = m_ship->GetEventMap();

		//disabled until it works
		//em->EventOnOff_Map["RequestAfterburner"].Subscribe(ehl, *this, &UI_Controller::AfterBurner_Thrust);
		em->EventOnOff_Map["Thrust"].Subscribe(ehl, *this, &UI_Controller::Thrust);
		em->EventOnOff_Map["Brake"].Subscribe(ehl, *this, &UI_Controller::Brake);
		em->Event_Map["Stop"].Subscribe(ehl, *this, &UI_Controller::Stop);
		em->EventOnOff_Map["Turn_R"].Subscribe(ehl, *this, &UI_Controller::Turn_R);
		em->EventOnOff_Map["Turn_L"].Subscribe(ehl, *this, &UI_Controller::Turn_L);
		em->Event_Map["Turn_180"].Subscribe(ehl, *this, &UI_Controller::Turn_180);
		em->Event_Map["UserResetPos"].Subscribe(ehl, *this, &UI_Controller::UserResetPos);
		em->Event_Map["ResetPos"].Subscribe(ehl, *this, &UI_Controller::ResetPos);
		em->Event_Map["Slide"].Subscribe(ehl, *this, &UI_Controller::ToggleSlide);
		em->EventOnOff_Map["SlideHold"].Subscribe(ehl, *this, &UI_Controller::SlideHold);
		em->EventOnOff_Map["StrafeLeft"].Subscribe(ehl, *this, &UI_Controller::StrafeLeft);
		em->EventOnOff_Map["StrafeRight"].Subscribe(ehl, *this, &UI_Controller::StrafeRight);
		em->Event_Map["ToggleAutoPilot"].Subscribe(ehl, *this, &UI_Controller::TryToggleAutoPilot);
		em->EventOnOff_Map["SPAWN"].Subscribe(ehl, *this, &UI_Controller::OnSpawn);

		//em->Event_Map["UseMouse"].Subscribe(ehl, *this, &UI_Controller::UseMouse);
		em->EventOnOff_Map["Test1"].Subscribe(ehl, *this, &UI_Controller::Test1);
		em->EventOnOff_Map["Test2"].Subscribe(ehl, *this, &UI_Controller::Test2);

		// And a button for the Speed PDCB
		//em->Event_Map["ShowHUD"].Subscribe(m_HUD_UI->ehl, *m_HUD_UI.get(), &HUD_PDCB::ToggleEnabled);

		// Listen for blackout
		em->EventValue_Map["BLACKOUT"].Subscribe(ehl, *this, &UI_Controller::BlackoutHandler);

		em->EventValue_Map["POV_Turn"].Subscribe(ehl,*this, &UI_Controller::Ship_Turn90_POV);
		em->EventValue_Map["Analog_Turn"].Subscribe(ehl,*this, &UI_Controller::JoyStick_Ship_Turn);
		em->EventValue_Map["Analog_StrafeRight"].Subscribe(ehl,*this, &UI_Controller::StrafeRight);
		em->EventValue_Map["Analog_Slider_Accel"].Subscribe(ehl,*this, &UI_Controller::Slider_Accel);
		em->EventValue_Map["Joystick_SetCurrentSpeed"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed);
		em->EventValue_Map["Joystick_SetCurrentSpeed_2"].Subscribe(ehl,*this, &UI_Controller::Joystick_SetCurrentSpeed_2);

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
	switch (dir)
	{
		case Dir_None:
		case Dir_Left:
		case Dir_Right:
			m_Ship_Keyboard_rotAcc_rad_s=(double)dir*m_ship->GetHeadingSpeed()*m_ship->GetCameraRestraintScaler();
			m_Ship_UseHeadingSpeed=true;
			break;
		case Dir_90Left:
			m_ship->SetIntendedOrientation(-Half_Pi,false);
			m_Ship_UseHeadingSpeed=false;
			break;
		case Dir_90Right:
			m_ship->SetIntendedOrientation(Half_Pi,false);
			m_Ship_UseHeadingSpeed=false;
			break;
		case Dir_180:
			m_ship->SetIntendedOrientation(Pi,false);
			m_Ship_UseHeadingSpeed=false;
			break;
	}
}

void UI_Controller::Turn_RelativeOffset(double value) 
{
	if (AreControlsDisabled()) return;
	m_ship->SetIntendedOrientation(value,false);
	m_Ship_UseHeadingSpeed=false;
}

void UI_Controller::Ship_Turn90_POV (double value)
{
	//We put the typical case first (save the amount of branching)
	if (value!=-1)
	{
		if (!m_POVSetValve)
		{
			m_POVSetValve=true;
			m_ship->SetIntendedOrientation(DEG_2_RAD(value),false);
			m_Ship_UseHeadingSpeed=false;
		}
	}
	else 
		m_POVSetValve=false;
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
		//m_mouseDriver->DriveShip();

		//Now for the ship
		if (!AreControlsDisabled())
		{
			//factor in the auxiliary control velocities
			double AuxiliaryVelocity=0.0;
			{
				Vec2d AuxLinearAcceleration=Vec2d(0.0,0.0);
				double AuxAngularAcceleration=0.0;
				m_ship->UpdateController(AuxiliaryVelocity,AuxLinearAcceleration,AuxAngularAcceleration,m_Ship_UseHeadingSpeed,dTime_s);
				m_Ship_JoyMouse_currAccel+=AuxLinearAcceleration;
				m_Ship_JoyMouse_rotAcc_rad_s+=AuxAngularAcceleration;
			}

			// Normally we pass the the ship the addition of the keyboard and mouse accel
			Vec2d shipAccel = m_Ship_Keyboard_currAccel+m_Ship_JoyMouse_currAccel;

			// apply various input sources to current acceleration
			if (m_ship->GetAlterTrajectory())
			{
				m_ShipKeyVelocity+=(shipAccel[1]*dTime_s);
				m_ship->SetRequestedVelocity(Vec2d(shipAccel[0],m_CruiseSpeed+AuxiliaryVelocity+m_ShipKeyVelocity)); //this will check implicitly for which mode to use
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
		//DebugOut_PDCB::TEXT2 = BuildString("%s", ThrustStateNames[m_ship->GetThrustState()]);
	}
}

//////////////////////////////////////////////////////////////////////////

void UI_Controller::HookUpUI(bool ui)
{
	m_isControlled = ui;
	//GG_Framework::UI::MainWindow& mainWin = *GG_Framework::UI::MainWindow::GetMainWindow();
	if (m_isControlled)
	{
		//Note: This is handled explicitly in the main; however, I may want to add a member here... probably not since I do not wish to deviate from
		//the original source
		//mainWin.GetJoystick().SetControlledEventMap(m_ship->GetEventMap());
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




