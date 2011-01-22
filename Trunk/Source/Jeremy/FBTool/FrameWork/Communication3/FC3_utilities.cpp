#include "StdAfx.h"
#include "FrameWork.Communication3.h"

using namespace FC3;
using namespace FC3::implementation;

// Get the current instantenous queue depth on a remote server
const DWORD FC3::utilities::queue_depth( const wchar_t server_name[] )
{	// Get the server pointer
	implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
	if ( !p_server ) return 0;

	// Get the queue depth
	const DWORD ret = p_server->queue_depth();

	// Release the server
	p_server->release();

	// Return the result
	return ret;
}

// This will recover the heart-beat time for a particular server.
// This represents the last ::QueryPeformanceCounter time that the server in
// question was considered alive.
const __int64 FC3::utilities::heart_beat( const wchar_t server_name[] )
{	// Get the server pointer
	implementation::server *p_server = implementation::server_cache::get_cache().get_server( server_name );
	if ( !p_server ) return 0;

	// Get the queue depth
	const __int64 ret = p_server->heart_beat();

	// Release the server
	p_server->release();

	// Return the result
	return ret;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0 )
{	// Return whether it timed out
	return t0.wait( time_out );
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

bool FC3::utilities::multiwait::all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ), get_event_handle( t3 ) };
	return ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, TRUE, time_out ) != WAIT_TIMEOUT;
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0 )
{	// Return whether it timed out
	return t0.wait( time_out ) ? 0 : -1;
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return 0;
		case WAIT_OBJECT_0 + 1:		return 1;
		default:					return -1;
	}
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return 0;
		case WAIT_OBJECT_0 + 1:		return 1;
		case WAIT_OBJECT_0 + 2:		return 2;
		default:					return -1;
	}
}

int FC3::utilities::multiwait::any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 )
{	// Get the objects to wait on
	HANDLE objs[] = { get_event_handle( t0 ), get_event_handle( t1 ), get_event_handle( t2 ), get_event_handle( t3 ) };
	switch( ::WaitForMultipleObjects( sizeof(objs)/sizeof(objs[0]), objs, FALSE, time_out ) ) 
	{	case WAIT_OBJECT_0 + 0:		return  0;
		case WAIT_OBJECT_0 + 1:		return  1;
		case WAIT_OBJECT_0 + 2:		return  2;
		case WAIT_OBJECT_0 + 3:		return  3;
		default:					return -1;
	}
}


// This is a utility that will only send messaegs to a destination if it is currently running.
// This helps avoid queue depth problems in most cases. 
bool FC3::utilities::safe_send_message( const FC3::implementation::message &msg, const wchar_t server_name[] )
{	// We first ping the destination
	const __int64 dst_heart_beat = FC3::utilities::heart_beat( server_name );

	// This server is not yet running
	if ( !dst_heart_beat ) return false;

	// Send the message
	return msg.send( server_name );
}