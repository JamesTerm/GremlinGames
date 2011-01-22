#include "stdafx.h"
#include "Fringe.Base.h"

#include "..\..\DebugPrintMacros.hpp"

// CONSTANTS for TWEAKING the STARDUST
const int STARDUST_NUMSTARS = 600;	//!< How many stars there are in a "block"
const double STARDUST_RANGE = 3000.0; //!< The width of a block of stars
const double STARDUST_MAXDISSOLVE = 0.3; //!< The most opaque they can be (0 is not visible, 1 is fully opaque)
const double STARDUST_SPEED_STREAKSIZE_SCALE = 0.15/250.0; //! The streak length is based on the vel squared times this


/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/


float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }


struct StardustDrawCallback : public osg::Drawable::DrawCallback
{
	mutable bool _firstTime;
	mutable osg::Vec3d _camPos;
	mutable osg::Vec3d _myPos;
	double m_maxDist2;

	GG_Framework::Logic::Entity3D* FollowingObject;

    StardustDrawCallback(double maxDist) : m_maxDist2(maxDist*maxDist), _firstTime(true), FollowingObject(NULL) {}

	virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const 
    {
		if (!_firstTime)
		{
			osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(const_cast<osg::Drawable*>(drawable));
			osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
			osg::Vec4Array* colours = dynamic_cast<osg::Vec4Array*>(geometry->getColorArray());

			osg::Vec3d camDistOffset = _myPos-_camPos;
			osg::Vec3d offset(0.0,0.0,0.01);
			osg::Vec3d vel(m_offsetAverager.GetAverage(FollowingObject->GetLinearVelocity()));
			if (FollowingObject)
				offset = vel * (STARDUST_SPEED_STREAKSIZE_SCALE * vel.length());

			// Work through each set, setting the trail and the offset
			for(unsigned int i=0;i+1<vertices->size();i+=2)
			{
				(*vertices)[i+1] = (*vertices)[i]+offset;

				// Dissolve based on distance from camera
				double dist2 = ((*vertices)[i] + camDistOffset).length2();
				double distCoeff = 1.0 - (dist2 / m_maxDist2);
				if (distCoeff < 0.0)
					(*colours)[i][3] = 0.0;	// All faded out
				else
				{
					double alpha = STARDUST_MAXDISSOLVE*distCoeff;
					(*colours)[i][3] = (alpha < 1.0) ? alpha : 1.0;	// We can see part of it
				}
			}
		}
		drawable->drawImplementation(renderInfo);
    }

	// Use an averager to limit crazy camera offsets
	mutable Averager<osg::Vec3d, 10> m_offsetAverager;

	void SetCameraPos(osg::Vec3d camPos, osg::Vec3d myPos)
	{
		_firstTime = false;
		_camPos = camPos;
		_myPos = myPos;
	}
};

osg::Geode* CreateStardust(int numStars, float cubeWidth, StardustDrawCallback* callback)
{
    osg::Geometry* geometry = new osg::Geometry;
    
    // set up the original vertices (will be repeated 27 times

	osg::ref_ptr<osg::Vec3Array> oV = new osg::Vec3Array(numStars);
	osg::ref_ptr<osg::Vec4Array> oC = new osg::Vec4Array(numStars);
	osg::Vec3Array& origVerts(*(oV.get()));
	osg::Vec4Array& origColors(*(oC.get()));
    osg::Vec3Array* vertices = new osg::Vec3Array(numStars*2*27);
	osg::Vec4Array* colours = new osg::Vec4Array(numStars*2*27);
    geometry->setVertexArray(vertices);
	geometry->setColorArray(colours);
	geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    
    float max = cubeWidth / 2.0;
    float min = -max;
    int j = 0;
    int i = 0;
    for(i=0;i<numStars;++i)
    {
        origVerts[i].set(random(min,max),random(min,max),random(min,max));
		origColors[i].set(random(0.6f,0.9f),random(0.6f,0.9f),random(0.6f,0.9f), STARDUST_MAXDISSOLVE);
    } 

	// Multiply the original set 27 times about the center
	int m = 0;
	osg::Vec3d smallOffset(0.0, 0.0, 0.01);
	for (i = -1; i <= 1; ++i)
	{
		for (int j = -1; j <= 1; ++j)
		{
			for (int k = -1; k <= 1; ++k)
			{
				osg::Vec3d cubeOffset(i*cubeWidth, j*cubeWidth, k*cubeWidth);
				for (int l = 0; l < numStars; ++l)
				{
					osg::Vec3d v = origVerts[l] + cubeOffset;
					(*vertices)[m] = v;
					(*vertices)[m+1] = v + smallOffset;
					(*colours)[m] = origColors[l];
					(*colours)[m+1] = origColors[l];
					(*colours)[m+1][3] = 0.0;	// Fade to nothing
					m+=2;
				}
			}
		}
	}

    // set up the primitive set to draw lines
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES,0,numStars*27*2));

    // set up the points for the stars.
    osg::DrawElementsUShort* points = new osg::DrawElementsUShort(GL_POINTS,numStars*27);
    geometry->addPrimitiveSet(points);
    for(i=0;i<(numStars*27);++i)
    {
        (*points)[i] = i*2;
    }

    geometry->setUseDisplayList(false);
    geometry->setDrawCallback(callback);
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	{
		osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
		bf->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE);
		osg::StateSet* ss = geode->getOrCreateStateSet();
		ss->setAttributeAndModes(bf.get());
		ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
	}      
    
    return geode;
}
//////////////////////////////////////////////////////////////////////////
class StardustBlock : public osg::PositionAttitudeTransform
{
public:
	StardustBlock(unsigned int noStarsPerBlock, float cubeWidthPerBlock) : m_cubeWidthPerBlock(cubeWidthPerBlock)
	{
		// Make a callback that the stardust will use
		Callback = new StardustDrawCallback(cubeWidthPerBlock);

		// Make a single block, used each time
		addChild(CreateStardust(noStarsPerBlock, cubeWidthPerBlock, Callback));

		// Listen for when the main camera changes its position so we can set ourselves
		GG_Framework::UI::MainWindow::GetMainWindow()->GetMainCamera()->MatrixUpdate.Subscribe(ehl, *this, &StardustBlock::CameraUpdateCallback);
	}

	StardustDrawCallback* Callback;

private:
	IEvent::HandlerList ehl;
	float m_cubeWidthPerBlock;

	void CameraUpdateCallback(const osg::Matrix& camMatrix)
	{
		// Find the out where the camera is
		osg::Vec3d eye, center, up;
		camMatrix.getLookAt(eye, center, up);

		// Decide where this block should be if it needs to be moved
		double x = (eye[0] > 0) ? ((int)((eye[0] / m_cubeWidthPerBlock)+0.5f)*m_cubeWidthPerBlock) : ((int)((eye[0] / m_cubeWidthPerBlock)-0.5f)*m_cubeWidthPerBlock);
		double y = (eye[1] > 0) ? ((int)((eye[1] / m_cubeWidthPerBlock)+0.5f)*m_cubeWidthPerBlock) : ((int)((eye[1] / m_cubeWidthPerBlock)-0.5f)*m_cubeWidthPerBlock);
		double z = (eye[2] > 0) ? ((int)((eye[2] / m_cubeWidthPerBlock)+0.5f)*m_cubeWidthPerBlock) : ((int)((eye[2] / m_cubeWidthPerBlock)-0.5f)*m_cubeWidthPerBlock);

		osg::Vec3d newPos(x,y,z);

		// This is where we want to be, no reason to set if already there
		if (getPosition() != newPos)
			setPosition(newPos);

		// Tell the callback where the camera is now
		Callback->SetCameraPos(eye, newPos);
	}
};
//////////////////////////////////////////////////////////////////////////

bool Fringe::Base::UI_GameClient::LoadOrnamentalOSGV()
{
	bool ret = __super::LoadOrnamentalOSGV();
#ifndef IGNORE_SPACEWARP
	if (ret)
	{
		m_stardust = new StardustBlock(STARDUST_NUMSTARS, STARDUST_RANGE);
		UI_ActorScene.GetScene()->addChild(m_stardust);
		m_stardust->Callback->FollowingObject = GetControlledEntity();
	}
#endif
	return ret;
}
//////////////////////////////////////////////////////////////////////////

void Fringe::Base::UI_GameClient::ControlledEntityChangedCallback(GG_Framework::Logic::Entity3D* oldEntity, GG_Framework::Logic::Entity3D* newEntity)
{
	if (m_stardust)
		m_stardust->Callback->FollowingObject = newEntity;
}
//////////////////////////////////////////////////////////////////////////



