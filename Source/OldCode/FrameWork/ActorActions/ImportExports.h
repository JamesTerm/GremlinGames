#pragma once

#ifdef FRAMEWORK_ACTORACTIONS_EXPORTS
#define FRAMEWORK_ACTORACTIONS_API __declspec(dllexport)
#else
#define FRAMEWORK_ACTORACTIONS_API __declspec(dllimport)
#endif
