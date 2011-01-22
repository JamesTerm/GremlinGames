// osgvAXMFCPropPage.cpp : Implementation of the CosgvAXMFCPropPage property page class.

#include "stdafx.h"
#include "osgvAXMFC.h"
#include "osgvAXMFCPropPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CosgvAXMFCPropPage, COlePropertyPage)



// Message map

BEGIN_MESSAGE_MAP(CosgvAXMFCPropPage, COlePropertyPage)
END_MESSAGE_MAP()



// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CosgvAXMFCPropPage, "OSGVAXMFC.osgvAXMFCPropPage.1",
	0x24c63ac8, 0xe566, 0x41b8, 0xae, 0x71, 0xc9, 0xb3, 0x43, 0xe6, 0xa6, 0x9d)



// CosgvAXMFCPropPage::CosgvAXMFCPropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CosgvAXMFCPropPage

BOOL CosgvAXMFCPropPage::CosgvAXMFCPropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_OSGVAXMFC_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}



// CosgvAXMFCPropPage::CosgvAXMFCPropPage - Constructor

CosgvAXMFCPropPage::CosgvAXMFCPropPage() :
	COlePropertyPage(IDD, IDS_OSGVAXMFC_PPG_CAPTION)
{
}



// CosgvAXMFCPropPage::DoDataExchange - Moves data between page and properties

void CosgvAXMFCPropPage::DoDataExchange(CDataExchange* pDX)
{
	DDP_PostProcessing(pDX);
}



// CosgvAXMFCPropPage message handlers
