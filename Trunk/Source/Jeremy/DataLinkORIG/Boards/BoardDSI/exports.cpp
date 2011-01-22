#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::DSI;
ParseReg_DSI *g_DSIPlugInContainer=NULL;

HANDLE live_updates_start( void )
{	
	assert(!g_DSIPlugInContainer);
	g_DSIPlugInContainer=new ParseReg_DSI(cwsz_PlugInName);
	(*g_DSIPlugInContainer)();
	HANDLE ret= g_DSIPlugInContainer->live_updates_start();
	return ret;
}

const wchar_t *live_updates_get_changed_key( void )
{	
	assert(g_DSIPlugInContainer);
	return g_DSIPlugInContainer->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{
	assert(g_DSIPlugInContainer);
	return g_DSIPlugInContainer->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	
	assert(g_DSIPlugInContainer);
	return g_DSIPlugInContainer->live_updates_get_value(key);
}

void live_updates_release_value( const wchar_t value[] )
{	
	assert(g_DSIPlugInContainer);
	return g_DSIPlugInContainer->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	
	CloseHandle(hHandle);
	delete g_DSIPlugInContainer;
	g_DSIPlugInContainer=NULL;
}
