#include "..\scenarioplanner\aboutdlg.h"
// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/AboutDlg.cpp,v 1.2 2004/07/14 21:48:23 pingrri Exp $
// aboutdlg.cpp : implementation of the CAboutDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>

#include "aboutdlg.h"
#include "Utility.h"

LRESULT CAboutDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CenterWindow(GetParent());

	// Set the text in the Caption
	HWND buildLabel = GetDlgItem(IDC_HELPABOUT_EDIT);
	char helpAboutString[512];

	char productName[64];
	GetProductInfo("ProductName", productName, 64);
	char comments[64];
	GetProductInfo("Comments", comments, 64);
	char productVersion[64];
	GetProductInfo("ProductVersion", productVersion, 64);
	char legalCopyright[64];
	GetProductInfo("LegalCopyright", legalCopyright, 64);
	char companyName[64];
	GetProductInfo("CompanyName", companyName, 64);

	sprintf(helpAboutString, "%s\r\n%s\r\nVersion: %s\r\n\r\n%s\r\n%s\r\n",
				productName, comments, productVersion, legalCopyright, companyName);

	::SetWindowText(buildLabel, helpAboutString);
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CAboutDlg::OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	EndDialog(wID);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: AboutDlg.cpp,v $
// Revision 1.2  2004/07/14 21:48:23  pingrri
// ScenarioPlanner - Basic Structure
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//
