// osgvAXMFCCtrl.cpp : Implementation of the CosgvAXMFCCtrl ActiveX Control class.

#include "stdafx.h"
#include "osgvAXMFC.h"
#include "osgvAXMFCCtrl.h"
#include "osgvAXMFCPropPage.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CosgvAXMFCCtrl, COleControl)



// Message map

BEGIN_MESSAGE_MAP(CosgvAXMFCCtrl, COleControl)
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
	ON_WM_TIMER()
END_MESSAGE_MAP()



// Dispatch map

BEGIN_DISPATCH_MAP(CosgvAXMFCCtrl, COleControl)
	DISP_FUNCTION_ID(CosgvAXMFCCtrl, "InitOSG", dispidInitOSG, InitOSG, VT_EMPTY, VTS_BSTR)
	DISP_PROPERTY_NOTIFY_ID(CosgvAXMFCCtrl, "SceneName", dispidSceneName, m_SceneName, OnSceneNameChanged, VT_BSTR)
	DISP_PROPERTY_NOTIFY_ID(CosgvAXMFCCtrl, "BackgroundColor", dispidBackgroundColor, m_BackgroundColor, OnBackgroundColorChanged, VT_COLOR)
END_DISPATCH_MAP()



// Event map

BEGIN_EVENT_MAP(CosgvAXMFCCtrl, COleControl)
	EVENT_CUSTOM_ID("Frame1000", eventidFrame1000, Frame1000, VTS_I4)
END_EVENT_MAP()



// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CosgvAXMFCCtrl, 1)
	PROPPAGEID(CosgvAXMFCPropPage::guid)
END_PROPPAGEIDS(CosgvAXMFCCtrl)



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CosgvAXMFCCtrl, "OSGVAXMFC.osgvAXMFCCtrl.1",
	0x59451646, 0xabd9, 0x4de4, 0x88, 0xa9, 0x65, 0x55, 0x24, 0x82, 0x8b, 0xb5)



// Type library ID and version

IMPLEMENT_OLETYPELIB(CosgvAXMFCCtrl, _tlid, _wVerMajor, _wVerMinor)



// Interface IDs

const IID BASED_CODE IID_DosgvAXMFC =
		{ 0x75C2661F, 0x87DB, 0x47D6, { 0x96, 0xB4, 0xE2, 0x93, 0xF8, 0xC4, 0x39, 0xAD } };
const IID BASED_CODE IID_DosgvAXMFCEvents =
		{ 0xA09B39CD, 0xC74A, 0x433C, { 0x88, 0xBC, 0x7D, 0xDF, 0xDA, 0x2E, 0xED, 0x51 } };



// Control type information

static const DWORD BASED_CODE _dwosgvAXMFCOleMisc =
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CosgvAXMFCCtrl, IDS_OSGVAXMFC, _dwosgvAXMFCOleMisc)



// CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CosgvAXMFCCtrl

BOOL CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_OSGVAXMFC,
			IDB_OSGVAXMFC,
			afxRegApartmentThreading,
			_dwosgvAXMFCOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}



// Licensing strings

static const TCHAR BASED_CODE _szLicFileName[] = _T("osgvAXMFC.lic");

static const WCHAR BASED_CODE _szLicString[] =
	L"Copyright (c) 2009 ";



// CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::VerifyUserLicense -
// Checks for existence of a user license

BOOL CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::VerifyUserLicense()
{
	return AfxVerifyLicFile(AfxGetInstanceHandle(), _szLicFileName,
		_szLicString);
}



// CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::GetLicenseKey -
// Returns a runtime licensing key

BOOL CosgvAXMFCCtrl::CosgvAXMFCCtrlFactory::GetLicenseKey(DWORD dwReserved,
	BSTR FAR* pbstrKey)
{
	if (pbstrKey == NULL)
		return FALSE;

	*pbstrKey = SysAllocString(_szLicString);
	return (*pbstrKey != NULL);
}



// CosgvAXMFCCtrl::CosgvAXMFCCtrl - Constructor

CosgvAXMFCCtrl::CosgvAXMFCCtrl() : m_eventTimer(0), m_OSG(NULL)
{
	InitializeIIDs(&IID_DosgvAXMFC, &IID_DosgvAXMFCEvents);
}



// CosgvAXMFCCtrl::~CosgvAXMFCCtrl - Destructor

CosgvAXMFCCtrl::~CosgvAXMFCCtrl()
{
}

struct ColorChar
{
	unsigned char R,G,B,A;
};
union ColorUnion
{
	DWORD dw;
	ColorChar cc;
};


// CosgvAXMFCCtrl::OnDraw - Drawing function

void CosgvAXMFCCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	if (!pdc)
		return;

	// this only needs to happen one time.  By this time, we KNOW we have an HWND
	if (!m_contentRegKey.IsEmpty() && !m_SceneName.IsEmpty())
	{
		if (!m_OSG)
		{
			// Start a Timer to watch for events that get fired
			m_eventTimer = this->SetTimer(1,50,0);

			USES_CONVERSION;
			m_OSG = new OSG_HWND(m_hWnd);
			ColorUnion cu;
			cu.dw = m_BackgroundColor;
			m_OSG->SetBackgroundColor(cu.cc.R, cu.cc.G, cu.cc.B);
			m_OSG->InitOSG(T2A(m_SceneName.GetBuffer()), T2A(m_contentRegKey.GetBuffer()));
			m_threadHandle = (HANDLE)_beginthread(&OSG_HWND::Render, 0, m_OSG);
			SetThreadPriority(m_threadHandle, THREAD_PRIORITY_TIME_CRITICAL);
		}
	}
	else
	{
		pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
		pdc->Ellipse(rcBounds);
		CRect textRect(rcBounds);
		pdc->DrawText(_T("osgvAXMFC Control"), textRect, DT_WORDBREAK | DT_CENTER);
	}	
}



// CosgvAXMFCCtrl::DoPropExchange - Persistence support

void CosgvAXMFCCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.
}



// CosgvAXMFCCtrl::OnResetState - Reset control to default state

void CosgvAXMFCCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}



// CosgvAXMFCCtrl message handlers

void CosgvAXMFCCtrl::InitOSG(LPCTSTR contentRegKey)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	m_contentRegKey = contentRegKey;
	Invalidate(); // Make sure we are drawing
}

void CosgvAXMFCCtrl::OnSceneNameChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Add your property handler code here

	SetModifiedFlag();
}

void CosgvAXMFCCtrl::OnBackgroundColorChanged(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// TODO: Add your property handler code here

	SetModifiedFlag();
}

void CosgvAXMFCCtrl::OnTimer(UINT_PTR nIDEvent)
{
	// Here is where we watch for all events that get fired back out to the control
	if (m_OSG && m_OSG->CurrFrameNum)
	{
		// OK, not so thread safe, but we are just testing anyway
		long frNum = m_OSG->CurrFrameNum;
		m_OSG->CurrFrameNum = 0;
		Frame1000(frNum);
	}

	// Base Class
	COleControl::OnTimer(nIDEvent);
}
