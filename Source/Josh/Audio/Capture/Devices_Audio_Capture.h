// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CAPTURE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CAPTURE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DEVICES_AUDIO_CAPTURE_EXPORTS
#define DEVICES_AUDIO_CAPTURE_API __declspec(dllexport)
#else
#define DEVICES_AUDIO_CAPTURE_API __declspec(dllimport)
#endif

#include "../../Base/Devices_Base.h"
#include "../../../FrameWork/Base/Framework_Base.h"
#include "../../../FrameWork/Threads/FrameWork.Threads.h"
#include "../Base/Devices_Audio_Base.h"
#include "../PeakLevels/Devices_Audio_PeakLevels.h"

namespace Devices { namespace AudioVideo { namespace DSCapture {
#include "../../AudioVideo/DSCapture/Interfaces.h"
} } }

namespace Devices
{	
	namespace Audio
	{	
		namespace Capture
		{
			#include "DeviceBase.h"
			#include "DirectSoundDevice.h"
			#include "WAVOutput.h"
		}
	}
}

