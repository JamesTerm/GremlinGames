#include "stdafx.h"
#include "Main.h"

// static constants
const wchar_t *stat_base::STAT_LIBRARY_EXTENSION = L".stat";

// sync_op array constants
const sync_op stat_base::SYNC_KEY_VALUE_BEGIN[] =
{
	{ SYNC_LOCK, STAT_KEY, 0 },
	{ SYNC_LOCK, STAT_VALUE, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_KEY_VALUE_END[] =
{
	{ SYNC_UNLOCK, STAT_KEY, 0 },
	{ SYNC_UNLOCK, STAT_VALUE, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_ALL_KEYS_BEGIN[] =
{
	{ SYNC_LOCK, STAT_KEY_MULTIFUNC, 0 },
	{ SYNC_LOCK, STAT_KEY, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_ALL_KEYS_END[] =
{
	{ SYNC_UNLOCK, STAT_KEY, 0 },
	{ SYNC_UNLOCK, STAT_KEY_MULTIFUNC, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_ALL_VALUES_BEGIN[] =
{
	{ SYNC_LOCK, STAT_VALUE_MULTIFUNC, 0 },
	{ SYNC_LOCK, STAT_VALUE, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_ALL_VALUES_END[] =
{
	{ SYNC_UNLOCK, STAT_VALUE, 0 },
	{ SYNC_UNLOCK, STAT_VALUE_MULTIFUNC, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_KEY_BEGIN[] =
{
	{ SYNC_LOCK, STAT_KEY, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_KEY_END[] =
{
	{ SYNC_UNLOCK, STAT_KEY, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_VALUE_BEGIN[] =
{
	{ SYNC_LOCK, STAT_VALUE, 0 },
	{ SYNC_RETURN, 1, 0 }
};
const sync_op stat_base::SYNC_VALUE_END[] =
{
	{ SYNC_UNLOCK, STAT_VALUE, 0 },
	{ SYNC_RETURN, 1, 0 }
};

// static member variables
HMODULE stat_base::m_hmodule = NULL;
wchar_t stat_base::m_module_folder_path[_MAX_PATH] = L"";
wchar_t stat_base::m_module_file_name[_MAX_PATH] = L"";

// Constructor
stat_base::stat_base( const wchar_t *library_file_name, short additional_locks, short additional_threads ):
	m_stat_base_sync(DEFAULT_NUM_STAT_LOCKS + additional_locks, DEFAULT_NUM_STAT_THREADS + additional_threads),
	m_event_handle(NULL),
	m_is_key_locked(false),
	m_is_value_locked(false)
{
	set_module_info(library_file_name);
}

// Destructor
stat_base::~stat_base()
{
}

// set_module_info
// Called from the constructor to set the module handle, folder path, and file name.
void stat_base::set_module_info( const wchar_t *library_file_name )
{
	wchar_t drive[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];

	if ((m_hmodule = ::GetModuleHandle(library_file_name)) != NULL)
		::GetModuleFileName(m_hmodule, m_module_folder_path, _MAX_PATH);

	::_wsplitpath( m_module_folder_path, drive, dir, fname, ext );
	::_wmakepath( m_module_folder_path, drive, dir, L"", L"" );
	::_wmakepath( m_module_file_name, L"", L"", fname, ext );
}

// get_event_handle
// Returns the main event handle.
HANDLE stat_base::get_event_handle( void ) const
{
	return m_event_handle;
}

// create_event_handle
// If the main event handle is not NULL, create it. Return whether the event handle was created
// successfully during this call to create_event_handle.
bool stat_base::create_event_handle( void )
{
	if (m_event_handle != NULL)
		return false;

	return ((m_event_handle = ::CreateEvent(NULL, FALSE, FALSE, NULL)) != NULL);
}

// close_event_handle
// If the main event handle is not NULL and is equal to the handle passed to the function,
// close the event handle.
bool stat_base::close_event_handle( HANDLE handle )
{
	if (m_event_handle == NULL || handle != m_event_handle)
		return false;

	::CloseHandle(m_event_handle);
	m_event_handle = NULL;

	return true;
}

// reset_num_changed_pairs
// Sets the number of changed pairs to 0.
bool stat_base::reset_num_changed_pairs()
{
	STAT_AUTO_SYNC_KEY();

	m_num_changed_pairs = 0;

	return true;
}

// changes_complete
// If the number of changed pairs is greater than 0, set the event handle.
bool stat_base::changes_complete(long num_changed_pairs)
{
	if (num_changed_pairs <= 0)
		return false;

	{
		STAT_AUTO_SYNC_KEY();

		::SetEvent(m_event_handle);
	}

	return true;
}

// operator()
// Overrides operator() in FrameWork::Threads::Thread. Called repeatedly between calls to THREAD_START
// and THREAD_STOP.
void stat_base::operator()( const void* )
{
	Sleep( 1 );
}
