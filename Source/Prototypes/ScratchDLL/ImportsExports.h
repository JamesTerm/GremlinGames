#ifndef __ScratchDLL_ImportsExports_
#define __ScratchDLL_ImportsExports_


#ifdef SCRATCHDLL_EXPORTS
#define SCRATCHDLL_API __declspec(dllexport)
#else
#define SCRATCHDLL_API __declspec(dllimport)
#endif


#endif __ScratchDLL_ImportsExports_