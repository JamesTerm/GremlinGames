#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

// This is an (optional) interface that you can have which 
// is used by someone connected to a device. This is used to
// notify the downstream filter whether you currently have enough
// buffers or not. 
struct DeviceConfig
{	// These are recommendations. I suggest that you
	// put a range on the "correct" buffer length, like
	// if (NoBuffers<3) return BufferLength_ToShort;
	// if (NoBuffers>5) return BufferLength_ToLong;
	// return BufferLength_CorrectLength;
	enum eBufferLength
	{	BufferLength_ToShort ,
		BufferLength_CorrectLength ,
		BufferLength_ToLong
	};

	// If you want another buffer to be sent to you, return true.
	virtual eBufferLength IsBufferQueueSufficientlyLong( void ) const = 0;
};