#ifndef __RAKWRAPPER_ImportsExports_
#define __RAKWRAPPER_ImportsExports_


#ifdef RAKWRAPPER_EXPORTS
#define RAKWRAPPER_API __declspec(dllexport)
#else
#define RAKWRAPPER_API __declspec(dllimport)
#endif


#endif __RAKWRAPPER_ImportsExports_