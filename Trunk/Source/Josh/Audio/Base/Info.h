#pragma once


//Note: these extensive format types are used exclusively within the host domain, 
//because everything upstream from here will be working with floating point

/*
class AudConvert_FormatInterface : public FormatInterface
{
	public:

		// Get the sample size
		enum eSampleFormat
		{	eSampleFormat_Unknown ,		// Unknown format
			eSampleFormat_FP_32 ,		// 32 bit floating point
			eSampleFormat_I_32 ,		// 32 bit signed integer
			eSampleFormat_I_16 ,		// 16 bit signed integer
			eSampleFormat_U_8 			// 8 bit unsigned integer
		};

		virtual eSampleFormat GetSampleFormat( void ) const = 0;

		//Return true If the buffer is interleaved
		virtual bool GetIsBufferInterleaved( void ) const=0;
};
*/

// This returns the byte depth really.  Badly named function IMHO.
DEVICES_AUDIO_BASE_API size_t GetFormatBitDepth(const Devices::Audio::FormatInterface::eSampleFormat SampleFormat);
DEVICES_AUDIO_BASE_API Devices::Audio::FormatInterface::eSampleFormat GetFormatFromWaveFormat(const WAVEFORMATEX &WaveFormat);
