//
// OpenThread library, Copyright (C) 2002 - 2003  The Open Thread Group
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 

//
// SprocThreadPrivateActions.c++ - Thread private actions for sprocs
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <OpenThreads/Thread>
#include <list>

#ifndef SPROC_THREAD_PRIVATE_ACTIONS
#define SPROC_THREAD_PRIVATE_ACTIONS

namespace OpenThreads {

    class ThreadPrivateActions {

	//-------------------------------------------------------------------------
	// We're friendly to Thread, so it can issue the methods.
	//
	friend class Thread;
	
    public:

	static void ThreadCancelTest();
	
	static void PushCancelFunction(void (*routine)(void *), void *arg);

	static void PopCancelFunction();

    private:
	
	static bool *GetDeathFlag(Thread *thread);
	
	static Thread *GetThread(pid_t thread_id);
	
	static void ThreadCancelHandler(int sigid);
	
	//-------------------------------------------------------------------------
	// standard start routine.
	//
	static void StartThread(void *data);
	
	static void AddThread(Thread *thread);
	
	static void RemoveThread(Thread *thread);
	
	static void PrintThreadSchedulingInfo(Thread *thread);
	
	static int SetThreadSchedulingParams(Thread *thread);
	
    private:

	static std::list<Thread *> s_threadList;
	
    };
    
}

#endif // !SPROC_THREAD_PRIVATE_ACTIONS
