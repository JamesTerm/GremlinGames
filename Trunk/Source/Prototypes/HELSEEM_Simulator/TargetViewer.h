// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/TargetViewer.h,v 1.4 2004/08/02 16:50:03 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////
#ifndef __TargetViewer_Included_
#define __TargetViewer_Included_

#include "Viewer3D.h"

#include <Producer/Camera>
#include <osgProducer/OsgSceneHandler>
#include <osg/PositionAttitudeTransform>

class CMainFrame;
class CTargetViewer : public IViewer3D
{
public:
	CTargetViewer();
	virtual void V3D_UpdateOnce();
	virtual void V3D_OnMouseMove(int xPos, int yPos, int flags);
	virtual void V3D_OnLButtonDown(int xPos, int yPos, int flags);
	virtual void V3D_OnLButtonUp(int xPos, int yPos, int flags);

	virtual void V3D_ConnectHWND(HWND hWnd){m_camera->getRenderSurface()->setWindow(hWnd);}
	virtual void V3D_SetSceneNode(osg::Node* sceneNode)
	{
		m_validScene = (sceneNode != NULL);
		m_sceneHandler->getSceneView()->setSceneData(sceneNode);
	}

	void SetMainFrame(CMainFrame* mainFrame){m_mainFrame = mainFrame;}

protected:
	Producer::ref_ptr<osgProducer::OsgSceneHandler> m_sceneHandler;
	Producer::ref_ptr<Producer::Camera>				m_camera;
	CMainFrame*										m_mainFrame;

	// Used for zooming in and out
	float m_setLensPersp, m_tempLensPersp;
	int   m_startDragX, m_startDragY, m_dragMode;

	enum {
		Drag_None,
		Drag_Rotate,
		Drag_Move,
		Drag_Zoom
	};
};

#endif	// __TargetViewer_Included_
/////////////////////////////////////////////////////////////////////////////
// $Log: TargetViewer.h,v $
// Revision 1.4  2004/08/02 16:50:03  pingrri
// ZEUS_Simulator - Fixed the bug that makes the scene not show up
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