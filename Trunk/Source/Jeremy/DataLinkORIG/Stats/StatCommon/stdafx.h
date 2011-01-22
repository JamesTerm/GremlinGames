// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//


#pragma once

#include "targetver.h"

#include "../../DataLinkTester/compiler_settings.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT			0x0500	// So that we have TryEnterCriticalSection
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#include <queue>
#include <set>
#include <string>
#include <tchar.h>
#include <cassert>

// For local Framework::Threads
#include <iostream>

// For local Framework::XML
// expat
#include "expat\expat.h"

typedef BYTE byte;

#define wchar2char(wchar2char_pwchar_source) \
	const size_t wchar2char_Length=wcstombs(NULL,wchar2char_pwchar_source,0)+1; \
	char *wchar2char_pchar = (char *)_alloca(wchar2char_Length);; /* ";;" is needed to fix a compiler bug */ \
	wcstombs(wchar2char_pchar,wchar2char_pwchar_source,wchar2char_Length);

#define char2wchar(char2wchar_pchar_source) \
	const size_t char2wchar_Length=((strlen(char2wchar_pchar_source)+1)*sizeof(wchar_t)); \
	wchar_t *char2wchar_pwchar = (wchar_t *)_alloca(char2wchar_Length);; /* ";;" is needed to fix a compiler bug */ \
	mbstowcs(char2wchar_pwchar,char2wchar_pchar_source,char2wchar_Length);