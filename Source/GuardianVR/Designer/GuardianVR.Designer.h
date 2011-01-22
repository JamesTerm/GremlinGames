// GuardianVR.Designer.h
#pragma once

#ifdef GUARDIANVR_DESIGNER_EXPORTS
#define GUARDIANVR_DESIGNER_API __declspec(dllexport)
#else
#define GUARDIANVR_DESIGNER_API __declspec(dllimport)
#endif

GUARDIANVR_DESIGNER_API int fnUI(void);
