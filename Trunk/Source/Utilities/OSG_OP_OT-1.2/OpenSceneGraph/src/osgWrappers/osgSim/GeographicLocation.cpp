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

#include <osgSim/GeographicLocation>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgSim::GeographicLocation)
	I_BaseType(osg::Referenced);
	I_Constructor0();
	I_Constructor2(IN, double, lat, IN, double, lon);
	I_Method0(double *, ptr);
	I_Method0(const double *, ptr);
	I_Method2(void, set, IN, double, lat, IN, double, lon);
	I_Method0(double &, latitude);
	I_Method0(double &, longitude);
	I_Method0(double, latitude);
	I_Method0(double, longitude);
	I_Method0(bool, valid);
	I_Method0(bool, isNaN);
END_REFLECTOR
