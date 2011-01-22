#pragma once

namespace GG_Framework
{
	namespace UI
	{

class ConfigLoadSaveInterface
{
	public:
		// TODO Rename to load / save something.
		virtual void BindToPrefs(const XMLNode &userPrefsNode) = 0;
		virtual void WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes) = 0;
		//This will associate each interface to a group within XML file 
		virtual const char *GetConfigGroupName() {return "Miscellaneous";}
};


class ConfigurationManager
{
	public:
		ConfigurationManager(bool useUserPrefs);
		void AddConfigLoadSaveInterface(ConfigLoadSaveInterface *node);
		/// This is called by client once all interface children have been populated. Typically this is only called once at startup.
		void UpdateSettings_Load();
		void UpdateSettings_Save();

		/// Called by AddKeyBinding where it may override the key assignment by user preferences
		/// \return true if a key binding was found implied to not bind the default key for this event
		bool InterceptDefaultKey(const std::string &eventName,const char KeyType[]);

		//Some helper functions to easily obtain attributes
		static bool GetAttributeBool(XMLNode &eventNode,const char Name[],bool default_value);
		static double GetAttributeDouble(XMLNode &eventNode,const char Name[],double default_value);
		static const char *GetAttributeString(XMLNode &eventNode,const char Name[],const char default_value[]);

	private:
		std::vector<ConfigLoadSaveInterface *> m_ConfigLoadSaveInterface;
		XMLNode m_userPrefsNode;
		bool m_useUserPrefs;

};


class AudioVolumeControls_PlusBLS : public Audio::VolumeControls,
									public ConfigLoadSaveInterface
{
	public:
		AudioVolumeControls_PlusBLS(Event0& VolumeSelect,Event0& VolumeUp,Event0& VolumeDown);
	protected:
		virtual void BindToPrefs(const XMLNode &userPrefsNode);
		virtual void WriteSettings(XMLNode &node,std::vector<XMLNode> &vectorXMLNodes);
		virtual const char *GetConfigGroupName() {return "AudioSettings";}

	private:
		const char *GetModeName(VolumeModes mode);
		VolumeModes GetModeFromName( const char *mode);
};

	}
}