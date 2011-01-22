#include "stdafx.h"
#include "Main.h"

// Constructor
stat_container::stat_container( const wchar_t *library_file_name, short additional_locks, short additional_threads ):
	stat_base(library_file_name, additional_locks, additional_threads)
{
}

// Destructor
stat_container::~stat_container()
{
}

// clear_containers
// Empties the queue and dictionary. Should be called at the beginning of stop_updates, immediately after
// any initial locking / unlocking of critical sections, by all classes that inherit from stat_container.
void stat_container::clear_containers()
{
	while (!m_changed_queue.empty())
		m_changed_queue.pop();
	if (!m_dictionary.empty())
		m_dictionary.clear();
}

// set_pair
// Adds a pair to the dictionary if it doesn't exist. Adds it to the changed queue if it's new or if its
// value has changed.
bool stat_container::set_pair( const wchar_t *key, const wchar_t *value )
{
	// The new pair
    key_value_pair new_item( key, value );

	{
		STAT_AUTO_SYNC_ALL_VALUES(return false);

		// Locate this item on the queue
		dictionary_type::iterator on_list = m_dictionary.find( new_item );

	    if ( on_list == m_dictionary.end() )
			// This item is not on the list
			m_dictionary.insert( new_item );
	    else
	    {
			// If this item is unchanged, just return immediately
			if ( !auto_sync_all_values.additional_test_succeeded(on_list->second != value) )
				return false;

	        // The item is on the list, so only store the value
		    // No need to mess with the key.
			on_list->second = value;
		}
    }

	{
		STAT_AUTO_SYNC_ALL_KEYS(return false);

		// Add it to the changed queue
		m_changed_queue.push( new_item );
		m_num_changed_pairs++;
	}

	return true;
}

// get_changed_key
// Gets the first key from the changed queue.
const wchar_t *stat_container::get_changed_key( void )
{
	STAT_AUTO_SYNC_GET_KEY();

    // Nothing on the changed list
    if ( !auto_sync_get_key.additional_test_succeeded(!m_changed_queue.empty()) )
        return NULL;

	// Return the value
	return m_changed_queue.front().first.c_str();
}

// release_changed_key
// Releases the key returned from get_changed_key.
void stat_container::release_changed_key( const wchar_t *key )
{
	STAT_AUTO_SYNC_RELEASE_KEY();

	if (!m_changed_queue.empty())
		m_changed_queue.pop();
}

// get_value
// Gets a key's value from the dictionary.
const wchar_t *stat_container::get_value( const wchar_t *key )
{
	STAT_AUTO_SYNC_GET_VALUE();

	// The new pair
    key_value_pair new_item( key, L"" ); // No need to set the second one since it is not searched on.
    // Locate this item on the queue
	dictionary_type::const_iterator on_list = m_dictionary.find( new_item );

    // Not on the list
    if ( !auto_sync_get_value.additional_test_succeeded(on_list != m_dictionary.end()) )
        return NULL;

	// Return the value
	return on_list->second.c_str();
}

// release_value
// Releases the value returned from get_value.
void stat_container::release_value( const wchar_t *value )
{
	STAT_AUTO_SYNC_RELEASE_VALUE();
}
