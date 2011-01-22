#include "stdafx.h"
#include "Main.h"

// Error
bool folder_observer::error( void ) const
{
	return ( m_observer_handle == INVALID_HANDLE_VALUE );
}

// Get the folder path
const wchar_t* folder_observer::folder_path( void ) const
{
	return m_folder_path.c_str();
}

// Constructor
folder_observer::folder_observer( bool check_all_files ):
	folder_traverser(check_all_files),
	m_folder_observer_sync(0, 1),
	m_observer_handle(INVALID_HANDLE_VALUE),
	m_thread_handle(NULL)
{
}

// Destructor
folder_observer::~folder_observer( void )
{
	stop_observing();
}

// start_observing
// Starts observing a folder.
bool folder_observer::start_observing( const wchar_t *folder_path, bool observe_subtree, DWORD notify_filter, bool call_folder_changed_first )
{
	wchar_t path_buffer[_MAX_PATH];
	wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	if ( m_observer_handle != INVALID_HANDLE_VALUE )
		return true;

	::_wsplitpath( folder_path, drive, dir, fname, ext );
	::_wmakepath( path_buffer, drive, dir, L"", L"" );

	m_folder_path = path_buffer;
	m_observe_subtree = observe_subtree;

	m_observer_handle = ::FindFirstChangeNotification( m_folder_path.c_str(), m_observe_subtree? TRUE: FALSE, notify_filter );
	if ( m_observer_handle == INVALID_HANDLE_VALUE )
		return false;

	m_call_folder_changed_first = call_folder_changed_first;
	m_folder_observer_sync.THREAD_START(0, &m_thread_handle, this);

	return true;
}

// Stop observing
// Stops observing a folder.
void folder_observer::stop_observing()
{
	if ( m_observer_handle != INVALID_HANDLE_VALUE )
	{
		m_folder_observer_sync.THREAD_STOP(0);

		FindCloseChangeNotification( m_observer_handle );
		m_observer_handle = INVALID_HANDLE_VALUE;

		m_file_time_list.clear();
	}
}

// operator()
// Overrides operator() in FrameWork::Threads::Thread. Called repeatedly between calls to THREAD_START
// and THREAD_STOP.
void folder_observer::operator()( const void* )
{
	if (!m_folder_observer_sync.THREAD_CHECK(0, 5))
		return;

	HANDLE wait_handles[2] = { m_observer_handle, m_thread_handle };

	if ( m_call_folder_changed_first || ::WaitForMultipleObjects( 2, wait_handles, FALSE, INFINITE ) == WAIT_OBJECT_0 )
	{
		m_call_folder_changed_first = false;
		traverse( m_folder_path, m_observe_subtree );
		::FindNextChangeNotification( m_observer_handle );
	}
}

// is_latest_file_time
// Checks to see whether the file time for the file in question is later than the stored
// file time of that file.
bool folder_observer::is_latest_file_time( const wchar_t *file_path, const ULONGLONG file_time )
{
	file_path_time_pair new_item(file_path, file_time);
	file_time_list_type::iterator on_list = m_file_time_list.find(new_item);

	return (on_list == m_file_time_list.end() || file_time > on_list->second);
}

// set_file_time_if_latest
// Sets the file time if the file time of the file in question is later than the stored
// file time of that file.
bool folder_observer::set_file_time_if_latest( const wchar_t *file_path, const ULONGLONG file_time )
{
	file_path_time_pair new_item(file_path, file_time);
	file_time_list_type::iterator on_list = m_file_time_list.find(new_item);

	if (on_list == m_file_time_list.end())
		m_file_time_list.insert(new_item);
	else
	{
		if (file_time <= on_list->second)
			return false;
		on_list->second = file_time;
	}

	return true;
}
