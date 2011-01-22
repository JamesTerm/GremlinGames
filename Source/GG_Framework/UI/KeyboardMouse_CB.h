// KeyboardMouse_CB.h
#pragma once

#include <osgGA/GUIEventHandler>
#include <queue>

namespace GG_Framework
{
	namespace UI
	{
		class FRAMEWORK_UI_API KeyboardMouse_CB : public osgGA::GUIEventHandler,
												  public ConfigLoadSaveInterface
		{
		   public :
			   KeyboardMouse_CB(ConfigurationManager *config);
			~KeyboardMouse_CB();

			// Used internally to ignore all mouse motions temporarily.  For button presses, sends 0,0 for position
			bool IgnoreMouseMotion;

			virtual void mouseMotion( float mx, float my);
			// For now, any mouse motion will call the same callbacks, up to event map listeners to keep the difference
			virtual void passiveMouseMotion( float mx, float my){mouseMotion(mx,my);}
			virtual void mouseScroll( int sm);

			virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

			void ProcessThreadedEvents();

			// Work with key bindings
			std::vector<GG_Framework::Base::Key>* GetBindingsForEventName(std::string eventName, bool useOnOff);
			std::vector<std::string>* GetBindingsForKey(GG_Framework::Base::Key key, bool useOnOff);

			/// \param key which key to use that *may* be overwritten by InterceptKey
			/// \param ForceBindThisKey overrides the intercept this is used mostly for loading the preferences
			/// \note I may break this up into 2 methods, but it may be useful for clients to force bind (e.g. test purposes)
			bool AddKeyBinding(GG_Framework::Base::Key key,const std::string eventName, bool useOnOff, bool ForceBindThisKey=false);
			void RemoveKeyBinding(GG_Framework::Base::Key key, std::string eventName, bool useOnOff);

			/// This version of the function is just for easy porting from the old KB technique
			void AddKeyBindingR(bool useOnOff, std::string eventName, GG_Framework::Base::Key key)
				{AddKeyBinding(key, eventName, useOnOff);}

			// These events are ALWAYS fired
			GG_Framework::UI::EventMap GlobalEventMap;

			// There can be only one of these
			GG_Framework::UI::EventMap* GetControlledEventMap(){return m_controlledEventMap;}
			void SetControlledEventMap(GG_Framework::UI::EventMap* em);
			Event3<KeyboardMouse_CB*, GG_Framework::UI::EventMap*, GG_Framework::UI::EventMap*> EventMapChanged; //!< <this, old, new> fired before change
			
			void IncrementTime(double dTick_s) {m_eventTime += dTick_s;}

		protected: // From config load save interface.
			virtual void BindToPrefs(const XMLNode &userPrefsNode);
			/// Only called when writing other settings
			virtual void WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes);
			virtual const char *GetConfigGroupName() {return "UserInput";}

		private:
			// Keep track of the buttons already pressed
			std::set<int> _pressedKeys;
			int m_flags;
			GG_Framework::UI::EventMap* m_controlledEventMap;
			GG_Framework::Base::IJoystick &m_Joystick;
			int m_lastReleasedKey;
			double m_lastReleaseTime;
			double m_eventTime;
			GG_Framework::Base::Key* m_dblPress;

			ConfigurationManager * const m_Config;

			// When using multi-threading, keep the messages in a queue
			void AddToQueue(const osgGA::GUIEventAdapter& ea);
			std::queue<osg::ref_ptr<osgGA::GUIEventAdapter> *> m_msgQueue;
			OpenThreads::Mutex m_queueMutex;

			// Have an inner handle that does the work based on where the message is coming from
			bool innerHandle(const osgGA::GUIEventAdapter& ea, bool fromEA);

			void KeyPressRelease(int key, bool press);
			void KeyPressRelease(GG_Framework::Base::Key key, bool press);
			static int TranslateMouseButton(int mb);

			std::map<GG_Framework::Base::Key, std::vector<std::string>*, std::greater<GG_Framework::Base::Key> > m_KeyBindings;
			std::map<std::string, std::vector<GG_Framework::Base::Key>*, std::greater<std::string> > m_AssignedKeys;
			std::map<GG_Framework::Base::Key, std::vector<std::string>*, std::greater<GG_Framework::Base::Key> > m_KeyBindings_OnOff;
			std::map<std::string, std::vector<GG_Framework::Base::Key>*, std::greater<std::string> > m_AssignedKeys_OnOff;

		};
	}
}