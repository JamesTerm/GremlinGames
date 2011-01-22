// GG_Framework.UI.Audio.h
#pragma once

#ifdef FRAMEWORK_UI_AUDIO_EXPORTS
#define FRAMEWORK_UI_AUDIO_API __declspec(dllexport)
#else
#define FRAMEWORK_UI_AUDIO_API __declspec(dllimport)
#endif

//////////////////
// Other Libraries
#include "..\..\..\CompilerSettings.h"
#include "..\..\Base\GG_Framework.Base.h"

///////////////////////
// Fringe.UI.Audio Includes
#include "Interfaces.h"
#include "SoundEffects3d.h"
#include "VolumeControls.h"
