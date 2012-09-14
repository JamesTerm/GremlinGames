#pragma once

class Mouse_ShipDriver
{
public:
	Mouse_ShipDriver(Ship_2D& ship,UI_Controller *parent, unsigned avgFrames);
	void OnMouseMove(float mx, float my);
	void DriveShip();

private:
	// Use this handler to tie events to this manipulator
	IEvent::HandlerList ehl;
	void OnMouseRoll(bool onoff){m_mouseRoll = onoff;}

	Ship_2D& m_ship;
	UI_Controller * const m_ParentUI_Controller;
	osg::Vec2f m_lastMousePos;
	osg::Vec2f* m_mousePosHist;
	unsigned m_avgFrames;
	unsigned m_currFrame;
	bool m_mouseRoll;
};

class UI_Controller
{
	public:
		enum Directions
		{
			Dir_None = 0,
			//Reversed from game since the 2D world uses the topview axis
			Dir_Left = -1,
			Dir_Right = 1,
		};

		UI_Controller(AI_Base_Controller *base_controller=NULL,bool AddJoystickDefaults=true);
		virtual ~UI_Controller();

		///This is the most important method, as we must have a ship to control
		void Set_AI_Base_Controller(AI_Base_Controller *controller);
		void TryFireMainWeapon(bool on)
		{
			if (AreControlsDisabled() && on) return;
			//m_ship->GetEventMap()->EventOnOff_Map["Ship.TryFireMainWeapon"].Fire(on);
		}

		void AfterBurner_Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_AfterBurner_Thrust(on);}
		void Thrust(bool on){if (AreControlsDisabled() && on) return; Ship_Thrust(on);}
		void Slider_Accel(double Intensity);
		void Brake(bool on){if (AreControlsDisabled() && on) return; Ship_Brake(on);}
		void Stop() {if (AreControlsDisabled()) return; m_ship->Stop();}
		void MatchSpeed(double speed) {if (AreControlsDisabled()) return; m_ship->SetRequestedVelocity(speed);}
		void Turn_R(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Right:Dir_None);}
		void Turn_L(bool on){if (AreControlsDisabled() && on) return; Ship_Turn(on?Dir_Left:Dir_None);}
		virtual void ResetPos();
		void UserResetPos();
		void SlideHold(bool holdslide) {if (AreControlsDisabled()) return; m_ship->SetSimFlightMode(!holdslide);}
		void ToggleSlide() {if (AreControlsDisabled()) return; m_ship->SetSimFlightMode(!m_ship->GetAlterTrajectory());}
		void ToggleAutoLevel();
		void StrafeLeft(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeLeft(on);}
		void StrafeLeft(double dir) {if (!AreControlsDisabled()) Ship_StrafeLeft(dir); }
		void StrafeRight(bool on) {if (AreControlsDisabled() && on) return; Ship_StrafeRight(on);}
		void StrafeRight(double dir) {if (!AreControlsDisabled()) Ship_StrafeRight(dir); }

		void TryToggleAutoPilot();

		// This may not always happen.  Some ships can ONLY be in auto pilot
		bool SetAutoPilot(bool autoPilot);
		bool GetAutoPilot(){return m_autoPilot;}

		void OnSpawn(bool on);

		void UseMouse();
		void Test1(bool on);
		void Test2(bool on);

		//osg::Geometry* MakeVelLine(osg::Vec2 vel);
		void HookUpUI(bool ui);
		bool GetControlled(){return m_isControlled;}

		///This is called from the ship's time change (first before the ship does its physics)
		virtual void UpdateController(double dTime_s);
		void UpdateUI(double dTime_s);

		virtual void CancelAllControls();

		//returns NULL if no error
		struct Controller_Element_Properties
		{
			std::string Event;
			enum ElementType
			{
				eJoystickAnalog,
				eJoystickButton
			} Type;
			union ElementTypeSpecific
			{
				struct AnalogSpecifics_rw
				{
					GG_Framework::UI::JoyStick_Binder::JoyAxis_enum JoyAxis;
					bool IsFlipped;
					double Multiplier;
					double FilterRange;
					double CurveIntensity;
				} Analog;
				struct ButtonSpecifics_rw
				{
					size_t WhichButton;
					bool useOnOff;
					bool dbl_click;
				} Button;
			} Specifics;
		};
		//Return if element was successfully created (be sure to check as some may not be present)
		static const char *ExtractControllerElementProperties(Controller_Element_Properties &Element,const char *Eventname,GG_Framework::Logic::Scripting::Script& script);
		GG_Framework::UI::JoyStick_Binder &GetJoyStickBinder();
	protected:
		friend Mouse_ShipDriver;

		void BlackoutHandler(double bl);

		///All non-right button mouse movements will come here to be dispatched to the proper place
		void Mouse_Turn(double dir);

		/// \param dir When UseHeadingSpeed=true this is a velocity scaler applied to the scripted turning speed.
		/// When UseHeadingSpeed=false the dir parameter becomes the actual velocity in radians per second.
		/// \param UseHeadingSpeed Typically the Joystick and Keyboard will set UseHeadingSpeed to true, and mouse will be false
		/// when this is true it also implicitly applies the ships torque restraints to the intended speed.
		/// \todo implement these to work with intended orientation quaternion.
		void Ship_Turn(double dir,bool UseHeadingSpeed=true);

		void JoyStick_Ship_Turn(double dir);

		void Ship_Turn(Directions dir);

		void Ship_AfterBurner_Thrust(bool on);
		void Ship_Thrust(bool on);
		void Ship_Brake(bool on);

		void Ship_Thrust(double Intensity);
		void Ship_Brake(double Intensity);

		void Joystick_SetCurrentSpeed(double Speed);
		void Joystick_SetCurrentSpeed_2(double Speed);

		void Ship_StrafeLeft(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? -m_ship->GetStrafeSpeed() : 0.0;	}
		void Ship_StrafeRight(bool on)	{		m_Ship_Keyboard_currAccel[0]= on? m_ship->GetStrafeSpeed() : 0.0;	}

		void Ship_StrafeLeft(double Intensity);
		void Ship_StrafeRight(double Intensity);

		IEvent::HandlerList ehl;
	private:
		void Flush_AI_BaseResources(); //i.e. chase plane and mouse driver
		void Init_AutoPilotControls();
		AI_Base_Controller *m_Base;
		Ship_2D *m_ship; //there is an overwhelming use of the ship so we'll cache a pointer of it here
		//osg::ref_ptr<HUD_PDCB> m_HUD_UI;
		Mouse_ShipDriver *m_mouseDriver;

		double m_LastSliderTime[2]; //Keep track of the slider to help it stay smooth;
		bool m_isControlled;

		///This is used exclusively for keyboard turn methods
		double m_Ship_Keyboard_rotAcc_rad_s;
		///This one is used exclusively for the Joystick and Mouse turn methods
		double m_Ship_JoyMouse_rotAcc_rad_s;
		osg::Vec2d m_Ship_Keyboard_currAccel,m_Ship_JoyMouse_currAccel;

		//void ConnectHUD_Elements(bool connect);
		//bool m_hud_connected;


		//I have to monitor when it is down then up
		bool m_SlideButtonToggle;
		bool m_FireButton;
		double m_CruiseSpeed; ///< This is used with the Joystick control to only apply speed changes when a change occurs

		// Build the various HUD elements
		//void BuildHUD();
		//bool m_targetLeadRetShowing;
		//bool m_targetInRangeLeadRetShowing;

		//osg::ref_ptr<GG_Framework::UI::ActorTransform> m_shipSiteRetical;
		//osg::ref_ptr<osg::Geode> m_tgtDirLineGeode;

		//osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_targetOutRangeLeadRetical;
		//osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_targetInRangeLeadRetical;
		//osg::ref_ptr<GG_Framework::UI::OSG::ThreadUpdatedPosAttTransform> m_tgtDirLineTransform;

		// Are we flying in auto-pilot?
		bool m_autoPilot;
		bool m_enableAutoLevelWhenPiloting;

		// Are we disabling UI controls?
		bool AreControlsDisabled();

		bool m_Ship_UseHeadingSpeed;
		bool m_Test1,m_Test2; //Testing

};


class UI_Controller_GameClient : public UI_GameClient
{
	public:
		UI_Controller_GameClient();
		~UI_Controller_GameClient();
		virtual void SetControlledEntity(Entity2D* newEntity,bool AddJoystickDefaults=true);
		virtual void AboutTo_RemoveEntity(Entity2D *Entity) {if (Entity==m_controlledEntity) SetControlledEntity(NULL);}
	private:
		//The one, the only!
		UI_Controller *m_UI_Controller;  //unfortunately this is late binding once the window is setup
		Entity2D* m_controlledEntity;
};