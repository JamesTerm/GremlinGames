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
#include <osg/LineStipple>
#include <osg/Object>
#include <osg/State>
#include <osg/StateAttribute>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osg::LineStipple)
	I_BaseType(osg::StateAttribute);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::LineStipple &, lw, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method0(osg::StateAttribute::Type, getType);
	I_Method1(int, compare, IN, const osg::StateAttribute &, sa);
	I_Method1(bool, getModeUsage, IN, osg::StateAttribute::ModeUsage &, usage);
	I_Method1(void, setFactor, IN, GLint, factor);
	I_Method0(GLint, getFactor);
	I_Method1(void, setPattern, IN, GLushort, pattern);
	I_Method0(GLushort, getPattern);
	I_Method1(void, apply, IN, osg::State &, state);
	I_Property(GLint, Factor);
	I_Property(GLushort, Pattern);
	I_ReadOnlyProperty(osg::StateAttribute::Type, Type);
END_REFLECTOR
