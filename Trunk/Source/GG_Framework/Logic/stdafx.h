// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <vector>
#include <set>
#include <map>

#include <GetTime.h>
#include <osgDB/Registry>
#include <OpenThreads/Thread>
#include <OpenThreads/Mutex>
#include <osg/Vec3>
#include <osg/Quat>

// [Rick] James, I was all the sudden having internal compiler error C1001 problems.  
// I don't know why, but commenting this line out fixed them.  As I worked through the files, It appears there is something
// VS no longet likes about having "ServerManager.h" in the stdafx.h file.  Something about ServerManager being defined twice.
// I have no desire to track it down further.
// #include "GG_Framework.Logic.h"

// I will go ahead and include the external library headers.

//////////////////
// Other Libraries
#include "Scripting\GG_Framework.Logic.Scripting.h"
#include "Network\GG_Framework.Logic.Network.h"
#include "..\Base\GG_Framework.Base.h"
#include "..\UI\GG_Framework.UI.h"
#include "Physics\GG_Framework.Logic.Physics.h"
