#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;
using namespace GG_Framework::Base;

  /***********************************************************************************************************************/
 /*												ConfigurationManager													*/
/***********************************************************************************************************************/

ConfigurationManager::ConfigurationManager(bool useUserPrefs) : m_useUserPrefs(useUserPrefs)
{
}

void ConfigurationManager::AddConfigLoadSaveInterface(ConfigLoadSaveInterface *node)
{
	m_ConfigLoadSaveInterface.push_back(node);
}

bool Exists(const std::vector<const char *>&list,const char *Entry)
{
	bool ret=false;
	for (size_t i=0;i<list.size();i++)
		if (!strcmp(list[i],Entry))
		{
			ret=true;
			break;
		}	
	return ret;
}

void ConfigurationManager::UpdateSettings_Load()
{
	if (m_useUserPrefs)
	{
		m_userPrefsNode = XMLNode::parseFile("userPrefs.xml", "UserPrefs");

		if (!m_userPrefsNode.isEmpty())
		{
			std::vector<const char *> list;
			//This outer loop really is to obtain each unique group name.  It could easily be written more efficient, but I've chose
			//to keep this simple by re-using the same functionality of "Exists"
			for (size_t i = 0; i < m_ConfigLoadSaveInterface.size(); i++)
			{
				const char *GroupName=m_ConfigLoadSaveInterface[i]->GetConfigGroupName();
				//Only enter here one time per group
				if (!Exists(list,GroupName))
				{
					list.push_back(GroupName);
					XMLNode userInputNode = m_userPrefsNode.getChildNode(GroupName);
					if (!userInputNode.isEmpty())
					{
						//Yikes this is an n^2 operation... hopefully this should not be a problem
						for (size_t j = 0; j < m_ConfigLoadSaveInterface.size(); j++)
						{
							if (!strcmp(GroupName,m_ConfigLoadSaveInterface[j]->GetConfigGroupName()))
								m_ConfigLoadSaveInterface[j]->BindToPrefs(userInputNode);
						}
					}
				}
			}
		}
	}
}

bool operator < (const XMLNode& a, const XMLNode& b)
{
	return std::string(a.getName()) < std::string(b.getName());
}

void ConfigurationManager::UpdateSettings_Save()
{
	if (m_useUserPrefs)
	{
		// Creating the real XML file structure
		XMLNode userPrefsNode = XMLNode::createXMLTopNode("UserPrefs");
		std::vector<const char *> list;
		for (size_t i = 0; i < m_ConfigLoadSaveInterface.size(); i++)
		{
			XMLNode pseudoParent = XMLNode::createXMLTopNode("Temporary"); // XMLNode work around for creating children nodes
			std::vector<XMLNode> vectorXMLNodes; // used to make sorting the XMLNodes significantly easier
			m_ConfigLoadSaveInterface[i]->WriteSettings(pseudoParent,vectorXMLNodes);

			const char *GroupName=m_ConfigLoadSaveInterface[i]->GetConfigGroupName();
			//Check for duplicate groups... we can only add unique groups to userPrefsNode
			if (!Exists(list,GroupName))
			{
				userPrefsNode.addChild(GroupName);
				list.push_back(GroupName);
			}
			XMLNode userInputNode = userPrefsNode.getChildNode(GroupName);

			std::sort(vectorXMLNodes.begin(), vectorXMLNodes.end());
			for (unsigned int j = 0; j < vectorXMLNodes.size(); j++)
				userInputNode.addChild(vectorXMLNodes[j]);
		}

		userPrefsNode.writeToFile("userPrefs.xml"); // obvious ... just writing the XMLNode structure to a file
	}
}


//Doing it this way any failure will revert to the default
bool ConfigurationManager::GetAttributeBool(XMLNode &eventNode,const char Name[],bool default_value)
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


double ConfigurationManager::GetAttributeDouble(XMLNode &eventNode,const char Name[],double default_value)
{
	double ret=default_value;
	XMLCSTR attribute = eventNode.getAttribute(Name);
	if (attribute)
		ret=atof(attribute);
	return ret;
}

const char *ConfigurationManager::GetAttributeString(XMLNode &eventNode,const char Name[],const char default_value[])
{
	const char *ret=default_value;
	XMLCSTR attribute = eventNode.getAttribute(Name);
	if (attribute)
		ret=attribute;
	return ret;
}

bool ConfigurationManager::InterceptDefaultKey(const std::string &eventName,const char KeyType[])
{
	bool ret=false;
	// intercepting with user preferences from "userPrefs.xml"
	if (m_useUserPrefs && !m_userPrefsNode.isEmpty())
	{
		XMLNode userInputNode = m_userPrefsNode.getChildNode("UserInput");
		if (!userInputNode.isEmpty())
		{
			size_t i=0;

			//Be sure to iterate all eventName matches in case the first entries are not the correct type
			while (!userInputNode.getChildNode(eventName.c_str(),i).isEmpty())
			{
				XMLNode eventNode = userInputNode.getChildNode(eventName.c_str(),i++);
				XMLCSTR attribute=eventNode.getAttribute("type");
				if (attribute && (strcmp(attribute,KeyType)==0))
				{
					//here is a special case for keyboard... to ensure the entry is good (Note: this is redundant but that is ok)
					if (strcmp(KeyType,"keyboard")==0)
					{
						// grab the preference key
						if (Key::KEY_STRING_MAPS.stringKeyMap.find(eventNode.getAttribute("key")) !=
							Key::KEY_STRING_MAPS.stringKeyMap.end())
						{
							ret=true; 
							break;  //got it break out
						}
						else
							ASSERT(false);  //The initial check should have kicked in
					}
					else
					{
						//No other special cases defined... should be good to go :)
						ret=true; 
						break;  //got it break out
					}
				}
			}
		}
	}
	return ret;
}

  /***********************************************************************************************************************/
 /*											AudioVolumeControls_PlusBLS													*/
/***********************************************************************************************************************/

AudioVolumeControls_PlusBLS::AudioVolumeControls_PlusBLS(Event0& VolumeSelect,Event0& VolumeUp,Event0& VolumeDown) : 
	VolumeControls(VolumeSelect,VolumeUp,VolumeDown)
{
}

const char *AudioVolumeControls_PlusBLS::GetModeName(VolumeModes mode)
{
	const char *ret=NULL;
	switch(mode)
	{
		case eMaster:
			ret="Volume_Master";
			break;
		case eSoundEffects:
			ret="Volume_SoundEffects";
			break;
		case eMusic:
			ret="Volume_Music";
			break;
		default:
			ASSERT(false);
	}
	return ret;
}

AudioVolumeControls_PlusBLS::VolumeModes AudioVolumeControls_PlusBLS::GetModeFromName( const char *mode)
{
	VolumeModes ret=(VolumeControls::VolumeModes)-1;
	for (size_t i=0;i<eNoModes;i++)
	{
		if (stricmp(mode,GetModeName((VolumeControls::VolumeModes)i))==0)
		{
			ret=(VolumeControls::VolumeModes)i;
			break;
		}
	}
	ASSERT(ret!=(VolumeControls::VolumeModes)-1);
	return ret;
}

void AudioVolumeControls_PlusBLS::BindToPrefs(const XMLNode &userPrefsNode)
{
	size_t i=0;

	//Go down the list
	while (!userPrefsNode.getChildNode(i).isEmpty())
	{
		XMLNode eventNode = userPrefsNode.getChildNode(i++);
		//Only read audio types
		XMLCSTR attribute=eventNode.getAttribute("type");
		if (attribute)
		{
			if (strcmp(attribute,"audio_volume")==0)
			{
				double ratio=ConfigurationManager::GetAttributeDouble(eventNode,"Ratio",1.0);
				VolumeModes mode=GetModeFromName(eventNode.getName());
				switch (mode)
				{
				case eMaster:
					m_MasterVolume=ratio;
					SOUNDSYSTEM.SetMasterVolume(m_MasterVolume);
					break;
				case eSoundEffects:
					m_SoundsVolume=ratio;
					SOUNDSYSTEM.SetSoundsVolume(m_SoundsVolume);
					break;
				case eMusic:
					m_StreamsVolume=ratio;
					SOUNDSYSTEM.SetStreamsVolume(m_StreamsVolume);
					break;
				default:
					ASSERT(false);
				}
			}
		}
	}

}

void AudioVolumeControls_PlusBLS::WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes)
{
	for (size_t i=0;i<Audio::VolumeControls::eNoModes;i++)
	{
		float ratio;
		switch (i)
		{
			case eMaster:
				ratio=m_MasterVolume;
				break;
			case eSoundEffects:
				ratio=m_SoundsVolume;
				break;
			case eMusic:
				ratio=m_StreamsVolume;
				break;
			default:
				ASSERT(false);
		}

		XMLNode childNode = node.addChild(GetModeName((VolumeControls::VolumeModes)i)); // <EventName />
		childNode.addAttribute("type", "audio_volume");
		childNode.addAttribute("Ratio", BuildString("%f",ratio).c_str());
		vectorXMLNodes.push_back(childNode);
	}
}
