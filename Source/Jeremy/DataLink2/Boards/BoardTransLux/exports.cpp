#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::TransLux;
ParseReg_TransLux *g_TransLuxPlugInContainer=NULL;

HANDLE live_updates_start( void )
{	
	assert(!g_TransLuxPlugInContainer);
	g_TransLuxPlugInContainer=new ParseReg_TransLux(cwsz_PlugInName);
	(*g_TransLuxPlugInContainer)();
	HANDLE ret= g_TransLuxPlugInContainer->live_updates_start();
	return ret;
}

const wchar_t *live_updates_get_changed_key( void )
{	
	assert(g_TransLuxPlugInContainer);
	return g_TransLuxPlugInContainer->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{
	assert(g_TransLuxPlugInContainer);
	return g_TransLuxPlugInContainer->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	
	assert(g_TransLuxPlugInContainer);
	return g_TransLuxPlugInContainer->live_updates_get_value(key);
}

void live_updates_release_value( const wchar_t value[] )
{	
	assert(g_TransLuxPlugInContainer);
	return g_TransLuxPlugInContainer->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	
	CloseHandle(hHandle);
	delete g_TransLuxPlugInContainer;
	g_TransLuxPlugInContainer=NULL;
}
