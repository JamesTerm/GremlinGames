
#pragma once

#ifdef TBC_EXPORTS
#define AudioTBC_DLL __declspec(dllexport)
//#pragma message( "     * DeckControl : Exporting Functions")
#else
#define AudioTBC_DLL __declspec(dllimport)
//#pragma message( "     . DeckControl : Importing Functions")
#endif TBC_EXPORTS

#ifdef TBC_EXPORTS
#else TBC_EXPORTS
#endif TBC_EXPORTS

//Project Dependencies...
#include "../../../FrameWork/Threads/FrameWork.Threads.h"
#include "../../../FrameWork/Base/FrameWork_Base.h"
#include "../Base/Devices_Audio_Base.h"

//Local includes
namespace Devices
{
	namespace Audio
	{
		namespace TBC
		{
			#include "AudioTBC.h"

		}; // namespace TBC
	}; // namespace Audio
}; // namespace Devices