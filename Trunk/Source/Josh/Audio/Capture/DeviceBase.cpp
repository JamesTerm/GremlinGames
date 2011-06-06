#include "stdafx.h"
#include "Devices_Audio_Capture.h"
#include "WindowsMixerBase.h"
#include "WindowsMixerBaseEx.h"

//*************************************************************************************************************************************************

using namespace Devices::Audio::Capture;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

/*
DeviceBase::DeviceBase( void ) : m_pMicVolumeController(new CMixerFader())
{
	// Seriously...if init fails, I don't want to be calling any of it's methods b/c it will fuckin' crash!!!
	if (!m_pMicVolumeController->Init(MIXERLINE_COMPONENTTYPE_DST_WAVEIN,
										MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE,
										MIXERCONTROL_CONTROLTYPE_VOLUME, 
										CMixerFader::MAIN))
	{
		delete m_pMicVolumeController;
		m_pMicVolumeController = NULL;
	}
}

DeviceBase::~DeviceBase( void )
{
	delete m_pMicVolumeController;
}

bool DeviceBase::CanDoVolume( void ) const
{
	return (NULL != m_pMicVolumeController);
}

void DeviceBase::SetMicVolume(float fVolume)
{
	//assert(m_pMicVolumeController);
	if (m_pMicVolumeController)
		m_pMicVolumeController->SetSliderValue(fVolume);
}
*/

DeviceBase::DeviceBase( void )
{
	for (UINT i=0; i<CMixerDevice_WaveIn::GetNumDevicesTotal(); i++)
	{
		CMixerDevice_WaveIn *pNewMixerDevice = new CMixerDevice_WaveIn(i);
		UINT dstWaveInLineIdx = 0;
		if (!pNewMixerDevice->GetDstLineIdx_WaveIn(dstWaveInLineIdx))	// TODO: This will only use the 1st WAVEIN dest line found!!!
		{
			// Doesn't do WAVEIN, so I don't care.  Carry on!!!
			delete pNewMixerDevice;
			continue;
		}

		// First try to instantiate a master recording volume fader
		CMixerFader *pMasterFader = new CMixerFader();
		if (pMasterFader->Init(MIXERLINE_COMPONENTTYPE_DST_WAVEIN, CMixerFader::NO_SOURCE, MIXERCONTROL_CONTROLTYPE_VOLUME, CMixerFader::MAIN, i))
		{
			// I have a master volume fader, goto next device
			m_MixerDevList.push_back(pNewMixerDevice);
			m_VolumeControllerList.push_back(pMasterFader);
			continue;
		}
		else
		{
			// Try to get the selected source line
			UINT selectedSrcIdx = 0;
			std::wstring selectedSrcLineName;
			if (pNewMixerDevice->GetSelectedSrcLineIdx(selectedSrcIdx, selectedSrcLineName) && !selectedSrcLineName.empty())
			{
				// Create a mixer fader based on this selected source line index
				if (pMasterFader->InitFromSrcLineName(MIXERLINE_COMPONENTTYPE_DST_WAVEIN, selectedSrcLineName, 
														MIXERCONTROL_CONTROLTYPE_VOLUME, CMixerFader::MAIN, i))
				{
					// I'm using the selected source line fader.  Success.  Carry on to next mixer device!!!
					m_MixerDevList.push_back(pNewMixerDevice);
					m_VolumeControllerList.push_back(pMasterFader);
					continue;
				}
			}
		}

		// If I got here, something went haywire so I needs to dealloc mem
		delete pNewMixerDevice;
		delete pMasterFader;
	}
}

DeviceBase::~DeviceBase( void )
{
	for (MixerDevListIter iter1=m_MixerDevList.begin(); iter1<m_MixerDevList.end(); iter1++)
		delete (*iter1);
	for (VolumeControllerListIter iter2=m_VolumeControllerList.begin(); iter2<m_VolumeControllerList.end(); iter2++)
		delete (*iter2);
}

bool DeviceBase::CanDoVolume( void ) const
{
	return !m_VolumeControllerList.empty();
}

void DeviceBase::SetMicVolume(float fVolume)
{
	for (VolumeControllerListIter iter=m_VolumeControllerList.begin(); iter<m_VolumeControllerList.end(); iter++)
	{
		CMixerFader *pFader = (*iter);
		if (pFader)
			pFader->SetSliderValue(fVolume);
	}
}

//*************************************************************************************************************************************************

void DeviceBase::SetMicBoost(bool bBoost)
{
	assert(false);	// TODO
}

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

