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

#include <osg/Vec3s>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(short, osg::Vec3s::value_type);

BEGIN_VALUE_REFLECTOR(osg::Vec3s)
	I_Constructor0();
	I_Constructor3(IN, osg::Vec3s::value_type, r, IN, osg::Vec3s::value_type, g, IN, osg::Vec3s::value_type, b);
	I_Method0(osg::Vec3s::value_type *, ptr);
	I_Method0(const osg::Vec3s::value_type *, ptr);
	I_Method3(void, set, IN, osg::Vec3s::value_type, r, IN, osg::Vec3s::value_type, g, IN, osg::Vec3s::value_type, b);
	I_Method1(void, set, IN, const osg::Vec3s &, rhs);
	I_Method0(osg::Vec3s::value_type &, x);
	I_Method0(osg::Vec3s::value_type &, y);
	I_Method0(osg::Vec3s::value_type &, z);
	I_Method0(osg::Vec3s::value_type, x);
	I_Method0(osg::Vec3s::value_type, y);
	I_Method0(osg::Vec3s::value_type, z);
	I_Method0(osg::Vec3s::value_type &, r);
	I_Method0(osg::Vec3s::value_type &, g);
	I_Method0(osg::Vec3s::value_type &, b);
	I_Method0(osg::Vec3s::value_type, r);
	I_Method0(osg::Vec3s::value_type, g);
	I_Method0(osg::Vec3s::value_type, b);
	I_WriteOnlyProperty(const osg::Vec3s &, );
END_REFLECTOR
