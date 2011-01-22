// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/TargetViewer.cpp,v 1.5 2004/05/21 21:18:13 pingrri Exp $
//////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "TargetViewer.h"
#include "MainFrm.h"
#include "Utility.h"
#include <osgUtil/IntersectVisitor>

extern bool computeIntersections(float pixel_x,float pixel_y,Producer::Camera* camera,osg::Node *node,osgUtil::IntersectVisitor::HitList& hits);

const float TV_NEARCLIP = 1.0f;
const float TV_FARCLIP = 80.0f;

CTargetViewer::CTargetViewer()
{
	m_mainFrame = NULL;
	m_camera = new Producer::Camera;
	m_sceneHandler = new osgProducer::OsgSceneHandler;
	m_sceneHandler->getSceneView()->setDefaults();
	m_sceneHandler->getSceneView()->getState()->setContextID(s_contextID++);
	m_camera->setSceneHandler( m_sceneHandler.get() );

	// The targeting camera has a tighter FOV
	m_tempLensPersp = m_setLensPersp = 3.0f;
	m_camera->setLensPerspective(m_tempLensPersp, m_tempLensPersp, TV_NEARCLIP, TV_FARCLIP);
	m_camera->setLensAutoAspect(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////

void CTargetViewer::V3D_UpdateOnce()
{
	osg::PositionAttitudeTransform* targetNode = m_mainFrame->GetSelectedNode();
	if (targetNode)
	{
		osg::Vec3& cp = SceneRoot::s_cameraPosition;
		osg::Vec3 lookAt = targetNode->getPosition();
		m_camera->setViewByLookat(	cp[0], cp[1], cp[2],
									lookAt.x(), lookAt.y(), lookAt.z(),
									0, 0, 1 );
		m_camera->frame();
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////

void CTargetViewer::V3D_OnMouseMove(int xPos, int yPos, int flags)
{
	if ((flags & MK_LBUTTON) && m_dragging)
	{
		// Find out where we are from -1 to +1 horz and vert
			RECT mRect;
			GetWindowRect(&mRect);
			float width = mRect.right - mRect.left;
			float height = mRect.bottom - mRect.top;
			float dX = ((float)xPos - m_startDragX) / (width/2.0f);
			float dY = ((float)yPos - m_startDragY) / (height/2.0f);

		if (m_dragMode == Drag_Rotate)
		{
		// Create the rotations and send them to the main frame
			float heading = dX * M_PI;
			float pitch = dY * M_PI;
			osg::Quat Q = osg::Quat(pitch, osg::Vec3(1.0,0.0,0.0)) * osg::Quat(heading, osg::Vec3(0.0,0.0,1.0));
			m_mainFrame->SetSelAttitude(Q);
		}
		else if (m_dragMode == Drag_Move)
		{
			// Moves the target, just slightly up and down
			osg::Vec3 pos = m_mainFrame->GetSelectedNode()->getPosition();
			pos[2] -= dY / 100.0f;
			m_mainFrame->SetSelPosition(pos);
		}
		else if (m_dragMode == Drag_Zoom)
		{
			m_tempLensPersp = m_setLensPersp + (yPos-m_startDragY)*0.1;
			if (m_tempLensPersp > 60.0) m_tempLensPersp = 60.0;
			if (m_tempLensPersp < 0.5) m_tempLensPersp = 0.5;
			float pX, pY;
			if (width < height) {
				pX = m_tempLensPersp;
				pY = m_tempLensPersp * (height / width);
			}
			else {
				pY = m_tempLensPersp;
				pX = m_tempLensPersp * (width / height);
			}
			m_camera->setLensPerspective(pX, pY, TV_NEARCLIP, TV_FARCLIP);
		}
	}
	else if (flags & MK_RBUTTON)
	{
		// Try to move the target aim point on the target
		osg::PositionAttitudeTransform* targetNode = m_mainFrame->GetSelectedNode();

		// Find where this ray hits the targetNode
		osgUtil::IntersectVisitor::HitList hlist;
		computeIntersections(xPos, yPos, m_camera.get(), targetNode, hlist);

		if (!hlist.empty()) {
			osg::Vec3 closestPt = hlist[0].getWorldIntersectPoint();
			osg::Vec3 aimNormal = hlist[0].getWorldIntersectNormal();
			float closestDist = closestPt.length();
			for (unsigned i = 1; i < hlist.size(); ++i) {
				osg::Vec3 thisPt = hlist[i].getWorldIntersectPoint();
				float thisDist = thisPt.length();
				if (thisDist < closestDist) {
					closestDist = thisDist;
					closestPt = thisPt;
					aimNormal = hlist[i].getWorldIntersectNormal();
				}
			}

			// Set the position of the aim point
			m_mainFrame->SetAimPosition(closestPt);

			// Rotate the aim position normal to the intersect
				osg::Vec3 xVec(1.0f,0,0);
				osg::Vec3 rotVec = xVec^aimNormal;
				float angle = acos((aimNormal*xVec) / aimNormal.length());
				m_mainFrame->SetAimAttitude(osg::Quat(angle,rotVec));
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////

void CTargetViewer::V3D_OnLButtonDown(int xPos, int yPos, int flags)
{
	m_startDragX = xPos;
	m_startDragY = yPos;
	if (flags & MK_SHIFT)
		m_dragMode = Drag_Rotate;
	else if (flags & MK_CONTROL)
		m_dragMode = Drag_Move;
	else
		m_dragMode = Drag_Zoom;
}
//////////////////////////////////////////////////////////////////////////////////////////////

void CTargetViewer::V3D_OnLButtonUp(int xPos, int yPos, int flags)
{
	m_setLensPersp = m_tempLensPersp;
	m_dragMode = Drag_None;
	m_camera->setLensAutoAspect(true);
}
//////////////////////////////////////////////////////////////////////////////////////////////

// $Log: TargetViewer.cpp,v $
// Revision 1.5  2004/05/21 21:18:13  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.4  2004/05/21 17:54:59  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.3  2004/05/21 17:38:11  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.2  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//
// Revision 1.1  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//