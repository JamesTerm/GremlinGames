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

#include <osg/Drawable>
#include <osg/Matrix>
#include <osg/State>
#include <osgUtil/RenderLeaf>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

#include <osgUtil/StateGraph>
	
BEGIN_OBJECT_REFLECTOR(osgUtil::RenderLeaf)
	I_BaseType(osg::Referenced);
	I_ConstructorWithDefaults4(IN, osg::Drawable *, drawable, , IN, osg::RefMatrix *, projection, , IN, osg::RefMatrix *, modelview, , IN, float, depth, 0.0f);
	I_MethodWithDefaults4(void, set, IN, osg::Drawable *, drawable, , IN, osg::RefMatrix *, projection, , IN, osg::RefMatrix *, modelview, , IN, float, depth, 0.0f);
	I_Method0(void, reset);
	I_Method2(void, render, IN, osg::State &, state, IN, osgUtil::RenderLeaf *, previous);
	I_PublicMemberProperty(osgUtil::StateGraph *, _parent);
	I_PublicMemberProperty(osg::Drawable *, _drawable);
	I_PublicMemberProperty(osg::ref_ptr< osg::RefMatrix >, _projection);
	I_PublicMemberProperty(osg::ref_ptr< osg::RefMatrix >, _modelview);
	I_PublicMemberProperty(float, _depth);
END_REFLECTOR

