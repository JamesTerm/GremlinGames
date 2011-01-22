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

// SprocBarrierPrivateData.h - private data structure for barrier
// ~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _SPROCBARRIERPRIVATEDATA_H_
#define _SPROCBARRIERPRIVATEDATA_H_

#include <ulocks.h>
#include <OpenThreads/Barrier>

#ifndef USE_IRIX_NATIVE_BARRIER

#include <OpenThreads/Condition>
#include <OpenThreads/Mutex>

#endif

namespace OpenThreads {

class SprocBarrierPrivateData {

    friend class Barrier;

private:

    SprocBarrierPrivateData() {};
    
    virtual ~SprocBarrierPrivateData() {};

#ifdef USE_IRIX_NATIVE_BARRIER

    barrier_t *barrier;

    unsigned int numThreads;

#else

    OpenThreads::Condition _cond;
    
    OpenThreads::Mutex _mutex;
    
    volatile int maxcnt;
    
    volatile int cnt;

    volatile int phase;

#endif

};

}

#endif //_SPROCBARRIERPRIVATEDATA_H_
