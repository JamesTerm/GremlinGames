// GG_Framework.UI ViewportReference.cpp
#include "stdafx.h"
#include "GG_Framework.UI.h"

namespace GG_Framework
{
	namespace UI
	{
unsigned ViewPortReference::s_unnamedCounter = 0;

// Construct with the centerNode already in the graph
ViewPortReference::ViewPortReference
(osg::Node& centerNode, ViewPortReference* parent, unsigned parentIndex) :
m_node(&centerNode), m_parent(parent), m_parentIndex(parentIndex), m_actor((Actor*)-1)
{
	// If there is not a parent, the parent index SHOULD be -1
	if (!m_parent)
		m_parentIndex = (unsigned)-1;

	GG_Framework::UI::OSG::FillNodePath(m_nodePath, m_node.get());
	m_haveSetMatrix = false;
	R.makeIdentity();
	T.makeIdentity();

	m_minDist = m_node->getBound()._radius * 0.5f;
	m_dist = m_initDist = m_minDist * 6.0f;
	m_maxDist = 3e8f;

	if (m_node->getName() == "")
	{
		char newName[64];
		sprintf(newName, "%s_%i", m_node->className(), ++s_unnamedCounter);
		m_node->setName(newName);
	}

	osg::Group* group = m_node->asGroup();
	if (group)
	{
		unsigned numChidren = group->getNumChildren();
		for (unsigned i = 0; i < numChidren; ++i)
			m_children.push_back(new ViewPortReference(*(group->getChild(i)), this, i));
	}
}
////////////////////////////////////////////////////////////////////////////////////////

ViewPortReference* ViewPortReference::AddChild(osg::Node& centerNode)
{
	ViewPortReference* ret = new ViewPortReference(centerNode, this, m_children.size());
	m_minDist = m_node->getBound()._radius * 0.5f;
	m_dist = m_initDist = m_minDist * 6.0f;
	m_children.push_back(ret);
	return ret;
}
////////////////////////////////////////////////////////////////////////////////////////

ViewPortReference::~ViewPortReference()
{
	unsigned numChildren = m_children.size();
	for (unsigned i = 0; i < numChildren; ++i)
		delete m_children[i];
	m_children.clear();
}
////////////////////////////////////////////////////////////////////////////////////////

void ViewPortReference::StoreTrackBall(GG_Framework::UI::OSG::TrackBall& tb)
{
	tb.GetCurrent(R, T, m_dist);
	m_haveSetMatrix = true;
}
////////////////////////////////////////////////////////////////////////////////////////

void ViewPortReference::UpdateTrackBall(GG_Framework::UI::OSG::TrackBall& tb, bool keepOldRot)
{
	if (!keepOldRot)
	{
		if (m_haveSetMatrix)	// Remember the last saved position
			tb.SetReference(R, T, m_dist);
		else
		{	// Set the default position the first time
			tb.reset();
			tb.setDistance(m_initDist, false);
			tb.setReference();
		}
	}

	// Set the minimum & maximum distance
	tb.setMinimumDistance(m_minDist);
	tb.setMaximumDistance(m_maxDist);
}
////////////////////////////////////////////////////////////////////////////////////////

osg::Matrix ViewPortReference::GetInverseTransform() const 
{
	return osg::computeLocalToWorld(m_nodePath);
}
////////////////////////////////////////////////////////////////////////////////////////
#pragma warning ( disable : 4311 )	// So it does not complain about the pointer truncation
Actor* ViewPortReference::GetActor()
{
	if ((int)m_actor == -1)
	{
		// Just find it the first time
		m_actor = NULL;
		osg::Node* node = m_node.get();
		while (!m_actor && node)
		{
			m_actor = dynamic_cast<Actor*>(node);
			node = node->getNumParents() ? node->getParent(0) : NULL;
		}
	}
	return m_actor;
}
////////////////////////////////////////////////////////////////////////////////////////
	}
}