#pragma once

namespace GuardianVR
{
	namespace Base
	{

class GUARDIANVR_BASE_API Mech_Controller : public Mech_ControllerBase
{
	public:
		enum Directions
		{
			Dir_None = 0,
			Dir_Left = 1,
			Dir_Right = -1,
		};

		Mech_Controller(Mech& mech, bool canUserPilot);

		// virtual void ReadScriptParameters(Mech_TransmittedEntity& te);

		virtual void ResetPos();
		void UserResetPos();

		/*
		void WalkForward(bool on){if (AreControlsDisabled() && on) return; Mech_WalkForward(on);}
		void WalkBack(bool on){if (AreControlsDisabled() && on) return; Mech_WalkBack(on);}
		void Turn_R(bool on){if (AreControlsDisabled() && on) return; Mech_Turn(on?Dir_Right:Dir_None);}
		void Turn_L(bool on){if (AreControlsDisabled() && on) return; Mech_Turn(on?Dir_Left:Dir_None);}
		void StrafeLeft(bool on) {if (AreControlsDisabled() && on) return; Mech_StrafeLeft(on);}
		void StrafeRight(bool on) {if (AreControlsDisabled() && on) return; Mech_StrafeRight(on);}
		*/

		void TryToggleAutoPilot();

		// This may not always happen.  Some mechs can ONLY be in auto pilot
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

		void HookUpUI(bool ui);
		bool GetControlled(){return m_isControlled;}
		bool GetCanUserPilot();

		///This is called from the mech's time change (first before the mech does its physics)
		virtual void UpdateController(double dTime_s);

		///This is also called from the mech's time change (last after the physics have been applied)
		void UpdateUI(double dTime_s);

		virtual void CancelAllControls();

		///All non-right button mouse movements will come here to be dispatched to the proper place
		void Mouse_Turn(double dir);
		void Mouse_Pitch(double dir);

		// Read the parameters from the script
		void ReadScriptParameters(Mech_TransmittedEntity& mte) {}

	protected:
		void POV_Turn(double dir,bool UseKeyboard=true);
		void POV_Pitch(double dir,bool UseKeyboard=true);
		void POV_PitchTurn(double Pitchdir,double TurnDir,bool UseKeyboard=true) {POV_Turn(TurnDir,UseKeyboard),POV_Pitch(Pitchdir,UseKeyboard);}


		/// \param dir When UseHeadingSpeed=true this is a velocity scaler applied to the scripted turning speed
		/// When UseHeadingSpeed=false the dir parameter becomes the actual velocity in radians per second.
		/// \param UseHeadingSpeed Typically the Joystick and Keyboard will set UseHeadingSpeed to true, and mouse will be false
		/// when this is true it also implicitly applies the mechs torque restraints to the intended speed.
		void Mech_Turn(double dir,bool UseHeadingSpeed=true);
		void Mech_Pitch(double dir,bool UsePitchSpeed=true);

		void Mech_Turn(Directions dir);

		/*
		void Mech_WalkForward(bool on)	{	m_Mech_Keyboard_currAccel[1] = on ? m_mech.ACCEL : 0.0;	}
		void Mech_WalkBack(bool on)	    {	m_Mech_Keyboard_currAccel[1] = on ? -m_mech.BRAKE : 0.0;}

		void Mech_WalkForward(double Intensity)	{	m_Mech_JoyMouse_currAccel[1] = m_mech.ACCEL*Intensity;	}
		void Mech_WalkBack(double Intensity)	{	m_Mech_JoyMouse_currAccel[1] = -m_mech.BRAKE*Intensity;	}

		void Mech_StrafeLeft(double Intensity)	{		m_Mech_JoyMouse_currAccel[0]=  -m_mech.STRAFE*Intensity;	}
		void Mech_StrafeRight(double Intensity)	{		m_Mech_JoyMouse_currAccel[0]=  m_mech.STRAFE*Intensity;	}
		*/

	private:
		GG_Framework::AppReuse::ChasePlane_CamManipulator m_chasePlaneCamManip;
		Mouse_MechDriver m_mouseDriver;

		bool m_isControlled;

		///This is used exclusively for keyboard turn methods
		osg::Vec3d m_Mech_Keyboard_rotVel_rad_s;

		///This one is used exclusively for the Joystick and Mouse turn methods
		osg::Vec3d m_Mech_JoyMouse_rotVel_rad_s;
		osg::Vec3d m_Mech_Keyboard_currAccel,m_Mech_JoyMouse_currAccel;
		void UpdateIntendedOrientaton(double dTime_s);

		//This allow Joystick Mouse and Keyboard integration
		bool m_UseMousePOV;
		osg::Vec3d m_POV_rad_s;
		osg::Vec3d m_POV_keyboard_rad_s; //This is what the keyboard uses

		// Are we flying in auto-pilot?
		bool m_autoPilot;

		// Some mechs cannot be controlled at ALL, and are ALWAYS in auto-pilot
		bool m_canUserPilot;

		// Are we disabling UI controls?
		bool AreControlsDisabled();

		// Use the speeds provided in the script
		bool m_Mech_UseHeadingSpeed;
		bool m_Mech_UsePitchSpeed;
};

	}
}