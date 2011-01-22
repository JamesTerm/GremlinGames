// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
#pragma once

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif _CRT_SECURE_NO_DEPRECATE


// UNCOMMENT THIS TO BUILD PROJECT OUTSIDE OF VT SOLUTION
#define SCOREBOARD_BUILD_OUTSIDE_VT_SOLUTION

// UNCOMMENT THIS TO USE OLD IMPLEMENTATION
// #define SCOREBOARD_USE_OLD_IMPLEMENTATION

// UNCOMMENT THIS TO TEST READING FROM TXT FILES
// TODO This should be commented out in the working version
#undef SCOREBOARD_READ_FROM_SAMPLE_TEXT

#define	SCOREBOARD_FILENAME_OES					"C:\\Temp\\Washington_OES.txt"
#define	SCOREBOARD_FILENAME_WHITEWAY			"c:\\Temp\\whiteway.txt"
#define	SCOREBOARD_FILENAME_DAKTRONICS			"c:\\Temp\\DaktronicsBasketballDump2.txt"


#ifndef SCOREBOARD_BUILD_OUTSIDE_VT_SOLUTION
#include "Source/FrameWork/Base/misc.h"
#include "..\..\Controls_ControlDLL.h"
#include "../../../Controls/SerialSwitcher/StdAfx.h"
#endif

#ifdef SCOREBOARD_USE_OLD_IMPLEMENTATION
#include "StatsDak.h"
#else SCOREBOARD_USE_OLD_IMPLEMENTATION

#ifdef SCOREBOARD_BUILD_OUTSIDE_VT_SOLUTION
#include <windows.h>
#include <stdio.h>
#include <cassert>
#include <malloc.h>
#include "SCSerialCommunications.h"
#include "Threads.h"
#endif

#include "exports.h"
#include "StatsBoardBase.h"
#include "StatsDaktronics.h"
#include "StatsWhiteway.h"
#include "StatsDSI.h"
#include "StatsOES.h"
#include "StatsTransLuxMP70.h"
#endif SCOREBOARD_USE_OLD_IMPLEMENTATION

#ifdef SCOREBOARD_BUILD_OUTSIDE_VT_SOLUTION

typedef BYTE byte;

#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);

#endif


// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

