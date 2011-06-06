#pragma once

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

struct BufferInterface : public Devices::BufferInterface , public FormatInterface
{		// Video frames have a line pitch that might not be equal to their
		// actual image width
		// Important note : You can only call this once you already have the buffer
		// data.
			virtual int GetFramePitch( void ) const = 0;

		// This is called by the driver BEFORE the buffer is released to let
		// the buffer know what it's field number is.
			virtual void SetFrameInformation( const int FieldNumber ) = 0;
			virtual void GetFrameInformation( int &FieldNumber ) const = 0;
			// I'm assuming that based on the name of this function, we possibly plan on added more
			// information to a frame.  Ergo, the field number is returned as an outparam rather than 
			// a function ret val.
};