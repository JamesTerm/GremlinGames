// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/MainViewer.h,v 1.3 2004/08/02 16:50:03 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////

#ifndef __MainViewer_Included_
#define __MainViewer_Included_

#include "Viewer3D.h"

#include <Producer/Camera>
#include <osgProducer/OsgSceneHandler>

class CMainFrame;

class CMainViewer : public IViewer3D
{
public:
	CMainViewer();

public:	// IViewer3D
	virtual void V3D_UpdateOnce();
	virtual void V3D_ConnectHWND(HWND hWnd){m_camera->getRenderSurface()->setWindow(hWnd);}
	virtual void V3D_SetSceneNode(osg::Node* sceneNode)
	{
		m_validScene = (sceneNode != NULL);
		m_sceneHandler->getSceneView()->setSceneData(sceneNode);
	}
	virtual void V3D_OnMouseMove(int xPos, int yPos, int flags);
	virtual void V3D_OnLButtonDown(int xPos, int yPos, int flags);
	void V3D_OnMouseLeave(){m_headingChange_rad = m_pitchChange_rad = 0.0f;}

	void SetTargetGroup(osg::Node* targetGroup){m_targetGroup = targetGroup;}
	void SetLandNode(osg::Node* landNode){m_landNode = landNode;}
	void SetMainFrame(CMainFrame* mainFrame){m_mainFrame = mainFrame;}

protected:
	Producer::ref_ptr<osgProducer::OsgSceneHandler> m_sceneHandler;
	Producer::ref_ptr<Producer::Camera>				m_camera;

	osg::ref_ptr<osg::Node>							m_targetGroup;
	osg::ref_ptr<osg::Node>							m_landNode;
	CMainFrame*										m_mainFrame;

	float	m_heading_rad, m_headingChange_rad;
	float	m_pitch_rad, m_pitchChange_rad;
};


#endif	// __MainViewer_Included_
/////////////////////////////////////////////////////////////////////////////
// $Log: MainViewer.h,v $
// Revision 1.3  2004/08/02 16:50:03  pingrri
// ZEUS_Simulator - Fixed the bug that makes the scene not show up
//
// Revision 1.2  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//
// Revision 1.1  2004/05/14 18:56:09  pingrri
// Initial ZEUS_Simulator commit
//