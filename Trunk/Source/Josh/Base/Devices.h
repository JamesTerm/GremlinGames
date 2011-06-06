#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

struct DeviceInterface
{		// Add a queue to the list of either input or output buffers
			virtual bool AddBufferToQueue( BufferInterface *pBuffer ) = 0;

		// This function will flush all the buffers that are "in flight" in the queue.
		// upon return there should be no more buffers submitted or pending a return
		// value. This is important so that someone calling the device can correctly
		// free all their buffers when they are closing down.
			virtual void FlushAllBuffers( void ) = 0;

		// Return the name of this device. This is how inputs are referred to in the
		// switcher.
			virtual const wchar_t *pGetDeviceName( void ) const = 0;
};	
