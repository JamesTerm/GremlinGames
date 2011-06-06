// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "../../../CompilerSettings.h"

#include <stdio.h>
#include <string.h>
#include <math.h>
//I'm using the waitable timers so I need to up the spec here
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <malloc.h>
#include <assert.h>
#include <algorithm>

#include <mmreg.h>
