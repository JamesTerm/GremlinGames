// GG_Framework.UI.OSG PosAttTransCallback.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

PosAttTransCallback::PosAttTransCallback(osg::PositionAttitudeTransform* sceneParent) : m_sceneParent(sceneParent)
{
	m_Position = m_sceneParent->getPosition();
	m_Attitude = m_sceneParent->getAttitude();
	m_Scale = m_sceneParent->getScale();
	m_sceneParent->addUpdateCallback(this);
}
//////////////////////////////////////////////////////////////////////////

void PosAttTransCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if (node == m_sceneParent)
	{
		m_sceneParent->setPosition(m_Position);
		m_sceneParent->setAttitude(m_Attitude);
		m_sceneParent->setScale(m_Scale);
	}
	// must call any nested node callbacks and continue subgraph traversal.
	NodeCallback::traverse(node,nv);
}
//////////////////////////////////////////////////////////////////////////