// SerialTest0.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


// The entry point for different DLL values
typedef HANDLE				(*PS_START)( void );
typedef const wchar_t *		(*PS_GETVALUE)( const wchar_t [] );
typedef void				(*PS_RELEASEVALUE)( const wchar_t [] );
typedef const wchar_t *		(*PS_GETKEY)( void );
typedef void				(*PS_RELEASEKEY)( const wchar_t [] );
typedef void				(*PS_END)( HANDLE );


int _tmain(int argc, _TCHAR* argv[])
{	// Load the stats library
	HMODULE hStats = ::LoadLibraryW( L"Aggregator.stat" );

	// Check fo failures
	if ( !hStats ) 
	{	::fwprintf( stderr, L"Failed to load the stats dll's." );
		return 1;
	}

	// Get the entry points
	PS_START		live_updates_start					= (PS_START)GetProcAddress( hStats, "live_updates_start");
	PS_GETVALUE		live_updates_get_value				= (PS_GETVALUE)GetProcAddress( hStats, "live_updates_get_value");
	PS_RELEASEVALUE	live_updates_release_value			= (PS_RELEASEVALUE)GetProcAddress( hStats, "live_updates_release_value");
	PS_GETKEY		live_updates_get_changed_key		= (PS_GETKEY)GetProcAddress( hStats, "live_updates_get_changed_key");
	PS_RELEASEKEY	live_updates_release_changed_key	= (PS_RELEASEKEY)GetProcAddress( hStats, "live_updates_release_changed_key");
	PS_END			live_updates_end					= (PS_END)GetProcAddress( hStats, "live_updates_end");
	
	// Check for errors
	if ( ! ( live_updates_start || live_updates_get_value || live_updates_release_value ||
		     live_updates_get_changed_key || live_updates_release_changed_key || live_updates_end ) )
	{	::fwprintf( stderr, L"Could not find entry point in stats dll's." );
		return 1;
	}

	// Get the changed handle
	HANDLE	stats_changed = live_updates_start();

	// Now display stats forever
	while( true )
	{	// Wait for a stats update
		::WaitForSingleObject( stats_changed, 10000 );

		while( true )
		{	// Get the next changed key
			const wchar_t *p_next_key = live_updates_get_changed_key();

			// If there is no new key, just wait again
			if ( !p_next_key )  break;

			// Get the value
			const wchar_t *p_next_val = live_updates_get_value( p_next_key );
			if ( p_next_val )
			{	// Display it on output
				::fwprintf( stdout, L"[%s] = %s\n", p_next_key, p_next_val );

				// Release the value
				live_updates_release_value( p_next_val );	
			}

			// Release the key
			live_updates_release_changed_key( p_next_key );
		}
	}

	return 0;
}

