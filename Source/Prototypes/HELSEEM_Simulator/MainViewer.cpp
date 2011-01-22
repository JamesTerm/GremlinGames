// $Header: /home/cvs/Projects/HELSEEM/src/HELSEEM/HELSEEM_Simulator/MainViewer.cpp,v 1.3 2004/05/21 21:18:13 pingrri Exp $
/////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MainViewer.h"
#include "MainFrm.h"
#include "Utility.h"
#include "SceneRoot.h"

#include <osgUtil/IntersectVisitor>
#include <osg/Projection>
#include <osg/PositionAttitudeTransform>

unsigned IViewer3D::s_contextID = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////

class PickIntersectVisitor : public osgUtil::IntersectVisitor
{
public:
    PickIntersectVisitor()
    { 
    }
    virtual ~PickIntersectVisitor() {}
    
    HitList& getIntersections(osg::Node *scene, osg::Vec3 nr, osg::Vec3 fr)
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
private:
    osg::ref_ptr<osg::LineSegment> _lineSegment;
    friend class osgUtil::IntersectVisitor;
};
/////////////////////////////////////////////////////////////////////////////////////////////////

// PickVisitor traverses whole scene and checks below all Projection nodes
class PickVisitor : public osg::NodeVisitor
{
public:
    PickVisitor()
    { 
        xp=yp=0;    
        setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    }
    ~PickVisitor() {}

    void setTraversalMask(osg::Node::NodeMask traversalMask)
    {
        NodeVisitor::setTraversalMask(traversalMask);
        _piv.setTraversalMask(traversalMask);   
    }

	// Aug 2003 added to pass the nodemaskOverride to the PickIntersectVisitor
     //   may be used make the visitor override the nodemask to visit invisible actions
    inline void setNodeMaskOverride(osg::Node::NodeMask mask) {
		_piv.setNodeMaskOverride(mask);
		_nodeMaskOverride = mask; }


    virtual void apply(osg::Projection& pr)
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

    osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Vec3& near_point, const osg::Vec3& far_point)
    {
        // High level get intersection with sceneview using a ray from x,y on the screen
        // sets xp,yp as pixels scaled to a mapping of (-1,1, -1,1); needed for Projection from x,y pixels

        // first get the standard hits in un-projected nodes
        _PIVsegHitList=_piv.getIntersections(node,near_point,far_point); // fill hitlist

        // then get hits in projection nodes
        traverse(*node); // check for projection nodes
        return _PIVsegHitList;
    }

    osgUtil::IntersectVisitor::HitList& getHits(osg::Node *node, const osg::Matrixd &projm, const float x, const float y)
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

    inline void setxy(float xpt, float ypt) { xp=xpt; yp=ypt; }
    inline bool hits() const { return _PIVsegHitList.size()>0;}
    
private:

    PickIntersectVisitor _piv;
    float xp, yp; // start point in viewport fraction coordiantes
    osgUtil::IntersectVisitor::HitList       _PIVsegHitList;
};
/////////////////////////////////////////////////////////////////////////////////////////////////

bool computePixelCoords(float x,float y,Producer::Camera* camera,float& pixel_x,float& pixel_y)
{
    Producer::RenderSurface* rs = camera->getRenderSurface();

	if (rs)
    {
        //std::cout << "    compute pixel coords "<<pixel_x<<"  "<<pixel_y<<std::endl;

        int pr_wx, pr_wy;
        unsigned int pr_width, pr_height;
        camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );

        int rs_wx, rs_wy;
        unsigned int rs_width, rs_height;
        rs->getWindowRectangle( rs_wx, rs_wy, rs_width, rs_height );

        pixel_x -= (float)rs_wx;
        pixel_y -= (float)rs_wy;

        //std::cout << "    wx = "<<pr_wx<<"  wy = "<<pr_wy<<" width="<<pr_width<<" height="<<pr_height<<std::endl;

        if (pixel_x<(float)pr_wx) return false;
        if (pixel_x>(float)(pr_wx+pr_width)) return false;

        if (pixel_y<(float)pr_wy) return false;
        if (pixel_y>(float)(pr_wy+pr_height)) return false;

        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

osg::Vec3 GetFarClickPoint(float pixel_x,float pixel_y,Producer::Camera* camera)
{
	int pr_wx, pr_wy;
    unsigned int pr_width, pr_height;
    camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );
	pixel_y = pr_height-(pixel_y-pr_wy);

    // convert into clip coords.
    float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
    float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

    osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
    osgUtil::SceneView* sv = sh?sh->getSceneView():0;
    osg::Matrixd vum;
    if (sv!=0)
    {
        vum.set(sv->getViewMatrix() *
                sv->getProjectionMatrix());
    }
    else
    {
        vum.set(osg::Matrixd(camera->getViewMatrix()) *
                osg::Matrixd(camera->getProjectionMatrix()));
    }

	osg::Matrixd inverseMVPW;
    inverseMVPW.invert(vum);
    osg::Vec3 far_point = osg::Vec3(rx,ry,1.0f)*inverseMVPW;
	return far_point;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

bool computeIntersections(float pixel_x,float pixel_y,Producer::Camera* camera,osg::Node *node,osgUtil::IntersectVisitor::HitList& hits)
{
	osg::Node::NodeMask traversalMask = 0xffffffff;
    //float pixel_x,pixel_y;
    // if (computePixelCoords(x,y,camera,pixel_x,pixel_y))
    {
        int pr_wx, pr_wy;
        unsigned int pr_width, pr_height;
        camera->getProjectionRectangle( pr_wx, pr_wy, pr_width, pr_height );
		pixel_y = pr_height-(pixel_y-pr_wy);

        // convert into clip coords.
        float rx = 2.0f*(pixel_x - (float)pr_wx)/(float)pr_width-1.0f;
        float ry = 2.0f*(pixel_y - (float)pr_wy)/(float)pr_height-1.0f;

        //std::cout << "    rx "<<rx<<"  "<<ry<<std::endl;

        osgProducer::OsgSceneHandler* sh = dynamic_cast<osgProducer::OsgSceneHandler*>(camera->getSceneHandler());
        osgUtil::SceneView* sv = sh?sh->getSceneView():0;
        osg::Matrixd vum;
        if (sv!=0)
        {
            vum.set(sv->getViewMatrix() *
                    sv->getProjectionMatrix());
        }
        else
        {
            vum.set(osg::Matrixd(camera->getViewMatrix()) *
                    osg::Matrixd(camera->getProjectionMatrix()));
        }

        PickVisitor iv;
        iv.setTraversalMask(traversalMask);
        
        osgUtil::IntersectVisitor::HitList localHits;        
        localHits = iv.getHits(node, vum, rx,ry);
        
        if (localHits.empty()) return false;
        
        hits.insert(hits.begin(),localHits.begin(),localHits.end());
        
        return true;
    }
    return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////////


//! C'tor Sets up the camera and scene handler
CMainViewer::CMainViewer()
{
	m_camera = new Producer::Camera;
	m_sceneHandler = new osgProducer::OsgSceneHandler;
	m_sceneHandler->getSceneView()->setDefaults();
	m_sceneHandler->getSceneView()->getState()->setContextID(s_contextID++);
	m_camera->setSceneHandler( m_sceneHandler.get() );

	// Start looking straight ahead
	m_heading_rad = m_pitch_rad = 0.0f;
	m_headingChange_rad = m_pitchChange_rad = 0.0f;
	m_mainFrame = NULL;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

void CMainViewer::V3D_UpdateOnce()
{
	osg::Vec3& cp = SceneRoot::s_cameraPosition;
	m_heading_rad += m_headingChange_rad;
	m_pitch_rad += m_pitchChange_rad;
	float piO8 = M_PI / 8.0f;
	if (m_pitch_rad > piO8) m_pitch_rad = piO8;
	else if (m_pitch_rad < -piO8) m_pitch_rad = -piO8;
	m_camera->setViewByLookat(	cp[0], cp[1], cp[2],
									cp[0] + cos(m_pitch_rad)*sin(m_heading_rad), 
									cp[1] + cos(m_pitch_rad)*cos(m_heading_rad), 
									cp[2] + sin(m_pitch_rad),
								0, 0, 1 );
	m_camera->frame();
}
/////////////////////////////////////////////////////////////////////////////////////////////////

void CMainViewer::V3D_OnMouseMove(int xPos, int yPos, int flags)
{
	// If we are pressing the mouse button, find where it hits the land and place the selection there
	if ((flags & MK_LBUTTON) && m_dragging)
	{
		if (flags & MK_SHIFT)
		{
			// Find where this ray hits the ground
			osgUtil::IntersectVisitor::HitList hlist;
			computeIntersections(xPos, yPos, m_camera.get(), m_landNode.get(), hlist);

			if (!hlist.empty()) {
				osg::Vec3 closestPt = hlist[0].getWorldIntersectPoint();
				float closestDist = closestPt.length();
				for (unsigned i = 1; i < hlist.size(); ++i) {
					osg::Vec3 thisPt = hlist[i].getWorldIntersectPoint();
					float thisDist = thisPt.length();
					if (thisDist < closestDist) {
						closestDist = thisDist;
						closestPt = thisPt;
					}
				}
				m_mainFrame->SetSelPosition(closestPt);
			}
		}

		// We can also move the window if close to the edges
		RECT mRect;
		GetWindowRect(&mRect);
		long width = mRect.right - mRect.left;
		long height = mRect.bottom - mRect.top;

		if (xPos < (width/3))
			m_headingChange_rad = 0.00003f * (xPos-(width/3));
		else if (xPos > (2*width/3))
			m_headingChange_rad = 0.00003f * (xPos-(2*width/3));
		else m_headingChange_rad = 0.0f;

		if (m_headingChange_rad == 0.0)
		{
			if (yPos < (height/4))
				m_pitchChange_rad = 0.00001f * ((height/4)-yPos);
			else if (yPos > (3*height/4))
				m_pitchChange_rad = 0.00001f * ((3*height/4)-yPos);
			else m_pitchChange_rad = 0.0f;
		}
		else m_pitchChange_rad = 0.0f;
	}
	else m_headingChange_rad = m_pitchChange_rad = 0.0f;
}
/////////////////////////////////////////////////////////////////////////////////////////////////

void CMainViewer::V3D_OnLButtonDown(int xPos, int yPos, int flags)
{
	// Find the ray from where we are clicking
	osg::Vec3& myPos = SceneRoot::s_cameraPosition;
	osg::Vec3 pointVec = GetFarClickPoint(xPos, yPos, m_camera.get()) - myPos;
	pointVec.normalize();

	// Loop through the targets to see which one is clsest to where we are looking
	float currAngle = M_PI / 32.0f;
	osg::PositionAttitudeTransform* selNode = NULL;
	for (unsigned i = 0; i < SceneRoot::s_transList.size(); ++i)
	{
		// Find the vector to this target and the angle between the click
		osg::Vec3 thisVec = SceneRoot::s_transList[i]->getPosition() - myPos;
		float thisAngle = acos(thisVec*pointVec / thisVec.length());
		if (thisAngle < currAngle)
		{
			currAngle = thisAngle;
			selNode = SceneRoot::s_transList[i];
		}
	}

	// If we have a selection, choose the model with the name as our selected node
	if (selNode)
		m_mainFrame->SelectNode(selNode->getChild(0));

	/*
	// Try to select one of the targets
		osgUtil::IntersectVisitor::HitList hlist;
		computeIntersections(xPos, yPos, m_camera.get(), m_targetGroup.get(), hlist);

	// See if one was selected, if so, set the target crosshairs
		if (!hlist.empty() && hlist[0]._geode.valid())
			m_mainFrame->SelectNode(hlist[0]._geode.get());
			*/
}
/////////////////////////////////////////////////////////////////////////////////////////////////
// $Log: MainViewer.cpp,v $
// Revision 1.3  2004/05/21 21:18:13  pingrri
// ZEUS_Simulator - 1.0.0.4 - Awesome Laser Pointer
//
// Revision 1.2  2004/05/20 21:30:06  pingrri
// ZEUS_Simulator - Almost there
//
// Revision 1.1  2004/05/18 21:30:37  pingrri
// Allows Target Placement
//