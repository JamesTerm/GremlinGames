// GG_Framework.UI CenteredTrackball_CamManipulator.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

using namespace GG_Framework::UI;

CenteredTrackball_CamManipulator::CenteredTrackball_CamManipulator() : _mx(-1.0), _my(-1.0), _button_state(0)
{
}

void CenteredTrackball_CamManipulator::OnMouseMove(float mx, float my)
{
	_mx = mx;
	_my = my;
}
////////////////////////////////////////////////////////////////////////////////////////////

void CenteredTrackball_CamManipulator::KeyPress(GG_Framework::Base::Key key, bool onoff)
{
	if (	(key.key == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) ||
			(key.key == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) ||
			(key.key == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON))
	{
		if (onoff)
			_button_state |= key.key;
		else
			_button_state &= ~key.key;
	}
}
//////////////////////////////////////////////////////////////////////////

void CenteredTrackball_CamManipulator::UpdateCamera(GG_Framework::UI::OSG::ICamera* activeCamera, double dTime_s)
{
	ASSERT(activeCamera);

	// Get the matrix from just the trackball
	input(_mx, _my, _button_state, dTime_s, true);
	osg::Matrix trackball(getMatrix());

	// Find the position of my center node
	osg::Matrix cam_center;
	if (m_node.valid())
	{
		// Perhaps do not need to do this each frame, but if it changes, it is cleaner to do it here
		cam_center = GG_Framework::UI::OSG::GetNodeMatrix(m_node.get(), NULL, NULL);
	}
	osg::Matrix camMatrix =	osg::Matrix::translate(-cam_center(3,0), -cam_center(3,1), -cam_center(3,2)) *
		trackball;

	// Let the real camera finish up
	activeCamera->SetMatrix(camMatrix);
}
//////////////////////////////////////////////////////////////////////////

void CenteredTrackball_CamManipulator::SetCenteredNode(osg::Node* node)
{
	m_node = node;
	if (node)
	{
		float minDist = m_node->getBound()._radius * 0.5f;
		this->ResetFixed();
		this->setMinimumDistance(minDist);
		this->setDistance(minDist * 6.0f);
		this->setMaximumDistance(3e8f);
		this->setReference();
	}
}
//////////////////////////////////////////////////////////////////////////