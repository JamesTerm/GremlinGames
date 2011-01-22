#include "StdAfx.h"
#include "FrameWork_Communication3_CLR.h"

namespace FrameWork { namespace Communication3 { namespace CLR {

// Determine how many messages a destination has pending to be processed.
// A negative value indicates that an error occured, for instance that the
// destination no longer exists.
int Utilities::QueueDepth( System::String ^ name )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(name));
	int iret = FrameWork::Communication3::utilities::queue_depth( xstr );
	return iret;
}

// Get the destination process ID
__int64 Utilities::HeartBeat( System::String ^ name )
{
	pin_ptr<const wchar_t> xstr(PtrToStringChars(name));
	__int64 iret = FrameWork::Communication3::utilities::heart_beat( xstr );
	return iret;
}

}}} //namespaces