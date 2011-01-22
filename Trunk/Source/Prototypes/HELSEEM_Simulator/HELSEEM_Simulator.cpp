// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/HELSEEM_Simulator.cpp,v 1.2 2004/05/20 21:30:06 pingrri Exp $
// HELSEEM_Simulator.cpp : main source file for ZEUS_Simulator.exe and ATL_Simulator.exe
//

#include "stdafx.h"

#ifdef ZEUS_SIM
#include "ZEUS_resource.h"
#else
#include "ATL_resource.h"
#endif

#include "MainFrm.h"

CAppModule _Module;

int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	// Find the target config file, also sets the CWD
	CFileDialog dlg(TRUE,"txt",NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,"ZEUS Target Config Files (*.txt)\0*.txt\0\0", NULL);

	if (dlg.DoModal() == IDOK)
		SceneRoot::s_targetConfigFile = dlg.m_szFileName;
	else {
		MessageBox(NULL, "You MUST select a valid target configuration file to run ZEUS_Simulator", "Needs Target Config, File", MB_OK);
		return 0;
	}

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, SW_MAXIMIZE);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}

// $Log: HELSEEM_Simulator.cpp,v $
// Revision 1.2  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//
