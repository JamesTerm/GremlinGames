// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include "../../DataLinkTester/compiler_settings.h"

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// TODO: reference additional headers your program requires here
#import "C:\Program Files\Common Files\System\ADO\msado15.dll" rename("EOF", "ADOEOF")

#include <list>
#include <queue>
#include <set>
#include <string>

#include <cassert>
#include <tchar.h>
#include <iostream>