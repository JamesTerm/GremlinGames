#include "stdafx.h"
#include "ClockThread.h"
#include "exports.h"

ClockThread *p_ClockThread = NULL;

HANDLE live_updates_start( void )
{	// Create a global clock
	if ( !p_ClockThread ) p_ClockThread = new ClockThread;
	return p_ClockThread->get_handle();
}

const wchar_t *live_updates_get_changed_key( void )
{	// Return the changed key
	assert( p_ClockThread );
	
	// Get the value
	return p_ClockThread->p_get_changed();
}

void live_updates_release_changed_key( const wchar_t key[] )
{	
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	// Return the value
	assert( p_ClockThread );

	// Get the value
	return p_ClockThread->p_get_value( key );
}

void live_updates_release_value( const wchar_t value[] )
{	// Return the value
	assert( p_ClockThread );

	// Release the value
	p_ClockThread->release_value();
}

void live_updates_end( HANDLE hHandle )
{	// Stop the clock
	assert( p_ClockThread );
	delete p_ClockThread;
	p_ClockThread = NULL;
}