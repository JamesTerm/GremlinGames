#pragma once

namespace Fringe
{
	namespace Base
	{

class FRINGE_BASE_API UI_Controller
{
	public:
		enum Directions
		{
			Dir_None = 0,

			Dir_Up = -1,
			Dir_Down = 1,

			Dir_Left = 1,
			Dir_Right = -1,

			Dir_CW = 1,
			Dir_CCW = -1,
		};

		UI_Controller(AI_Base_Controller *base_controller=NULL);
		virtual ~UI_Controller();

		///This is the most important method, as we must have a ship to control
		void Set_AI_Base_Controller(AI_Base_Controller *controller);
		void TryFireMainWeapon(bool on)
		{
			if (AreControlsDisabled() && on) return;
			m_ship->GetEventMap()->EventOnOff_Map["Ship.TryFireMainWeapon"].Fire(on);
		}

		void AfterBurner_Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_AfterBurner_Thrust(on);}
		void Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_Thrust(on);}
		void Slider_Accel(double Intensity);
		void Brake(bool on){if (AreControlsDisabled() && on) return; Ship_Brake(on);}
		void Stop() {if (AreControlsDisabled()) return; m_ship->Stop();}
		void MatchSpeed(double speed) {if (AreControlsDisabled()) return; m_ship->SetRequestedSpeed(speed);}
		void Turn_R(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Right:Dir_None);}
		void Turn_L(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Left:Dir_None);}
		void Roll_CW(bool on){if (AreControlsDisabled() && on) return; Ship_Roll(on?Dir_CW:Dir_None);}
		void Roll_CCW(bool on){if (AreControlsDisabled() && on) return; Ship_Roll(on?Dir_CCW:Dir_None);}
		void Pitch_Up(bool on){if (AreControlsDisabled() && on) return; Ship_Pitch(on?Dir_Up:Dir_None);}
		void Pitch_Dn(bool on){if (AreControlsDisabled() && on) return; Ship_Pitch(on?Dir_Down:Dir_None);}
		virtual void ResetPos();
		void UserResetPos();
		void ToggleSlide() {if (AreControlsDisabled()) return; m_ship->SetSimFlightMode(!m_ship->GetAlterTrajectory());}
		void ToggleAutoLevel();
		void StrafeUp(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeUp(on);}
		void StrafeUp(double dir) {if (!AreControlsDisabled()) Ship_StrafeUp(dir); }
		void StrafeDown(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeDown(on);}
		void StrafeDown(double dir) {if (!AreControlsDisabled()) Ship_StrafeDown(dir); }
		void StrafeLeft(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeLeft(on);}
		void StrafeLeft(double dir) {if (!AreControlsDisabled()) Ship_StrafeLeft(dir); }
		void StrafeRight(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeRight(on);}
		void StrafeRight(double dir) {if (!AreControlsDisabled()) Ship_StrafeRight(dir); }
		void Slider_StrafeDown(double Intensity);

		void TryToggleAutoPilot();

		// This may not always happen.  Some ships can ONLY be in auto pilot
		bool SetAutoPilot(bool autoPilot);
		bool GetAutoPilot(){return m_autoPilot;}

		void ToggleMousePOV(bool on);
		void POV_Down(bool on) {on?POV_Pitch(1.0):POV_Pitch(0.0);}
		void POV_DL(bool on) {on?POV_PitchTurn(1.0,1.0):POV_PitchTurn(0.0,0.0);}
		void POV_DR(bool on) {on?POV_PitchTurn(1.0,-1.0):POV_PitchTurn(0.0,0.0);}
		void POV_Up(bool on) {on?POV_Pitch(-1.0):POV_Pitch(0.0);}
		void POV_UL(bool on) {on?POV_PitchTurn(-1.0,1.0):POV_PitchTurn(0.0,0.0);}
		void POV_UR(bool on) {on?POV_PitchTurn(-1.0,-1.0):POV_PitchTurn(0.0,0.0);}
		void POV_Left(bool on) {on?POV_Turn(1.0):POV_Turn(0.0);}
		void POV_Right(bool on) {on?POV_Turn(-1.0):POV_Turn(0.0);}
		void OnSpawn(bool on);

		void Test1(bool on);
		void Test2(bool on);

		osg::Geometry* MakeVelLine(osg::Vec3 vel);
		void HookUpUI(bool ui);
		bool GetControlled(){return m_isControlled;}

		///This is called from the ship's time change (first before the ship does its physics)
		virtual void UpdateController(double dTime_s);
		void UpdateUI(double dTime_s);

		virtual void CancelAllControls();

	protected:
		friend Mouse_ShipDriver;

		void BlackoutHandler(double bl);

		void POV_Turn(double dir,bool UseKeyboard=true);
		void POV_Pitch(double dir,bool UseKeyboard=true);
		void POV_PitchTurn(double Pitchdir,double TurnDir,bool UseKeyboard=true) {POV_Turn(TurnDir,UseKeyboard),POV_Pitch(Pitchdir,UseKeyboard);}
		void Joystick_POV(double Degrees);

		///All non-right button mouse movements will come here to be dispatched to the proper place
		void Mouse_Turn(double dir);
		void Mouse_Pitch(double dir);
		void Mouse_Roll(double dir);

		/// \param dir When UseHeadingSpeed=true this is a velocity scaler applied to the scripted turning speed.
		/// When UseHeadingSpeed=false the dir parameter becomes the actual velocity in radians per second.
		/// \param UseHeadingSpeed Typically the Joystick and Keyboard will set UseHeadingSpeed to true, and mouse will be false
		/// when this is true it also implicitly applies the ships torque restraints to the intended speed.
		/// \todo implement these to work with intended orientation quaternion.
		void Ship_Turn(double dir,bool UseHeadingSpeed=true);
		void Ship_Pitch(double dir,bool UsePitchSpeed=true);
		void Ship_Roll(double dir,bool UseRollSpeed=true);

		void JoyStick_Ship_Turn(double dir);
		void JoyStick_Ship_Pitch(double dir);

		void Ship_Turn(Directions dir);
		void Ship_Pitch(Directions dir);
		void Ship_Roll(Directions dir);

		void Ship_AfterBurner_Thrust(bool on);
		void Ship_Thrust(bool on);
		void Ship_Brake(bool on);

		void Ship_Thrust(double Intensity);
		void Ship_Brake(double Intensity);

		void Joystick_SetCurrentSpeed(double Speed);

		void Ship_StrafeUp(bool on)		{		m_Ship_Keyboard_currAccel[2]= on? m_ship->STRAFE : 0.0;	}
		void Ship_StrafeDown(bool on)	{		m_Ship_Keyboard_currAccel[2]= on? -m_ship->STRAFE : 0.0;	}
		void Ship_StrafeLeft(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? -m_ship->STRAFE : 0.0;	}
		void Ship_StrafeRight(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? m_ship->STRAFE : 0.0;	}

		void Ship_StrafeUp(double Intensity);
		void Ship_StrafeDown(double Intensity);
		void Ship_StrafeLeft(double Intensity);
		void Ship_StrafeRight(double Intensity);

		IEvent::HandlerList ehl;
	private:
		void Flush_AI_BaseResources(); //i.e. chase plane and mouse driver
		void Init_AutoPilotControls();
		AI_Base_Controller *m_Base;
		ThrustShip2 *m_ship; //there is an overwhelming use of the ship so we'll cache a pointer of it here
		osg::ref_ptr<HUD_PDCB> m_HUD_UI;
		GG_Framework::AppReuse::ChasePlane_CamManipulator *m_chasePlaneCamManip;
		Mouse_ShipDriver *m_mouseDriver;

		double m_LastSliderTime[2]; //Keep track of the slider to help it stay smooth;
		bool m_isControlled;

		///This is used exclusively for keyboard turn methods
		osg::Vec3d m_Ship_Keyboard_rotVel_rad_s;
		///This one is used exclusively for the Joystick and Mouse turn methods
		osg::Vec3d m_Ship_JoyMouse_rotVel_rad_s;
		osg::Vec3d m_Ship_Keyboard_currAccel,m_Ship_JoyMouse_currAccel;

		void ConnectHUD_Elements(bool connect);
		bool m_hud_connected;


		//I have to monitor when it is down then up
		bool m_SlideButtonToggle;
		bool m_FireButton;
		double m_CruiseSpeed; ///< This is used with the Joystick control to only apply speed changes when a change occurs

		// Build the various HUD elements
		void BuildHUD();
		bool m_targetLeadRetShowing;
		bool m_targetInRangeLeadRetShowing;

		osg::ref_ptr<GG_Framework::UI::ActorTransform> m_shipSiteRetical;
		osg::ref_ptr<osg::Geode> m_tgtDirLineGeode;

		osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_targetOutRangeLeadRetical;
		osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_targetInRangeLeadRetical;
		osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_tgtDirLineTransform;

		//This allow Joystick Mouse and Keyboard integration
		bool m_UseMousePOV;
		osg::Vec3d m_POV_rad_s;
		osg::Vec3d m_POV_keyboard_rad_s; //This is what the keyboard uses

		// Are we flying in auto-pilot?
		bool m_autoPilot;
		bool m_enableAutoLevelWhenPiloting;

		// Are we disabling UI controls?
		bool AreControlsDisabled();

		bool m_Ship_UseHeadingSpeed;
		bool m_Ship_UsePitchSpeed;
		bool m_Ship_UseRollSpeed;

		bool m_Test1,m_Test2; //Testing

};

	}
}