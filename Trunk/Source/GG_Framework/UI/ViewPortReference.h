// GG_Framework.UI ViewportReference.h
#pragma once

namespace GG_Framework
{
	namespace UI
	{

class FRAMEWORK_UI_API ViewPortReference
{
public:
	// Construct with the centerNode already in the graph
	ViewPortReference(osg::Node& centerNode, ViewPortReference* parent, unsigned parentIndex);
	~ViewPortReference();

	osg::Node* GetNode() {return m_node.get();}

	// Grab the point to center on
	osg::Matrix GetInverseTransform() const;

	// Remember the trackball position
	void StoreTrackBall(GG_Framework::UI::OSG::TrackBall& tb);

	// Update the trackball position
	void UpdateTrackBall(GG_Framework::UI::OSG::TrackBall& tb, bool keepOldRot);

	ViewPortReference* GetParent(){return m_parent;}
	unsigned GetNumChildren(){return m_children.size();}
	ViewPortReference* GetChild(unsigned i){return (i<m_children.size())?m_children[i]:NULL;}
	unsigned GetParentIndex(){return m_parentIndex;}

	// Find the Actor this is a sub-tree of
	Actor* GetActor();

	// Add a child dynamically
	ViewPortReference* AddChild(osg::Node& centerNode);

protected:
	// The initial distance from the object relative to its size
	float	m_initDist;
	float	m_minDist;
	float	m_maxDist;

	// The m_node we are interested in
	osg::ref_ptr<osg::Node> m_node;

	ViewPortReference* m_parent;
	unsigned m_parentIndex;
	std::vector<ViewPortReference*> m_children;

	// The completed path to the m_node through its parents
	osg::NodePath m_nodePath;

	Producer::Matrix R, T; double m_dist;
	bool m_haveSetMatrix;

	Actor* m_actor;

	static unsigned s_unnamedCounter;	//!< Thread safe?  Only used for name
};
	}
}