#pragma once
#define Devices_Base_Included

#ifdef DEVICES_BASE_EXPORTS
#define DEVICES_BASE_API __declspec(dllexport)
#else
#define DEVICES_BASE_API __declspec(dllimport)
#endif

#include "../../FrameWork/Base/FrameWork_Base.h"

// Standard C++ support
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <algorithm>
#include <functional>
#include <string>
#include <boost/smart_ptr.hpp>

//Be nice to projects which use the WIN32_LEAN_AND_MEAN define
#ifndef byte
typedef BYTE byte;
#endif

namespace Devices
{
	#include "TimeStamps.h"

	#include "Buffers.h"
	#include "Format.h"
	#include "Devices.h"

	#include "Devices_Config.h"	

	namespace Streams
	{
		#include "DeviceStreamInterfaces.h"
		#include "DeviceStream.h"
	}

	namespace Audio
	{
		#include "Format_Audio.h"
		#include "Buffers_Audio.h"
		#include "Devices_Audio.h"
		namespace Base
		{
			namespace Streams
			{
				#include "Audio_DeviceStream.h"
			};
		};
	};

	namespace Video
	{
		#include "Format_Video.h"
		#include "Buffers_Video.h"
		#include "Devices_Video.h"
		namespace Base
		{
			namespace Streams
			{
				#include "Video_DeviceStream.h"
			};
		}
	};

} // namespace Devices