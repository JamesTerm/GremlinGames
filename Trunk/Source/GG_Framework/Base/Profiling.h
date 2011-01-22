#pragma once

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