#include "StdAfx.h"
// Include the main project
#include "Devices_Audio_Base.h"

namespace Devices
{
	namespace Audio
	{
		namespace Base
		{
			namespace aud_convert
			{

size_t GetFormatBitDepth(const Devices::Audio::FormatInterface::eSampleFormat SampleFormat)
{
	size_t BytesPerSample;
	switch(SampleFormat)
	{	case Devices::Audio::FormatInterface::eSampleFormat_FP_32:
			BytesPerSample = sizeof(float);
			break;

		case Devices::Audio::FormatInterface::eSampleFormat_I_32:
			BytesPerSample = sizeof(int);
			break;

		case Devices::Audio::FormatInterface::eSampleFormat_I_16:
			BytesPerSample = sizeof(unsigned short);
			break;

		case Devices::Audio::FormatInterface::eSampleFormat_U_8:
			BytesPerSample = sizeof(unsigned char);
			break;

		case Devices::Audio::FormatInterface::eSampleFormat_Unknown:
		default:	
			assert(false);
			BytesPerSample = 0;
			break;
	}
	return BytesPerSample;
}

//note: -32 is float 32 is long
Devices::Audio::FormatInterface::eSampleFormat GetFormatFromWaveFormat(const WAVEFORMATEX &WaveFormat)
{
	Devices::Audio::FormatInterface::eSampleFormat ret=Devices::Audio::FormatInterface::eSampleFormat_Unknown;
	switch (WaveFormat.wBitsPerSample)
	{
	case 32:
		if (WaveFormat.wFormatTag==WAVE_FORMAT_PCM)
			ret=Devices::Audio::FormatInterface::eSampleFormat_I_32;
		else if ((WaveFormat.wFormatTag==WAVE_FORMAT_EXTENSIBLE)||(WaveFormat.wFormatTag==WAVE_FORMAT_IEEE_FLOAT))
			ret=Devices::Audio::FormatInterface::eSampleFormat_FP_32;
		break;
	case 16:
		ret=Devices::Audio::FormatInterface::eSampleFormat_I_16;
		break;
	case 8:
		ret=Devices::Audio::FormatInterface::eSampleFormat_U_8;
		break;
	}
	return ret;
}

			}// namespace aud_convert
		}
	}
}