// GG_Framework.Logic.Scripting.h
#pragma once

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

// We will have to include all of these at once anyway
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

///////////////////////
// Fringe.Logic.Scripting Includes
#include "Register_C_Functions.h"
#include "Script.h"
