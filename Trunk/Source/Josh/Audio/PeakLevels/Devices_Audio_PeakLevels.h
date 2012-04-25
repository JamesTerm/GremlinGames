// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DEVICESVIDEODIRECTDRAW_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DEVICESVIDEODIRECTDRAW_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DEVICES_AUDIO_PEAKLEVELS_EXPORTS
#define DEVICES_AUDIO_PEAKLEVELS_API __declspec(dllexport)
#else
#define DEVICES_AUDIO_PEAKLEVELS_API __declspec(dllimport)
#endif

#include "..\..\Base\Devices_Base.h"
#include "..\..\..\FrameWork\Base\Framework_Base.h"
#include "..\..\..\FrameWork\Threads\FrameWork.Threads.h"

namespace Devices
{	
	namespace Audio
	{	
		namespace PeakLevels
		{
			#include "Device.h"
		}
	}
}