// AddRemoveChildCallback.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

AddRemoveChildCallback::AddRemoveChildCallback(osg::Group* sceneParent) : m_sceneParent(sceneParent)
{
	m_updateNeeded = false;
	m_sceneParent->addUpdateCallback(this);
}
//////////////////////////////////////////////////////////////////////////

void AddRemoveChildCallback::AddRemoveChild(osg::Node* child, bool adding, osg::Group* parent)
{
	m_mutex.lock();
	m_nodesToAddRemove.push_back(AddRemoveNode(child, adding, parent));
	m_updateNeeded = true;
	m_mutex.unlock();
}
//////////////////////////////////////////////////////////////////////////

void AddRemoveChildCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
	if ((node == m_sceneParent) && m_updateNeeded)
	{
		m_mutex.lock();
		unsigned numNodes = m_nodesToAddRemove.size();
		for (unsigned i = 0; i < numNodes; ++i)
		{
			osg::Group* parent = m_nodesToAddRemove[i].GetParentNode();
			if (!parent) parent = m_sceneParent;
			if (m_nodesToAddRemove[i].IsAdding())
				parent->addChild(m_nodesToAddRemove[i].GetChildNode());
			else
				parent->removeChild(m_nodesToAddRemove[i].GetChildNode());
			parent->dirtyBound();
		}
		m_updateNeeded = false;
		m_nodesToAddRemove.clear();
		m_mutex.unlock();
	}
	// must call any nested node callbacks and continue subgraph traversal.
	NodeCallback::traverse(node,nv);
}
//////////////////////////////////////////////////////////////////////////