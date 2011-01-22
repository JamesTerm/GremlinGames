// PickVisitor.h
#pragma once
#include <osg/NodeVisitor>
#include <osgUtil/IntersectVisitor>

namespace GG_Framework
{
	namespace UI
	{
		namespace OSG
		{
			class FRAMEWORK_UI_OSG_API LineSegmentIntersectVisitor : public osgUtil::IntersectVisitor
			{
			public:
				LineSegmentIntersectVisitor(){}
				virtual ~LineSegmentIntersectVisitor() {}

				HitList& getIntersections(osg::Node *scene, osg::Vec3 nr, osg::Vec3 fr);
			private:
				osg::ref_ptr<osg::LineSegment> _lineSegment;
				friend class osgUtil::IntersectVisitor;
			};

			/////////////////////////////////////////////////////////////////////////////////////////////////
			class FRAMEWORK_UI_OSG_API PickVisitor : public osg::NodeVisitor
			{
			public:
				PickVisitor();
				~PickVisitor() {}

				void setTraversalMask(osg::Node::NodeMask traversalMask);

				// Aug 2003 added to pass the nodemaskOverride to the PickIntersectVisitor
				//   may be used make the visitor override the nodemask to visit invisible actions
				inline void setNodeMaskOverride(osg::Node::NodeMask mask) {
					_piv.setNodeMaskOverride(mask);
					_nodeMaskOverride = mask; }


				virtual void apply(osg::Projection& pr);
				osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Vec3& near_point, const osg::Vec3& far_point);
				osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Matrixd &projm, const float x, const float y);

				inline void setxy(float xpt, float ypt) { xp=xpt; yp=ypt; }
				inline bool hits() const { return _PIVsegHitList.size()>0;}

			private:

				LineSegmentIntersectVisitor _piv;
				float xp, yp; // start point in viewport fraction coordinates
				osgUtil::IntersectVisitor::HitList       _PIVsegHitList;
			};
		}
	}
}