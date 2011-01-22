#include "stdafx.h"

static unsigned s_SBT = SBNone;

HANDLE live_updates_start( void )
{
	HANDLE h = CreateEvent	(NULL,false,false,NULL);
	for (unsigned i=1;i<SBCount;i++)
	{
		if (!getStatsBoard(i)->GetNumBytesToRead())
			continue;
		getStatsBoard(i)->Initialize();
		getStatsBoard(i)->StartUpdates();
		getStatsBoard(i)->SetHandle(h);
	}
	ResetEvent(h);
	return(h);
}

void live_updates_release_value( const wchar_t value[] )
{
	getStatsBoard(s_SBT)->ReleaseValue(value);
}

void live_updates_release_changed_key( const wchar_t key[] )
{

}

const wchar_t *live_updates_get_changed_key( void )
{
	if ((s_SBT == SBNone) || (s_SBT == SBCount))
		s_SBT = 1; // Start with the first Board
	const wchar_t * k = NULL;
	while (!k && s_SBT < SBCount)
	{
		k = getStatsBoard(s_SBT)->GetNextChangedKey();
		if (!k)
			s_SBT++;
	}
	return k;
}

const wchar_t *live_updates_get_value( const wchar_t key[] )
{
	// Need to change this to check more than 1 board?
	const wchar_t * v = getStatsBoard(s_SBT)->GetValue(key);
	return v;
}

void live_updates_end( HANDLE hHandle )
{
	for (unsigned i=1;i<SBCount;i++)
	{
		if (!getStatsBoard(i)->GetNumBytesToRead())
			break;
		getStatsBoard(i)->StopUpdates();
		getStatsBoard(i)->Destroy();
		HANDLE h = getStatsBoard(i)->GetHandle();
		ResetEvent(h);
	}
}