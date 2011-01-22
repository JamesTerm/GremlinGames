// GG_Framework.UI.OSG PickVisitor.cpp
#include "stdafx.h"
#include "GG_Framework.UI.OSG.h"

using namespace GG_Framework::UI::OSG;

PickVisitor::PickVisitor()
{ 
	xp=yp=0;    
	setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
}
//////////////////////////////////////////////////////////////////////////

void PickVisitor::setTraversalMask(osg::Node::NodeMask traversalMask)
{
	NodeVisitor::setTraversalMask(traversalMask);
	_piv.setTraversalMask(traversalMask);   
}
//////////////////////////////////////////////////////////////////////////

void PickVisitor::apply(osg::Projection& pr)
{ // stack the intersect rays, transform to new projection, traverse
	// Assumes that the Projection is an absolute projection
	osg::Matrixd mt;
	mt.invert(pr.getMatrix());
	osg::Vec3 npt=osg::Vec3(xp,yp,-1.0f) * mt, farpt=osg::Vec3(xp,yp,1.0f) * mt;

	// traversing the nodes children, using the projection direction
	for (unsigned int i=0; i<pr.getNumChildren(); i++) 
	{
		osg::Node *nodech=pr.getChild(i);
		osgUtil::IntersectVisitor::HitList &hli=_piv.getIntersections(nodech,npt, farpt);
		for(osgUtil::IntersectVisitor::HitList::iterator hitr=hli.begin();
			hitr!=hli.end();
			++hitr)
		{ // add the projection hits to the scene hits.
			// This is why _lineSegment is retained as a member of PickIntersectVisitor
			_PIVsegHitList.push_back(*hitr);
		}
		traverse(*nodech);
	}
}
//////////////////////////////////////////////////////////////////////////

osgUtil::IntersectVisitor::HitList& PickVisitor::getHits(
	osg::Node *node, const osg::Vec3& near_point, const osg::Vec3& far_point)
{
	// High level get intersection with sceneview using a ray from x,y on the screen
	// sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels

	// first get the standard hits in un-projected nodes
	_PIVsegHitList=_piv.getIntersections(node,near_point,far_point); // fill hitlist

	// then get hits in projection nodes
	// traverse(*node); // check for projection nodes
	return _PIVsegHitList;
}
//////////////////////////////////////////////////////////////////////////

osgUtil::IntersectVisitor::HitList& PickVisitor::getHits(
	osg::Node *node, const osg::Matrixd &projm, const float x, const float y)
{ 
	// utility for non=sceneview viewers
	// x,y are values returned by 
	osg::Matrixd inverseMVPW;
	inverseMVPW.invert(projm);
	osg::Vec3 near_point = osg::Vec3(x,y,-1.0f)*inverseMVPW;
	osg::Vec3 far_point = osg::Vec3(x,y,1.0f)*inverseMVPW;
	setxy(x,y);    
	getHits(node,near_point,far_point);
	return _PIVsegHitList;
}
//////////////////////////////////////////////////////////////////////////

osgUtil::IntersectVisitor::HitList& LineSegmentIntersectVisitor::getIntersections(
	osg::Node *scene, osg::Vec3 nr, osg::Vec3 fr)
{ 
	// option for non-sceneView users: you need to get the screen perp line and call getIntersections
	// if you are using Projection nodes you should also call setxy to define the xp,yp positions for use with
	// the ray transformed by Projection
	_lineSegment = new osg::LineSegment;
	_lineSegment->set(nr,fr); // make a line segment

	//std::cout<<"near "<<nr<<std::endl;
	//std::cout<<"far "<<fr<<std::endl;

	addLineSegment(_lineSegment.get());

	scene->accept(*this);
	return getHitList(_lineSegment.get());
}
//////////////////////////////////////////////////////////////////////////