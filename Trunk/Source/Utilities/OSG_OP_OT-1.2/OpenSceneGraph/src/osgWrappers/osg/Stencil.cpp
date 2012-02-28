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
#include <osg/State>
#include <osg/StateAttribute>
#include <osg/Stencil>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ENUM_REFLECTOR(osg::Stencil::Function)
	I_EnumLabel(osg::Stencil::NEVER);
	I_EnumLabel(osg::Stencil::LESS);
	I_EnumLabel(osg::Stencil::EQUAL);
	I_EnumLabel(osg::Stencil::LEQUAL);
	I_EnumLabel(osg::Stencil::GREATER);
	I_EnumLabel(osg::Stencil::NOTEQUAL);
	I_EnumLabel(osg::Stencil::GEQUAL);
	I_EnumLabel(osg::Stencil::ALWAYS);
END_REFLECTOR

BEGIN_ENUM_REFLECTOR(osg::Stencil::Operation)
	I_EnumLabel(osg::Stencil::KEEP);
	I_EnumLabel(osg::Stencil::ZERO);
	I_EnumLabel(osg::Stencil::REPLACE);
	I_EnumLabel(osg::Stencil::INCR);
	I_EnumLabel(osg::Stencil::DECR);
	I_EnumLabel(osg::Stencil::INVERT);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::Stencil)
	I_BaseType(osg::StateAttribute);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::Stencil &, stencil, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method0(osg::StateAttribute::Type, getType);
	I_Method1(int, compare, IN, const osg::StateAttribute &, sa);
	I_Method1(bool, getModeUsage, IN, osg::StateAttribute::ModeUsage &, usage);
	I_Method3(void, setFunction, IN, osg::Stencil::Function, func, IN, int, ref, IN, unsigned int, mask);
	I_Method1(void, setFunction, IN, osg::Stencil::Function, func);
	I_Method0(osg::Stencil::Function, getFunction);
	I_Method1(void, setFunctionRef, IN, int, ref);
	I_Method0(int, getFunctionRef);
	I_Method1(void, setFunctionMask, IN, unsigned int, mask);
	I_Method0(unsigned int, getFunctionMask);
	I_Method3(void, setOperation, IN, osg::Stencil::Operation, sfail, IN, osg::Stencil::Operation, zfail, IN, osg::Stencil::Operation, zpass);
	I_Method1(void, setStencilFailOperation, IN, osg::Stencil::Operation, sfail);
	I_Method0(osg::Stencil::Operation, getStencilFailOperation);
	I_Method1(void, setStencilPassAndDepthFailOperation, IN, osg::Stencil::Operation, zfail);
	I_Method0(osg::Stencil::Operation, getStencilPassAndDepthFailOperation);
	I_Method1(void, setStencilPassAndDepthPassOperation, IN, osg::Stencil::Operation, zpass);
	I_Method0(osg::Stencil::Operation, getStencilPassAndDepthPassOperation);
	I_Method1(void, setWriteMask, IN, unsigned int, mask);
	I_Method0(unsigned int, getWriteMask);
	I_Method1(void, apply, IN, osg::State &, state);
	I_Property(osg::Stencil::Function, Function);
	I_Property(unsigned int, FunctionMask);
	I_Property(int, FunctionRef);
	I_Property(osg::Stencil::Operation, StencilFailOperation);
	I_Property(osg::Stencil::Operation, StencilPassAndDepthFailOperation);
	I_Property(osg::Stencil::Operation, StencilPassAndDepthPassOperation);
	I_ReadOnlyProperty(osg::StateAttribute::Type, Type);
	I_Property(unsigned int, WriteMask);
END_REFLECTOR
