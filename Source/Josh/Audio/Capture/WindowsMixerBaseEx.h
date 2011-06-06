#pragma once


namespace Devices { namespace Audio { namespace Capture {

//***********************************************************************************************************************************
//***********************************************************************************************************************************

class DEVICES_AUDIO_CAPTURE_API CMixerDevice_WaveIn
{
	// Interface
public:

	static UINT GetNumDevicesTotal( void );

	// C'tor/D'tor
	explicit CMixerDevice_WaveIn(UINT idx);
	virtual ~CMixerDevice_WaveIn( void );

	// NOTE: This impl must be the same as CMixerBase::GetDestinLineIndex
	// It will only get the idx of the very first WAVEIN dest line found!!!
	bool GetDstLineIdx_WaveIn(UINT &idx);	// The WaveIn destination line idx
	bool GetSelectedSrcLineIdx(UINT &idx, std::wstring &name);	// The selected/active source line idx that is sent to the WaveIn destination output

	operator HMIXER ( void );	// Cast to my device handle
	operator UINT ( void );	// Cast to my device index

	// Implementation
protected:

	// Data
protected:

	HMIXER m_hDevice;
	UINT m_DeviceIdx;
	DWORD m_WaveInDestLineIdx;

	// Illegal
private:

	CMixerDevice_WaveIn(CMixerDevice_WaveIn&) { assert(false); }
	void operator = (CMixerDevice_WaveIn&) { assert(false); }
};

//***********************************************************************************************************************************
//***********************************************************************************************************************************

} } }	// end namespaces