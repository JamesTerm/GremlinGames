#pragma once

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

// Predecls
class CMixerFader;
class CMixerDevice_WaveIn;

class DEVICES_AUDIO_CAPTURE_API DeviceBase
{
	// Interface
public:

	// C'tor/D'tor
	DeviceBase( void );
	virtual ~DeviceBase( void );

	virtual bool CanDoVolume( void ) const;
	virtual void SetMicVolume(float fVolume);
	virtual void SetMicBoost(bool bBoost);

	// Implementation
protected:

	// Data
protected:

	//CMixerFader *m_pMicVolumeController;				// The 1st Mic volume fader/controller for the 1st WAVEIN mixer found
	std::vector<CMixerDevice_WaveIn*> m_MixerDevList;	// List of all Windows mixer devices that have WAVEIN dest lines
	std::vector<CMixerFader*> m_VolumeControllerList;	// The corresponding mixer device volume fader/controller

	typedef std::vector<CMixerDevice_WaveIn*>::iterator MixerDevListIter;
	typedef std::vector<CMixerFader*>::iterator VolumeControllerListIter;

	// Illegal
private:
};

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************
