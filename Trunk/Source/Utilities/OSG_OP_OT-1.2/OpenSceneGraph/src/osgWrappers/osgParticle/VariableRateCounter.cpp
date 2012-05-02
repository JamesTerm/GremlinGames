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
#include <osgParticle/VariableRateCounter>
#include <osgParticle/range>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osgParticle::VariableRateCounter)
	I_BaseType(osgParticle::Counter);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osgParticle::VariableRateCounter &, copy, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const osgParticle::rangef &, getRateRange);
	I_Method1(void, setRateRange, IN, const osgParticle::rangef &, r);
	I_Method2(void, setRateRange, IN, float, minrange, IN, float, maxrange);
	I_Property(const osgParticle::rangef &, RateRange);
END_REFLECTOR
