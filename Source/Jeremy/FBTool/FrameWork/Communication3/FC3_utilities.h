#pragma once

// Get the current instantenous queue depth on a remote server
FRAMEWORKCOMMUNICATION3_API const DWORD queue_depth( const wchar_t server_name[] );

// This will recover the heart-beat time for a particular server.
// This represents the last ::QueryPeformanceCounter time that the server in
// question was considered alive.
FRAMEWORKCOMMUNICATION3_API const __int64 heart_beat( const wchar_t server_name[] );

// This is a utility that will only send messaegs to a destination if it is currently running.
// This helps avoid queue depth problems in most cases. 
FRAMEWORKCOMMUNICATION3_API bool safe_send_message( const FrameWork::Communication3::implementation::message &msg, const wchar_t server_name[] );

// This is a function that allows you to wait on increasing number of triggers. 
namespace multiwait
{
	typedef FrameWork::Communication3::implementation::trigger	trigger;

	// Returns whether it was triggered or not
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 );
	FRAMEWORKCOMMUNICATION3_API bool all( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 );

	// The return value is the index of the trigger that got hit. -1 means that time_out expired.
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2 );
	FRAMEWORKCOMMUNICATION3_API int any( const DWORD time_out, const trigger& t0, const trigger& t1, const trigger& t2, const trigger& t3 );
};