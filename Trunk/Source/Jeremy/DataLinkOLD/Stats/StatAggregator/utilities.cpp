#include "stdafx.h"
#include "utilities.h"

// Constructor
critical_section::critical_section( void )
{	::InitializeCriticalSection( &m_CS );
#ifdef _DEBUG
	m_c_locks = 0L;
#endif
}

// Destructor
critical_section::~critical_section( void )
{	::DeleteCriticalSection( &m_CS );
#ifdef _DEBUG
	//assert( m_c_locks==0 );
#endif
}

// Lock this critical section
void critical_section::lock( void )
{	::EnterCriticalSection( &m_CS );
#ifdef _DEBUG
	InterlockedIncrement(&m_c_locks);
#endif
}

// Unlock this critical section
void critical_section::unlock( void )
{	
#ifdef _DEBUG
	LONG cLocks = InterlockedDecrement(&m_c_locks);
	assert( cLocks>=0 );
#endif
	::LeaveCriticalSection( &m_CS );
}

// Try locking this critical section and return true if success
bool critical_section::try_lock( void )
{	//return ( ::TryEnterCriticalSection( &m_CS ) ) ? true : false;
	if ( !::TryEnterCriticalSection( &m_CS ) ) return false;
#ifdef _DEBUG
	InterlockedIncrement(&m_c_locks);
#endif
	return true;
}