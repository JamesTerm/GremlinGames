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
#include <osg/Matrix>
#include <osg/Object>
#include <osg/State>
#include <osg/StateAttribute>
#include <osg/Viewport>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osg::Viewport)
	I_BaseType(osg::StateAttribute);
	I_Constructor0();
	I_Constructor4(IN, int, x, IN, int, y, IN, int, width, IN, int, height);
	I_ConstructorWithDefaults2(IN, const osg::Viewport &, vp, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method0(osg::StateAttribute::Type, getType);
	I_Method1(int, compare, IN, const osg::StateAttribute &, sa);
	I_Method4(void, setViewport, IN, int, x, IN, int, y, IN, int, width, IN, int, height);
	I_Method4(void, getViewport, IN, int &, x, IN, int &, y, IN, int &, width, IN, int &, height);
	I_Method0(int &, x);
	I_Method0(int, x);
	I_Method0(int &, y);
	I_Method0(int, y);
	I_Method0(int &, width);
	I_Method0(int, width);
	I_Method0(int &, height);
	I_Method0(int, height);
	I_Method0(bool, valid);
	I_Method0(float, aspectRatio);
	I_Method0(const osg::Matrix, computeWindowMatrix);
	I_Method1(void, apply, IN, osg::State &, state);
	I_ReadOnlyProperty(osg::StateAttribute::Type, Type);
END_REFLECTOR
