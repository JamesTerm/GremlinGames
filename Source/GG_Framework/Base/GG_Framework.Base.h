// GG_Framework.Base.h
#pragma once

#include "../../CompilerSettings.h"

#ifdef FRAMEWORK_BASE_EXPORTS
#define FRAMEWORK_BASE_API __declspec(dllexport)
#else
#define FRAMEWORK_BASE_API __declspec(dllimport)
#endif

//////////////////
// Other Libraries

///////////////////////
// Fringe.Base Includes
#include "Thread.h"
#include "ClassFactory.h"
#include "FileOpenSave.h"
#include "Misc.h"
#include "ASSERT.h"
#include "Event.h"
#include "Profiling.h"
#include "ConfigFileIO_Base.h"
#include "ConfigFileIO.h"
#include "Joystick.h"
#include "EventMap.h"
#include "ArgumentParser.h"
#include "AVI_Writer.h"
#include "ReleaseDebugFile.h"

namespace GG_Framework
{
	namespace Base
	{
		extern FRAMEWORK_BASE_API bool TEST_IGNORE_COLLISIONS;
	}
}
