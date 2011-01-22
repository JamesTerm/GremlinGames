#pragma once

#include "folder_traverser.h"
#include "synchronization.h"
#include "FrameWork_Threads.h"

// class folder_observer
// Observes a folder for changes.
class STATCOMMON_API folder_observer : public folder_traverser
{
public:
	// Constructor
	folder_observer( bool check_all_files = false );

	// Destructor
	virtual ~folder_observer( void );

	bool start_observing( const wchar_t *folder_path, bool observe_subtree, DWORD notify_filter, bool call_folder_changed_first );
	void stop_observing();

	// Error
	bool error( void ) const;

	// Get the folder path
	const wchar_t* folder_path( void ) const;

protected:
	// Typedef for a file path string-time pair
	typedef std::pair<std::wstring, ULONGLONG> file_path_time_pair;

	// Typedef of a functor to compare two file_path_time_pair objects
	typedef struct file_path_time_pair_cmp
	{
		bool operator() ( const file_path_time_pair& pair1, const file_path_time_pair& pair2 ) const
		{
			return pair1.first < pair2.first;
		}
	};

	// Typedef of a set of file_path_time_pair objects
	typedef std::set<file_path_time_pair, file_path_time_pair_cmp> file_time_list_type;

	// Synchronization objects
	synchronization<folder_observer> m_folder_observer_sync;
	HANDLE m_thread_handle;

	file_time_list_type m_file_time_list;
	std::wstring m_folder_path;
	bool m_observe_subtree;
	bool m_call_folder_changed_first;

	bool is_latest_file_time( const wchar_t *file_path, const ULONGLONG file_time );
	bool set_file_time_if_latest( const wchar_t *file_path, const ULONGLONG file_time );

private:
	// The observer
	void operator()( const void* );

	// The handle to directory observer
	HANDLE m_observer_handle;

	// This can access my inner workings
	friend Thread<folder_observer>;
};
