// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

//This is temporary until OSG has been successfully ported over to OSG_SVN
#include "../../CompilerSettings.h"

////////////////////
// Standard Includes
#include <stdio.h>
#include <string>
#include <direct.h>
#include <queue>
#include <list>

///////////////////
// OSG and Producer
#include <osg\Node>
#include <osg/Timer>
#include <osg\PositionAttitudeTransform>
#include <osg\Geode>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Billboard>
#include <osg/MatrixTransform>
#include <osg/Notify>
#include <osg/PolygonMode>
#include <osg/lod>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>

#include <osgUtil\UpdateVisitor>
#include <osgUtil/Optimizer>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>
#include <osgParticle/SectorPlacer>
#include <osg/CameraNode>
#include <osg/NodeCallback>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <OpenThreads/Thread>

// XML Parsing
#include <xmlParser.h>

// My own DLL header
// [James] Rick, I was all the sudden having internal compiler error C1001 problems.  (In debug mode)
//#include "GG_Framework.UI.h"