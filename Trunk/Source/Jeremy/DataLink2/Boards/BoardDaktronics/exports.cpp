#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::Daktronics;
ParseReg_Daktronics *g_DaktronicsPlugInContainer=NULL;

HANDLE live_updates_start( void )
{	
	assert(!g_DaktronicsPlugInContainer);
	g_DaktronicsPlugInContainer=new ParseReg_Daktronics(cwsz_PlugInName);
	(*g_DaktronicsPlugInContainer)();
	HANDLE ret= g_DaktronicsPlugInContainer->live_updates_start();
	return ret;
}

const wchar_t *live_updates_get_changed_key( void )
{	
	assert(g_DaktronicsPlugInContainer);
	return g_DaktronicsPlugInContainer->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{
	assert(g_DaktronicsPlugInContainer);
	return g_DaktronicsPlugInContainer->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	
	assert(g_DaktronicsPlugInContainer);
	return g_DaktronicsPlugInContainer->live_updates_get_value(key);
}

void live_updates_release_value( const wchar_t value[] )
{	
	assert(g_DaktronicsPlugInContainer);
	return g_DaktronicsPlugInContainer->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	
	CloseHandle(hHandle);
	delete g_DaktronicsPlugInContainer;
	g_DaktronicsPlugInContainer=NULL;
}
