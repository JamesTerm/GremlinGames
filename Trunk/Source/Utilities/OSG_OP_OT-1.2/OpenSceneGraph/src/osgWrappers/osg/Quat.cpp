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

#include <osg/Matrixd>
#include <osg/Matrixf>
#include <osg/Quat>
#include <osg/Vec3d>
#include <osg/Vec3f>
#include <osg/Vec4d>
#include <osg/Vec4f>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

#include <osg/io_utils>
	
TYPE_NAME_ALIAS(double, osg::Quat::value_type);

BEGIN_VALUE_REFLECTOR(osg::Quat)
	I_ReaderWriter(osgIntrospection::StdReaderWriter<reflected_type>);	// user-defined
	I_Comparator(osgIntrospection::PartialOrderComparator<reflected_type>);	// user-defined
	I_Constructor0();
	I_Constructor4(IN, osg::Quat::value_type, x, IN, osg::Quat::value_type, y, IN, osg::Quat::value_type, z, IN, osg::Quat::value_type, w);
	I_Constructor1(IN, const osg::Vec4f &, v);
	I_Constructor1(IN, const osg::Vec4d &, v);
	I_Constructor2(IN, osg::Quat::value_type, angle, IN, const osg::Vec3f &, axis);
	I_Constructor2(IN, osg::Quat::value_type, angle, IN, const osg::Vec3d &, axis);
	I_Constructor6(IN, osg::Quat::value_type, angle1, IN, const osg::Vec3f &, axis1, IN, osg::Quat::value_type, angle2, IN, const osg::Vec3f &, axis2, IN, osg::Quat::value_type, angle3, IN, const osg::Vec3f &, axis3);
	I_Constructor6(IN, osg::Quat::value_type, angle1, IN, const osg::Vec3d &, axis1, IN, osg::Quat::value_type, angle2, IN, const osg::Vec3d &, axis2, IN, osg::Quat::value_type, angle3, IN, const osg::Vec3d &, axis3);
	I_Method0(osg::Vec4d, asVec4);
	I_Method0(osg::Vec3d, asVec3);
	I_Method4(void, set, IN, osg::Quat::value_type, x, IN, osg::Quat::value_type, y, IN, osg::Quat::value_type, z, IN, osg::Quat::value_type, w);
	I_Method1(void, set, IN, const osg::Vec4f &, v);
	I_Method1(void, set, IN, const osg::Vec4d &, v);
	I_Method1(void, set, IN, const osg::Matrixf &, matrix);
	I_Method1(void, set, IN, const osg::Matrixd &, matrix);
	I_Method1(void, get, IN, osg::Matrixf &, matrix);
	I_Method1(void, get, IN, osg::Matrixd &, matrix);
	I_Method0(osg::Quat::value_type &, x);
	I_Method0(osg::Quat::value_type &, y);
	I_Method0(osg::Quat::value_type &, z);
	I_Method0(osg::Quat::value_type &, w);
	I_Method0(osg::Quat::value_type, x);
	I_Method0(osg::Quat::value_type, y);
	I_Method0(osg::Quat::value_type, z);
	I_Method0(osg::Quat::value_type, w);
	I_Method0(bool, zeroRotation);
	I_Method0(osg::Quat::value_type, length);
	I_Method0(osg::Quat::value_type, length2);
	I_Method0(osg::Quat, conj);
	I_Method0(const osg::Quat, inverse);
	I_Method4(void, makeRotate, IN, osg::Quat::value_type, angle, IN, osg::Quat::value_type, x, IN, osg::Quat::value_type, y, IN, osg::Quat::value_type, z);
	I_Method2(void, makeRotate, IN, osg::Quat::value_type, angle, IN, const osg::Vec3f &, vec);
	I_Method2(void, makeRotate, IN, osg::Quat::value_type, angle, IN, const osg::Vec3d &, vec);
	I_Method6(void, makeRotate, IN, osg::Quat::value_type, angle1, IN, const osg::Vec3f &, axis1, IN, osg::Quat::value_type, angle2, IN, const osg::Vec3f &, axis2, IN, osg::Quat::value_type, angle3, IN, const osg::Vec3f &, axis3);
	I_Method6(void, makeRotate, IN, osg::Quat::value_type, angle1, IN, const osg::Vec3d &, axis1, IN, osg::Quat::value_type, angle2, IN, const osg::Vec3d &, axis2, IN, osg::Quat::value_type, angle3, IN, const osg::Vec3d &, axis3);
	I_Method2(void, makeRotate, IN, const osg::Vec3f &, vec1, IN, const osg::Vec3f &, vec2);
	I_Method2(void, makeRotate, IN, const osg::Vec3d &, vec1, IN, const osg::Vec3d &, vec2);
	I_Method2(void, makeRotate_original, IN, const osg::Vec3d &, vec1, IN, const osg::Vec3d &, vec2);
	I_Method4(void, getRotate, IN, osg::Quat::value_type &, angle, IN, osg::Quat::value_type &, x, IN, osg::Quat::value_type &, y, IN, osg::Quat::value_type &, z);
	I_Method2(void, getRotate, IN, osg::Quat::value_type &, angle, IN, osg::Vec3f &, vec);
	I_Method2(void, getRotate, IN, osg::Quat::value_type &, angle, IN, osg::Vec3d &, vec);
	I_Method3(void, slerp, IN, osg::Quat::value_type, t, IN, const osg::Quat &, from, IN, const osg::Quat &, to);
	I_WriteOnlyProperty(const osg::Vec4f &, );
END_REFLECTOR
