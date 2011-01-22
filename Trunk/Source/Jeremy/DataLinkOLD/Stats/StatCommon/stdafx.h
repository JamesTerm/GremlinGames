// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT			0x0500	// So that we have TryEnterCriticalSection
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#include <queue>
#include <set>
#include <string>

// For local Framework::Threads
#include <iostream>

// For local Framework::XML
// expat
#include "expat\expat.h"