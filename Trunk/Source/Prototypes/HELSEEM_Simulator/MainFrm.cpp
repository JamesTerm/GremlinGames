// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/MainFrm.cpp,v 1.5 2004/05/24 20:21:02 pingrri Exp $
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "mainfrm.h"
#include "aboutdlg.h"
#include "SceneRoot.h"

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if(CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
		return TRUE;

	if(m_hWndClient != NULL)
		return (BOOL)::SendMessage(m_hWndClient, WM_FORWARDMSG, 0, (LPARAM)pMsg);

	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL CMainFrame::OnIdle()
{
	UIUpdateToolBar();
	return FALSE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);
		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
		SetIcon(hIconSmall, FALSE);

	// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);
	// remove old menu
	SetMenu(NULL);

	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	if (hWndToolBar) AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	CreateSimpleStatusBar();
	m_hWndClient = CreateClientWindow();

	// Create my scene and select the first item
	m_selectedNode = NULL;
	m_sceneRoot = new SceneRoot;
	osg::Group* targetGroup = m_sceneRoot->GetTargetGroup();
	unsigned numChildren = targetGroup->getNumChildren();
	osg::Group* firstGroup = targetGroup->getChild(0)->asGroup();
	numChildren = firstGroup->getNumChildren();
	this->SelectNode(firstGroup->getChild(0));

	// Set up all of the viewers
	m_mainViewer.SetMainFrame(this);
	m_mainViewer.V3D_SetSceneNode(m_sceneRoot);
	m_mainViewer.SetLandNode(m_sceneRoot->GetLandNode());
	m_mainViewer.SetTargetGroup(m_sceneRoot->GetTargetGroup());
	m_targetViewer.V3D_SetSceneNode(m_sceneRoot);
	m_targetViewer.SetMainFrame(this);
	m_PropertiesPanel.SetHelseemClient(this);
	

	UIAddToolBar(hWndToolBar);
	UISetCheck(ID_VIEW_TOOLBAR, 1);
	UISetCheck(ID_VIEW_STATUS_BAR, 1);

	// register object for message filtering and idle updates
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnFileExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	PostMessage(WM_CLOSE);
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnViewToolBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_hWndToolBar) {
		static BOOL bVisible = TRUE;	// initially visible
		bVisible = !bVisible;
		CReBarCtrl rebar = m_hWndToolBar;
		int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);	// toolbar is 2nd added band
		rebar.ShowBand(nBandIndex, bVisible);
		UISetCheck(ID_VIEW_TOOLBAR, bVisible);
		UpdateLayout();
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnViewStatusBar(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (m_hWndStatusBar) {
		BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
		::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
		UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
		UpdateLayout();
	}
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnAppAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	CAboutDlg dlg;
	dlg.DoModal();
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::SelectNode(osg::Node* selNode)
{
	osg::PositionAttitudeTransform* firstPosAtTrans = NULL;
	const char* nodeName = selNode->getName().c_str();
	while (selNode && !firstPosAtTrans)
	{
		firstPosAtTrans = dynamic_cast<osg::PositionAttitudeTransform*>(selNode);
		if (firstPosAtTrans)
		{
			// Check for the right one
			const char* nodeName = firstPosAtTrans->getName().c_str();
			if (!nodeName || (nodeName[0] != '#'))
				firstPosAtTrans = NULL;
		}
		selNode = selNode->getNumParents() ? selNode->getParent(0) : NULL;
		if (!nodeName || (nodeName[0] != '~'))
			nodeName = selNode->getName().c_str();
	}

	if (firstPosAtTrans && (m_selectedNode != firstPosAtTrans)) {
		m_selectedNode = firstPosAtTrans;
		SetSelPosition(firstPosAtTrans->getPosition());
		SetSelAttitude(firstPosAtTrans->getAttitude());
		m_PropertiesPanel.SetSelectionName(nodeName+1);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::SetSelPosition(const osg::Vec3& pos)
{
	m_selectedNode->setPosition(pos);
	m_sceneRoot->GetSelTargetFlag()->setPosition(pos);
	SetAimPosition(pos);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::SetSelAttitude(const osg::Quat& quat)
{
	m_selectedNode->setAttitude(quat);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::SetAimPosition(const osg::Vec3& pos)
{
	m_sceneRoot->GetSelTargetTrans()->setPosition(pos);
	m_PropertiesPanel.SetSelectionDist(pos.length());
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::SetAimAttitude(const osg::Quat& quat)
{
	m_sceneRoot->GetSelTargetTrans()->setAttitude(quat);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

HWND CMainFrame::CreateClientWindow()
{
	// Get the Client RECT for my entire window as a starting size
		CRect rcClient;
		GetClientRect(&rcClient);

	// On the right side, create another splitter
		m_splitterHOR.Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_splitterHOR.m_cxyMin = 70; // minimum size
		m_splitterHOR.SetSplitterPos((rcClient.Height() * 2) / 3); // from top

	// On the bottom, make another splitter window
		m_splitterVERT.Create(m_splitterHOR.m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_splitterVERT.m_cxyMin = 50; // minimum size
		m_splitterVERT.SetSplitterPos((rcClient.Width()) / 2); // from left
		m_splitterHOR.SetSplitterPane(SPLIT_PANE_BOTTOM, m_splitterVERT.m_hWnd, false);

	// On the top, make a pane for the OSG main window temp
		m_mainViewer.Create(m_splitterHOR.m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_splitterHOR.SetSplitterPane(SPLIT_PANE_TOP, m_mainViewer.m_hWnd, false);

	// On the right of that, make a pane for the OSG target window
		m_targetViewer.Create(m_splitterVERT.m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_splitterVERT.SetSplitterPane(SPLIT_PANE_RIGHT, m_targetViewer.m_hWnd, false);

	// Place the Left Side Properties Panel
		m_PropertiesPanel.Create(m_splitterVERT.m_hWnd, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		m_splitterVERT.SetSplitterPane(SPLIT_PANE_LEFT, m_PropertiesPanel.m_hWnd, false);
		m_PropertiesPanel.ShowWindow(WM_SHOWWINDOW);

	// The first splitter window is ultimately my client window
		return m_splitterHOR.m_hWnd;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

void CMainFrame::RunHelseem()
{
	// Collect the data
		// Laser position
			osg::Vec3& laserPos = SceneRoot::s_cameraPosition;

		// Laser Orientation
			// Get the vector to the aim spot (include the range)
				osg::Vec3 targetV = m_sceneRoot->GetSelTargetTrans()->getPosition() - laserPos;
				float dist = targetV.length();

			// Find the QUAT for the rotation from (1,0,0)?
				osg::Vec3 xVec(1.0f,0,0);
				osg::Vec3 rotVec = xVec^targetV;
				float angle = acos((targetV*xVec) / targetV.length());
				osg::Quat Q(angle,rotVec);

			// Convert the QUAT to a matrix
				osg::Matrixd laserMat;
				Q.get(laserMat);

		// Laser Parameters
			float power = m_PropertiesPanel.GetLaserPower();
			float aper = m_PropertiesPanel.GetBeamAperture();

		// Target NSM name
			std::string nsmName = m_selectedNode->getChild(0)->getName();
			nsmName += ".nsm";

		// Target position
			osg::Vec3 targetPos = m_selectedNode->getPosition();

		// Target orientation
			osg::Matrixd targetMat;
			m_selectedNode->getAttitude().get(targetMat);

	// Create a sim file that defines these things
		const double* LM = laserMat.ptr();
		const double* TM = targetMat.ptr();
		FILE* simFile = fopen("ZEUS_Temp.sim", "w");
		fprintf(simFile, "// This File is generated automatically by ZEUS_Simulator.exe\n");
		fprintf(simFile, "/////////////////////////////////////////////////////////////// \n\n");

		fprintf(simFile, "#define ZEUS_LOC %f %f %f\n", laserPos[0], laserPos[1], laserPos[2]);
		fprintf(simFile, "#define TARGET_LOC %f %f %f\n", targetPos[0], targetPos[1], targetPos[2]);
		fprintf(simFile, "#define ZEUS_ROT %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", 
			LM[0], LM[1], LM[2], LM[4], LM[5], LM[6], LM[8], LM[9], LM[10]);
		fprintf(simFile, "#define FOCAL_LEN %f\n", dist);
		fprintf(simFile, "#define TARGET_ROT %lf %lf %lf %lf %lf %lf %lf %lf %lf\n",
			TM[0], TM[1], TM[2], TM[4], TM[5], TM[6], TM[8], TM[9], TM[10]);
		fprintf(simFile, "#define TARGET_FILE %s\n", (nsmName.c_str()+1));
		fprintf(simFile, "#define HEL_POWER %f\n", power);
		fprintf(simFile, "#define APERTURE %f\n", aper);
		fprintf(simFile, "#define HEATING_ON\n");

		fprintf(simFile, "\n#include \"00main.sim\"\n");
		fclose(simFile);

	// Find the exe path and run it
		char cmd[512];
		GetModuleFileName(NULL, cmd, 512);
		char* lastSlash = strrchr(cmd, '\\');
		if (lastSlash) *(lastSlash+1) = 0;
		strcat(cmd, "runJMPS ZEUS_Temp.sim");
		system(cmd);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: MainFrm.cpp,v $
// Revision 1.5  2004/05/24 20:21:02  pingrri
// Added Win32 CreateDirectory to Util/BuildDirs
// Added #define HEATING_ON to ZEUS temp sim file
// Added scale functionality to the #D viewer component and app
//
// Revision 1.4  2004/05/21 17:38:11  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.3  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//
// Revision 1.2  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//