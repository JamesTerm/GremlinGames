// SoundEffects3D.h
#pragma once


namespace GG_Framework
{
	namespace UI
	{
		namespace Audio
		{

class FRAMEWORK_UI_AUDIO_API VolumeControls
{
	public:
		VolumeControls(Event0& VolumeSelect,Event0& VolumeUp,Event0& VolumeDown);
		void VolumeUp();
		void VolumeDown();
		void VolumeSelect();
	protected:
		enum VolumeModes
		{
			eMaster,
			eSoundEffects,
			eMusic,
			eNoModes
		} m_Mode;
		float m_MasterVolume,m_SoundsVolume,m_StreamsVolume;
private:
	IEvent::HandlerList ehl;
};

		}
	}
}