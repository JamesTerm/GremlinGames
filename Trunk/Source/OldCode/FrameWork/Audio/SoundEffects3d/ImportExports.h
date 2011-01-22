#pragma once

#ifdef FRAMEWORK_AUDIO_SOUNDEFFECTS3D_EXPORTS
#define FRAMEWORK_AUDIO_SOUNDEFFECTS3D_API __declspec(dllexport)
#else
#define FRAMEWORK_AUDIO_SOUNDEFFECTS3D_API __declspec(dllimport)
#endif
