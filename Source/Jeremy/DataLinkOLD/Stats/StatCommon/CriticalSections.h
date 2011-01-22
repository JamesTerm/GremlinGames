#pragma once

#ifdef STATCOMMON_EXPORTS
#define STATCOMMON_API __declspec(dllexport)
#else
#define STATCOMMON_API __declspec(dllimport)
#endif

// A basic critical section class
struct STATCOMMON_API CriticalSection
{				// Constructor
				CriticalSection( void );

				// Destructor
				~CriticalSection( void );

				// Lock this critical section
				void Lock( void );

				// Unlock this critical section
				void UnLock( void );

				// Try locking this critical section and return true if success
				bool TryLock( void );

	private:	// Internal data
				CRITICAL_SECTION	m_CS;
#ifdef _DEBUG
				volatile LONG m_cLocks;
#endif
};

// A stack based lock for the critical section
struct STATCOMMON_API AutoLock
{				// Constructor
				AutoLock( CriticalSection *pCritSec );
				AutoLock( CriticalSection &rCritSec );

				// Destructor
				~AutoLock( void );

	private:	// Internal data
				CriticalSection		*m_pCritSec;
};

// A stack based lock for the critical section
struct STATCOMMON_API AutoUnLock
{				// Constructor
				AutoUnLock( CriticalSection *pCritSec );
				AutoUnLock( CriticalSection &rCritSec );

				// Destructor
				~AutoUnLock( void );

	private:	// Internal data
				CriticalSection		*m_pCritSec;
};