#pragma once

// osgvAXMFC.h : main header file for osgvAXMFC.DLL

#if !defined( __AFXCTL_H__ )
#error "include 'afxctl.h' before including this file"
#endif

#include "resource.h"       // main symbols


// CosgvAXMFCApp : See osgvAXMFC.cpp for implementation.

class CosgvAXMFCApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

