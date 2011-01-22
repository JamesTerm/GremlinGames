#ifndef PLANE_H
#define PLANE_H

#include <osg/Geode>

class Plane : public osg::Geode
{
	public:
		Plane( float west, float east, float south, float north, float alt, float density );
};

#endif
