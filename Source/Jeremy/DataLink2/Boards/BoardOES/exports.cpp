#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::OES;
ParseReg_OES *g_OESPlugInContainer=NULL;

HANDLE live_updates_start( void )
{	
	assert(!g_OESPlugInContainer);
	g_OESPlugInContainer=new ParseReg_OES(cwsz_PlugInName);
	(*g_OESPlugInContainer)();
	HANDLE ret= g_OESPlugInContainer->live_updates_start();
	return ret;
}

const wchar_t *live_updates_get_changed_key( void )
{	
	assert(g_OESPlugInContainer);
	return g_OESPlugInContainer->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{
	assert(g_OESPlugInContainer);
	return g_OESPlugInContainer->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	
	assert(g_OESPlugInContainer);
	return g_OESPlugInContainer->live_updates_get_value(key);
}

void live_updates_release_value( const wchar_t value[] )
{	
	assert(g_OESPlugInContainer);
	return g_OESPlugInContainer->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	
	CloseHandle(hHandle);
	delete g_OESPlugInContainer;
	g_OESPlugInContainer=NULL;
}
