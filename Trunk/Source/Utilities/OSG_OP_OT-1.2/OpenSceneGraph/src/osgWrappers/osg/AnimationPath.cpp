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

#include <osg/AnimationPath>
#include <osg/CopyOp>
#include <osg/Matrixd>
#include <osg/Matrixf>
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osg/Quat>
#include <osg/Vec3d>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::map< double COMMA  osg::AnimationPath::ControlPoint >, osg::AnimationPath::TimeControlPointMap);

BEGIN_ENUM_REFLECTOR(osg::AnimationPath::LoopMode)
	I_EnumLabel(osg::AnimationPath::SWING);
	I_EnumLabel(osg::AnimationPath::LOOP);
	I_EnumLabel(osg::AnimationPath::NO_LOOPING);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::AnimationPath)
	I_VirtualBaseType(osg::Object);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::AnimationPath &, ap, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method2(bool, getMatrix, IN, double, time, IN, osg::Matrixf &, matrix);
	I_Method2(bool, getMatrix, IN, double, time, IN, osg::Matrixd &, matrix);
	I_Method2(bool, getInverse, IN, double, time, IN, osg::Matrixf &, matrix);
	I_Method2(bool, getInverse, IN, double, time, IN, osg::Matrixd &, matrix);
	I_Method2(bool, getInterpolatedControlPoint, IN, double, time, IN, osg::AnimationPath::ControlPoint &, controlPoint);
	I_Method2(void, insert, IN, double, time, IN, const osg::AnimationPath::ControlPoint &, controlPoint);
	I_Method0(double, getFirstTime);
	I_Method0(double, getLastTime);
	I_Method0(double, getPeriod);
	I_Method1(void, setLoopMode, IN, osg::AnimationPath::LoopMode, lm);
	I_Method0(osg::AnimationPath::LoopMode, getLoopMode);
	I_Method1(void, setTimeControlPointMap, IN, osg::AnimationPath::TimeControlPointMap &, tcpm);
	I_Method0(osg::AnimationPath::TimeControlPointMap &, getTimeControlPointMap);
	I_Method0(const osg::AnimationPath::TimeControlPointMap &, getTimeControlPointMap);
	I_Method0(bool, empty);
	I_Method1(void, read, IN, std::istream &, in);
	I_Method1(void, write, IN, std::ostream &, out);
	I_ReadOnlyProperty(double, FirstTime);
	I_ReadOnlyProperty(double, LastTime);
	I_Property(osg::AnimationPath::LoopMode, LoopMode);
	I_ReadOnlyProperty(double, Period);
	I_Property(osg::AnimationPath::TimeControlPointMap &, TimeControlPointMap);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::AnimationPath::ControlPoint)
	I_Constructor0();
	I_Constructor1(IN, const osg::Vec3d &, position);
	I_Constructor2(IN, const osg::Vec3d &, position, IN, const osg::Quat &, rotation);
	I_Constructor3(IN, const osg::Vec3d &, position, IN, const osg::Quat &, rotation, IN, const osg::Vec3d &, scale);
	I_Method1(void, setPosition, IN, const osg::Vec3d &, position);
	I_Method0(const osg::Vec3d &, getPosition);
	I_Method1(void, setRotation, IN, const osg::Quat &, rotation);
	I_Method0(const osg::Quat &, getRotation);
	I_Method1(void, setScale, IN, const osg::Vec3d &, scale);
	I_Method0(const osg::Vec3d &, getScale);
	I_Method3(void, interpolate, IN, float, ratio, IN, const osg::AnimationPath::ControlPoint &, first, IN, const osg::AnimationPath::ControlPoint &, second);
	I_Method3(void, interpolate, IN, double, ratio, IN, const osg::AnimationPath::ControlPoint &, first, IN, const osg::AnimationPath::ControlPoint &, second);
	I_Method1(void, getMatrix, IN, osg::Matrixf &, matrix);
	I_Method1(void, getMatrix, IN, osg::Matrixd &, matrix);
	I_Method1(void, getInverse, IN, osg::Matrixf &, matrix);
	I_Method1(void, getInverse, IN, osg::Matrixd &, matrix);
	I_Property(const osg::Vec3d &, Position);
	I_Property(const osg::Quat &, Rotation);
	I_Property(const osg::Vec3d &, Scale);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::AnimationPathCallback)
	I_BaseType(osg::NodeCallback);
	I_Constructor0();
	I_Constructor2(IN, const osg::AnimationPathCallback &, apc, IN, const osg::CopyOp &, copyop);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_ConstructorWithDefaults3(IN, osg::AnimationPath *, ap, , IN, double, timeOffset, 0.0, IN, double, timeMultiplier, 1.0);
	I_Constructor3(IN, const osg::Vec3d &, pivot, IN, const osg::Vec3d &, axis, IN, float, angularVelocity);
	I_Method1(void, setAnimationPath, IN, osg::AnimationPath *, path);
	I_Method0(osg::AnimationPath *, getAnimationPath);
	I_Method0(const osg::AnimationPath *, getAnimationPath);
	I_Method1(void, setPivotPoint, IN, const osg::Vec3d &, pivot);
	I_Method0(const osg::Vec3d &, getPivotPoint);
	I_Method1(void, setUseInverseMatrix, IN, bool, useInverseMatrix);
	I_Method0(bool, getUseInverseMatrix);
	I_Method1(void, setTimeOffset, IN, double, offset);
	I_Method0(double, getTimeOffset);
	I_Method1(void, setTimeMultiplier, IN, double, multiplier);
	I_Method0(double, getTimeMultiplier);
	I_Method0(void, reset);
	I_Method1(void, setPause, IN, bool, pause);
	I_Method0(bool, getPause);
	I_Method0(double, getAnimationTime);
	I_Method1(void, update, IN, osg::Node &, node);
	I_Property(osg::AnimationPath *, AnimationPath);
	I_ReadOnlyProperty(double, AnimationTime);
	I_Property(bool, Pause);
	I_Property(const osg::Vec3d &, PivotPoint);
	I_Property(double, TimeMultiplier);
	I_Property(double, TimeOffset);
	I_Property(bool, UseInverseMatrix);
	I_PublicMemberProperty(osg::ref_ptr< osg::AnimationPath >, _animationPath);
	I_PublicMemberProperty(osg::Vec3d, _pivotPoint);
	I_PublicMemberProperty(bool, _useInverseMatrix);
	I_PublicMemberProperty(double, _timeOffset);
	I_PublicMemberProperty(double, _timeMultiplier);
	I_PublicMemberProperty(double, _firstTime);
	I_PublicMemberProperty(double, _latestTime);
	I_PublicMemberProperty(bool, _pause);
	I_PublicMemberProperty(double, _pauseTime);
END_REFLECTOR

STD_MAP_REFLECTOR(std::map< double COMMA  osg::AnimationPath::ControlPoint >);
