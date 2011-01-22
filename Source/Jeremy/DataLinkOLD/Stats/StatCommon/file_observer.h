#pragma once

#include "main.h"

struct STATCOMMON_API file_observer
{			// Constructor
			file_observer( const wchar_t filename[] );

			// Destructor
			~file_observer( void );

			// Overload this if you want to know when the file is changed
			virtual void file_changed( const __int64 time_changed );

			// Error
			bool error( void ) const;

			// Get the filename
			const wchar_t* filename( void ) const;

private:	// The observer
			void operator() ( const void* );

			// The handle to observe
			std::wstring	m_filename;

			// The last change time
			__int64	m_last_change;
			
			// The handle to directory observer
			HANDLE	m_observer_handle;			
			HANDLE	m_close;

			// Get last changed time
			__int64	get_file_time( void );	// -1 == failure

			// We need a thread to observe folder changes
			Thread<file_observer>	*m_p_thread;

			// This can access my inner workings
			friend Thread<file_observer>;
};