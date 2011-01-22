// Fringe.Designer.h
#pragma once

#ifdef FRINGE_DESIGNER_EXPORTS
#define FRINGE_DESIGNER_API __declspec(dllexport)
#else
#define FRINGE_DESIGNER_API __declspec(dllimport)
#endif

FRINGE_DESIGNER_API int fnUI(void);
