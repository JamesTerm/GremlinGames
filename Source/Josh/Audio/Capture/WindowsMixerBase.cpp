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

#include "stdafx.h"
#include "Devices_Audio_Capture.h"
#include "WindowsMixerBase.h"

#pragma comment (lib, "Winmm")

//*************************************************************************************************************************************************

using namespace Devices::Audio::Capture;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

//static 
const DWORD CMixerBase::NO_SOURCE = MIXERLINE_COMPONENTTYPE_SRC_LAST + 1;

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************

std::wstring CMixerBase::GetControlName() const
{
	if( m_HMixer )
		return std::wstring(m_Control.szName);
	else
		return std::wstring(L"");
}

std::wstring CMixerBase::GetControlShortName() const
{
	if( m_HMixer )
		return std::wstring(m_Control.szShortName);
	else
		return std::wstring(L"");
}

std::wstring CMixerBase::GetLineName() const
{
	return m_LineName;
}

std::wstring CMixerBase::GetLineShortName() const
{
	return m_LineShortName;
}

void CMixerBase::CloseMixer()
{
	if ( m_HMixer ) mixerClose( m_HMixer );
	m_HMixer = 0;
	
	if( m_val ) delete[] m_val;
	m_val = NULL;
}

void PrintMessage( const char *title, const char *fmt, ... )
{
#ifdef DO_MIXER_WARNING
   va_list marker;
   static char buffer[256];

   va_start( marker, fmt );
   _vsnprintf( buffer, 255, fmt, marker ); 
   va_end( marker );

   ::MessageBox( NULL, buffer, title, MB_OK|MB_ICONEXCLAMATION );
#else
   title; fmt;
#endif

}

CMixerBase::CMixerBase()
{
	m_HMixer = 0;
	m_val = NULL;
}

CMixerBase::~CMixerBase()
{
	CloseMixer();
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : Init
//          
// Descr. : Opens the mixer device and get a handle. Queries the mixer device for
//          the desired control type. This control should belong either to a
//          destination line or to a source line. 
//        - If the control belongs to a destination line, the 'DstType' parameter
//          should specify its component type, and the 'SrcType' parameter should
//          be set to NO_SOURCE.
//        - If the control belongs to a source line, the 'DstType' parameter
//          should specify the component type of the line that owns the source line,
//          and the 'SrcType' parameter should specify the component type of the
//          source line that owns the control.
//
//          Called by the derived classes only ( protected ).
//          If the desired mixer line is found, the specified control is queried,
//          and memory il allocated to hold the values that will be sent to and
//          queried from the mixer device.
//          
// Return : int 1 if successful, 0 otherwise
//
// Arg    : DWORD DstType : destination line component type
//          
//          These are valid arguments ( if available ):
//
//              MIXERLINE_COMPONENTTYPE_DST_DIGITAL 
//              MIXERLINE_COMPONENTTYPE_DST_LINE
//              MIXERLINE_COMPONENTTYPE_DST_MONITOR
//              MIXERLINE_COMPONENTTYPE_DST_SPEAKERS
//              MIXERLINE_COMPONENTTYPE_DST_HEADPHONES
//              MIXERLINE_COMPONENTTYPE_DST_TELEPHONE 
//              MIXERLINE_COMPONENTTYPE_DST_WAVEIN
//              MIXERLINE_COMPONENTTYPE_DST_MONITOR
//              MIXERLINE_COMPONENTTYPE_DST_VOICEIN
//
// Arg    : DWORD SrcType : source line component type
//          
//          These are valid arguments ( if available ):
//
//              NO_SOURCE ( use when the desired control belongs to a destination line )
//
//              MIXERLINE_COMPONENTTYPE_SRC_DIGITAL 
//              MIXERLINE_COMPONENTTYPE_SRC_LINE
//              MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE
//              MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER
//              MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC
//              MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE 
//              MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER
//              MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT
//              MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY
//              MIXERLINE_COMPONENTTYPE_SRC_ANALOG
//
// Arg    : DWORD ControlType : desired control type
//
// Arg    : HWND hwnd : handle of window that will handle mixer specific messages
//                      this handle is supplied by the derived classes.
//
//-----------------------------------------------------------------------------
int CMixerBase::Init( DWORD DstType, DWORD SrcType, DWORD ControlType, HWND hwnd, UINT mixerIdx/*mwatkins*/ )
{	
	UINT nbMixers = mixerGetNumDevs();
	//if( nbMixers < 1 )
	if (mixerIdx >= nbMixers)	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "No mixer device present" );
		return 0;
	}

	// ---- open the mixer device ----

	if ( m_HMixer )
		mixerClose( m_HMixer );

	if( mixerOpen( &m_HMixer, /*0*/mixerIdx, ( DWORD )hwnd, 0, CALLBACK_WINDOW ) != MMSYSERR_NOERROR )	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "Could not open mixer device" );
		return 0;
	}

	// ----- find the desired destination line -----
	
	MIXERLINE line;

	if( GetDestinLineIndex( line, DstType ) == -1 )
	{
		PrintMessage( "CMixerBase::Init",
		              "Warning: the destination line \n%s\nwas not found.", GetLineTypeString( DstType ) );
		CloseMixer();
		return 0;
	}
	
	m_DestLineName = line.szName;

	// ----- find the desired source line -----

	if( SrcType != NO_SOURCE )
	{
		if(  GetSourceLineIndex( line, SrcType ) == -1 )
		{
			PrintMessage( "CMixerBase::Init",
		                  "Warning: the source line \n%s\n not found.\n"
						  "\n(Associated to Destination line \n%s)", 
						  GetLineTypeString( SrcType ), m_DestLineName );
			CloseMixer();
			return 0;
		}
	}

	// we found the line! let's see if it has any controls...

	m_nChannels     = line.cChannels;
	m_LineName      = line.szName;
	m_LineShortName = line.szShortName;

	//TRACE( "Number of controls available: %d\n", line.cControls );
	if( line.cControls == 0 )
	{
		PrintMessage( "CMixerBase::Init", 
		              "No controls available for this audio line: %s", line.szName );
		CloseMixer();
		return 0;
	}

	// ---------- We try and find the desired control for the mixer line ---------
    
	MIXERLINECONTROLS   mixerLineControl;

	mixerLineControl.cbStruct       = sizeof( MIXERLINECONTROLS );
	mixerLineControl.dwLineID       = line.dwLineID;
	mixerLineControl.dwControlType  = ControlType;
	mixerLineControl.cControls      = 1;
	mixerLineControl.cbmxctrl       = sizeof( MIXERCONTROL );
	mixerLineControl.pamxctrl       = &m_Control;
	
	if( mixerGetLineControls( ( HMIXEROBJ )m_HMixer, &mixerLineControl, 
		MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
	{
		//TRACE( "CMixerBase::Init - mixerGetLineControls failed\n" );
		PrintMessage( "CMixerBase::Init", "Could not find specified mixer control. "
			           "Associated Windows control will be disabled.\n"
					   "Control type: %s,\n\nDest line type: %s,\nSrc  line type: %s\n", 
					   GetControlTypeString( ControlType ), 
					   GetLineTypeString( DstType ), 
					   GetLineTypeString( SrcType ) );
		CloseMixer();
		return 0;
	}

	// we got the control, now store some values for later queries

	if( !InitMXDetails() )
		return 0;
	
	return 1;
}
// mwatkins
int CMixerBase::InitFromSrcLineName( DWORD DstType, const std::wstring &inName, DWORD ControlType, HWND hwnd, UINT mixerIdx )
{
	UINT nbMixers = mixerGetNumDevs();
	//if( nbMixers < 1 )
	if (mixerIdx >= nbMixers)	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "No mixer device present" );
		return 0;
	}

	// ---- open the mixer device ----

	if ( m_HMixer )
		mixerClose( m_HMixer );

	if( mixerOpen( &m_HMixer, /*0*/mixerIdx, ( DWORD )hwnd, 0, CALLBACK_WINDOW ) != MMSYSERR_NOERROR )	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "Could not open mixer device" );
		return 0;
	}

	// ----- find the desired destination line -----
	
	MIXERLINE line;

	if( GetDestinLineIndex( line, DstType ) == -1 )
	{
		PrintMessage( "CMixerBase::Init",
		              "Warning: the destination line \n%s\nwas not found.", GetLineTypeString( DstType ) );
		CloseMixer();
		return 0;
	}
	
	m_DestLineName = line.szName;

	//***********************************************************************************
	// ----- find the desired source line -----

	if(  GetSourceLineIndex( line, inName ) == -1 )
	{
		assert(false);
		CloseMixer();
		return 0;
	}
	//************************************************************************************

	// we found the line! let's see if it has any controls...

	m_nChannels     = line.cChannels;
	m_LineName      = line.szName;
	m_LineShortName = line.szShortName;

	//TRACE( "Number of controls available: %d\n", line.cControls );
	if( line.cControls == 0 )
	{
		PrintMessage( "CMixerBase::Init", 
		              "No controls available for this audio line: %s", line.szName );
		CloseMixer();
		return 0;
	}

	// ---------- We try and find the desired control for the mixer line ---------
    
	MIXERLINECONTROLS   mixerLineControl;

	mixerLineControl.cbStruct       = sizeof( MIXERLINECONTROLS );
	mixerLineControl.dwLineID       = line.dwLineID;
	mixerLineControl.dwControlType  = ControlType;
	mixerLineControl.cControls      = 1;
	mixerLineControl.cbmxctrl       = sizeof( MIXERCONTROL );
	mixerLineControl.pamxctrl       = &m_Control;
	
	if( mixerGetLineControls( ( HMIXEROBJ )m_HMixer, &mixerLineControl, 
		MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
	{
		//TRACE( "CMixerBase::Init - mixerGetLineControls failed\n" );
		assert(false);
		CloseMixer();
		return 0;
	}

	// we got the control, now store some values for later queries

	if( !InitMXDetails() )
		return 0;
	
	return 1;
}
// mwatkins
int CMixerBase::InitFromSrcLineIdx( DWORD DstType, UINT srcLineIdx, DWORD ControlType, HWND hwnd, UINT mixerIdx )
{
	UINT nbMixers = mixerGetNumDevs();
	//if( nbMixers < 1 )
	if (mixerIdx >= nbMixers)	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "No mixer device present" );
		return 0;
	}

	// ---- open the mixer device ----

	if ( m_HMixer )
		mixerClose( m_HMixer );

	if( mixerOpen( &m_HMixer, /*0*/mixerIdx, ( DWORD )hwnd, 0, CALLBACK_WINDOW ) != MMSYSERR_NOERROR )	// mwatkins
	{
		PrintMessage( "CMixerBase::Init", "Could not open mixer device" );
		return 0;
	}

	// ----- find the desired destination line -----
	
	MIXERLINE line;

	if( GetDestinLineIndex( line, DstType ) == -1 )
	{
		PrintMessage( "CMixerBase::Init",
		              "Warning: the destination line \n%s\nwas not found.", GetLineTypeString( DstType ) );
		CloseMixer();
		return 0;
	}
	
	m_DestLineName = line.szName;

	//*********************************************************************
	// ----- find the desired source line -----

	if (srcLineIdx >= line.cConnections)	// Given source line idx must be in range!!!
	{
		assert(false);
		CloseMixer();
		return 0;
	}

	line.cbStruct = sizeof(MIXERLINE);
	line.dwSource = srcLineIdx;
	if (MMSYSERR_NOERROR != ::mixerGetLineInfo((HMIXEROBJ)m_HMixer, &line, MIXER_GETLINEINFOF_SOURCE))
	{
		assert(false);
		CloseMixer();
		return 0;
	}

	//*********************************************************************

	// we found the line! let's see if it has any controls...

	m_nChannels     = line.cChannels;
	m_LineName      = line.szName;
	m_LineShortName = line.szShortName;

	//TRACE( "Number of controls available: %d\n", line.cControls );
	if( line.cControls == 0 )
	{
		PrintMessage( "CMixerBase::Init", 
		              "No controls available for this audio line: %s", line.szName );
		CloseMixer();
		return 0;
	}

	// ---------- We try and find the desired control for the mixer line ---------
    
	MIXERLINECONTROLS   mixerLineControl;

	mixerLineControl.cbStruct       = sizeof( MIXERLINECONTROLS );
	mixerLineControl.dwLineID       = line.dwLineID;
	mixerLineControl.dwControlType  = ControlType;
	mixerLineControl.cControls      = 1;
	mixerLineControl.cbmxctrl       = sizeof( MIXERCONTROL );
	mixerLineControl.pamxctrl       = &m_Control;
	
	if( mixerGetLineControls( ( HMIXEROBJ )m_HMixer, &mixerLineControl, 
		MIXER_GETLINECONTROLSF_ONEBYTYPE ) != MMSYSERR_NOERROR )
	{
		assert(false);	// Notify developer
		CloseMixer();
		return 0;
	}

	// we got the control, now store some values for later queries

	if( !InitMXDetails() )
		return 0;
	
	return 1;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetDestinLineIndex
//          
// Descr. : Loops through all possible destination lines, and returns the
//          index of the line that matches the type passed as argument.
//          The MIXERLINE structure is also filled along the way by the call
//          to mixerGetLineInfo().
//          
// Return : int : index of line found, otherwise -1
//
// Arg    : MIXERLINE &line : 
// Arg    : DWORD lineType  : 
//-----------------------------------------------------------------------------
int CMixerBase::GetDestinLineIndex( MIXERLINE &line, DWORD lineType )
{
	// ----- find out how many destination lines are available -----

	MIXERCAPS caps;
	if( mixerGetDevCaps( (UINT)m_HMixer, &caps, sizeof(MIXERCAPS) ) != MMSYSERR_NOERROR )
	{
		return -1;
	}

	int ndest = caps.cDestinations;
	for( int i = 0; i < ndest; i++ )
	{
		line.cbStruct = sizeof( MIXERLINE );
		line.dwSource = 0;
		line.dwDestination = i;
		if( mixerGetLineInfo( ( HMIXEROBJ )m_HMixer, &line, 
							  MIXER_GETLINEINFOF_DESTINATION ) != MMSYSERR_NOERROR )
		{
			return -1;
		}
		
		if( line.dwComponentType == lineType )
		{
			return( i );
		}		
	}
	return -1;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : GetSourceLineIndex
//          
// Descr. : Loops through all possible source lines associated with a given
//          destination line, and returns the index of the line that matches
//          the type passed as argument. The destination line index is taken
//          from the dwDestination member of the MIXERLINE structure, so this
//          member must be properly set before calling this function.
//          The MIXERLINE structure is also filled along the way by the call
//          to mixerGetLineInfo().
//          
// Return : int
// Arg    : MIXERLINE &line : 
// Arg    : DWORD lineType  : 
//-----------------------------------------------------------------------------
int CMixerBase::GetSourceLineIndex( MIXERLINE &line, DWORD lineType )
{
	UINT  nconn    = line.cConnections;
	DWORD DstIndex = line.dwDestination;

	for( UINT j = 0; j < nconn; j++ )
	{
		line.cbStruct = sizeof( MIXERLINE );
		line.dwSource = j;
		line.dwDestination = DstIndex;
		if( mixerGetLineInfo( ( HMIXEROBJ )m_HMixer, &line, 
							  MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR )
		{
			return -1;
		}
		
		if( line.dwComponentType == lineType )
		{
			return( j );
		}			
	}
	return -1;
}
// mwatkins
int CMixerBase::GetSourceLineIndex( MIXERLINE &line, const std::wstring &inName )
{
	UINT  nconn    = line.cConnections;
	DWORD DstIndex = line.dwDestination;

	for( UINT j = 0; j < nconn; j++ )
	{
		line.cbStruct = sizeof( MIXERLINE );
		line.dwSource = j;
		line.dwDestination = DstIndex;
		if( mixerGetLineInfo( ( HMIXEROBJ )m_HMixer, &line, 
							  MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR )
		{
			return -1;
		}
		
		if( 0 == ::wcscmp(line.szName, inName.c_str()) )
		{
			return( j );
		}			
	}
	return -1;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : InitMXDetails
//          
// Descr. : prepare the MIXERCONTROLDETAILS structure for queries, and allocate
//          the proper amount of memory to hold the values that will be sent to
//          and queried from the mixer device.
//
// Return : void
//-----------------------------------------------------------------------------
int CMixerBase::InitMXDetails()
{
	m_Details.cbStruct       = sizeof( MIXERCONTROLDETAILS );
    m_Details.dwControlID    = m_Control.dwControlID;
	m_Details.cMultipleItems = m_Control.cMultipleItems;
	
	if( m_Control.fdwControl & MIXERCONTROL_CONTROLF_UNIFORM )
	{
		m_Details.cChannels = 1;
	}
	else
	{
		m_Details.cChannels = m_nChannels;
	}

	switch( m_Control.dwControlType & MIXERCONTROL_CT_UNITS_MASK )
	{
		case MIXERCONTROL_CT_UNITS_UNSIGNED:
		case MIXERCONTROL_CT_UNITS_DECIBELS:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_UNSIGNED );
			break;
		
		case MIXERCONTROL_CT_UNITS_PERCENT:
		case MIXERCONTROL_CT_UNITS_SIGNED:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_SIGNED );
			break;

		case MIXERCONTROL_CT_UNITS_BOOLEAN:
			m_Details.cbDetails = sizeof( MIXERCONTROLDETAILS_BOOLEAN );
			break;
		
		default:
			PrintMessage( "CMixerBase::InitMXDetails", "Control uses unknown units." );
			CloseMixer();
			return 0;
	}

	int nItems = 1;
	if( m_Control.fdwControl & MIXERCONTROL_CONTROLF_MULTIPLE )
	{
		nItems = m_Control.cMultipleItems;
	}
	nItems *= m_Details.cChannels;

	if( m_val ) delete[] m_val;
	m_val = new char[ nItems* m_Details.cbDetails ];
	if( m_val == NULL )
	{
		PrintMessage( "CMixerBase::InitMXDetails", "Memory allocation error." );
		CloseMixer();
		return 0;
	}

	m_Details.paDetails = m_val;

	return 1;
}

int CMixerBase::SetMixerControlValue( )
{
	if( mixerSetControlDetails( ( HMIXEROBJ )m_HMixer, &m_Details, 
			                      MIXER_SETCONTROLDETAILSF_VALUE ) 
		!= MMSYSERR_NOERROR )
	{
		//TRACE("CMixerBase::SetMixerControlValue mixerSetControlDetails error\n");
		return 0;
	}

	return 1;
}

int CMixerBase::GetMixerControlValue( )
{
	if( mixerGetControlDetails( ( HMIXEROBJ )m_HMixer, &m_Details, 
			                      MIXER_SETCONTROLDETAILSF_VALUE ) 
		!= MMSYSERR_NOERROR )
	{
		//TRACE("CMixerDialog::GetMixerControlValue mixerGetControlDetails error\n");
		return 0;
	}
	
	return 1;
}

LPCTSTR CMixerBase::GetControlTypeString( DWORD type )
{
	switch( type )
	{
		case MIXERCONTROL_CONTROLTYPE_CUSTOM:
			return (  L"MIXERCONTROL_CONTROLTYPE_CUSTOM" );
		case MIXERCONTROL_CONTROLTYPE_BOOLEANMETER:
			return (  L"MIXERCONTROL_CONTROLTYPE_BOOLEANMETER" );
		case MIXERCONTROL_CONTROLTYPE_SIGNEDMETER:
			return (  L"MIXERCONTROL_CONTROLTYPE_SIGNEDMETER" );
		case MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER:
			return (  L"MIXERCONTROL_CONTROLTYPE_UNSIGNEDMETER" );
		case MIXERCONTROL_CONTROLTYPE_PEAKMETER:
			return (  L"MIXERCONTROL_CONTROLTYPE_PEAKMETER" );
		case MIXERCONTROL_CONTROLTYPE_BOOLEAN:
			return (  L"MIXERCONTROL_CONTROLTYPE_BOOLEAN" );
		case MIXERCONTROL_CONTROLTYPE_ONOFF:
			return (  L"MIXERCONTROL_CONTROLTYPE_ONOFF" );
		case MIXERCONTROL_CONTROLTYPE_MUTE:
			return (  L"MIXERCONTROL_CONTROLTYPE_MUTE" );
		case MIXERCONTROL_CONTROLTYPE_MONO:
			return (  L"MIXERCONTROL_CONTROLTYPE_MONO" );
		case MIXERCONTROL_CONTROLTYPE_LOUDNESS:
			return (  L"MIXERCONTROL_CONTROLTYPE_LOUDNESS" );
		case MIXERCONTROL_CONTROLTYPE_STEREOENH:
			return (  L"MIXERCONTROL_CONTROLTYPE_STEREOENH" );
		case MIXERCONTROL_CONTROLTYPE_BUTTON:
			return (  L"MIXERCONTROL_CONTROLTYPE_BUTTON" );
		case MIXERCONTROL_CONTROLTYPE_DECIBELS:
			return (  L"MIXERCONTROL_CONTROLTYPE_DECIBELS" );
		case MIXERCONTROL_CONTROLTYPE_SIGNED:
			return (  L"MIXERCONTROL_CONTROLTYPE_SIGNED" );
		case MIXERCONTROL_CONTROLTYPE_UNSIGNED:
			return (  L"MIXERCONTROL_CONTROLTYPE_UNSIGNED" );
		case MIXERCONTROL_CONTROLTYPE_PERCENT:
			return (  L"MIXERCONTROL_CONTROLTYPE_PERCENT" );
		case MIXERCONTROL_CONTROLTYPE_SLIDER:
			return (  L"MIXERCONTROL_CONTROLTYPE_SLIDER" );
		case MIXERCONTROL_CONTROLTYPE_PAN:
			return (  L"MIXERCONTROL_CONTROLTYPE_PAN" );
		case MIXERCONTROL_CONTROLTYPE_QSOUNDPAN:
			return (  L"MIXERCONTROL_CONTROLTYPE_QSOUNDPAN" );
		case MIXERCONTROL_CONTROLTYPE_FADER:
			return (  L"MIXERCONTROL_CONTROLTYPE_FADER" );
		case MIXERCONTROL_CONTROLTYPE_VOLUME:
			return (  L"MIXERCONTROL_CONTROLTYPE_VOLUME" );
		case MIXERCONTROL_CONTROLTYPE_BASS:
			return (  L"MIXERCONTROL_CONTROLTYPE_BASS" );
		case MIXERCONTROL_CONTROLTYPE_TREBLE:
			return (  L"MIXERCONTROL_CONTROLTYPE_TREBLE" );
		case MIXERCONTROL_CONTROLTYPE_EQUALIZER:
			return (  L"MIXERCONTROL_CONTROLTYPE_EQUALIZER" );
		case MIXERCONTROL_CONTROLTYPE_SINGLESELECT:
			return (  L"MIXERCONTROL_CONTROLTYPE_SINGLESELECT" );
		case MIXERCONTROL_CONTROLTYPE_MUX:
			return (  L"MIXERCONTROL_CONTROLTYPE_MUX" );
		case MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT:
			return (  L"MIXERCONTROL_CONTROLTYPE_MULTIPLESELECT" );
		case MIXERCONTROL_CONTROLTYPE_MIXER:
			return (  L"MIXERCONTROL_CONTROLTYPE_MIXER" );
		case MIXERCONTROL_CONTROLTYPE_MICROTIME:
			return (  L"MIXERCONTROL_CONTROLTYPE_MICROTIME" );
		case MIXERCONTROL_CONTROLTYPE_MILLITIME:
			return (  L"MIXERCONTROL_CONTROLTYPE_MILLITIME" );
			
		default:
			return (  L"---- UNKNOWN CONTROL TYPE ----" );
	}
}

LPCTSTR CMixerBase::GetLineTypeString( DWORD type )
{
	switch( type )
	{
		case MIXERLINE_COMPONENTTYPE_DST_DIGITAL:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_DIGITAL" );
		case MIXERLINE_COMPONENTTYPE_DST_UNDEFINED:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_UNDEFINED" );
		case MIXERLINE_COMPONENTTYPE_DST_LINE:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_LINE");
		case MIXERLINE_COMPONENTTYPE_DST_MONITOR:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_MONITOR");
		case MIXERLINE_COMPONENTTYPE_DST_SPEAKERS:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_SPEAKERS");
		case MIXERLINE_COMPONENTTYPE_DST_HEADPHONES:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_HEADPHONES");
		case MIXERLINE_COMPONENTTYPE_DST_TELEPHONE:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_TELEPHONE");
		case MIXERLINE_COMPONENTTYPE_DST_WAVEIN:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_WAVEIN");
		case MIXERLINE_COMPONENTTYPE_DST_VOICEIN:
			return ( L"MIXERLINE_COMPONENTTYPE_DST_VOICEIN");
		case MIXERLINE_COMPONENTTYPE_SRC_ANALOG:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_ANALOG");
		case MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_AUXILIARY");
		case MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_COMPACTDISC");
		case MIXERLINE_COMPONENTTYPE_SRC_DIGITAL:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_DIGITAL");
		case MIXERLINE_COMPONENTTYPE_SRC_LINE:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_LINE");
		case MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE");
		case MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_PCSPEAKER");
		case MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_SYNTHESIZER");
		case MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_TELEPHONE");
		case MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_UNDEFINED");
		case MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT:
			return ( L"MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT");
		case NO_SOURCE:
			return ( L"No source line");
		default:
			return (  L"---- UNKNOWN LINE TYPE ----" );
	}
}

//*************************************************************************************************************************************************

CMixerFader::CMixerFader() : CMixerBase()
{
	m_Value = NULL;
	m_DeltaOverMax = 0;
	m_dwSliderValue = 0;
}

CMixerFader::~CMixerFader()
{
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : Init
//          
// Descr. : Initializes the mixer control. Most of the work is done in the
//          base class' implementation of Init.
//          Init queries the mixer device for the specified mixer control type.
//          If such a control is found, its current value is queried, and used
//          to initialize the windows control.
//        - If the call to CMixerBase::Init() is succesfull, m_Value is set to
//          the address of the memory allocated by CMixerBase.
//          
// Return : int 0 if failed, 1 otherwise
//
// Arg    : DWORD DstType    : see definition of CMixerBase::Init() for a description
// Arg    : DWORD SrcType    : see definition of CMixerBase::Init() for a description
// Arg    : DWORD ControlType : type of desired fader.
//
//          These are valid arguments:
//
//              MIXERCONTROL_CONTROLTYPE_FADER 
//              MIXERCONTROL_CONTROLTYPE_VOLUME
//              MIXERCONTROL_CONTROLTYPE_BASS
//              MIXERCONTROL_CONTROLTYPE_TREBLE
//              MIXERCONTROL_CONTROLTYPE_EQUALIZER
//
// Arg    : int subType: If the mixer line has two channels, the subtype can be used to
//                       create a fader for the left or right channel, or as a balance
//                       between the channels, or as a main fader that controls both
//                       channels.
//                       If only one channel is available, only the MAIN subtype may be
//                       specified.
//                       These are valid arguments:
//                       
//                       CMixerFader::LEFT
//                       CMixerFader::RIGHT
//                       CMixerFader::MAIN
//                       CMixerFader::BALANCE
//                       
//-----------------------------------------------------------------------------
int CMixerFader::Init( DWORD DstType, DWORD SrcType, DWORD ControlType, int subType, UINT mixerIdx/*mwatkins*/ )
{	
	// start by checking the type and subtypes 

	if( ( ControlType & MIXERCONTROL_CT_CLASS_MASK ) != MIXERCONTROL_CT_CLASS_FADER )
	{
		PrintMessage( "CMixerFader::Init", 
		              "Wrong control type for a fader control: %s", 
		              GetControlTypeString( ControlType ) );
		return 0;
	}
	if( subType < LEFT || subType > BALANCE )
	{
		PrintMessage( "CMixerFader::Init", "Wrong sub-type for a fader control: %d", subType );
		return 0;
	}

	m_subType = subType;

	if( CMixerBase::Init( DstType, SrcType, ControlType, NULL, mixerIdx ) == 0 )	// mwatkins
		return 0;

	// set the slider range and number of steps from the information obtained
	// on the mixer control
	m_FaderSpan  = m_Control.Bounds.dwMaximum - m_Control.Bounds.dwMinimum;
	m_FaderStep  = m_FaderSpan/m_Control.Metrics.cSteps;
	m_SliderSpan = m_Control.Metrics.cSteps;

	if( m_nChannels < 2 )
	{
		// a balance, left, or right control on a single channel line doesn't make sense
		// abort and set the balance slider to mid-course
		if( m_subType == LEFT || m_subType == RIGHT || m_subType == BALANCE )
		{
			PrintMessage( "CMixerFader::Init", 
			              "Only one channel available on this line. Use Fader type MAIN only.\n"
			              "Windows control will be disabled.\n"
						  "\nItem: %s.\n"
					      "\n( Line: %s )", 
						  GetControlName(),
					      GetLineName() );

			CloseMixer();
			return 0;
		}
	}

	m_Value = (MIXERCONTROLDETAILS_UNSIGNED *)GetValuePtr();
	if( m_Value == NULL )
	{
		PrintMessage( "CMixerFader::Init", "m_Value NULL." );
		CloseMixer();
		return 0;
	}

	// mwatkins TODO: Should I bail if this fails as well???
	if( GetMixerControlValue() )
	{
	}

	return 1;
}
// mwatkins
int CMixerFader::InitFromSrcLineName( DWORD DstType, const std::wstring &inName, DWORD ControlType, int subType, UINT mixerIdx )
{
	// start by checking the type and subtypes 

	if( ( ControlType & MIXERCONTROL_CT_CLASS_MASK ) != MIXERCONTROL_CT_CLASS_FADER )
	{
		PrintMessage( "CMixerFader::Init", 
		              "Wrong control type for a fader control: %s", 
		              GetControlTypeString( ControlType ) );
		return 0;
	}
	if( subType < LEFT || subType > BALANCE )
	{
		PrintMessage( "CMixerFader::Init", "Wrong sub-type for a fader control: %d", subType );
		return 0;
	}

	m_subType = subType;

	if ( CMixerBase::InitFromSrcLineName( DstType, inName, ControlType, NULL, mixerIdx ) == 0 )	// mwatkins
		return 0;

	// set the slider range and number of steps from the information obtained
	// on the mixer control
	m_FaderSpan  = m_Control.Bounds.dwMaximum - m_Control.Bounds.dwMinimum;
	m_FaderStep  = m_FaderSpan/m_Control.Metrics.cSteps;
	m_SliderSpan = m_Control.Metrics.cSteps;

	if( m_nChannels < 2 )
	{
		// a balance, left, or right control on a single channel line doesn't make sense
		// abort and set the balance slider to mid-course
		if( m_subType == LEFT || m_subType == RIGHT || m_subType == BALANCE )
		{
			PrintMessage( "CMixerFader::Init", 
			              "Only one channel available on this line. Use Fader type MAIN only.\n"
			              "Windows control will be disabled.\n"
						  "\nItem: %s.\n"
					      "\n( Line: %s )", 
						  GetControlName(),
					      GetLineName() );

			CloseMixer();
			return 0;
		}
	}

	m_Value = (MIXERCONTROLDETAILS_UNSIGNED *)GetValuePtr();
	if( m_Value == NULL )
	{
		PrintMessage( "CMixerFader::Init", "m_Value NULL." );
		CloseMixer();
		return 0;
	}

	// mwatkins TODO: Should I bail if this fails as well???
	if( GetMixerControlValue() )
	{
	}

	return 1;
}
// mwatkins
int CMixerFader::InitFromSrcLineIdx( DWORD DstType, UINT srcLineIdx, DWORD ControlType, int subType, UINT mixerIdx )
{
	// start by checking the type and subtypes 

	if( ( ControlType & MIXERCONTROL_CT_CLASS_MASK ) != MIXERCONTROL_CT_CLASS_FADER )
	{
		PrintMessage( "CMixerFader::Init", 
		              "Wrong control type for a fader control: %s", 
		              GetControlTypeString( ControlType ) );
		return 0;
	}
	if( subType < LEFT || subType > BALANCE )
	{
		PrintMessage( "CMixerFader::Init", "Wrong sub-type for a fader control: %d", subType );
		return 0;
	}

	m_subType = subType;

	if( CMixerBase::InitFromSrcLineIdx( DstType, srcLineIdx, ControlType, NULL, mixerIdx ) == 0 )	// mwatkins
		return 0;

	// set the slider range and number of steps from the information obtained
	// on the mixer control
	m_FaderSpan  = m_Control.Bounds.dwMaximum - m_Control.Bounds.dwMinimum;
	m_FaderStep  = m_FaderSpan/m_Control.Metrics.cSteps;
	m_SliderSpan = m_Control.Metrics.cSteps;

	if( m_nChannels < 2 )
	{
		// a balance, left, or right control on a single channel line doesn't make sense
		// abort and set the balance slider to mid-course
		if( m_subType == LEFT || m_subType == RIGHT || m_subType == BALANCE )
		{
			PrintMessage( "CMixerFader::Init", 
			              "Only one channel available on this line. Use Fader type MAIN only.\n"
			              "Windows control will be disabled.\n"
						  "\nItem: %s.\n"
					      "\n( Line: %s )", 
						  GetControlName(),
					      GetLineName() );

			CloseMixer();
			return 0;
		}
	}

	m_Value = (MIXERCONTROLDETAILS_UNSIGNED *)GetValuePtr();
	if( m_Value == NULL )
	{
		PrintMessage( "CMixerFader::Init", "m_Value NULL." );
		CloseMixer();
		return 0;
	}

	// mwatkins TODO: Should I bail if this fails as well???
	if( GetMixerControlValue() )
	{
	}

	return 1;
}

//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : SetFaderValue
//          
// Descr. : the slider position changed, update the values and send to mixer device.
//
//          To determine the values to send to the mixer device, this algorithm is used:
//
//          - If the MAIN slider is moved:
//          The idea is to keep D = R - L constant. In other words, D1/M1 = D2/M2 where
//          D2 and M2 are the new values.
//          Now, if D1 > 0, this means that R1 = M1. This, in the abvove equality, gives:
//          -> L2 = R2( 1 - D1/R1 ) =  R2( 1 - DOM1 )
//          If D1 < 0,  L1 = M1, which leads to:
//          ->  R2 = L2( 1 + DOM1 )
//
//          - If the BALANCE slider is moved:
//          This will change D. Starting from the equation P = S / 2 * ( D/M + 1 )
//          and rearranging the terms, we obtain respectively for the rigth and left channels:
//
//          -> R = M ( 2P/S - 1 ) + L
//          -> L = R - M ( 2P/S - 1 )
//
//          If D > 0, R = M. This gives: L = 2R ( 1 - P/S )
//          If D < 0, L = M. This gives: R = 2LP/S
//
// Return : void
//-----------------------------------------------------------------------------
void CMixerFader::SetFaderValue()
{
	// mwatkins
	if (NULL == m_Value)
	{
		assert(false);
		return;
	}

	DWORD newVal = GetSliderValue();
	
	float MaxVal;

	if( m_nChannels > 1 )
	{
		MaxVal = ( float )std::max( m_Value[RIGHT].dwValue, m_Value[LEFT].dwValue );
	}
	else
	{
		MaxVal = ( float )m_Value[0].dwValue;
	}

	switch ( m_subType )
	{
		case MAIN:
			if( m_nChannels > 1 )
			{
				if( m_DeltaOverMax > 0 )
				{
					m_Value[RIGHT].dwValue = newVal;
					m_Value[LEFT].dwValue = DWORD( newVal*( 1.0f - m_DeltaOverMax ) );
				}
				else
				{
					m_Value[LEFT].dwValue = newVal;
					m_Value[RIGHT].dwValue = DWORD( newVal*( 1.0f + m_DeltaOverMax ) );
				}
			}
			else
			{
				m_Value[0].dwValue = newVal;
			}
			break;

		case BALANCE:
			if( ( int )newVal > m_FaderSpan/2 )
			{
				m_Value[RIGHT].dwValue = DWORD( MaxVal );
				m_Value[LEFT].dwValue = DWORD( 2*MaxVal*( 1.0 - ( float )newVal/m_FaderSpan ) );
			}
			else
			{
				m_Value[RIGHT].dwValue = DWORD( 2*MaxVal*( float )newVal/m_FaderSpan );
				m_Value[LEFT].dwValue = DWORD( MaxVal );
			}
			break;

		case LEFT:
			m_Value[LEFT].dwValue = newVal;
			break;

		case RIGHT:
			m_Value[RIGHT].dwValue = newVal;
			break;

		default:
			PrintMessage( "CMixerFader::SetFaderValue", "Wrong sub-type for a fader control." );
			return;
	}
	
	SetMixerControlValue();
}

void CMixerFader::SetSliderValue(float fValue)
{
	float fTemp = fValue * 65535.0f;
	fTemp = std::max(0.0f, std::min(fTemp, 65535.0f));
	DWORD dwVal = static_cast<DWORD>(fTemp);
	SetSliderValue(dwVal);
}

void CMixerFader::SetSliderValue(DWORD dwValue)
{
	m_dwSliderValue = dwValue;
	SetFaderValue();
}

DWORD CMixerFader::GetSliderValue()
{
	return m_dwSliderValue;
}

/*
//mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
// Name   : OnMixerControlChanged
//          
// Descr. : Message handler called whenever the MCI device sends the MM_MIXM_CONTROL_CHANGE
//          message. The MCI mixer device will send this message for any changing
//          setting, so we need to check that the MCI control that sent the message
//          corresponds to the current fader control.
//          
// Return : LONG
// Arg    : UINT         : 
// Arg    : LONG message : 
//-----------------------------------------------------------------------------
LONG CMixerFader::OnMixerControlChanged( UINT hMix, LONG message )
{
	UINT id = ( UINT )message;

	if( id == m_Control.dwControlID && m_HMixer == ( HMIXER )hMix )
	{
		if( GetMixerControlValue() )
		{
			SetHVSliderPos();
		}
		else
		{
			TRACE( "CMixerFader::OnMixerControlChanged --- error\n" );
		}
	}

	return 0L;
}
*/

//*************************************************************************************************************************************************
//*************************************************************************************************************************************************
