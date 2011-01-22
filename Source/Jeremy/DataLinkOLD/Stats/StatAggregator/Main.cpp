// StatAggregator.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Main.h"
#include "aggregator.h"

// These are the API functions for the aggregator dll.

HANDLE live_updates_start( void )
{
	return aggregator::get_singleton().start_updates();
}

void live_updates_end( HANDLE handle )
{
	aggregator::get_singleton().stop_updates( handle );
}

const wchar_t* live_updates_get_changed_key( void )
{
	return aggregator::get_singleton().get_changed_key();
}

void live_updates_release_changed_key( const wchar_t *key )
{
	aggregator::get_singleton().release_changed_key( key );
}

const wchar_t* live_updates_get_value( const wchar_t *key )
{
	return aggregator::get_singleton().get_value( key );
}

void live_updates_release_value( const wchar_t *value )
{
	aggregator::get_singleton().release_value( value );
}
