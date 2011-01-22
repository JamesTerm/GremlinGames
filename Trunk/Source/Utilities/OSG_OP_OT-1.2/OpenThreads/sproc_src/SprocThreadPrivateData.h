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
// SprocThreadPrivateData.h - private data for sproc thread
// ~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _SPROCTHREADPRIVATEDATA_H_
#define _SPROCTHREADPRIVATEDATA_H_

#include <sys/types.h>
#include <ulocks.h>
#include <stack>

#include <OpenThreads/Thread>
#include "SprocThreadPrivateActions.h"

namespace OpenThreads {

class SprocThreadPrivateData {
    
    friend class Thread;

    friend class ThreadPrivateActions;

private:
    
    struct CancelFuncStruct {
	
	void (*routine)(void *);
	void *arg;
    };

    SprocThreadPrivateData() {};

    virtual ~SprocThreadPrivateData() {};

    volatile unsigned int stackSize;

    volatile bool stackSizeLocked;

    volatile bool isRunning;

    volatile bool isCanceled;

    volatile bool idSet;

    volatile bool cancelActive;

    volatile bool detached;

    volatile bool dieFlag;

    volatile Thread::ThreadPriority threadPriority;
    
    volatile Thread::ThreadPolicy threadPolicy;

    volatile pid_t pid;

    volatile int uniqueId;

    std::stack<CancelFuncStruct> cancelFuncStack; 

    static int nextId;

};

}

#endif // !_SPROCTHREADPRIVATEDATA_H_
