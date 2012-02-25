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

#include <osg/Vec3f>
#include <osg/Vec4f>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

#include <osg/io_utils>
	
TYPE_NAME_ALIAS(float, osg::Vec4f::value_type);

BEGIN_VALUE_REFLECTOR(osg::Vec4f)
	I_ReaderWriter(osgIntrospection::StdReaderWriter<reflected_type>);	// user-defined
	I_Comparator(osgIntrospection::PartialOrderComparator<reflected_type>);	// user-defined
	I_Constructor0();
	I_Constructor4(IN, osg::Vec4f::value_type, x, IN, osg::Vec4f::value_type, y, IN, osg::Vec4f::value_type, z, IN, osg::Vec4f::value_type, w);
	I_Constructor2(IN, const osg::Vec3f &, v3, IN, osg::Vec4f::value_type, w);
	I_Method0(osg::Vec4f::value_type *, ptr);
	I_Method0(const osg::Vec4f::value_type *, ptr);
	I_Method4(void, set, IN, osg::Vec4f::value_type, x, IN, osg::Vec4f::value_type, y, IN, osg::Vec4f::value_type, z, IN, osg::Vec4f::value_type, w);
	I_Method0(osg::Vec4f::value_type &, x);
	I_Method0(osg::Vec4f::value_type &, y);
	I_Method0(osg::Vec4f::value_type &, z);
	I_Method0(osg::Vec4f::value_type &, w);
	I_Method0(osg::Vec4f::value_type, x);
	I_Method0(osg::Vec4f::value_type, y);
	I_Method0(osg::Vec4f::value_type, z);
	I_Method0(osg::Vec4f::value_type, w);
	I_Method0(osg::Vec4f::value_type &, r);
	I_Method0(osg::Vec4f::value_type &, g);
	I_Method0(osg::Vec4f::value_type &, b);
	I_Method0(osg::Vec4f::value_type &, a);
	I_Method0(osg::Vec4f::value_type, r);
	I_Method0(osg::Vec4f::value_type, g);
	I_Method0(osg::Vec4f::value_type, b);
	I_Method0(osg::Vec4f::value_type, a);
	I_Method0(unsigned int, asABGR);
	I_Method0(unsigned int, asRGBA);
	I_Method0(bool, valid);
	I_Method0(bool, isNaN);
	I_Method0(osg::Vec4f::value_type, length);
	I_Method0(osg::Vec4f::value_type, length2);
	I_Method0(osg::Vec4f::value_type, normalize);
END_REFLECTOR
