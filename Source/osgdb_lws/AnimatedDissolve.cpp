/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include "AnimatedDissolve.h"
#include <osg/BlendFunc>

using namespace lwosg;
osg::ref_ptr<osg::BlendFunc> STANDARD_BLEND = new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA );

void AnimatedDissolveCallback::operator()(Node* node, NodeVisitor* nv)
{
    if (nv->getVisitorType()==NodeVisitor::UPDATE_VISITOR && 
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getSimulationTime();
        _latestTime = time;

        if (!_pause)
        {
            // Only update _firstTime the first time, when its value is still DBL_MAX
            if (_firstTime==DBL_MAX) _firstTime = time;
			if (!m_material.valid())
			{
				// Make this node (and all of its children) semi-transparent
				osg::StateSet* state = node->getOrCreateStateSet(); 
				m_material = new osg::Material; 
				state->setAttributeAndModes(m_material.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
				state->setAttributeAndModes(STANDARD_BLEND); 
				state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
			}
            update(*node);
        }
    }
    
    // must call any nested node callbacks and continue subgraph traversal.
	NodeCallback::traverse(node,nv);
}

double AnimatedDissolveCallback::getAnimationTime() const
{
    return ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier;
}

double AnimatedDissolveCallback::GetInterpolatedDissolve(double time)
{
	double dissolve = 0.0;
	Dissolve_Key_map::const_iterator second = Key_Map.lower_bound(time);
	if (second==Key_Map.begin())
	{
		dissolve = second->second;
	}
	else if (second!=Key_Map.end())
	{
		Dissolve_Key_map::const_iterator first = second;
		--first;        

		// we have both a lower bound and the next item.

		// delta_time = second.time - first.time
		double delta_time = second->first - first->first;

		if (delta_time==0.0)
			dissolve = first->second;
		else
		{
			double t = (time - first->first)/delta_time;
			dissolve = ((1.0-t)*first->second) + (t * second->second); // Linear Interpolate
		}        
	}
	else // (second==Key_Map.end())
	{
		dissolve = Key_Map.rbegin()->second;
	}
	return dissolve;
}

void AnimatedDissolveCallback::ApplyDissolve(osg::Node* node, double dissolve)
{
	if (dissolve > 0.0)
	{
		// Make this node (and all of its children) semi-transparent
		osg::StateSet* state = node->getOrCreateStateSet();
		state->setMode(GL_BLEND,osg:: StateAttribute::ON| osg::StateAttribute::OVERRIDE); 
		osg::Material* mat = new osg::Material; 
		mat->setAlpha(osg::Material::FRONT_AND_BACK, 1.0 - dissolve);
		state->setAttributeAndModes(mat,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		state->setAttributeAndModes(STANDARD_BLEND); 
		state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN); 
		// state->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	}
}

void AnimatedDissolveCallback::update(osg::Node& node)
{
	double dissolve = GetInterpolatedDissolve(getAnimationTime());
	osg::StateSet* state = node.getOrCreateStateSet();
	m_material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0 - dissolve);
	if (dissolve <= 0.0)
	{
		state->setMode(GL_BLEND,osg:: StateAttribute::OFF);
		state->setAttributeAndModes(m_material.get(),osg::StateAttribute::OFF);
		state->setRenderingHint(osg::StateSet::OPAQUE_BIN); 
	}
	else
	{
		if (dissolve > 1.0) dissolve = 1.0;
		state->setMode(GL_BLEND,osg:: StateAttribute::ON| osg::StateAttribute::OVERRIDE); 
		state->setAttributeAndModes(m_material.get(),osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		state->setRenderingHint(osg::StateSet::TRANSPARENT_BIN); 
	}
}


void AnimatedDissolveCallback::reset()
{
#if 1
    _firstTime = DBL_MAX;
    _pauseTime = DBL_MAX;
#else
    _firstTime = _latestTime;
    _pauseTime = _latestTime;
#endif
}

void AnimatedDissolveCallback::setPause(bool pause)
{
    if (_pause==pause)
    {
        return;
    }
    
    _pause = pause;

    if (_firstTime==DBL_MAX) return;

    if (_pause)
    {
        _pauseTime = _latestTime;
    }
    else
    {
        _firstTime += (_latestTime-_pauseTime);
    }
}
