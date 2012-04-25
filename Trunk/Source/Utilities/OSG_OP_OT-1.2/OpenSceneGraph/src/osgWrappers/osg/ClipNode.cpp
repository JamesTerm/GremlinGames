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

#include <osg/BoundingBox>
#include <osg/BoundingSphere>
#include <osg/ClipNode>
#include <osg/ClipPlane>
#include <osg/CopyOp>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osg/StateAttribute>
#include <osg/StateSet>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::vector< osg::ref_ptr< osg::ClipPlane > >, osg::ClipNode::ClipPlaneList);

BEGIN_OBJECT_REFLECTOR(osg::ClipNode)
	I_BaseType(osg::Group);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::ClipNode &, es, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, className);
	I_Method0(const char *, libraryName);
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv);
	I_MethodWithDefaults2(void, createClipBox, IN, const osg::BoundingBox &, bb, , IN, unsigned int, clipPlaneNumberBase, 0);
	I_Method1(bool, addClipPlane, IN, osg::ClipPlane *, clipplane);
	I_Method1(bool, removeClipPlane, IN, osg::ClipPlane *, clipplane);
	I_Method1(bool, removeClipPlane, IN, unsigned int, pos);
	I_Method0(unsigned int, getNumClipPlanes);
	I_Method1(osg::ClipPlane *, getClipPlane, IN, unsigned int, pos);
	I_Method1(const osg::ClipPlane *, getClipPlane, IN, unsigned int, pos);
	I_Method1(void, getClipPlaneList, IN, const osg::ClipNode::ClipPlaneList &, cpl);
	I_Method0(osg::ClipNode::ClipPlaneList &, getClipPlaneList);
	I_Method0(const osg::ClipNode::ClipPlaneList &, getClipPlaneList);
	I_Method2(void, setStateSetModes, IN, osg::StateSet &, x, IN, osg::StateAttribute::GLModeValue, x);
	I_MethodWithDefaults1(void, setLocalStateSetModes, IN, osg::StateAttribute::GLModeValue, x, osg::StateAttribute::ON);
	I_Method0(osg::BoundingSphere, computeBound);
	I_ArrayProperty_GA(osg::ClipPlane *, ClipPlane, ClipPlanes, unsigned int, bool);
	I_ReadOnlyProperty(osg::ClipNode::ClipPlaneList &, ClipPlaneList);
	I_WriteOnlyProperty(osg::StateAttribute::GLModeValue, LocalStateSetModes);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ref_ptr< osg::ClipPlane >)
	I_Constructor0();
	I_Constructor1(IN, osg::ClipPlane *, ptr);
	I_Constructor1(IN, const osg::ref_ptr< osg::ClipPlane > &, rp);
	I_Method0(osg::ClipPlane *, get);
	I_Method0(bool, valid);
	I_Method0(osg::ClipPlane *, release);
	I_Method1(void, swap, IN, osg::ref_ptr< osg::ClipPlane > &, rp);
	I_ReadOnlyProperty(osg::ClipPlane *, );
END_REFLECTOR

STD_VECTOR_REFLECTOR(std::vector< osg::ref_ptr< osg::ClipPlane > >);
