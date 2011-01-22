// StatFolderObserver.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Main.h"
#include "stat_folder_observer.h"

HANDLE live_updates_start( void )
{
	return stat_folder_observer::get_singleton().start_updates();
}

void live_updates_end( HANDLE handle )
{
	stat_folder_observer::get_singleton().stop_updates( handle );
}

const wchar_t* live_updates_get_changed_key( void )
{
	return stat_folder_observer::get_singleton().get_changed_key();
}

void live_updates_release_changed_key( const wchar_t *key )
{
	stat_folder_observer::get_singleton().release_changed_key( key );
}

const wchar_t* live_updates_get_value( const wchar_t *key )
{
	return stat_folder_observer::get_singleton().get_value( key );
}

void live_updates_release_value( const wchar_t *value )
{
	stat_folder_observer::get_singleton().release_value( value );
}
