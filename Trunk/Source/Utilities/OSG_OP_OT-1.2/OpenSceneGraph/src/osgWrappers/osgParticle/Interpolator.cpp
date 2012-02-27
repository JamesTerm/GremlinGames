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
#include <osg/Object>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osgParticle/Interpolator>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osgParticle::Interpolator)
	I_BaseType(osg::Object);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osgParticle::Interpolator &, copy, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method3(float, interpolate, IN, float, t, IN, float, y1, IN, float, y2);
	I_Method3(osg::Vec2, interpolate, IN, float, t, IN, const osg::Vec2 &, y1, IN, const osg::Vec2 &, y2);
	I_Method3(osg::Vec3, interpolate, IN, float, t, IN, const osg::Vec3 &, y1, IN, const osg::Vec3 &, y2);
	I_Method3(osg::Vec4, interpolate, IN, float, t, IN, const osg::Vec4 &, y1, IN, const osg::Vec4 &, y2);
END_REFLECTOR
