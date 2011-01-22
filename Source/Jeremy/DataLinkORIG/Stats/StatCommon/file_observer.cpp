#include "StdAfx.h"
#include "Main.h"

using namespace Frameworks;

// Error
bool file_observer::error( void ) const
{
	return ( m_observer_handle == INVALID_HANDLE_VALUE );
}

// Get the filename
const wchar_t* file_observer::filename( void ) const
{
	return m_filename.c_str();
}

void file_observer::file_changed( const __int64 time_changed )
{
	std::cout << "File changed !" << std::endl;
}

// Constructor
file_observer::file_observer( const wchar_t filename[] )
	:	m_observer_handle( INVALID_HANDLE_VALUE ),
		m_close( ::CreateEvent( NULL, FALSE, TRUE, NULL ) ),
		m_filename( filename ), m_last_change( -1 )
{	// We split the filename
	wchar_t path_buffer[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	// Split the path
	::_wsplitpath( filename, drive, dir, fname, ext );

	// And rebuild the path
	::_wmakepath( path_buffer, drive, dir, L"", L"" );

	// We now try to observe the file
	m_observer_handle = ::FindFirstChangeNotificationW( path_buffer, FALSE, FILE_NOTIFY_CHANGE_LAST_WRITE );

	//
	if ( m_observer_handle != INVALID_HANDLE_VALUE )
	{	// We need to start the observer thread
		m_p_thread = ::new Thread<file_observer>( this );
	}
}

// Destructor
file_observer::~file_observer( void )
{
	// Trigger the handle
	if ( m_observer_handle != INVALID_HANDLE_VALUE )
	{	// Trigger
		::SetEvent( m_close );

		// Stop the thread
		delete m_p_thread;

		// CLose the handle
		FindCloseChangeNotification( m_observer_handle );
	}

	// Close the handle
	::CloseHandle( m_close );
}

void file_observer::operator() ( const void* )
{
	// Wait for two events
	HANDLE	events[] = { m_observer_handle, m_close };
	if ( ::WaitForMultipleObjects( 2, events, FALSE, INFINITE ) == WAIT_OBJECT_0 )
			// Signal the next change
			::FindNextChangeNotification( m_observer_handle );

	// Get the change time
	const __int64 last_change = get_file_time();	

	// Is it more recent than the last change ?
	if ( last_change > m_last_change )
	{	// Call the change
		if ( last_change != -1 ) file_changed( last_change );

		// This is the last change time.
		m_last_change = last_change;
	}
}

// Get last changed time
__int64	file_observer::get_file_time( void )
{
	// Open the file
	HANDLE file_handle = ::CreateFileW( m_filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
	if ( file_handle == INVALID_HANDLE_VALUE ) return -1;

	// Get the file times
	FILETIME last_write_time;
	::GetFileTime( file_handle, NULL, NULL, &last_write_time );

	// Close the file
	::CloseHandle( file_handle );

	// Success
	return *(__int64*)&last_write_time;
}