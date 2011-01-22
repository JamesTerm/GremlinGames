#pragma once

#ifdef FRAMEWORK_RAKNET_EXPORTS
#define FRAMEWORK_RAKNET_API __declspec(dllexport)
#else
#define FRAMEWORK_RAKNET_API __declspec(dllimport)
#endif