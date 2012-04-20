// GG_Framework.Logic.Scripting.h
#pragma once

//The stand alone works in AI tester, but not in the game (we may want to fix that at some point)
#undef __UseStandAloneVersion__

#ifdef FRAMEWORK_LOGIC_SCRIPTING_EXPORTS
#define FRAMEWORK_LOGIC_SCRIPTING_API __declspec(dllexport)
#else
#define FRAMEWORK_LOGIC_SCRIPTING_API __declspec(dllimport)
#endif

///////////////////
// Useful Constants

//////////////////
// Other Libraries
#include "..\..\..\CompilerSettings.h"
#include "..\..\Base\GG_Framework.Base.h"

#ifndef __UseStandAloneVersion__
// We will have to include all of these at once anyway
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#endif

///////////////////////
// Fringe.Logic.Scripting Includes
#ifndef __UseStandAloneVersion__
#include "Register_C_Functions.h"
#endif

#include "Script.h"
