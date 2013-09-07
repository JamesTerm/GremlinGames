/*----------------------------------------------------------------------------*/
/* Copyright (c) FIRST 2008. All Rights Reserved.							  */
/* Open Source Software - may be modified and shared by FRC teams. The code   */
/* must be accompanied by the FIRST BSD license file in $(WIND_BASE)/WPILib.  */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "OSAL/Task.h"

//#include "NetworkCommunication/UsageReporting.h"
//#include "WPIErrors.h"
//#include <errnoLib.h>
#include <string.h>
//#include <taskLib.h>
//#include <usrLib.h>
#include <Windows.h>


//const UINT32 NTTask::kDefaultPriority;
//const INT32 NTTask::kInvalidTaskID;

/**
 * Create but don't launch a task.
 * @param name The name of the task.  "FRC_" will be prepended to the task name.
 * @param function The address of the function to run as the new task.
 * @param priority The VxWorks priority for the task.
 * @param stackSize The size of the stack for the task
 */
NTTask::NTTask(const char* name, FUNCPTR function, INT32 priority, UINT32 stackSize)
{
	//m_taskID = kInvalidTaskID;
	m_Handle=NULL;
	m_function = function;
	m_priority = priority;
	m_stackSize = stackSize;
	m_taskName = new char[strlen(name) + 5];
	strcpy(m_taskName, "FRC_");
	strcpy(m_taskName+4, name);

	static INT32 instances = 0;
	instances++;
	//TODO find this method
	//nUsageReporting::report(nUsageReporting::kResourceType_Task, instances, 0, m_taskName);
}

NTTask::~NTTask()
{
	//if (m_taskID != kInvalidTaskID) Stop();
	if (m_Handle)
		Stop();
	delete [] m_taskName;
	m_taskName = NULL;
}

// The thread callback
DWORD thread_proc( void *p_ptr )
{	// Get the pointer to myself
	NTTask *p_this = (NTTask*)p_ptr;
	assert( p_this );

	(*p_this->m_function)( p_this->m_Arg[0],p_this->m_Arg[1],p_this->m_Arg[2],p_this->m_Arg[3],
						   p_this->m_Arg[4],p_this->m_Arg[5],p_this->m_Arg[6],p_this->m_Arg[7],
						   p_this->m_Arg[8],p_this->m_Arg[9]);
	return 0;
}

bool NTTask::StartInternal()
{
	if (m_Handle)
	{
		assert(false);  // This may be lifted... just want to see if it happens
		Stop();
	}

	m_Handle = ::CreateThread( NULL, m_stackSize, (LPTHREAD_START_ROUTINE)thread_proc, (void*)this, NULL, &m_ID );
	return m_Handle!=NULL;
}

/**
 * Starts this task.
 * If it is already running or unable to start, it fails and returns false.
 */
bool NTTask::Start(UINT32 arg0, UINT32 arg1, UINT32 arg2, UINT32 arg3, UINT32 arg4, 
		UINT32 arg5, UINT32 arg6, UINT32 arg7, UINT32 arg8, UINT32 arg9)
{
	//m_taskID = taskSpawn(m_taskName,
	//					m_priority,
	//					VX_FP_TASK,							// options
	//					m_stackSize,						// stack size
	//					m_function,							// function to start
	//					arg0, arg1, arg2, arg3, arg4,	// parameter 1 - pointer to this class
	//					arg5, arg6, arg7, arg8, arg9);// additional unused parameters
	m_Arg[0]=arg0,	m_Arg[1]=arg1,	m_Arg[2]=arg2,	m_Arg[3]=arg3;
	m_Arg[4]=arg4,	m_Arg[5]=arg5,	m_Arg[6]=arg6,	m_Arg[7]=arg7;
	m_Arg[8]=arg8,	m_Arg[9]=arg9;
	//bool ok = HandleError(m_taskID);
	//if (!ok) m_taskID = kInvalidTaskID;
	return StartInternal();
}

/**
 * Restarts a running task.
 * If the task isn't started, it starts it.
 * @return false if the task is running and we are unable to kill the previous instance
 */
bool NTTask::Restart()
{
	//return HandleError(taskRestart(m_taskID));
	Stop();
	return StartInternal();
}

/**
 * Kills the running task.
 * @returns true on success false if the task doesn't exist or we are unable to kill it.
 */
bool NTTask::Stop()
{
	if (!m_Handle) return false;
	bool ok = true;
	// Wait for the thread to finish
	#ifdef	_DEBUG
	try_again:
	#endif	_DEBUG

	//const int TimeOut=2000;
	const int TimeOut=INFINITE;
	if ( ::WaitForSingleObject( m_Handle , TimeOut ) == WAIT_TIMEOUT )
	{	// Signal the thread as having been terminated
		//if ( m_p_error ) *m_p_error = true;

		// If this gets triggered we have a bug in the code.
#ifdef	_DEBUG
		switch( ::MessageBoxW( NULL,	L"A thread being used by the application\n"
			L"has taken to long to exit and so is about\n"
			L"to be terminated to avoid locking-up\n"
			L"the application.\n\n"
			L"Click ABORT to debug.\n"
			L"Click RETRY to wait for a bit longer.\n"
			L"Click IGNORE to terminate the thread.\n\n"
			L"This message is NOT displayed in release mode.",
			L"Thread exit has timed out.",
			MB_ABORTRETRYIGNORE ) )
		{	case IDRETRY:	goto try_again;
		case IDABORT:	::DebugBreak(); break;
		case IDIGNORE:	break;
		}
#endif	_DEBUG

		// Free thread memory
		CONTEXT c_ = {0};
		c_.ContextFlags = CONTEXT_FULL;
		::GetThreadContext( m_Handle, &c_ );
		MEMORY_BASIC_INFORMATION Info_ = {0};

#ifdef _M_X64
		::VirtualQuery( (PVOID) c_.Rsp, &Info_, sizeof(Info_) );
#else
		::VirtualQuery( (PVOID) c_.Esp, &Info_, sizeof(Info_) );
#endif
		// Terminate the thread
		::TerminateThread( m_Handle, 0 );

		// Free the memory
		::VirtualFree( Info_.AllocationBase, 0, MEM_RELEASE ); 		
	}

	//if (Verify())
	//{
	//	ok = HandleError(taskDelete(m_taskID));
	//}
	//m_taskID = kInvalidTaskID;
	// The thread has finished

	CloseHandle( m_Handle );
	m_Handle = NULL;
	return ok;
}

/**
 * Returns true if the task is ready to execute (i.e. not suspended, delayed, or blocked).
 * @return true if ready, false if not ready.
 */
bool NTTask::IsReady()
{
	//return taskIsReady(m_taskID);
	return m_Handle!=NULL;
}

/**
 * Returns true if the task was explicitly suspended by calling Suspend()
 * @return true if suspended, false if not suspended.
 */
bool NTTask::IsSuspended()
{
	//return taskIsSuspended(m_taskID);
	return false;
}

/**
 * Pauses a running task.
 * Returns true on success, false if unable to pause or the task isn't running.
 */
bool NTTask::Suspend()
{
	//return HandleError(taskSuspend(m_taskID));
	assert(false);
	return false;
}

/**
 * Resumes a paused task.
 * Returns true on success, false if unable to resume or if the task isn't running/paused.
 */
bool NTTask::Resume()
{
	//return HandleError(taskResume(m_taskID));
	assert(false);
	return false;
}

/**
 * Verifies a task still exists.
 * @returns true on success.
 */
bool NTTask::Verify()
{
	//return taskIdVerify(m_taskID) == OK;
	return true;
}

/**
 * Gets the priority of a task.
 * @returns task priority or 0 if an error occured
 */
INT32 NTTask::GetPriority()
{
	//if (HandleError(taskPriorityGet(m_taskID, &m_priority)))
	//	return m_priority;
	//else
	//	return 0;
	return m_priority;
}

/**
 * This routine changes a task's priority to a specified priority.
 * Priorities range from 0, the highest priority, to 255, the lowest priority.
 * Default task priority is 100.
 * @param priority The priority the task should run at.
 * @returns true on success.
 */
bool NTTask::SetPriority(INT32 priority)
{
	m_priority = priority;
	//return HandleError(taskPrioritySet(m_taskID, m_priority));
	return true;
}

/**
 * Returns the name of the task.
 * @returns Pointer to the name of the task or NULL if not allocated
 */
const char* NTTask::GetName()
{
	return m_taskName;
}

/**
 * Get the ID of a task
 * @returns Task ID of this task.  NTTask::kInvalidTaskID (-1) if the task has not been started or has already exited.
 */
INT32 NTTask::GetID()
{
	//if (Verify())
	//	return m_taskID;
	if (m_Handle)
		return m_ID;
	return kInvalidTaskID;
}

/**
 * Handles errors generated by task related code.
 */
bool NTTask::HandleError(STATUS results)
{
	if (results != ERROR) return true;
	//switch(errnoGet())
	//{
	//case S_objLib_OBJ_ID_ERROR:
	//	wpi_setWPIErrorWithContext(TaskIDError, m_taskName);
	//	break;
	//	
	//case S_objLib_OBJ_DELETED:
	//	wpi_setWPIErrorWithContext(TaskDeletedError, m_taskName);
	//	break;
	//	
	//case S_taskLib_ILLEGAL_OPTIONS:
	//	wpi_setWPIErrorWithContext(TaskOptionsError, m_taskName);
	//	break;
	//	
	//case S_memLib_NOT_ENOUGH_MEMORY:
	//	wpi_setWPIErrorWithContext(TaskMemoryError, m_taskName);
	//	break;
	//	
	//case S_taskLib_ILLEGAL_PRIORITY:
	//	wpi_setWPIErrorWithContext(TaskPriorityError, m_taskName);
	//	break;

	//default:
	//	printErrno(errnoGet());
	//	wpi_setWPIErrorWithContext(TaskError, m_taskName);
	//}
	return false;
}

