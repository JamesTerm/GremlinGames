#pragma once
#include "OSG_HWND.h"

// osgvAXMFCCtrl.h : Declaration of the CosgvAXMFCCtrl ActiveX Control class.


// CosgvAXMFCCtrl : See osgvAXMFCCtrl.cpp for implementation.

class CosgvAXMFCCtrl : public COleControl
{
	DECLARE_DYNCREATE(CosgvAXMFCCtrl)

private:
	OSG_HWND* m_OSG;
	HANDLE m_threadHandle;
	CString m_contentRegKey;
	UINT_PTR m_eventTimer;

// Constructor
public:
	CosgvAXMFCCtrl();

// Overrides
public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	afx_msg void OnTimer(UINT nIDEvent);

// Implementation
protected:
	~CosgvAXMFCCtrl();

	BEGIN_OLEFACTORY(CosgvAXMFCCtrl)        // Class factory and guid
		virtual BOOL VerifyUserLicense();
		virtual BOOL GetLicenseKey(DWORD, BSTR FAR*);
	END_OLEFACTORY(CosgvAXMFCCtrl)

	DECLARE_OLETYPELIB(CosgvAXMFCCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CosgvAXMFCCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CosgvAXMFCCtrl)		// Type name and misc status

// Message maps
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	DECLARE_DISPATCH_MAP()

// Event maps
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
		eventidFrame1000 = 1L,
		dispidBackgroundColor = 3,
		dispidSceneName = 2,
		dispidInitOSG = 1L
	};
protected:
	void InitOSG(LPCTSTR contentRegKey);
	void OnSceneNameChanged(void);
	CString m_SceneName;
	void OnBackgroundColorChanged(void);
	OLE_COLOR m_BackgroundColor;

	void Frame1000(LONG frameNum)
	{
		FireEvent(eventidFrame1000, EVENT_PARAM(VTS_I4), frameNum);
	}
};

