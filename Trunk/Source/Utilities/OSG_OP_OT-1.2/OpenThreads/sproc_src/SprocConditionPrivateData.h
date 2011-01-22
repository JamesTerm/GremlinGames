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
// SprocConditionPrivateData.h - Private data structure for Condition
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#ifndef _SPROCCONDITIONPRIVATEDATA_H_
#define _SPROCCONDITIONPRIVATEDATA_H_

#include <unistd.h>
#include <sys/types.h>
#include <bstring.h>
#include <sys/time.h>
#include <ulocks.h>
#include <list>

#include <OpenThreads/Mutex>
#include <OpenThreads/Condition>

namespace OpenThreads {

class SemaLink {
    
    friend class SprocConditionPrivatedata;
    
    friend class Condition;

    friend class ConditionDebug;

private:

    SemaLink() {};

    virtual ~SemaLink() {};

    SemaLink *next;

    usema_t *sema;

    int select_cond;  // 0=pre-select, 1=in-select, 2=post-select
    
};

class SprocConditionPrivateData {

    friend class Condition;

private:

    SprocConditionPrivateData() {
	
	pid_list.clear();

    };

    virtual ~SprocConditionPrivateData() {

	pid_list.clear();

    };

    std::list<pid_t> pid_list;

    Mutex mutex;

};

}

#endif // !_SPROCCONDITIONPRIVATEDATA_H_
