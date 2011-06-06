#pragma once

#ifdef SINEWAVEGENERATOR_EXPORTS
#define SINEWAVEGENERATOR_API __declspec(dllexport)
#else
#define SINEWAVEGENERATOR_API __declspec(dllimport)
#endif

#include "../Base/Devices_Audio_Base.h"
namespace Devices
{
	namespace Audio
	{
		namespace SineWave
		{
			#include "SineWaveGenerator.h"
		}; //namespace SineWave
	}; // namespace Audio
};  //Namespace Devices