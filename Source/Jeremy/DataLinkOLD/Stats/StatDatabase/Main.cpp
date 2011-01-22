#include "stdafx.h"
#include "Main.h"
#include "database.h"

HANDLE live_updates_start( void )
{
	return database::get_singleton().start_updates();
}

void live_updates_end( HANDLE handle )
{
	database::get_singleton().stop_updates( handle );
}

const wchar_t* live_updates_get_changed_key( void )
{
	return database::get_singleton().get_changed_key();
}

void live_updates_release_changed_key( const wchar_t *key )
{
	database::get_singleton().release_changed_key( key );
}

const wchar_t* live_updates_get_value( const wchar_t *key )
{
	return database::get_singleton().get_value( key );
}

void live_updates_release_value( const wchar_t *value )
{
	database::get_singleton().release_value( value );
}
