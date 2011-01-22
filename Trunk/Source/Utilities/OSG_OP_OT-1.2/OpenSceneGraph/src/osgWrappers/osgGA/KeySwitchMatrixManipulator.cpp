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
#include <osg/Vec3d>
#include <osgGA/GUIActionAdapter>
#include <osgGA/GUIEventAdapter>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/MatrixManipulator>
#include <osgUtil/SceneView>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::pair< std::string COMMA  osg::ref_ptr< osgGA::MatrixManipulator > >, osgGA::KeySwitchMatrixManipulator::NamedManipulator);

TYPE_NAME_ALIAS(std::map< int COMMA  osgGA::KeySwitchMatrixManipulator::NamedManipulator >, osgGA::KeySwitchMatrixManipulator::KeyManipMap);

BEGIN_OBJECT_REFLECTOR(osgGA::KeySwitchMatrixManipulator)
	I_BaseType(osgGA::MatrixManipulator);
	I_Constructor0();
	I_Method0(const char *, className);
	I_Method3(void, addMatrixManipulator, IN, int, key, IN, std::string, name, IN, osgGA::MatrixManipulator *, cm);
	I_Method1(void, addNumberedMatrixManipulator, IN, osgGA::MatrixManipulator *, cm);
	I_Method0(unsigned int, getNumMatrixManipulators);
	I_Method1(void, selectMatrixManipulator, IN, unsigned int, num);
	I_Method0(osgGA::KeySwitchMatrixManipulator::KeyManipMap &, getKeyManipMap);
	I_Method0(const osgGA::KeySwitchMatrixManipulator::KeyManipMap &, getKeyManipMap);
	I_Method0(osgGA::MatrixManipulator *, getCurrentMatrixManipulator);
	I_Method0(const osgGA::MatrixManipulator *, getCurrentMatrixManipulator);
	I_Method1(osgGA::MatrixManipulator *, getMatrixManipulatorWithIndex, IN, unsigned int, key);
	I_Method1(const osgGA::MatrixManipulator *, getMatrixManipulatorWithIndex, IN, unsigned int, key);
	I_Method1(osgGA::MatrixManipulator *, getMatrixManipulatorWithKey, IN, unsigned int, key);
	I_Method1(const osgGA::MatrixManipulator *, getMatrixManipulatorWithKey, IN, unsigned int, key);
	I_Method1(void, setMinimumDistance, IN, float, minimumDistance);
	I_Method1(void, setCoordinateFrameCallback, IN, osgGA::MatrixManipulator::CoordinateFrameCallback *, cb);
	I_Method1(void, setByMatrix, IN, const osg::Matrixd &, matrix);
	I_Method1(void, setByInverseMatrix, IN, const osg::Matrixd &, matrix);
	I_Method0(osg::Matrixd, getMatrix);
	I_Method0(osg::Matrixd, getInverseMatrix);
	I_Method0(osgUtil::SceneView::FusionDistanceMode, getFusionDistanceMode);
	I_Method0(float, getFusionDistanceValue);
	I_Method1(void, setNode, IN, osg::Node *, n);
	I_Method0(const osg::Node *, getNode);
	I_Method0(osg::Node *, getNode);
	I_MethodWithDefaults4(void, setHomePosition, IN, const osg::Vec3d &, eye, , IN, const osg::Vec3d &, center, , IN, const osg::Vec3d &, up, , IN, bool, autoComputeHomePosition, false);
	I_Method1(void, setAutoComputeHomePosition, IN, bool, flag);
	I_Method0(void, computeHomePosition);
	I_Method2(void, home, IN, const osgGA::GUIEventAdapter &, ee, IN, osgGA::GUIActionAdapter &, aa);
	I_Method2(void, init, IN, const osgGA::GUIEventAdapter &, ee, IN, osgGA::GUIActionAdapter &, aa);
	I_Method2(bool, handle, IN, const osgGA::GUIEventAdapter &, ea, IN, osgGA::GUIActionAdapter &, us);
	I_Method1(void, getUsage, IN, osg::ApplicationUsage &, usage);
	I_WriteOnlyProperty(bool, AutoComputeHomePosition);
	I_WriteOnlyProperty(const osg::Matrixd &, ByInverseMatrix);
	I_WriteOnlyProperty(const osg::Matrixd &, ByMatrix);
	I_WriteOnlyProperty(osgGA::MatrixManipulator::CoordinateFrameCallback *, CoordinateFrameCallback);
	I_ReadOnlyProperty(osgGA::MatrixManipulator *, CurrentMatrixManipulator);
	I_ReadOnlyProperty(osgUtil::SceneView::FusionDistanceMode, FusionDistanceMode);
	I_ReadOnlyProperty(float, FusionDistanceValue);
	I_ReadOnlyProperty(osg::Matrixd, InverseMatrix);
	I_ReadOnlyProperty(osgGA::KeySwitchMatrixManipulator::KeyManipMap &, KeyManipMap);
	I_ReadOnlyProperty(osg::Matrixd, Matrix);
	I_WriteOnlyProperty(float, MinimumDistance);
	I_Property(osg::Node *, Node);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ref_ptr< osgGA::MatrixManipulator >)
	I_Constructor0();
	I_Constructor1(IN, osgGA::MatrixManipulator *, ptr);
	I_Constructor1(IN, const osg::ref_ptr< osgGA::MatrixManipulator > &, rp);
	I_Method0(osgGA::MatrixManipulator *, get);
	I_Method0(bool, valid);
	I_Method0(osgGA::MatrixManipulator *, release);
	I_Method1(void, swap, IN, osg::ref_ptr< osgGA::MatrixManipulator > &, rp);
	I_ReadOnlyProperty(osgGA::MatrixManipulator *, );
END_REFLECTOR

STD_MAP_REFLECTOR(std::map< int COMMA  osgGA::KeySwitchMatrixManipulator::NamedManipulator >);

STD_PAIR_REFLECTOR(std::pair< std::string COMMA  osg::ref_ptr< osgGA::MatrixManipulator > >);

