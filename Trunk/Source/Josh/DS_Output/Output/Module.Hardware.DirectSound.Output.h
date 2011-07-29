// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OUTPUT_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OUTPUT_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef OUTPUT_EXPORTS
#define OUTPUT_API __declspec(dllexport)
#else
#define OUTPUT_API __declspec(dllimport)
#endif

#include "../../../../FrameWork/Debug/FrameWork.Debug.h"
#include "../../../../FrameWork/Threads/FrameWork.Threads.h"
#include "../../../../FrameWork/Communication3/FrameWork.Communication3.h"
#include "../../../../Processing/fx/Processing.FX.h"
#include "../../../../FrameWork/Versions/FrameWork.Versions.h"
//TODO we may want to move some of the common function from this into a separate base class that is independent on hardware (e.g. tools)
#include "../../HD/Base/Modules.Hardware.HD.Base.h"
#include <queue>
#include <map>
#include <set>
#include <vector>
#include <mmreg.h>
#ifndef SCOPED_PTR
#define SCOPED_PTR std::tr1::shared_ptr
#endif

namespace Modules
{
	namespace Hardware
	{
		namespace DirectSound
		{
			//Note: this file only exists in the 2008 database
			static const wchar_t *p_debug_category = L"DS_OUT";

			#include "DS_Output.h"
			#include "OutputModule.h"
		}
	}
}
