#pragma once

#include "../../../Utilities/UnitTest++/src/UnitTest++.h"
#include "../../../Utilities/UnitTest++/src/TestReporterStdout.h"

#include <map>
#include <iostream>

#include <osg\Node>
#include <osg\PositionAttitudeTransform>
#include <osg\Geode>

#include <osgDB/ReadFile>

#include <osgUtil\UpdateVisitor>
#include <osgUtil/Optimizer>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <Producer/Trackball>

#include "../../Base/Event.h"
