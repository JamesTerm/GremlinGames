#pragma

#ifndef Devices_Base_Included
	#error Devices Base must be included instead of including this file directly.
#endif Devices_Base_Included

struct FormatInterface
{		// Get the video resolution
		virtual size_t GetXResolution( void ) const = 0;
		virtual size_t GetYResolution( void ) const = 0;

		// Get the video format
		enum eFrameFormat
		{	eFrameFormat_Progressive ,
			eFrameFormat_Fielded_Field0_above_Field1 ,
			eFrameFormat_Fielded_Field1_above_Field0
		};

		// Is field 0 above field 1 ?
		virtual eFrameFormat GetFrameFormat( void ) const = 0;

		// Get the aspect ratio (e.g. 4.0f/3.0f or 16.0f/9.0f)
		virtual float GetAspectRatio( void ) const = 0;

		// Get the FourCC code
		// You can return NULL to this if you do not care about the format,
		// however you need to bear in mind that you do need to ensure that
		// whatever you are talking about does know what to do. An example
		// is in the VMR9 decoder, which can decode to a large number of formats
		// (anything that is render-able by the GPU)
		virtual const char *pGetFourCC( void ) const = 0;
};