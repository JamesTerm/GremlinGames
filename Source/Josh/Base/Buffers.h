#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

/*!	This is the interface that a buffer would provide. It is used in two different
	scenarios.
		1) For input devices (e.g. HD capture card)
			The buffer is passed from the switcher to the capture device.
			It then is filled with data from whatever the video device is.
			Upon "releasing" it is returned to the swticher.

		2) For output devices (e.g. HD output card)
			The buffer is passed from the switcher to the output device.
			The buffer is sent to output then "released".
*/
struct BufferInterface
{
		//! This is called by the input or output device to recover the 
		//! pointer into the buffer. and the size.
		//! For a fixed size buffer (e.g. and image) the value of pMemorySize is ignored. If you get something
		//! passed back that is not what you expected, you should ReleaseBufferData with an error.
		//! If you are working with variable size buffers, then you should pass in what you would like
		//! the memory size to be in this method. You should check the returned value to be sure that it is
		//! actually what you asked for, or at least something you can work with. 
			virtual void *pGetBufferData( size_t *pMemorySize ) = 0;

		//! Once the buffer contents have been finished with, this method is
		//! called and you can then use it as you see fit. If you are filling this buffer
		//! you can timestamp it by passing a non-NULL argument for the 1st param.  The Error message
		//! is NULL if there is no error message, or a string representing the
		//! error if there is one.
			virtual void ReleaseBufferData( const TimeStamp *pFrameTime = NULL , const wchar_t *pErrorMessage = NULL ) = 0;

		//! Accessor to get the timestamp of a buffer.  Timestamps are specified upstream.
		//! (e.g. An input device specifies the timestamp during capture.  An output device calls this
		//! accessor to get the display time of the sample.  This is how you detect dropped frames.)
			virtual const TimeStamp& GetTimeStamp( void ) const = 0;
};