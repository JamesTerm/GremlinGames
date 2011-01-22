// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include "..\..\CompilerSettings.h"

#include <set>

#include <osg\Camera>
#include <osgGA\GUIEventAdapter>
#include <osg\PositionAttitudeTransform>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/BlendFunc>

//Rick this is risky, but using it can increase compile time if you are building the same project to test changes
//I will leave it commented out by default
#include "Fringe.Base.h"
