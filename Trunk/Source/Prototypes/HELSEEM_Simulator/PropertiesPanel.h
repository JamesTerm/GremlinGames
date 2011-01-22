// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/PropertiesPanel.h,v 1.3 2004/05/20 21:30:06 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////

#ifndef AFX_PROPERTIESPANEL_H__
#define AFX_PROPERTIESPANEL_H__

#ifdef ZEUS_SIM
#include "ZEUS_resource.h"
#else
#include "ATL_resource.h"
#endif

#include "stdafx.h"

class IHelseemClient
{
public:
	virtual void RunHelseem()=0;
};
// ###############################################################################

class CPropertiesPanel : public CDialogImpl<CPropertiesPanel>
{
public:
	void SetSelectionName(const char* name)
	{
		::SetWindowText(GetDlgItem(IDC_EDIT_Type), name);
	}

	void SetSelectionDist(float dist)
	{
		char buff[32];
		sprintf(buff, "%0.1f", dist);
		::SetWindowText(GetDlgItem(IDC_EDIT_Range), buff);
	}

	void SetHelseemClient(IHelseemClient* hc){m_helseemClient = hc;}

	float GetLaserPower()
	{
		char buff[32];
		::GetWindowText(GetDlgItem(IDC_EDIT_Power), buff, 32);
		return (float)atof(buff);
	}
	float GetBeamAperture()
	{
		char buff[32];
		::GetWindowText(GetDlgItem(IDC_EDIT_Aperture), buff, 32);
		return (float)atof(buff);
	}

public:		// SDK and Callbacks

	//! Need to know the Dlg Box resource
		enum { IDD = IDD_PROPS_dlg };

	// Message Map
	   BEGIN_MSG_MAP(CPropertiesPanel)
		  MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		  COMMAND_HANDLER(IDC_BTN_RunHelseem, BN_CLICKED, OnBnClickedBtnRunhelseem)
		  REFLECT_NOTIFICATIONS()
	   END_MSG_MAP()


   // Message Handlers
	LRESULT OnBnClickedBtnRunhelseem(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
		if (m_helseemClient)
			m_helseemClient->RunHelseem();
		return 0;
	};
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/){
		::SetWindowText(GetDlgItem(IDC_EDIT_Power), "10000");
		::SetWindowText(GetDlgItem(IDC_EDIT_Aperture), "0.3");
		::SetWindowText(GetDlgItem(IDC_EDIT_BeamQuality), "1.1");
		::SetWindowText(GetDlgItem(IDC_EDIT_Efficiency), "0.10");
		::SetWindowText(GetDlgItem(IDC_EDIT_Range), "200");
		::SetWindowText(GetDlgItem(IDC_EDIT_Type), "BLU-97");
		::SetWindowText(GetDlgItem(IDC_EDIT_Temp), "400");
		m_helseemClient = NULL;
		return 0;
	};

	IHelseemClient* m_helseemClient;
};

#endif // AFX_PROPERTIESPANEL_H__
/////////////////////////////////////////////////////////////////////////////
// $Log: PropertiesPanel.h,v $
// Revision 1.3  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//
// Revision 1.2  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//
