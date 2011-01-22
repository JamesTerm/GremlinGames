// osgvAXMFC.cpp : Implementation of CosgvAXMFCApp and DLL registration.

#include "stdafx.h"
#include "osgvAXMFC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CosgvAXMFCApp theApp;

const GUID CDECL BASED_CODE _tlid =
		{ 0x6DF742EE, 0xF06E, 0x4292, { 0xBF, 0x28, 0xAD, 0xF2, 0x92, 0xA7, 0xB8, 0xF9 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;



// CosgvAXMFCApp::InitInstance - DLL initialization

BOOL CosgvAXMFCApp::InitInstance()
{
	BOOL bInit = COleControlModule::InitInstance();

	if (bInit)
	{
		// TODO: Add your own module initialization code here.
	}

	return bInit;
}



// CosgvAXMFCApp::ExitInstance - DLL termination

int CosgvAXMFCApp::ExitInstance()
{
	// TODO: Add your own module termination code here.

	return COleControlModule::ExitInstance();
}



// DllRegisterServer - Adds entries to the system registry

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}



// DllUnregisterServer - Removes entries from the system registry

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(_afxModuleAddrThis);

	if (!AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor))
		return ResultFromScode(SELFREG_E_TYPELIB);

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return ResultFromScode(SELFREG_E_CLASS);

	return NOERROR;
}
