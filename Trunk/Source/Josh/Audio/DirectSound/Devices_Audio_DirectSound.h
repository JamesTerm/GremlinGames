#pragma once

#ifdef DEVICES_AUDIO_DIRECTSOUND_EXPORTS
#define DEVICES_AUDIO_DIRECTSOUND_API __declspec(dllexport)
#else
#define DEVICES_AUDIO_DIRECTSOUND_API __declspec(dllimport)
#endif

#include <vector>
#include <dsound.h>

#include "../../../FrameWork/Threads/FrameWork.Threads.h"
#include "../../../FrameWork/Base/FrameWork_Base.h"

//I am dependant on the base only as long as I make the AudioHostDevice within this project
#include "../Base/Devices_Audio_Base.h"
//Otherwise I could just include these interfaces
/*
#include "../DevicesAudioInterface.h"
#include "../Base/StreamingAudioInterface.h"
*/

//Local includes
namespace Devices
{
	namespace Audio
	{
		namespace DirectSound
		{
			#include "DirectSoundHost.h"

		} //namespace DirectSound
	} // namespace Audio
}  //Namespace Devices
