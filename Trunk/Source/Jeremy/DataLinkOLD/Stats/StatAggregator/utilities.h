#pragma once

// A basic critical section class
struct critical_section
{				// Constructor
				critical_section( void );

				// Destructor
				~critical_section( void );

				// Lock this critical section
				void lock( void );

				// Unlock this critical section
				void unlock( void );

				// Try locking this critical section and return true if success
				bool try_lock( void );				

	private:	// Internal data
				CRITICAL_SECTION	m_CS;
#ifdef _DEBUG
				volatile LONG m_c_locks;
#endif
};