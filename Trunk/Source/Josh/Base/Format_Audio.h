#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

struct FormatInterface
{		// Get the sample rate
		virtual size_t GetSampleRate( void ) const = 0;

		// Get the number of channels
		virtual size_t GetNoChannels( void ) const = 0;

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
		virtual bool GetIsBufferInterleaved( void ) const = 0;
};