#pragma once

// Macros for synchronization in the stats dlls
#ifndef STAT_AUTO_SYNC_DEFS
#define STAT_AUTO_SYNC_DEFS

#define STAT_AUTO_SYNC_START() AUTO_SYNCHRONIZE(stat_base, key_value, &m_stat_base_sync, SYNC_KEY_VALUE_BEGIN, SYNC_KEY_VALUE_END, SYNC_KEY_VALUE_END, create_event_handle(), return m_event_handle)
#define STAT_AUTO_SYNC_STOP() AUTO_SYNCHRONIZE(stat_base, key_value, &m_stat_base_sync, SYNC_KEY_VALUE_BEGIN, SYNC_KEY_VALUE_END, SYNC_KEY_VALUE_END, close_event_handle(handle), return)
#define STAT_AUTO_SYNC_ALL_KEYS(if_failed) AUTO_SYNCHRONIZE(stat_base, all_keys, &m_stat_base_sync, SYNC_ALL_KEYS_BEGIN, SYNC_ALL_KEYS_END, SYNC_ALL_KEYS_END, (m_event_handle != NULL), if_failed)
#define STAT_AUTO_SYNC_ALL_VALUES(if_failed) AUTO_SYNCHRONIZE(stat_base, all_values, &m_stat_base_sync, SYNC_ALL_VALUES_BEGIN, SYNC_ALL_VALUES_END, SYNC_ALL_VALUES_END, (m_event_handle != NULL), if_failed)
#define STAT_AUTO_SYNC_KEY() AUTO_SYNCHRONIZE(stat_base, key, &m_stat_base_sync, SYNC_KEY_BEGIN, SYNC_KEY_END, SYNC_KEY_END, (m_event_handle != NULL), return false)
#define STAT_AUTO_SYNC_GET_KEY() AUTO_SYNCHRONIZE(stat_base, get_key, &m_stat_base_sync, SYNC_ALL_KEYS_BEGIN, SYNC_ALL_KEYS_END, SYNC_KEY_END, (m_event_handle != NULL), return NULL) \
	m_is_key_locked = true;
#define STAT_AUTO_SYNC_RELEASE_KEY() AUTO_SYNCHRONIZE(stat_base, release_key, &m_stat_base_sync, SYNC_KEY_BEGIN, SYNC_KEY_END, SYNC_ALL_KEYS_END, (m_is_key_locked), return) \
	m_is_key_locked = false;
#define STAT_AUTO_SYNC_GET_VALUE() AUTO_SYNCHRONIZE(stat_base, get_value, &m_stat_base_sync, SYNC_ALL_VALUES_BEGIN, SYNC_ALL_VALUES_END, SYNC_VALUE_END, (m_event_handle != NULL), return NULL) \
	m_is_value_locked = true;
#define STAT_AUTO_SYNC_RELEASE_VALUE() AUTO_SYNCHRONIZE(stat_base, release_value, &m_stat_base_sync, SYNC_VALUE_BEGIN, SYNC_VALUE_END, SYNC_ALL_VALUES_END, (m_is_value_locked), return) \
	m_is_value_locked = false;

#endif // STAT_AUTO_SYNC_DEFS

// class stat_base
// Base class for stats dlls.
class STATCOMMON_API stat_base
{
public:

	// Virtual functions called directly by the API
	virtual HANDLE start_updates( void ) = 0;
	virtual void stop_updates( HANDLE handle ) = 0;
	virtual const wchar_t *get_changed_key( void ) = 0;
	virtual void release_changed_key( const wchar_t *key ) = 0;
	virtual const wchar_t *get_value( const wchar_t *key ) = 0;
	virtual void release_value( const wchar_t *value ) = 0;

	// Constructor / destructor
	stat_base( const wchar_t *library_file_name = NULL, short additional_locks = 0, short additional_threads = 0 );
	virtual ~stat_base();

	HANDLE get_event_handle( void ) const;
	bool reset_num_changed_pairs();
	bool changes_complete(long num_changed_pairs);

protected:

	// Enumeration for the stats critical sections
	enum STAT_LOCK_INDEX
	{
		STAT_KEY = 0,
		STAT_KEY_MULTIFUNC = 1,
		STAT_VALUE = 2,
		STAT_VALUE_MULTIFUNC = 3
	};

	// Constants
	static const wchar_t *STAT_LIBRARY_EXTENSION;

	static const int DEFAULT_NUM_STAT_LOCKS = 4;
	static const int DEFAULT_NUM_STAT_THREADS = 0;

	static const sync_op SYNC_KEY_VALUE_BEGIN[];
	static const sync_op SYNC_KEY_VALUE_END[];
	static const sync_op SYNC_ALL_KEYS_BEGIN[];
	static const sync_op SYNC_ALL_KEYS_END[];
	static const sync_op SYNC_ALL_VALUES_BEGIN[];
	static const sync_op SYNC_ALL_VALUES_END[];
	static const sync_op SYNC_KEY_BEGIN[];
	static const sync_op SYNC_KEY_END[];
	static const sync_op SYNC_VALUE_BEGIN[];
	static const sync_op SYNC_VALUE_END[];

	// Static member variables
	static HMODULE m_hmodule;
	static wchar_t m_module_folder_path[_MAX_PATH];
	static wchar_t m_module_file_name[_MAX_PATH];

	// Synchronization variables
	synchronization<stat_base> m_stat_base_sync;
	HANDLE m_event_handle;
	bool m_is_key_locked;
	bool m_is_value_locked;

	long m_num_changed_pairs;

	// Static member functions
	static void set_module_info( const wchar_t *library_file_name );

	bool create_event_handle( void );
	bool close_event_handle( HANDLE handle );

private:

	// Called by Thread
	virtual void operator() ( const void* );

	// Make Thread a friend class
	friend Thread<stat_base>;
};
