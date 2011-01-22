#include "stdafx.h"
#include "utilities.h"
#include "aggregator.h"
#include "exports.h"

// Get the directory
extern HMODULE g_hModule;

// Constructor
aggregator::aggregator( void )
	:	// Create the event handle
		m_hUpdate( ::CreateEvent( NULL, FALSE, TRUE, NULL ) ),
		m_changed_key_idx( -1 ), m_get_key_idx( -1 ),
		m_thread_must_exit( false )
{	// Get the current path
	char	path_name[ MAX_PATH*2 ];
	::GetModuleFileNameA( g_hModule, path_name, sizeof( path_name )-1 );
	::PathRemoveFileSpecA( path_name );
	::strcat( path_name, "\\" );
	char*	p_path_name_end = path_name + ::strlen( path_name );

	// Our seatch name
	char	search_name[ MAX_PATH*2 ];
	::strcpy( search_name, path_name );
	::strcat( search_name, "*.stat" );

	// Now file the files
	WIN32_FIND_DATAA fSearch;
	HANDLE hSearch = ::FindFirstFileA( search_name, &fSearch );
	if ( hSearch != INVALID_HANDLE_VALUE )
	{	// 
		while( 1 )
		{	// Skip myself 
			if ( ::stricmp( fSearch.cFileName, "Aggregator.stat" ) )
			{	// Build the name
				::strcpy( p_path_name_end, fSearch.cFileName );
				
				// Handle the file
				dll_item *p_new_dll_item = new dll_item( path_name );
				if ( p_new_dll_item->m_hModule )
				{	// Keep this item
					m_stats_dlls.push_back( p_new_dll_item );
				}
				else
				{	// Discard this item
					delete p_new_dll_item;
				}
			}

			// Get the next file
			if ( !::FindNextFileA( hSearch, &fSearch ) ) break;
		}

		// CLose the file handle
		::FindClose( hSearch );
	}

	// Start the thread
	m_hThread = ::CreateThread( NULL, 64*1024, (LPTHREAD_START_ROUTINE)s_thread_proc, (void*)this, 0, NULL);
}

// Destructor
aggregator::~aggregator( void )
{	// The thread must exit
	m_thread_must_exit = true;
	if ( m_hThread )
	{	// Wait for the thread to exit
		if ( ::WaitForSingleObject( m_hThread, 10000 ) == WAIT_TIMEOUT ) 
			// This is bad
			assert( false );	

		// Close the thread 
		::CloseHandle( m_hThread );
	}

	// Cycle over the items
	for( int i=0; i<(int)m_stats_dlls.size(); i++ )
		delete m_stats_dlls[ i ];
	
	// Close the handle
	::CloseHandle( m_hUpdate );
}


void aggregator::s_thread_proc( void *p_this )
{	// 
	( (aggregator*)p_this )->thread_proc();
}

void aggregator::thread_proc( void )
{	// Allocate space for all of the handles
	HANDLE*	p_obj = new HANDLE [ m_stats_dlls.size() ];
	for( int i=0; i<(int)m_stats_dlls.size(); i++ )
		p_obj[ i ] = m_stats_dlls[ i ]->m_hUpdate;
	
	while( !m_thread_must_exit )
	{	// Trigger my event as needed
		if ( ::WaitForMultipleObjects( (DWORD)m_stats_dlls.size(), p_obj, FALSE, 100 ) != WAIT_TIMEOUT )
			::SetEvent( m_hUpdate );
	}

	// Finished
	delete [] p_obj;
}

aggregator::dll_item::dll_item( const char* p_fn )
	: m_hModule( NULL ), m_hUpdate( NULL )
{	// Try loading the library
	m_hModule = ::LoadLibraryA( p_fn );
	if ( m_hModule )
	{	m_live_updates_start = (PS_START)GetProcAddress( m_hModule, "live_updates_start");
		m_live_updates_get_value = (PS_GETVALUE)GetProcAddress( m_hModule, "live_updates_get_value");
		m_live_updates_release_value = (PS_RELEASEVALUE)GetProcAddress( m_hModule, "live_updates_release_value");
		m_live_updates_get_changed_key = (PS_GETKEY)GetProcAddress( m_hModule, "live_updates_get_changed_key");
		m_live_updates_release_changed_key	= (PS_RELEASEKEY)GetProcAddress( m_hModule, "live_updates_release_changed_key");
		m_live_updates_end = (PS_END)GetProcAddress( m_hModule, "live_updates_end");

		// Get the event
		if ( m_live_updates_start )
		{	m_hUpdate = m_live_updates_start();
			//assert( m_hUpdate );
		}
	}
}

aggregator::dll_item::~dll_item( void )
{	// Unload the module
	if ( m_hModule )
	{	// Release the event
		if ( m_live_updates_end )
			m_live_updates_end( m_hUpdate );

		// Free the library
		::FreeLibrary( m_hModule );
	}
}

// Start by calling this. It will return a handle that is "triggered"
// when a change has occured to the stats. For instance, you could do a
// WaitForSingleEvent( handle, 100 ); on this event to see when stats have
// changed.
HANDLE aggregator::live_updates_start( void )
{	return m_hUpdate;
}

// All done with stats.
void aggregator::live_updates_end( HANDLE handle )
{	assert( m_hUpdate == handle );
}

// Get the next key that has changed.
const wchar_t* aggregator::live_updates_get_changed_key( void )
{	// Cycle through all DLLs to get the changed key
	m_changed_key_idx_lock.lock();

	// Debugging
	assert( m_changed_key_idx == -1 );

	// Look for changed keys
	for( int i=0; i<(int)m_stats_dlls.size(); i++ )
	{	const wchar_t* p_ret = m_stats_dlls[ i ]->m_live_updates_get_changed_key();
		if ( p_ret )
		{	m_changed_key_idx = i;
			return p_ret;
		}
	}

	// Unlock and return
	m_changed_key_idx_lock.unlock();
	return NULL;
}

// Because you are returned a pointer that you do not own from live_updates_get_key, you
// will need to release it with this function. Do not call another live_updates_get_key
// until the key returned by the previous call has been released.
void aggregator::live_updates_release_changed_key( const wchar_t *key )
{	// If there was no key
	if ( !key ) return;

	// Debugging
	assert( m_changed_key_idx != -1 );
	assert( m_changed_key_idx < (int)m_stats_dlls.size() );

	// Release this item
	m_stats_dlls[ m_changed_key_idx ]->m_live_updates_release_changed_key( key );
	m_changed_key_idx = -1;

	// Unlock
	m_changed_key_idx_lock.unlock();
}

// Get a value from a key.
const wchar_t* aggregator::live_updates_get_value( const wchar_t *key )
{	// If there is no key
	if ( !key ) return NULL;

	// Lock the key
	m_get_key_lock.lock();

	// Debugging
	assert( m_get_key_idx == -1 );

	// Look for changed keys
	for( int i=0; i<(int)m_stats_dlls.size(); i++ )
	{	const wchar_t* p_ret = m_stats_dlls[ i ]->m_live_updates_get_value( key );
		if ( p_ret )
		{	m_get_key_idx = i;
			return p_ret;
		}
	}

	// Unlock and return
	m_get_key_lock.unlock();
	return NULL;
}

// Same as above, you need to release the pointer before you can read the next changed value.
void aggregator::live_updates_release_value( const wchar_t *value )
{	// Debugging
	if ( !value ) return;

	// Checking
	assert( m_get_key_idx != -1 );
	assert( m_get_key_idx < (int)m_stats_dlls.size() );

	// Release this item
	m_stats_dlls[ m_get_key_idx ]->m_live_updates_release_value( value );
	m_get_key_idx = -1;

	// Unlock
	m_get_key_lock.unlock();
}

aggregator *g_p_agregator=NULL;

// Get the aggregator
HANDLE live_updates_start( void )
{	// Create an aggregator if needed
	if ( !g_p_agregator ) g_p_agregator = new aggregator;
	return g_p_agregator->live_updates_start();
}

void live_updates_end( HANDLE handle )
{	assert( g_p_agregator );
	g_p_agregator->live_updates_end( handle );

	// We can safely close the aggregator now
	if ( g_p_agregator ) 
	{	delete g_p_agregator;
		g_p_agregator = NULL;
	}
}

const wchar_t* live_updates_get_changed_key( void )
{	assert( g_p_agregator );
	return g_p_agregator->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t *key )
{	assert( g_p_agregator );
	g_p_agregator->live_updates_release_changed_key( key );
}

const wchar_t* live_updates_get_value( const wchar_t *key )
{	assert( g_p_agregator );
	return g_p_agregator->live_updates_get_value( key );
}

void live_updates_release_value( const wchar_t *value )
{	assert( g_p_agregator );
	g_p_agregator->live_updates_release_value( value );
}

