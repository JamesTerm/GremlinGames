#ifdef FRAMEWORK_BASE_EXPORTS
#define FRAMEWORK_BASE_API __declspec(dllexport)
#else
#define FRAMEWORK_BASE_API __declspec(dllimport)
#endif

#include "../../CompilerSettings.h"

//Local includes
#include "Misc.h"
#include "ASSERT.h"
#include "Event.h"