#pragma once

typedef HANDLE				(*PS_START)();
typedef void				(*PS_END)(HANDLE);
typedef const wchar_t *		(*PS_GETKEY)();
typedef void				(*PS_RELEASEKEY)(const wchar_t *);
typedef const wchar_t *		(*PS_GETVALUE)(const wchar_t *);
typedef void				(*PS_RELEASEVALUE)(const wchar_t *);

struct aggregator
{			// Constructor
			aggregator( void );

			// Destructor
			~aggregator( void );

			// Start by calling this. It will return a handle that is "triggered"
			// when a change has occured to the stats. For instance, you could do a
			// WaitForSingleEvent( handle, 100 ); on this event to see when stats have
			// changed.
			HANDLE live_updates_start( void );

			// All done with stats.
			void live_updates_end( HANDLE handle );

			// Get the next key that has changed.
			const wchar_t* live_updates_get_changed_key( void );

			// Because you are returned a pointer that you do not own from live_updates_get_key, you
			// will need to release it with this function. Do not call another live_updates_get_key
			// until the key returned by the previous call has been released.
			void live_updates_release_changed_key( const wchar_t *key );

			// Get a value from a key.
			const wchar_t* live_updates_get_value( const wchar_t *key );

			// Same as above, you need to release the pointer before you can read the next changed value.
			void live_updates_release_value( const wchar_t *value );

private:	// The set of DLLs
			struct	dll_item
			{	// Constructor
				 dll_item( const char* p_fn );
				~dll_item( void );
				
				HMODULE			m_hModule;
				HANDLE			m_hUpdate;
				PS_START		m_live_updates_start;					// = (PS_START)GetProcAddress( hStats, "live_updates_start");
				PS_GETVALUE		m_live_updates_get_value;				// = (PS_GETVALUE)GetProcAddress( hStats, "live_updates_get_value");
				PS_RELEASEVALUE	m_live_updates_release_value;			// = (PS_RELEASEVALUE)GetProcAddress( hStats, "live_updates_release_value");
				PS_GETKEY		m_live_updates_get_changed_key;			// = (PS_GETKEY)GetProcAddress( hStats, "live_updates_get_changed_key");
				PS_RELEASEKEY	m_live_updates_release_changed_key;		// = (PS_RELEASEKEY)GetProcAddress( hStats, "live_updates_release_changed_key");
				PS_END			m_live_updates_end;						// = (PS_END)GetProcAddress( hStats, "live_updates_end");
			};

			// The list of DLLs
			std::vector< dll_item* >	m_stats_dlls;

			// The index of the changed key
			critical_section	m_changed_key_idx_lock;
			int	m_changed_key_idx;

			// Get actual indexes
			critical_section	m_get_key_lock;
			int	m_get_key_idx;

			// The update HANDLE
			HANDLE	m_hUpdate;
			HANDLE	m_hThread;
			
			// Thread Implementation.  SetEvent must be called
			// at some point in the thread for TriCaster™ to ask for Keys and Values.
			bool m_thread_must_exit;
			static void s_thread_proc( void *p_this );
			void thread_proc( void );
};