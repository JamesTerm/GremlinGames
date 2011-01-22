// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/Utility.h,v 1.2 2004/05/18 21:30:37 pingrri Exp $
// Utility.h : Miscellaneous Function Prototypes
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

extern const char* GetProjectName();
extern void GetProductInfo(const char* verInfo, char* retBuff, DWORD buffLen);

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define sqr(x)				((x)*(x))
#define deg2rad(x)			((x)*M_PI/180.0)
#define rad2deg(x)			((x)*180.0/M_PI)
#define inches2Meters(x)	((x)*0.0254)
#define Meters2Feet(x)		((x)*3.2808)
#define MIN(a,b)			((a)<(b)?(a):(b))
#define MAX(a,b)			((a)>(b)?(a):(b))
#define numberof(things)	((sizeof(things)/sizeof(*(things))))


/////////////////////////////////////////////////////////////////////////////
// $Log: Utility.h,v $
// Revision 1.2  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//