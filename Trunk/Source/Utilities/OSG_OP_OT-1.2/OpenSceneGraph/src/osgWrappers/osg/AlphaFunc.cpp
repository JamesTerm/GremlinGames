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

#include <osg/AlphaFunc>
#include <osg/CopyOp>
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

BEGIN_ENUM_REFLECTOR(osg::AlphaFunc::ComparisonFunction)
	I_EnumLabel(osg::AlphaFunc::NEVER);
	I_EnumLabel(osg::AlphaFunc::LESS);
	I_EnumLabel(osg::AlphaFunc::EQUAL);
	I_EnumLabel(osg::AlphaFunc::LEQUAL);
	I_EnumLabel(osg::AlphaFunc::GREATER);
	I_EnumLabel(osg::AlphaFunc::NOTEQUAL);
	I_EnumLabel(osg::AlphaFunc::GEQUAL);
	I_EnumLabel(osg::AlphaFunc::ALWAYS);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::AlphaFunc)
	I_BaseType(osg::StateAttribute);
	I_Constructor0();
	I_Constructor2(IN, osg::AlphaFunc::ComparisonFunction, func, IN, float, ref);
	I_ConstructorWithDefaults2(IN, const osg::AlphaFunc &, af, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method0(osg::StateAttribute::Type, getType);
	I_Method1(int, compare, IN, const osg::StateAttribute &, sa);
	I_Method1(bool, getModeUsage, IN, osg::StateAttribute::ModeUsage &, usage);
	I_Method2(void, setFunction, IN, osg::AlphaFunc::ComparisonFunction, func, IN, float, ref);
	I_Method1(void, setFunction, IN, osg::AlphaFunc::ComparisonFunction, func);
	I_Method0(osg::AlphaFunc::ComparisonFunction, getFunction);
	I_Method1(void, setReferenceValue, IN, float, value);
	I_Method0(float, getReferenceValue);
	I_Method1(void, apply, IN, osg::State &, state);
	I_Property(osg::AlphaFunc::ComparisonFunction, Function);
	I_Property(float, ReferenceValue);
	I_ReadOnlyProperty(osg::StateAttribute::Type, Type);
END_REFLECTOR
