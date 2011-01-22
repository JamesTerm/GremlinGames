// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/Viewer3D.h,v 1.5 2004/07/14 21:48:23 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////
#ifndef __Viewer3D_Included_
#define __Viewer3D_Included_

#include <osg/Node>

/** This Base Class allows you to hook up any osg camera type of configuration
 *	to a WTL style view window.  It hooks up the HWND and maintains the timer necessary to update 
 *	the view window */
class IViewer3D : public CWindowImpl<IViewer3D>
{
public:	// Override these functions

	virtual void V3D_UpdateOnce() = 0;
	virtual void V3D_ConnectHWND(HWND hWnd) = 0;
	virtual void V3D_SetSceneNode(osg::Node* sceneNode) = 0;


	virtual void V3D_OnMouseMove(int xPos, int yPos, int flags){}
	virtual void V3D_OnLButtonDown(int xPos, int yPos, int flags){}
	virtual void V3D_OnLButtonUp(int xPos, int yPos, int flags){}
	virtual void V3D_OnMouseLeave(){}

protected:
	static unsigned s_contextID;
	bool			m_validScene;

public:	// All of the Internal Windowing stuff
	DECLARE_WND_CLASS(NULL)
	BOOL PreTranslateMessage(MSG* pMsg){return FALSE;}

	IViewer3D()
	{
		m_tracking = false;
		m_dragging = false;
		m_validScene = false;
	}

	BEGIN_MSG_MAP(IViewer3D)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
	END_MSG_MAP()

	enum {V3D_TimerID = 0, V3D_TimerDur = 10 };
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		V3D_ConnectHWND(m_hWnd);
		::SetTimer(m_hWnd, V3D_TimerID, V3D_TimerDur, NULL);
		return 0;
	}

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		::KillTimer(m_hWnd, V3D_TimerID);
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_validScene && (wParam == V3D_TimerID))
			V3D_UpdateOnce();
		return 0;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_tracking = m_dragging;
		V3D_OnMouseLeave();
		return 0;
	}
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (!m_tracking) {
			TRACKMOUSEEVENT tmm;
			tmm.cbSize = sizeof(tmm);
			tmm.dwFlags = TME_LEAVE;
			tmm.dwHoverTime = HOVER_DEFAULT;
			tmm.hwndTrack = m_hWnd;
			_TrackMouseEvent(&tmm);
			m_tracking = true;
		}
		V3D_OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		return 0;
	}
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		SetCapture();
		m_dragging = true;
		V3D_OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		return 0;
	}
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		V3D_OnLButtonUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), wParam);
		m_dragging = false;
		ReleaseCapture();
		return 0;
	}
	bool m_tracking;
	bool m_dragging;
};


#include <osgProducer/Viewer>
class TestViewer : public IViewer3D, public osgProducer::Viewer
{
public:
	TestViewer()
	{
		setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
	}

	virtual void V3D_UpdateOnce()
	{
		sync();
		update();
		frame();
	}

	virtual void V3D_ConnectHWND(HWND hWnd)
	{
		getCamera(0)->getRenderSurface()->setWindow(hWnd);
		realize(Producer::CameraGroup::SingleThreaded);
	}
	virtual void V3D_SetSceneNode(osg::Node* sceneNode)
	{
		m_validScene = (sceneNode != NULL);
		setSceneData(sceneNode);
	}

};


#endif __Viewer3D_Included_
/////////////////////////////////////////////////////////////////////////////
// $Log: Viewer3D.h,v $
// Revision 1.5  2004/07/14 21:48:23  pingrri
// ScenarioPlanner - Basic Structure
//
// Revision 1.4  2004/05/21 21:18:13  pingrri
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