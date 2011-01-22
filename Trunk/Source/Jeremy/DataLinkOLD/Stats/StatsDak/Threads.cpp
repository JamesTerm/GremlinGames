#include "stdafx.h"

#ifdef SCOREBOARD_BUILD_OUTSIDE_VT_SOLUTION

FunctionBlock::FunctionBlock(VariableBlock *Fcn)
{	Ptr = Fcn;
	if (Ptr) Ptr->Block();
}

FunctionBlock::FunctionBlock(VariableBlock &Fcn)
{	Ptr=&Fcn;
	Ptr->Block();
}

FunctionBlock::~FunctionBlock(void)
{	if (Ptr) Ptr->UnBlock();
}

bool VariableBlock::TryBlock(void)
{	if (!TryEnterCriticalSection(&myBlock)) return false;
	m_Locks++;
	return true;
}

//The non-debug version
/**/
void VariableBlock::Block(void)		
{	EnterCriticalSection(&myBlock);	
	m_Locks++;
}

void VariableBlock::UnBlock(void)		
{	m_Locks--;
	assert( m_Locks>=0 );
	LeaveCriticalSection(&myBlock);	
}

VariableBlock::VariableBlock(void)	
{	InitializeCriticalSection(&myBlock);
	m_Locks=0;
}

VariableBlock::~VariableBlock(void)	
{	DeleteCriticalSection(&myBlock);	 
}

// Lets get those CPU fans working !!
void Thread::FireStarter(void *ptr)
{	// Get the pointer to myself
	Thread*	sfc=(Thread*)ptr;
	
	// Call It
	sfc->ThreadEntryPoint();

	// Set Exit Event
	ExitThread(NULL);
}

// start the thread
void Thread::StartThread( const int StackSize )
{	MyHandle=CreateThread( NULL, StackSize, (LPTHREAD_START_ROUTINE)FireStarter, (void *)this, NULL, &MyHandleID );
	assert( MyHandle );
	SetThreadPriorityBoost(MyHandle,TRUE);
}

// Setup
Thread::Thread(void)
{	MyHandle=NULL; 
}

Thread::~Thread(void)
{	CloseHandle(MyHandle);
}

void Thread::SetThreadPriority(int nPriority)
{	::SetThreadPriority(MyHandle,nPriority);
	SetThreadPriorityBoost(MyHandle,TRUE);
}

//**********************************************************************************************************
void WorkerThread::ThreadProcessor_Start(void)
{	
}

//**********************************************************************************************************
void WorkerThread::ThreadProcessor_End(void)
{	
}

//**********************************************************************************************************
bool WorkerThread::GetRunning(void)
{	FunctionBlock a(this);
	return IsThreadRunning;
}

//**********************************************************************************************************
void WorkerThread::SetRunning(bool Flag)
{	FunctionBlock a(this);
	IsThreadRunning=Flag;
}

//**********************************************************************************************************
bool WorkerThread::GetThreadMustExit(void)
{	FunctionBlock a(this);
	return ThreadMustExit;
}

//**********************************************************************************************************
void WorkerThread::SetThreadMustExit(bool Flag)
{	FunctionBlock a(this);
	ThreadMustExit=Flag;
}

//**********************************************************************************************************
void WorkerThread::ThreadEntryPoint(void)
{	SetRunning(true);

	//OutputDebugString("Thread Started ...\n");
	ThreadProcessor_Start();

#ifndef _DEBUG
	try	{
#endif _DEBUG
			while(!GetThreadMustExit()) 
				ThreadProcessor();
#ifndef _DEBUG
		} catch(...) {}
#endif _DEBUG

	ThreadProcessor_End();
	//OutputDebugString("Thread Finished ...\n");

	SetRunning(false);
}

//**********************************************************************************************************
void WorkerThread::StartThread(void)
{	SetThreadMustExit(false);
	SetRunning(false);
	Thread::StartThread();
	while(!GetRunning()) Sleep(1);	
	OutputDebugString("Thread started !\n");
}

//**********************************************************************************************************
void WorkerThread::StopThread( const int TimeOut )
{	// Ask it to exit, and wait for it to happen
	if (!GetRunning()) return;
	SetThreadMustExit(true);

	// Get the start time
	unsigned StartTickCount = GetTickCount();

	while( GetRunning() )
	{	// Ug, thread was terminated for me, or has already been terminated !
		DWORD Result;
		GetExitCodeThread(MyHandle,&Result);
		if (Result!=STILL_ACTIVE) return;	

		// Has to much time expired
		if ( (TimeOut!=INFINITE) && ( GetTickCount()-StartTickCount > TimeOut ) )
		{	
#ifdef _DEBUG
			// THREAD TOOK TO LONG TO EXIT
			//assert(false);
#endif _DEBUG
			// Forced thread stopping !
			OutputDebugString("Thread %x (%x) has not stopped, but we timed out waiting. *** THREAD DESTROYED ***\n");
			TerminateThread( MyHandle , 0 );
			SetRunning(false);
		}

		// Sleep
		Sleep(1);
	}
	
	// Finish closing
	SetThreadMustExit(false);
	CloseHandle(MyHandle);
	MyHandle=NULL;
	OutputDebugString("Thread stopped !\n");
}

WorkerThread::WorkerThread(void) : IsThreadRunning(false), ThreadMustExit(false)
{
}

WorkerThread::~WorkerThread(void)
{	StopThread();	
}

#endif