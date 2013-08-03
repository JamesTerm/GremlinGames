// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers


typedef   signed    char  int8_t;
typedef unsigned    char uint8_t;
typedef            short  int16_t;
typedef unsigned   short uint16_t;
typedef              int  int32_t;
typedef unsigned     int uint32_t;
typedef          __int64  int64_t;
typedef unsigned __int64 uint64_t;
typedef int                 BOOL;
typedef int 		(*FUNCPTR) (...);     /* ptr to function returning int */
// TODO: reference additional headers your program requires here
#include <intsafe.h>