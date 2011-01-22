// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// This seems to be needed for the WINGDI headers in gl.h, and I am wondering
// if we only need it for the existing Text class, perhaps we can remove it.
// Windows Header Files:
#include <windows.h>
#include "..\..\..\CompilerSettings.h"

#include <string>
#include <map>
#include <GL/gl.h>
#include <iostream>

// OSG and Producer Includes
#include <osgViewer/Viewer>
#include <osg/Matrix>
#include <osg/Transform>
#include <osg/AnimationPath>
#include <osg/Notify>
#include <osgDB/WriteFile>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>
#include <osg/Timer>
#include <osg/NodeVisitor>
#include <osgUtil/IntersectVisitor>
#include <osg/Projection>
#include <osgDB/ReadFile>
#include <osg\Depth>

// My own DLL Header
//#include "GG_Framework.UI.OSG.h"
