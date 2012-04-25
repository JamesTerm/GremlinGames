// ***************************************************************************
//
//   Generated automatically by genwrapper.
//   Please DO NOT EDIT this file!
//
// ***************************************************************************

#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/StaticMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/CopyOp>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osgSim/VisibilityGroup>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgSim::VisibilityGroup)
	I_BaseType(osg::Group);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osgSim::VisibilityGroup &, x, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, className);
	I_Method0(const char *, libraryName);
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv);
	I_Method1(void, traverse, IN, osg::NodeVisitor &, nv);
	I_Method1(void, setVisibilityVolume, IN, osg::Node *, node);
	I_Method0(osg::Node *, getVisibilityVolume);
	I_Method0(const osg::Node *, getVisibilityVolume);
	I_Method1(void, setVolumeIntersectionMask, IN, osg::Node::NodeMask, mask);
	I_Method0(osg::Node::NodeMask, getVolumeIntersectionMask);
	I_Method1(void, setSegmentLength, IN, float, length);
	I_Method0(float, getSegmentLength);
	I_Property(float, SegmentLength);
	I_Property(osg::Node *, VisibilityVolume);
	I_Property(osg::Node::NodeMask, VolumeIntersectionMask);
END_REFLECTOR
