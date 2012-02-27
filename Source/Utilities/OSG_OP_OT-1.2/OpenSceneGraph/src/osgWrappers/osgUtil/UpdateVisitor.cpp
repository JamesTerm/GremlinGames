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

#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LOD>
#include <osg/LightSource>
#include <osg/Node>
#include <osg/OccluderNode>
#include <osg/Projection>
#include <osg/Switch>
#include <osg/Transform>
#include <osgUtil/UpdateVisitor>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgUtil::UpdateVisitor)
	I_BaseType(osg::NodeVisitor);
	I_Constructor0();
	I_Method0(void, reset);
	I_Method1(void, apply, IN, osg::Node &, node);
	I_Method1(void, apply, IN, osg::Geode &, node);
	I_Method1(void, apply, IN, osg::Billboard &, node);
	I_Method1(void, apply, IN, osg::LightSource &, node);
	I_Method1(void, apply, IN, osg::Group &, node);
	I_Method1(void, apply, IN, osg::Transform &, node);
	I_Method1(void, apply, IN, osg::Projection &, node);
	I_Method1(void, apply, IN, osg::Switch &, node);
	I_Method1(void, apply, IN, osg::LOD &, node);
	I_Method1(void, apply, IN, osg::OccluderNode &, node);
END_REFLECTOR
