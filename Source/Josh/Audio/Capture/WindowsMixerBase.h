//=============================================================================
// Copyright Langis Pitre 1998
// You may do whatever you want with this code, as long as you include this
// copyright notice in your implementation files.
//
// If you wish to add new classes to this collection, feel free to do so.
// But please send me your code so that I can update the collection.
//
// Comments and bug reports: lpitre@sympatico.ca
//
//=============================================================================

#pragma once


namespace Devices { namespace Audio { namespace Capture {

//***********************************************************************************************************************************
//***********************************************************************************************************************************

class DEVICES_AUDIO_CAPTURE_API CMixerBase  
{
public:

	CMixerBase();
	virtual ~CMixerBase();

	void* GetValuePtr() { return m_val; }

	int GetDestinLineIndex( MIXERLINE &line, DWORD lineType );
	int GetSourceLineIndex( MIXERLINE &line, DWORD lineType );
	// mwatkins
	int GetSourceLineIndex( MIXERLINE &line, const std::wstring &inName );

	int GetMixerControlValue();
	int SetMixerControlValue();
	
	std::wstring GetControlName() const;
	std::wstring GetControlShortName() const;
	std::wstring GetLineName() const;
	std::wstring GetLineShortName() const;

	static LPCTSTR GetControlTypeString( DWORD type );
	static LPCTSTR GetLineTypeString( DWORD type );

	static const DWORD NO_SOURCE;


protected:
	
	int  Init( DWORD DstType, DWORD SrcType, DWORD ControlType, HWND hwnd, UINT mixerIdx=0/*mwatkins*/ );
	// mwatkins
	int InitFromSrcLineName( DWORD DstType, const std::wstring &inName, DWORD ControlType, HWND hwnd, UINT mixerIdx ); 
	int InitFromSrcLineIdx( DWORD DstType, UINT srcLineIdx, DWORD ControlType, HWND hwnd, UINT mixerIdx );
	
	void CloseMixer();

	std::wstring m_LineName;
	std::wstring m_DestLineName;
	std::wstring m_LineShortName;

	HMIXER       m_HMixer;
    MIXERCONTROL m_Control;
	int          m_nChannels;

private:
	int InitMXDetails();

    MIXERCONTROLDETAILS  m_Details;
	void *m_val;
};

//***********************************************************************************************************************************

class DEVICES_AUDIO_CAPTURE_API CMixerFader : public CMixerBase
{
public:
	CMixerFader();
	virtual ~CMixerFader();
	
	enum { LEFT, RIGHT, MAIN, BALANCE };

	int Init( DWORD DstType, DWORD SrcType, DWORD ControlType, int subType, UINT mixerIdx=0/*mwatkins*/ );
	// mwatkins
	int InitFromSrcLineName( DWORD DstType, const std::wstring &inName, DWORD ControlType, int subType, UINT mixerIdx );
	int InitFromSrcLineIdx( DWORD DstType, UINT srcLineIdx, DWORD ControlType, int subType, UINT mixerIdx );

	void SetSliderValue(float fValue);	// 0.0-1.0
	void SetSliderValue(DWORD dwValue);

protected:

	DWORD GetSliderValue();
	void SetFaderValue();

	int m_FaderSpan;
	int m_FaderStep;
	int m_SliderSpan;
	int m_subType;
	float m_DeltaOverMax;

	MIXERCONTROLDETAILS_UNSIGNED *m_Value;

	DWORD m_dwSliderValue;
};

//***********************************************************************************************************************************
//***********************************************************************************************************************************

} } }	// end namespaces