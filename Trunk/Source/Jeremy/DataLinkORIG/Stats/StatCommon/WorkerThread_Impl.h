#pragma once

template< typename threadcall_type, typename threadcall_param_type >
Thread<threadcall_type,threadcall_param_type>::Thread( threadcall_type *pCaller )
	:	m_pDestination( pCaller ) ,
		m_Handle( NULL ) , m_ThreadShouldExit( false )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type >
Thread<threadcall_type,threadcall_param_type>::Thread( threadcall_type &rCaller )
	:	m_pDestination( &rCaller ) ,
		m_Handle( NULL ) , m_ThreadShouldExit( false )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type >
Thread<threadcall_type,threadcall_param_type>::Thread( threadcall_type *pCaller, threadcall_param_type data )
	:	m_pDestination( pCaller ) ,
		m_Handle( NULL ) , 
		m_ThreadShouldExit( false ), 
		m_param( data )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

template< typename threadcall_type, typename threadcall_param_type >
Thread<threadcall_type,threadcall_param_type>::Thread( threadcall_type &rCaller, threadcall_param_type data )
	:	m_pDestination( &rCaller ) ,
		m_Handle( NULL ) , 
		m_ThreadShouldExit( false ),
		m_param( data )
{	// Start the thread
	const bool _Start = Start();
	assert(_Start);
}

// Destructor
template< typename threadcall_type, typename threadcall_param_type >
Thread<threadcall_type,threadcall_param_type>::~Thread( void )
{	// Stop the thread
	// Start the thread
	const bool _Stop = Stop();
	assert(_Stop);
}

template< typename threadcall_type, typename threadcall_param_type >
bool Thread<threadcall_type,threadcall_param_type>::Start( const int StackSize )
{	// We cannot be running
	if (m_Handle) return false;
	if (!m_pDestination) return false;

	// Start the thread
	m_ThreadShouldExit = false;
	m_Handle = ::CreateThread( NULL, StackSize, (LPTHREAD_START_ROUTINE)_ThreadProc_, (void*)this, NULL, NULL );
	assert(m_Handle);
	if (!m_Handle) return false;

	// Success
	return true;
}

template< typename threadcall_type, typename threadcall_param_type >
bool Thread<threadcall_type,threadcall_param_type>::Stop( void )
{	// We must be running
	if (!m_Handle) return false;

	// Signal an exit
	m_ThreadShouldExit = true;

	// Wait for the thread to finish
	::WaitForSingleObject( m_Handle , INFINITE );

	// The thread has finished
	CloseHandle( m_Handle );
	m_Handle = NULL;

	// Success
	return true;
}

// The thread callback
template< typename threadcall_type, typename threadcall_param_type >
DWORD Thread<threadcall_type,threadcall_param_type>::_ThreadProc_( void *Ptr )
{	// Get the pointer to myself
	Thread *p_this = (Thread*)Ptr;
	assert( p_this );
	assert( p_this->m_pDestination );

	while( !p_this->m_ThreadShouldExit )
		(*p_this->m_pDestination)( p_this->m_param );

	return 0;
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type >
void Thread<threadcall_type,threadcall_param_type>::RealTimePriority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_TIME_CRITICAL );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type >
void Thread<threadcall_type,threadcall_param_type>::HighPriority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_HIGHEST );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type >
void Thread<threadcall_type,threadcall_param_type>::MediumPriority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_NORMAL );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type >
void Thread<threadcall_type,threadcall_param_type>::LowPriority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_LOWEST );
}

// Set the priority
template< typename threadcall_type, typename threadcall_param_type >
void Thread<threadcall_type,threadcall_param_type>::IdlePriority( void )
{	assert( m_Handle );
	::SetThreadPriority( m_Handle , THREAD_PRIORITY_IDLE );
}