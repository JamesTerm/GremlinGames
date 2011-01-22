#pragma once

#ifdef FRAMEWORK_LUA_EXPORTS
#define FRAMEWORK_LUA_API __declspec(dllexport)
#else
#define FRAMEWORK_LUA_API __declspec(dllimport)
#endif