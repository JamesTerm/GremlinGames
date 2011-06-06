#pragma once


#ifdef DEVICES_AUDIO_ASIO_EXPORTS
#define ASIOHost_DLL __declspec(dllexport)
#else
#define ASIOHost_DLL __declspec(dllimport)
#endif

#include "../../../FrameWork/Base/FrameWork_Base.h"

//I am dependant on the base only as long as I make the AudioHostDevice within this project
#include "../Base/Devices_Audio_Base.h"
//Otherwise I could just include these interfaces
/*
#include "../DevicesAudioInterface.h"
#include "../Base/StreamingAudioInterface.h"
*/

namespace Devices
{
	namespace Audio
	{
		namespace ASIO
		{
			#include "asiosdk/iasiodrv.h"
			#include "asiosdk/host/asiolist.h"

			//Local includes
			#include "AsioDriver.h"
			#include "ASIOMgr.h"

		}; //namespace ASIO
	}; // namespace Audio
};  //Namespace Devices
