#include "StdAfx.h"
#include "FrameWork_Threads.h"

// Constructor
CriticalSection::CriticalSection( void )
{	::InitializeCriticalSection( &m_CS );
#ifdef _DEBUG
	m_cLocks = 0L;
#endif
}

// Destructor
CriticalSection::~CriticalSection( void )
{	::DeleteCriticalSection( &m_CS );
}

// Lock this critical section
void CriticalSection::Lock( void )
{	::EnterCriticalSection( &m_CS );
#ifdef _DEBUG
	InterlockedIncrement(&m_cLocks);
#endif
}

// Unlock this critical section
void CriticalSection::UnLock( void )
{	
#ifdef _DEBUG
	LONG cLocks = InterlockedDecrement(&m_cLocks);
	assert( cLocks>=0 );
#endif
	::LeaveCriticalSection( &m_CS );
}

// Try locking this critical section and return true if success
bool CriticalSection::TryLock( void )
{	//return ( ::TryEnterCriticalSection( &m_CS ) ) ? true : false;
	if ( !::TryEnterCriticalSection( &m_CS ) ) return false;
#ifdef _DEBUG
	InterlockedIncrement(&m_cLocks);
#endif
	return true;
}



// Constructor
AutoLock::AutoLock( CriticalSection *pCritSec ) : m_pCritSec( pCritSec )
{	assert( m_pCritSec );
	m_pCritSec->Lock();
}

AutoLock::AutoLock( CriticalSection &rCritSec ) : m_pCritSec( &rCritSec )
{	assert( m_pCritSec );
	m_pCritSec->Lock();
}

// Destructor
AutoLock::~AutoLock( void )
{	m_pCritSec->UnLock();
}


// Constructor
AutoUnLock::AutoUnLock( CriticalSection *pCritSec ) : m_pCritSec( pCritSec )
{	assert( m_pCritSec );
	m_pCritSec->UnLock();
}

AutoUnLock::AutoUnLock( CriticalSection &rCritSec ) : m_pCritSec( &rCritSec )
{	assert( m_pCritSec );
	m_pCritSec->UnLock();
}

// Destructor
AutoUnLock::~AutoUnLock( void )
{	m_pCritSec->Lock();
}
