#ifdef FRAMEWORK_WIN32_EXPORTS
#define FRAMEWORK_WIN32_API __declspec(dllexport)
#else
#define FRAMEWORK_WIN32_API __declspec(dllimport)
#endif

//Compiler settings
#include "../../CompilerSettings.h"

#include "../Base/FrameWork_Base.h"

using namespace std;

#include "FileOpenSave.h"
#include "LoadSave.h"
#include "Misc.h"
#include "Thread.h"