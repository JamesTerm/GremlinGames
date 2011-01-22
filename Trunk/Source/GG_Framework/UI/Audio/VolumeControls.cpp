#include "StdAfx.h"
#include "GG_Framework.UI.Audio.h"
#include "..\..\..\DebugPrintMacros.hpp"

using namespace GG_Framework::Base;
using namespace GG_Framework::UI::Audio;
using namespace std;
VolumeControls::VolumeControls(Event0& VolumeSelect,Event0& VolumeUp,Event0& VolumeDown)
{
	m_MasterVolume=0.7;
	m_SoundsVolume=0.8;
	m_StreamsVolume=0.5;
	VolumeSelect.Subscribe(ehl, *this, &VolumeControls::VolumeSelect);
	VolumeUp.Subscribe(ehl, *this, &VolumeControls::VolumeUp);
	VolumeDown.Subscribe(ehl, *this, &VolumeControls::VolumeDown);
	m_Mode=eMaster;

	// Set the first time
	SOUNDSYSTEM.SetMasterVolume(m_MasterVolume);
	SOUNDSYSTEM.SetSoundsVolume(m_SoundsVolume);
	SOUNDSYSTEM.SetStreamsVolume(m_StreamsVolume);
}
//For now I'll keep this a simple linear sweep we can make it more fancy as needed
const double c_IncrementUnit=0.1;
void VolumeControls::VolumeUp()
{
	switch (m_Mode)
	{
	case eMaster:
		m_MasterVolume=min(m_MasterVolume+c_IncrementUnit,1.0);
		SOUNDSYSTEM.SetMasterVolume(m_MasterVolume);
		break;
	case eSoundEffects:
		m_SoundsVolume=min(m_SoundsVolume+c_IncrementUnit,1.0);
		SOUNDSYSTEM.SetSoundsVolume(m_SoundsVolume);
		break;
	case eMusic:
		m_StreamsVolume=min(m_StreamsVolume+c_IncrementUnit,1.0);
		SOUNDSYSTEM.SetStreamsVolume(m_StreamsVolume);
		break;
	default:
		ASSERT(false);
	}
}
void VolumeControls::VolumeDown()
{
	switch (m_Mode)
	{
	case eMaster:
		m_MasterVolume=max(m_MasterVolume-c_IncrementUnit,0.0);
		SOUNDSYSTEM.SetMasterVolume(m_MasterVolume);
		break;
	case eSoundEffects:
		m_SoundsVolume=max(m_SoundsVolume-c_IncrementUnit,0.0);
		SOUNDSYSTEM.SetSoundsVolume(m_SoundsVolume);
		break;
	case eMusic:
		m_StreamsVolume=max(m_StreamsVolume-c_IncrementUnit,0.0);
		SOUNDSYSTEM.SetStreamsVolume(m_StreamsVolume);
		break;
	default:
		ASSERT(false);
	}
}
void VolumeControls::VolumeSelect()
{
	m_Mode = (VolumeModes)((m_Mode+1) % eNoModes);
	//TODO find some way to display mode
	const char *cs_Mode;
	switch (m_Mode)
	{
		case eMaster:
			cs_Mode="Master Volume";
			break;
		case eSoundEffects:
			cs_Mode="Sound Effects Volume";
			break;
		case eMusic:
			cs_Mode="Music Volume";
			break;
		default:
			ASSERT(false);
			cs_Mode="Doh";
			printf("mode=%d\n",m_Mode);
			break;
	}
	printf("Volume mode set to %s\n",cs_Mode);
}
