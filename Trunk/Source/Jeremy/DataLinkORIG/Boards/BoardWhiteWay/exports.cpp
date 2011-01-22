#include "stdafx.h"
#include "Main.h"

using namespace PlugIn::DataLink::WhiteWay;
ParseReg_WhiteWay *g_WhiteWayPlugInContainer=NULL;

HANDLE live_updates_start( void )
{	
	assert(!g_WhiteWayPlugInContainer);
	g_WhiteWayPlugInContainer=new ParseReg_WhiteWay(cwsz_PlugInName);
	(*g_WhiteWayPlugInContainer)();
	HANDLE ret= g_WhiteWayPlugInContainer->live_updates_start();
	return ret;
}

const wchar_t *live_updates_get_changed_key( void )
{	
	assert(g_WhiteWayPlugInContainer);
	return g_WhiteWayPlugInContainer->live_updates_get_changed_key();
}

void live_updates_release_changed_key( const wchar_t key[] )
{
	assert(g_WhiteWayPlugInContainer);
	return g_WhiteWayPlugInContainer->live_updates_release_changed_key(key);
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{	
	assert(g_WhiteWayPlugInContainer);
	return g_WhiteWayPlugInContainer->live_updates_get_value(key);
}

void live_updates_release_value( const wchar_t value[] )
{	
	assert(g_WhiteWayPlugInContainer);
	return g_WhiteWayPlugInContainer->live_updates_release_value(value);
}

void live_updates_end( HANDLE hHandle )
{	
	CloseHandle(hHandle);
	delete g_WhiteWayPlugInContainer;
	g_WhiteWayPlugInContainer=NULL;
}
