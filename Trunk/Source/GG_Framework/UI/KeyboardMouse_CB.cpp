// KeyboardMouse_CB.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

#define DEBUG_MOUSE_MOTION

const double DOUBLE_CLICK_TIME = 0.25;

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

  /***************************************************************************************************************/
 /*											KeyboardMouse_CB													*/
/***************************************************************************************************************/

KeyboardMouse_CB::KeyboardMouse_CB( ConfigurationManager *config) :  	
		m_controlledEventMap(NULL), m_flags(0), 
		m_Joystick(Base::GetDirectInputJoystick()), m_lastReleasedKey(0), 
		m_lastReleaseTime(0.0), m_dblPress(NULL), m_eventTime(0.0),
		IgnoreMouseMotion(true), m_Config(config)
{
	GlobalEventMap.SetKB_Controlled(true);
}
////////////////////////////////////////////////////////////////////////////////////////////

KeyboardMouse_CB::~KeyboardMouse_CB()
{
	delete m_dblPress;
#ifndef __UseSingleThreadMainLoop__
	GG_Framework::Base::RefMutexWrapper rmw(m_queueMutex);
	while (!m_msgQueue.empty())
	{
		delete (m_msgQueue.front());
		m_msgQueue.pop();
	}
#endif
}

void KeyboardMouse_CB::WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes)
{
	// iterator for m_AssignedKeys and m_AssignedKeys_OnOff
	std::map<std::string, std::vector<Key>*, std::greater<std::string> >::iterator iter;

	// creating XMLNodes of the form <EventName key="" onOff="" /> and adding them to vectorXMLNodes
	if (m_AssignedKeys.size() > 0)
	{
		for (iter = m_AssignedKeys.begin(); iter != m_AssignedKeys.end(); ++iter)
		{
			for (size_t i=0;i<iter->second->size();i++)  //There may be multiple key assignments... iterate these
			{
				XMLNode childNode = node.addChild(iter->first.c_str()); // <EventName />
				if (iter->second && iter->second->size() > 0) // test the pointer then the size
				{
					childNode.addAttribute("type", "keyboard");
					childNode.addAttribute("key",    (*iter->second)[i].getKeyString().c_str());
					childNode.addAttribute("alt"  , ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_ALT  ) ? "1" : "0");
					childNode.addAttribute("ctrl" , ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_CTRL ) ? "1" : "0");
					childNode.addAttribute("shift", ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_SHIFT) ? "1" : "0");
					childNode.addAttribute("dbl"  , ((*iter->second)[i].flags & Key::DBL        ) ? "1" : "0");
					childNode.addAttribute("onOff", "0");
				}
				vectorXMLNodes.push_back(childNode);
			}
		}
	}

	// there is really no simple way to combine these two loops ... or else I would have
	if (m_AssignedKeys_OnOff.size() > 0)
	{
		for (iter = m_AssignedKeys_OnOff.begin(); iter != m_AssignedKeys_OnOff.end(); ++iter)
		{
			for (size_t i=0;i<iter->second->size();i++)  //There may be multiple key assignments... iterate these
			{
				XMLNode childNode = node.addChild(iter->first.c_str()); // <EventName />
				if (iter->second && iter->second->size() > 0) // test the pointer then the size
				{
					childNode.addAttribute("type", "keyboard");
					childNode.addAttribute("key"  ,  (*iter->second)[i].getKeyString().c_str());
					childNode.addAttribute("alt"  , ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_ALT  ) ? "1" : "0");
					childNode.addAttribute("ctrl" , ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_CTRL ) ? "1" : "0");
					childNode.addAttribute("shift", ((*iter->second)[i].flags & osgGA::GUIEventAdapter::MODKEY_SHIFT) ? "1" : "0");
					childNode.addAttribute("dbl"  , ((*iter->second)[i].flags & Key::DBL        ) ? "1" : "0");
					childNode.addAttribute("onOff", "1");
				}
				vectorXMLNodes.push_back(childNode);
			}
		}
	}
}

//Doing it this way any failure will revert to the default
bool GetAttributeBool(XMLNode &eventNode,const char Name[],bool default_value)
{
	bool ret=default_value;
	XMLCSTR attribute = eventNode.getAttribute(Name);
	if (default_value)
	{
		if (attribute && (attribute[0] == '0' || attribute[0] == 'f' || attribute[0] == 'F'))
			ret = false;
	}
	else
	{
		if (attribute && (attribute[0] == '1' || attribute[0] == 't' || attribute[0] == 'T'))
			ret = true;
	}
	return ret;
}

double GetAttributeDouble(XMLNode &eventNode,const char Name[],double default_value)
{
	double ret=default_value;
	XMLCSTR attribute = eventNode.getAttribute(Name);
	if (attribute)
		ret=atof(attribute);
	return ret;
}

const char *GetAttributeString(XMLNode &eventNode,const char Name[],const char default_value[])
{
	const char *ret=default_value;
	XMLCSTR attribute = eventNode.getAttribute(Name);
	if (attribute)
		ret=attribute;
	return ret;
}
void KeyboardMouse_CB::BindToPrefs(const XMLNode &userPrefsNode)
{
	size_t i=0;

	//Go down the list
	while (!userPrefsNode.getChildNode(i).isEmpty())
	{
		XMLNode eventNode = userPrefsNode.getChildNode(i++);
		//Only read keyboard types
		XMLCSTR attribute=eventNode.getAttribute("type");
		if (attribute)
		{
			if (strcmp(attribute,"keyboard")==0)
			{
				// grab the preference key
				if (Key::KEY_STRING_MAPS.stringKeyMap.find(eventNode.getAttribute("key")) !=
					Key::KEY_STRING_MAPS.stringKeyMap.end())
				{
					Key key( Key::KEY_STRING_MAPS.stringKeyMap[eventNode.getAttribute("key")]);

					// clear the key's flags
					key.flags = 0;

					// reassign the flags
					if (GetAttributeBool(eventNode,"alt",false))
						key.flags |= osgGA::GUIEventAdapter::MODKEY_ALT;

					if (GetAttributeBool(eventNode,"ctrl",false))
						key.flags |= osgGA::GUIEventAdapter::MODKEY_CTRL;

					if (GetAttributeBool(eventNode,"shift",false))
						key.flags |= osgGA::GUIEventAdapter::MODKEY_SHIFT;

					if (GetAttributeBool(eventNode,"dbl",false))
						key.flags |= Key::DBL;

					bool useOnOff=GetAttributeBool(eventNode,"onOff",true);

					//force bind this key
					AddKeyBinding(key,eventNode.getName(),useOnOff,true);
				}
				else
					printf("ERROR: userPrefs.xml, eventName: (%s), key: (%s), is not a valid key and will not bind\n", eventNode.getName(), eventNode.getAttribute("key") );
			}
		}
	}
}

void KeyboardMouse_CB::mouseMotion( float mx, float my )
{
	if (IgnoreMouseMotion)
		return;

	DEBUG_MOUSE_MOTION("KeyboardMouse_CB::mouseMotion( %f, %f )\n", mx, my);
	GlobalEventMap.KBM_Events.MouseMove.Fire(mx, my);
	if (m_controlledEventMap)
		m_controlledEventMap->KBM_Events.MouseMove.Fire(mx, my);
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouse_CB::buttonPress( float mx, float my, unsigned int button )
{
	if (IgnoreMouseMotion)
		mx=my=0.0f;

	GlobalEventMap.KBM_Events.MouseBtnPress.Fire(mx, my, button);
	if (m_controlledEventMap)
		m_controlledEventMap->KBM_Events.MouseBtnPress.Fire(mx, my, button);
	KeyPressRelease(button, true);
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouse_CB::buttonRelease( float mx, float my, unsigned int button )
{
	if (IgnoreMouseMotion)
		mx=my=0.0f;

	GlobalEventMap.KBM_Events.MouseBtnRelease.Fire(mx, my, button);
	if (m_controlledEventMap)
		m_controlledEventMap->KBM_Events.MouseBtnRelease.Fire(mx, my, button);
	KeyPressRelease(button, false);
}
////////////////////////////////////////////////////////////////////////////////////////////
void KeyboardMouse_CB::mouseScroll (int sm)
{
	GlobalEventMap.KBM_Events.MouseScroll.Fire((int)sm);
	if (m_controlledEventMap)
		m_controlledEventMap->KBM_Events.MouseScroll.Fire((int)sm);
}
//////////////////////////////////////////////////////////////////////////

bool KeyboardMouse_CB::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
	return innerHandle(ea, true);
}
//////////////////////////////////////////////////////////////////////////

#ifndef __UseSingleThreadMainLoop__
// When using multi-threading, keep the messages in a queue
void KeyboardMouse_CB::AddToQueue(const osgGA::GUIEventAdapter& ea)
{
	osgGA::GUIEventAdapter* newEA = new osgGA::GUIEventAdapter(ea, osg::CopyOp::DEEP_COPY_ALL);
	GG_Framework::Base::RefMutexWrapper rmw(m_queueMutex);
	m_msgQueue.push(new osg::ref_ptr<osgGA::GUIEventAdapter>(newEA));
}
void KeyboardMouse_CB::ProcessThreadedEvents()
{
	while (true)
	{
		osg::ref_ptr<osgGA::GUIEventAdapter>* ea = NULL;
		{
			GG_Framework::Base::RefMutexWrapper rmw(m_queueMutex);
			if (!m_msgQueue.empty())
			{
				ea = m_msgQueue.front();
				m_msgQueue.pop();
			}
		}
		if (ea)
		{
			innerHandle(*(ea->get()), false);
			delete ea;
		}
		else
			return;	// All done with the queue
	}
}
#endif

bool KeyboardMouse_CB::innerHandle(const osgGA::GUIEventAdapter& ea, bool fromEA)
{
	switch(ea.getEventType())
	{
#ifndef __UseSingleThreadMainLoop__
	  	if (fromEA)
			AddToQueue(ea);
		else
#endif

   // Keys
	case(osgGA::GUIEventAdapter::KEYUP):
		KeyPressRelease(ea.getKey(), false);
		return true;
	case (osgGA::GUIEventAdapter::KEYDOWN):
		KeyPressRelease(ea.getKey(), true);
		return true;

   // Mouse
	case(osgGA::GUIEventAdapter::PUSH):
		buttonPress(ea.getXnormalized(), ea.getYnormalized(), ea.getButton());
		return true;
	case(osgGA::GUIEventAdapter::MOVE):
	case(osgGA::GUIEventAdapter::DRAG):
		mouseMotion(ea.getXnormalized(), ea.getYnormalized());
		return false; // Keep this on the windows QUEUE so it can be passed along
	case(osgGA::GUIEventAdapter::RELEASE):
		buttonRelease(ea.getXnormalized(), ea.getYnormalized(), ea.getButton());
		return true; 
	case(osgGA::GUIEventAdapter::SCROLL):
		mouseScroll((int)ea.getScrollingMotion());
		return true;

	default:
		return false;
	}
}

void KeyboardMouse_CB::KeyPressRelease(int key, bool press)
{
	if (press == !_pressedKeys.count(key))
	{
		if ((key == osgGA::GUIEventAdapter::KEY_Alt_L) || (key == osgGA::GUIEventAdapter::KEY_Alt_R))
         m_flags += press ? osgGA::GUIEventAdapter::MODKEY_ALT : -osgGA::GUIEventAdapter::MODKEY_ALT;
		if ((key == osgGA::GUIEventAdapter::KEY_Control_L) || (key == osgGA::GUIEventAdapter::KEY_Control_R))
         m_flags += press ? osgGA::GUIEventAdapter::MODKEY_CTRL : -osgGA::GUIEventAdapter::MODKEY_CTRL;
		if ((key == osgGA::GUIEventAdapter::KEY_Shift_L) || (key == osgGA::GUIEventAdapter::KEY_Shift_R))
         m_flags += press ? osgGA::GUIEventAdapter::MODKEY_SHIFT : -osgGA::GUIEventAdapter::MODKEY_SHIFT;

		if (press)
		{
			_pressedKeys.insert(key);
			if ((key == m_lastReleasedKey) && ((m_eventTime-m_lastReleaseTime) < DOUBLE_CLICK_TIME))
			{
				if (m_dblPress)
				{	// This should never happen, but in case something weird happens, we want to make sure we got the release
					KeyPressRelease(*m_dblPress, false);
					delete m_dblPress;
				}
            m_dblPress = new Key(key, m_flags + Key::DBL);

				//See if there are any event requests using double click for this key... if not use regular click (allows "nudging" on other keys)
				//  [4/5/2009 JamesK]
				std::vector<std::string>* DnUpEvents = GetBindingsForKey(*m_dblPress, true);
#if 0
				if (DnUpEvents)
					DOUT3("%d",DnUpEvents->size());
				else
					DOUT3("NULL");
#endif
				if (!DnUpEvents)
				{
					delete m_dblPress;
					m_dblPress=NULL;
					goto UseRegularClick;
				}

				KeyPressRelease(*m_dblPress, true);
				return;
			}
		}
		else
		{
			_pressedKeys.erase(key);
			m_lastReleaseTime = m_eventTime;
			if ((key == m_lastReleasedKey) && m_dblPress)
			{
				KeyPressRelease(*m_dblPress, false);
				delete m_dblPress;
				m_dblPress = NULL;
				return;
			}
			else
				m_lastReleasedKey = key;
		}
UseRegularClick:
		// Not a double click, just a regular click
		KeyPressRelease(Key(key, m_flags), press);
	}
}
//////////////////////////////////////////////////////////////////////////

void KeyboardMouse_CB::KeyPressRelease(Key key, bool press)
{
	// Find all of the bound events
	std::vector<std::string>* DnEvents = press ? GetBindingsForKey(key, false) : NULL;
	std::vector<std::string>* DnUpEvents = GetBindingsForKey(key, true);

	// Single press checks happen on the PRESS
	if (press)
	{
		GlobalEventMap.KBM_Events.KBCB_KeyDn.Fire(key);
		if (DnEvents)
		{
			std::vector<std::string>::iterator pos;
			for (pos = DnEvents->begin(); pos != DnEvents->end(); ++pos)
				GlobalEventMap.Event_Map[*pos].Fire();
		}
	}

	GlobalEventMap.KBM_Events.KBCB_KeyDnUp.Fire(key, press);
	if (DnUpEvents)
	{
		std::vector<std::string>::iterator pos;
		for (pos = DnUpEvents->begin(); pos != DnUpEvents->end(); ++pos)
			GlobalEventMap.EventOnOff_Map[*pos].Fire(press);
	}

	if (m_controlledEventMap)
	{
		if (press)
		{
			m_controlledEventMap->KBM_Events.KBCB_KeyDn.Fire(key);
			if (DnEvents)
			{
				std::vector<std::string>::iterator pos;
				for (pos = DnEvents->begin(); pos != DnEvents->end(); ++pos)
					m_controlledEventMap->Event_Map[*pos].Fire();
			}
		}
		m_controlledEventMap->KBM_Events.KBCB_KeyDnUp.Fire(key, press);
		if (DnUpEvents)
		{
			std::vector<std::string>::iterator pos;
			for (pos = DnUpEvents->begin(); pos != DnUpEvents->end(); ++pos)
				m_controlledEventMap->EventOnOff_Map[*pos].Fire(press);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////

void KeyboardMouse_CB::SetControlledEventMap(GG_Framework::UI::EventMap* em)
{
	if (m_controlledEventMap != em)
	{
		EventMapChanged.Fire(this, m_controlledEventMap, em);
		if (m_controlledEventMap)
			m_controlledEventMap->SetKB_Controlled(false);
		m_controlledEventMap = em;
		if (m_controlledEventMap)
			m_controlledEventMap->SetKB_Controlled(true);
	}
}
//////////////////////////////////////////////////////////////////////////

std::vector<Key>* KeyboardMouse_CB::GetBindingsForEventName(std::string eventName, bool useOnOff)
{
	return useOnOff ? m_AssignedKeys_OnOff[eventName] : m_AssignedKeys[eventName];
}

//////////////////////////////////////////////////////////////////////////
std::vector<std::string>* KeyboardMouse_CB::GetBindingsForKey(Key key, bool useOnOff)
{
	return useOnOff ? m_KeyBindings_OnOff[key] : m_KeyBindings[key];
}
//////////////////////////////////////////////////////////////////////////

bool KeyboardMouse_CB::AddKeyBinding(Key key,const std::string eventName, bool useOnOff, bool ForceBindThisKey)
{
	if (!ForceBindThisKey)
	{
		//See if there is a key assignment for this already
		if (m_Config->InterceptDefaultKey(eventName,"keyboard"))
			return false;		//All intercepted keys will be "force bound" so we can exit once that has happened
	}

	std::map<Key, std::vector<std::string>*, std::greater<Key> >& keyBindings(useOnOff ? m_KeyBindings_OnOff : m_KeyBindings);
	std::vector<std::string>* eventNames = keyBindings[key];
	if (!eventNames)
	{
		eventNames = new std::vector<std::string>;
		eventNames->push_back(eventName);
		keyBindings[key] = eventNames;
	}
	else
	{
		bool exists = false;
		std::vector<std::string>::iterator pos;
		for (pos = eventNames->begin(); pos != eventNames->end() && !exists; ++pos)
			exists = (eventName == *pos);
		if (!exists)
			eventNames->push_back(eventName);
	}

	std::map<std::string, std::vector<Key>*, std::greater<std::string> >& assignedKeys(useOnOff ? m_AssignedKeys_OnOff : m_AssignedKeys);
	std::vector<Key>* keys = assignedKeys[eventName];
	if (!keys)
	{
		keys = new std::vector<Key>;
		keys->push_back(key);
		assignedKeys[eventName] = keys;
	}
	else
	{
		bool exists = false;
		std::vector<Key>::iterator pos;
		//Check for duplicate entries of the same key (This may be a typical case)
		for (pos = keys->begin(); pos != keys->end() && !exists; ++pos)
			exists = (key == *pos);
		if (!exists)
			keys->push_back(key);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

void KeyboardMouse_CB::RemoveKeyBinding(Key key, std::string eventName, bool useOnOff)
{
	std::map<Key, std::vector<std::string>*, std::greater<Key> >& keyBindings(useOnOff ? m_KeyBindings_OnOff : m_KeyBindings);
	std::vector<std::string>* eventNames = keyBindings[key];
	if (eventNames)
	{
		std::vector<std::string>::iterator pos;
		for (pos = eventNames->begin(); pos != eventNames->end(); ++pos)
		{
			if (eventName == *pos)
			{
				eventNames->erase(pos);
				break;	// There should be only one if any
			}
		}
	}

	std::map<std::string, std::vector<Key>*, std::greater<std::string> >& assignedKeys(useOnOff ? m_AssignedKeys_OnOff : m_AssignedKeys);
	std::vector<Key>* keys = assignedKeys[eventName];
	if (keys)
	{
		std::vector<Key>::iterator pos;
		for (pos = keys->begin(); pos != keys->end(); ++pos)
		{
			if (key == *pos)
			{
				keys->erase(pos);
				break;	// There should be only one if any
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////