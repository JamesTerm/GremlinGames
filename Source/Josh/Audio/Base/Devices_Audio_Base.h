// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DEVICES_AUDIO_BASE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DEVICES_AUDIO_BASE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DEVICES_AUDIO_BASE_EXPORTS
#define DEVICES_AUDIO_BASE_API __declspec(dllexport)
#else
#define DEVICES_AUDIO_BASE_API __declspec(dllimport)
#endif

#include <MMReg.h>
#include <vector>
#include <queue>
#include <map>

#include "../../../CompilerSettings.h"

//Project Dependencies...
#include "../../../FrameWork/Threads/FrameWork.Threads.h"
#include "../../../FrameWork/Base/FrameWork_Base.h"
#include "../../Base/Devices_Base.h"
#include "../../../FrameWork/Audio/FrameWork.Audio.h"

namespace Devices
{
	namespace Audio
	{
		namespace Base
		{

			namespace aud_convert
			{
				#include "Info.h"
				#include "AudioConverters.h"
				#include "QueueConverters.h"
			} //namespace aud_convert

			#include "Operators.h"
			namespace Host
			{
				#include "HostAudioInterface.h"
			} // namespace Host
			namespace Streams
			{
				#include "Audio_DeviceStream.h"
			} // namespace Streams

			namespace Host
			{
				#include "AudioHost.h"
			} //namespace Host

			#include "AudioHostDevice.h"
		} // namespace Base
	} // namespace Audio
} // namespace Devices
