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

#ifndef ANIMATED_DISSOLVE_H
#define ANIMATED_DISSOLVE_H 1

#include <map>
#include <istream>
#include <float.h>
#include <osg/NodeCallback>
#include <osg/Material>
#include "SceneLoader.h"
using namespace osg;

namespace lwosg {

class AnimatedDissolveCallback : public NodeCallback
{	
	private:
		double GetInterpolatedDissolve(double time);
		osg::ref_ptr<osg::Material> m_material;

    public:
		static void ApplyDissolve(osg::Node* node, double dissolve);

		Dissolve_Key_map Key_Map;
        AnimatedDissolveCallback(Dissolve_Key_map& copyKeyMap):
            _timeOffset(0.0),
            _timeMultiplier(1.0),
            _firstTime(DBL_MAX),
            _latestTime(0.0),
            _pause(false),
            _pauseTime(0.0),
			Key_Map(copyKeyMap) {}

        AnimatedDissolveCallback(const AnimatedDissolveCallback& apc,const CopyOp& copyop):
            NodeCallback(apc,copyop),
            _timeOffset(apc._timeOffset),
            _timeMultiplier(apc._timeMultiplier),
            _firstTime(apc._firstTime),
            _latestTime(apc._latestTime),
            _pause(apc._pause),
			_pauseTime(apc._pauseTime),
			Key_Map(apc.Key_Map),
			m_material(apc.m_material.get()) {}

 
        void setTimeOffset(double offset) { _timeOffset = offset; }
        double getTimeOffset() const { return _timeOffset; }
        
        void setTimeMultiplier(double multiplier) { _timeMultiplier = multiplier; }
        double getTimeMultiplier() const { return _timeMultiplier; }


        virtual void reset();

        void setPause(bool pause);
        bool getPause() const { return _pause; }

        /** Get the animation time that is used to specify the position along
          * the AnimationPath. Animation time is computed from the formula:
          *   ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier.*/
        virtual double getAnimationTime() const;

        /** Implements the callback. */
        virtual void operator()(Node* node, NodeVisitor* nv);
        
        void update(osg::Node& node);

    public:
        double                  _timeOffset;
        double                  _timeMultiplier;
        double                  _firstTime;
        double                  _latestTime;
        bool                    _pause;
        double                  _pauseTime;

    protected:
    
        ~AnimatedDissolveCallback(){}

};

}

#endif
