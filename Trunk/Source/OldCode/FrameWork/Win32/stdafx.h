// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// Windows Header Files:
#include <windows.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Standard C++ support
#include <vector>
#include <queue>
#include <stack>
#include <set>
#include <algorithm>
#include <functional>

#define Profile_Begin \
	__int64 bf,at,freq;	\
	QueryPerformanceCounter((LARGE_INTEGER *)&bf);

#define Profile_End \
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq); \
	QueryPerformanceCounter((LARGE_INTEGER *)&at);	\
	DebugOutput(L"Time %d\n",((at-bf)*1000)/freq);

#define Profile_Chop \
	Profile_End \
	QueryPerformanceCounter((LARGE_INTEGER *)&bf);
