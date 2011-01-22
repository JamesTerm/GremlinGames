#pragma once

// osgvAXMFCPropPage.h : Declaration of the CosgvAXMFCPropPage property page class.


// CosgvAXMFCPropPage : See osgvAXMFCPropPage.cpp for implementation.

class CosgvAXMFCPropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CosgvAXMFCPropPage)
	DECLARE_OLECREATE_EX(CosgvAXMFCPropPage)

// Constructor
public:
	CosgvAXMFCPropPage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_OSGVAXMFC };

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	DECLARE_MESSAGE_MAP()
};

