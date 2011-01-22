#include "stdafx.h"
#include "utilities.h"
#include "aggregator.h"
#include "exports.h"

aggregator *g_pAggregator = NULL;

HANDLE live_updates_start( void )
{	// Create a global clock
	if ( !g_pAggregator ) g_pAggregator = new aggregator;
	return g_pAggregator->live_updates_start();
}

const wchar_t *live_updates_get_changed_key( void )
{	// Return the changed key
	assert( g_pAggregator );
	
	// Get the value
	return g_pAggregator->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{	
	assert( g_pAggregator );
	return g_pAggregator->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	// Return the value
	assert( g_pAggregator );

	// Get the value
	return g_pAggregator->live_updates_get_value( key );
}

void live_updates_release_value( const wchar_t value[] )
{	// Return the value
	assert( g_pAggregator );

	// Release the value	
	g_pAggregator->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	// Stop the clock
	assert( g_pAggregator );
	delete g_pAggregator;
	g_pAggregator = NULL;
}
