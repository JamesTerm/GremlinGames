#include "stdafx.h"
#include "exports.h"
#include "NetworkThread.h"

NetworkThread *p_NetworkThread = NULL;

HANDLE live_updates_start(void)
{
	// Create network thread
	if (!p_NetworkThread) p_NetworkThread = new NetworkThread;
	return p_NetworkThread->get_handle();
}

const wchar_t* live_updates_get_changed_key(void)
{
	// Return the changed key
	assert(p_NetworkThread);
	return p_NetworkThread->p_get_changed();
}

void live_updates_release_changed_key(const wchar_t *key)
{
	// Release the changed key
	p_NetworkThread->release_changed_key();
}

const wchar_t* live_updates_get_value(const wchar_t *key)
{
	// Return the value
	assert(p_NetworkThread);
	return p_NetworkThread->p_get_value(key);
}

void live_updates_release_value(const wchar_t *value)
{
	// Release the value
	assert(p_NetworkThread);
	p_NetworkThread->release_value();
}

void live_updates_end(HANDLE handle)
{
	// Stop the thread
	assert(p_NetworkThread);
	delete p_NetworkThread;
	p_NetworkThread = NULL;
}