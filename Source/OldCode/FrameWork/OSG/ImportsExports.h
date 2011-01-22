#ifdef FRAMEWORK_OSG_EXPORTS
#define FRAMEWORK_OSG_API __declspec(dllexport)
#else
#define FRAMEWORK_OSG_API __declspec(dllimport)
#endif