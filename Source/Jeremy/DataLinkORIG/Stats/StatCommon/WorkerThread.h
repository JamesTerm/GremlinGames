#pragma once

/*	An example of how to use the templated thread class :

		struct MyTest
		{	void operator() ( const int T )
			{	OutputDebugString("Hello World !\n");
				Sleep(100);
			}
		} _MyTest;

		Thread<MyTest, const int >	Thread1( _MyTest, 1 );
		Thread1.LowPriority();

		Thread<MyTest, const int >	Thread2( _MyTest, 2 );
		Thread2.HighPriority();
*/

template< typename threadcall_type, typename threadcall_param_type = const void* >
struct Thread
{			// Constructor
			Thread( threadcall_type *pCaller );
			Thread( threadcall_type &rCaller );

			Thread( threadcall_type *pCaller, threadcall_param_type data );
			Thread( threadcall_type &rCaller, threadcall_param_type data );

			// Destructor
			~Thread( void );			

			// Set the priority
			void RealTimePriority( void );
			void HighPriority( void );
			void MediumPriority( void );
			void LowPriority( void );
			void IdlePriority( void );

private:	// Deliberately private so that it cannot be copied.
			Thread( const Thread &CopyFrom ) { assert(false); }
	
			// Start the thread
			bool Start( const int StackSize=128*1024 );
			bool Stop( void );
	
			// The thread callback
			static DWORD _ThreadProc_( void *Ptr );

			// The method to call.
			threadcall_type			*m_pDestination;

			// A parameter to use
			threadcall_param_type	m_param;
	
			// The thread handle
			HANDLE	m_Handle;

			// The thread should exit now
			bool m_ThreadShouldExit;
};

#include "WorkerThread_Impl.h"