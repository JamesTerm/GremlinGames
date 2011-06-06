#include "stdafx.h"
#include "Devices_Audio_Capture.h"
#include "WindowsMixerBase.h"
#include "WindowsMixerBaseEx.h"

//*************************************************************************************************************************************************

using namespace Devices::Audio::Capture;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

//static 
UINT CMixerDevice_WaveIn::GetNumDevicesTotal( void )
{
	return ::mixerGetNumDevs();
}

CMixerDevice_WaveIn::CMixerDevice_WaveIn(UINT idx) : m_hDevice(NULL), m_DeviceIdx(idx), m_WaveInDestLineIdx(static_cast<DWORD>(-1))
{
	assert(idx < GetNumDevicesTotal());

	if (MMSYSERR_NOERROR != ::mixerOpen(&m_hDevice, idx, 0, 0, MIXER_OBJECTF_MIXER))
	{
		assert(false);
		m_hDevice = NULL;
	}
}

CMixerDevice_WaveIn::~CMixerDevice_WaveIn( void )
{
	if (m_hDevice)
		::mixerClose(m_hDevice);
}

bool CMixerDevice_WaveIn::GetDstLineIdx_WaveIn(UINT &idx)
{
	if (static_cast<DWORD>(-1) != m_WaveInDestLineIdx)
	{
		idx = m_WaveInDestLineIdx;
		return true;
	}
	if (!m_hDevice)
		return false;

	// Find out how many destination lines are available
	MIXERCAPS caps = { 0 };
	if (MMSYSERR_NOERROR != ::mixerGetDevCaps((UINT)m_hDevice, &caps, sizeof(MIXERCAPS)))
	{
		assert(false);	// Deserving of an assertion
		return false;
	}

	int i_cDest = caps.cDestinations;
	for (int i = 0; i < i_cDest; i++)
	{
		MIXERLINE line = { 0 };
		line.cbStruct = sizeof(MIXERLINE);
		line.dwDestination = i;
		if (MMSYSERR_NOERROR != ::mixerGetLineInfo((HMIXEROBJ)m_hDevice, &line, MIXER_GETLINEINFOF_DESTINATION))
		{
			assert(false);	// Deserving of an assertion
			return false;
		}
		
		if (MIXERLINE_COMPONENTTYPE_DST_WAVEIN == line.dwComponentType)	// Is this the one I want???
		{
			idx = static_cast<UINT>(i);
			m_WaveInDestLineIdx = idx;
			return true;
		}		
	}

	// If I get here, I fail miserably (in finding a dest WaveIn line)!!!
	// i.e. This means that this device doesn't support WaveIn recording which isn't that uncommon
	return false;
}

bool CMixerDevice_WaveIn::GetSelectedSrcLineIdx(UINT &idx, std::wstring &name)
{
	UINT dstLineIdx = 0;
	if (!this->GetDstLineIdx_WaveIn(dstLineIdx))
	{
		assert(false);	// Shouldn't be calling this shit if I don't have a WAVEIN dest line!!!
		return false;
	}

	bool bRetVal = false;

	MIXERLINE line = { 0 };
	line.cbStruct = sizeof(MIXERLINE);
	line.dwDestination = dstLineIdx;	// The WAVEIN dest line index
	::mixerGetLineInfo((HMIXEROBJ)m_hDevice, &line, MIXER_GETLINEINFOF_DESTINATION);
	if (0 == line.cControls)
		return false;
	const DWORD cTotalSrcLines = line.cConnections;
	if (0 ==  cTotalSrcLines)
		return false;		// There must be source lines to select

	MIXERCONTROL *pMixerControl = new MIXERCONTROL[line.cControls];
	assert(pMixerControl);

	MIXERLINECONTROLS mixerLineControl = { 0 };
	mixerLineControl.cbStruct  = sizeof(MIXERLINECONTROLS);
	mixerLineControl.dwLineID  = line.dwLineID;
	mixerLineControl.cControls = line.cControls;
	mixerLineControl.cbmxctrl  = sizeof(MIXERCONTROL);
	mixerLineControl.pamxctrl  = pMixerControl;
		
	if (MMSYSERR_NOERROR != ::mixerGetLineControls((HMIXEROBJ)m_hDevice, &mixerLineControl, MIXER_GETLINECONTROLSF_ALL))
	{
		delete [] pMixerControl;
		return false;
	}

	for (DWORD mixCtrlIdx=0; !bRetVal && mixCtrlIdx<line.cControls; mixCtrlIdx++)
	{
		MIXERCONTROL control = pMixerControl[mixCtrlIdx];
	
		// Is this a boolean control type???
		if ((MIXERCONTROL_CT_UNITS_BOOLEAN==(MIXERCONTROL_CT_UNITS_MASK & control.dwControlType)) && (0<control.cMultipleItems))
		{
			MIXERCONTROLDETAILS detailsBase = { 0 };
			detailsBase.cbStruct       = sizeof(MIXERCONTROLDETAILS);
			detailsBase.dwControlID    = control.dwControlID;
			detailsBase.cMultipleItems = control.cMultipleItems;
			
			int listSize = 0;
			if (MIXERCONTROL_CONTROLF_UNIFORM & control.fdwControl)
			{
				detailsBase.cChannels = 1;
				listSize = control.cMultipleItems;
			}
			else
			{
				detailsBase.cChannels = line.cChannels;
				listSize = control.cMultipleItems*line.cChannels;
			}

			if (0<listSize && cTotalSrcLines==listSize)	// There should be 1 bool for each source line!!!
			{
				MIXERCONTROLDETAILS details = detailsBase;
				details.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
				MIXERCONTROLDETAILS_BOOLEAN *list = new MIXERCONTROLDETAILS_BOOLEAN[listSize];
				details.paDetails = list;

				if (MMSYSERR_NOERROR == ::mixerGetControlDetails((HMIXEROBJ)m_hDevice, &details, MIXER_GETCONTROLDETAILSF_VALUE))
				{
					for (int i = 0; i < listSize; i++)
					{
						if (0 != list[i].fValue)
						{
							idx = static_cast<UINT>(i);
							bRetVal = true;
							break;
						}
					}
				}

				delete [] list;

				// I must get this src line name to use as the key
				if (bRetVal)
				{
					MIXERCONTROLDETAILS details = detailsBase;
					details.cbDetails = sizeof( MIXERCONTROLDETAILS_LISTTEXT );
					MIXERCONTROLDETAILS_LISTTEXT *list = new MIXERCONTROLDETAILS_LISTTEXT[listSize];
					details.paDetails = list;
					
					if (MMSYSERR_NOERROR == ::mixerGetControlDetails((HMIXEROBJ)m_hDevice, &details, MIXER_GETCONTROLDETAILSF_LISTTEXT))
					{
						name = list[idx].szName;
					}
					else
					{
						assert(false);
					}

					delete [] list;
				}
			}
		}

	}

	delete [] pMixerControl;

	return bRetVal;
}

CMixerDevice_WaveIn::operator HMIXER ( void )
{
	return m_hDevice;
}

CMixerDevice_WaveIn::operator UINT ( void )
{
	return m_DeviceIdx;
}

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************
