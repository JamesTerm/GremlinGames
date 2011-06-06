#include "StdAfx.h"

// Include the main project
#include "Devices_Audio_Base.h"

namespace Devices
{
	namespace Audio
	{
		namespace Base
		{

bool operator== ( Devices::Audio::FormatInterface &Op1 , Devices::Audio::FormatInterface &Op2 )
{	
	return ( (Op1.GetSampleRate()==Op2.GetSampleRate()) &&
			(Op1.GetSampleFormat()==Op2.GetSampleFormat()) &&
			(Op1.GetIsBufferInterleaved()==Op2.GetIsBufferInterleaved()) &&
			(Op1.GetNoChannels()==Op2.GetNoChannels()) ) ? true : false;
}

bool operator!= ( Devices::Audio::FormatInterface &Op1 , Devices::Audio::FormatInterface &Op2 )
{	
	return !(Op1 == Op2);
}

		} //namespace Base
	} //namespace Audio
} //namespace Devices
