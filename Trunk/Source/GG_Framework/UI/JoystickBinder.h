#pragma once

namespace GG_Framework
{
	namespace UI
	{

class FRAMEWORK_UI_API JoyStick_Binder : public ConfigLoadSaveInterface
{
public:
	//grant access to the Joystick interface to obtain info like number of joysticks and capabilities
	GG_Framework::Base::IJoystick &GetJoystick() const;

	enum JoyAxis_enum
	{
		eX_Axis,
		eY_Axis,
		eZ_Axis,
		eX_Rot,
		eY_Rot,
		eZ_Rot,
		eSlider0,
		eSlider1,
		ePOV_0,
		ePOV_1,
		ePOV_2,
		ePOV_3,
		eNoJoyAxis_Entries
	};

	JoyStick_Binder( ConfigurationManager *config);
	~JoyStick_Binder();
	///This binds all the axis, rotations, sliders to an event
	/// \param IsFlipped this will simply multiply a -1.0 coefficient
	void AddJoy_Analog_Default(JoyAxis_enum WhichAxis,const char eventName[],bool IsFlipped=false,double Multiplier=1.0,double FilterRange=0.0,
		double CurveIntensity=0.0,const char ProductName[]="any");
	/// \param WhichButton while in theory there are up to 128 buttons supported I'm only going to support the first 32 for now
	/// Use the JoystickTest program to determine the numbers of the buttons
	void AddJoy_Button_Default(size_t WhichButton,const char eventName[],bool useOnOff=true,bool dbl_click=false,const char ProductName[]="any");
	//TODO see if I really need RemoveJoy_x_Binding methods


	// This is here because producer's update is not virtual
	void UpdateJoyStick(double dTick_s);

	//TODO get with Rick to see if I need to fire events like keyboard for this mutator.  I know that a simple assignment is adequate for the current
	//stresses presented 
	//  [2/5/2010 JamesK]
	void SetControlledEventMap(GG_Framework::UI::EventMap* em);
	//Event3<JoyStick_Binder*, GG_Framework::UI::EventMap*, GG_Framework::UI::EventMap*> EventMapChanged; //!< <this, old, new> fired before change

protected: // From config load save interface.
	virtual void BindToPrefs(const XMLNode &userPrefsNode);
	/// Only called when writing other settings
	virtual void WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes);
	virtual const char *GetConfigGroupName() {return "UserInput";}

private:

	void AddJoy_Analog_Binding(JoyAxis_enum WhichAxis,const char eventName[],bool IsFlipped=false,double Multiplier=1.0,double FilterRange=0.0,
		double CurveIntensity=0.0,const char ProductName[]="any");
	void AddJoy_Button_Binding(size_t WhichButton,const char eventName[],bool useOnOff=true,bool dbl_click=false,const char ProductName[]="any");

	struct EventEntry_Base
	{
		EventEntry_Base(const char _ProductName[]) : ProductName(_ProductName) {}
		std::string ProductName;	//Default is "any"    Use Joystick test to find this if you configure multiple joysticks
		//std::string InstanceName;	//Default is "any"    Use Joystick test to find this if you configure multiple joysticks
	};
	//TODO support instance name for the > = operators, and allow any to pass test
	struct Analog_EventEntry : public EventEntry_Base
	{
		Analog_EventEntry(JoyAxis_enum _WhichAxis,const char _ProductName[]="any",bool _IsFlipped=false,double _Multiplier=1.0,
			double _FilterRange=0.0,double _CurveIntensity=false) : 
		WhichAxis(_WhichAxis),IsFlipped(_IsFlipped),EventEntry_Base(_ProductName),Multiplier(_Multiplier),
			FilterRange(_FilterRange),CurveIntensity(_CurveIntensity)
		{}

		JoyAxis_enum WhichAxis;
		double Multiplier;
		double FilterRange;
		double CurveIntensity;
		bool IsFlipped;
		bool operator >  (const Analog_EventEntry& rhs) const { return ((WhichAxis == rhs.WhichAxis) ? (ProductName > rhs.ProductName) : (WhichAxis > rhs.WhichAxis)); }
		bool operator == (const Analog_EventEntry& rhs) const { return (WhichAxis == rhs.WhichAxis) && (ProductName == rhs.ProductName); }
	};
	struct Button_EventEntry : public EventEntry_Base
	{
		Button_EventEntry(size_t _WhichButton,const char _ProductName[]="any",bool _useOnOff=true,bool _dbl_click=false) : 
	WhichButton(_WhichButton),EventEntry_Base(_ProductName),useOnOff(_useOnOff),dbl_click(_dbl_click)
	{}

	size_t WhichButton;
	bool useOnOff;
	bool dbl_click;
	bool operator >  (const Button_EventEntry& rhs) const { return ((WhichButton == rhs.WhichButton) ? 
		ProductName==rhs.ProductName ? 
		(dbl_click > rhs.dbl_click) : (ProductName > rhs.ProductName) : 
	(WhichButton > rhs.WhichButton)); }
	bool operator == (const Button_EventEntry& rhs) const { return (WhichButton == rhs.WhichButton) && (ProductName == rhs.ProductName) && (dbl_click == rhs.dbl_click); }
	};

	std::vector<std::string> *GetBindingsForJoyAnalog(Analog_EventEntry EventEntry) {return m_JoyAnalogBindings[EventEntry];}
	std::vector<std::string> *GetBindingsForJoyButton(Button_EventEntry EventEntry) {return m_JoyButtonBindings[EventEntry];}
	bool IsDoubleClicked(size_t i);

	typedef std::map<std::string, std::vector<Analog_EventEntry>*, std::greater<std::string> > AssignedJoyAnalogs;
	typedef std::map<std::string, std::vector<Button_EventEntry>*, std::greater<std::string> > AssignedJoyButtons;
	typedef std::map<Analog_EventEntry, std::vector<std::string>*, std::greater<Analog_EventEntry> > JoyAnalogBindings;
	typedef std::map<Button_EventEntry, std::vector<std::string>*, std::greater<Button_EventEntry> > JoyButtonBindings;
	JoyAnalogBindings m_JoyAnalogBindings;
	AssignedJoyAnalogs m_AssignedJoyAnalogs;
	JoyButtonBindings m_JoyButtonBindings;
	AssignedJoyButtons m_AssignedJoyButtons;

	GG_Framework::Base::IJoystick &m_Joystick;
	GG_Framework::UI::EventMap *m_controlledEventMap;
	std::vector<GG_Framework::Base::IJoystick::JoyState> m_FloodControl;
	//This will measure each buttons release time!
	double m_lastReleaseTime[32];
	JoyButtonBindings::iterator m_UseDoubleClickBindings[32]; //These are in essence pointers
	double m_eventTime;

	ConfigurationManager * const m_Config;
};

	}
}