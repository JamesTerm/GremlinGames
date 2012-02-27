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

#include <osg/ApplicationUsage>
#include <osg/Matrixd>
#include <osg/Node>
#include <osg/Vec3>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <osgGA/UFOManipulator>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgGA::UFOManipulator)
	I_BaseType(osgGA::MatrixManipulator);
	I_Constructor0();
	I_Method0(const char *, className);
	I_Method1(void, setByMatrix, IN, const osg::Matrixd &, matrix);
	I_Method1(void, setByInverseMatrix, IN, const osg::Matrixd &, invmat);
	I_Method0(osg::Matrixd, getMatrix);
	I_Method0(osg::Matrixd, getInverseMatrix);
	I_Method1(void, setNode, IN, osg::Node *, node);
	I_Method0(const osg::Node *, getNode);
	I_Method0(osg::Node *, getNode);
	I_Method0(void, computeHomePosition);
	I_Method2(void, home, IN, const osgGA::GUIEventAdapter &, x, IN, osgGA::GUIActionAdapter &, x);
	I_Method1(void, home, IN, double, x);
	I_Method2(void, init, IN, const osgGA::GUIEventAdapter &, x, IN, osgGA::GUIActionAdapter &, x);
	I_Method2(bool, handle, IN, const osgGA::GUIEventAdapter &, ea, IN, osgGA::GUIActionAdapter &, aa);
	I_Method1(void, getUsage, IN, osg::ApplicationUsage &, usage);
	I_Method3(void, getCurrentPositionAsLookAt, IN, osg::Vec3 &, eye, IN, osg::Vec3 &, center, IN, osg::Vec3 &, up);
	I_Method1(void, setMinHeight, IN, double, in_min_height);
	I_Method0(double, getMinHeight);
	I_Method1(void, setMinDistance, IN, double, in_min_dist);
	I_Method0(double, getMinDistance);
	I_Method1(void, setForwardSpeed, IN, double, in_fs);
	I_Method0(double, getForwardSpeed);
	I_Method1(void, setSideSpeed, IN, double, in_ss);
	I_Method0(double, getSideSpeed);
	I_Method1(void, setRotationSpeed, IN, double, in_rot_speed);
	I_Method0(double, getRotationSpeed);
	I_WriteOnlyProperty(const osg::Matrixd &, ByInverseMatrix);
	I_WriteOnlyProperty(const osg::Matrixd &, ByMatrix);
	I_Property(double, ForwardSpeed);
	I_ReadOnlyProperty(osg::Matrixd, InverseMatrix);
	I_ReadOnlyProperty(osg::Matrixd, Matrix);
	I_Property(double, MinDistance);
	I_Property(double, MinHeight);
	I_Property(osg::Node *, Node);
	I_Property(double, RotationSpeed);
	I_Property(double, SideSpeed);
END_REFLECTOR
