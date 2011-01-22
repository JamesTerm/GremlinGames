#pragma once

#include "stat_base.h"

// class stat_container
// Inherits from stat_base. Base class for any stat dlls that want to use its functionality.
class STATCOMMON_API stat_container : public stat_base
{
public:
	// Virtual functions called by the API
	virtual const wchar_t *get_changed_key( void );
	virtual void release_changed_key( const wchar_t *key );
	virtual const wchar_t *get_value( const wchar_t *key );
	virtual void release_value( const wchar_t *value );

	// Constructor / destructor
	stat_container( const wchar_t *library_file_name = NULL, short additional_locks = 0, short additional_threads = 0 );
	virtual ~stat_container();

	bool set_pair( const wchar_t *key, const wchar_t *value );

protected:
	// A key-value pair
	typedef std::pair<std::wstring, std::wstring> key_value_pair;

	// A functor to compare two key value items based on the key
	typedef struct key_value_pair_cmp
	{
		bool operator() ( const key_value_pair& pair1, const key_value_pair& pair2 ) const
		{
			return pair1.first < pair2.first;
		}
	};

	// Typedef for a set of key_value_pair objects
	typedef std::set<key_value_pair, key_value_pair_cmp> dictionary_type;

	dictionary_type m_dictionary;
	std::queue<key_value_pair> m_changed_queue;

	void clear_containers();
};
