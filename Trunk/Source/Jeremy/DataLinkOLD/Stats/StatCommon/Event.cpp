#include "StdAfx.h"
#include "FrameWork_Threads.h"

using namespace std;

// Constructor
Event::Event( const bool AutoReset )
: m_EventHandle( ::CreateEvent( NULL , AutoReset ? FALSE : TRUE , FALSE , NULL ) )
{	assert(m_EventHandle);
}

// Destructor
Event::~Event( void )
{	// Kill the event
	assert(m_EventHandle);
	::CloseHandle( m_EventHandle );
}

bool Event::Wait( const DWORD Time )
{	assert(m_EventHandle);
	return ( ::WaitForSingleObject( m_EventHandle , Time ) == WAIT_OBJECT_0 ) ? true : false;
}

// Set (and reset) the object
void Event::Set( const bool Flag /* false for reset */)
{	assert(m_EventHandle);
	if (Flag)	
		::SetEvent(m_EventHandle);
	else
		::ResetEvent(m_EventHandle);
}

// Reset the object. Same as calling Set(false)
void Event::Reset( void )
{	Set(false);
}

Event::operator HANDLE (void)
{
	return m_EventHandle;
}


  /*******************************************************************************************************/
 /*												EventArray												*/
/*******************************************************************************************************/

// Add an event to the list
void EventArray::push_back( Event &new_event )
{	
	// Add this
	m_events.push_back( new_event );
}

// Clear the list
void EventArray::clear( void )
{
	// Clear
	m_events.clear();
}

// Get the number of events
size_t EventArray::size( void ) const
{
	// Return the size
	return m_events.size();
}

int EventArray::Wait( const bool WaitAll, const DWORD Time )
{	// Perform the wait
	const DWORD result = ::WaitForMultipleObjects( (DWORD)size(), (HANDLE*)&m_events[0], WaitAll, Time );
	
	if (result==WAIT_FAILED)
	{
		DWORD error=GetLastError();
		//This shouldn't be failing
		assert(false);
	}

	if ((result>=WAIT_OBJECT_0)&&(result<WAIT_OBJECT_0+size()))
		return result-WAIT_OBJECT_0;
	else
		return-1;
}