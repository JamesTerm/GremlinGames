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

#include <osgDB/Output>
#include <osgDB/ParameterOutput>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_VALUE_REFLECTOR(osgDB::ParameterOutput)
	I_Constructor1(IN, osgDB::Output &, fw);
	I_Constructor2(IN, osgDB::Output &, fw, IN, int, numItemsPerLine);
	I_Method0(void, begin);
	I_Method0(void, newLine);
	I_Method0(void, end);
END_REFLECTOR

