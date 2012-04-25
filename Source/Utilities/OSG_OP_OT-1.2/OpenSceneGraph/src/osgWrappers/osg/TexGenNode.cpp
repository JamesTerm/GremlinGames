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

#include <osg/CopyOp>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osg/TexGen>
#include <osg/TexGenNode>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ENUM_REFLECTOR(osg::TexGenNode::ReferenceFrame)
	I_EnumLabel(osg::TexGenNode::RELATIVE_RF);
	I_EnumLabel(osg::TexGenNode::ABSOLUTE_RF);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TexGenNode)
	I_BaseType(osg::Group);
	I_Constructor0();
	I_Constructor1(IN, osg::TexGen *, texgen);
	I_ConstructorWithDefaults2(IN, const osg::TexGenNode &, tgb, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, className);
	I_Method0(const char *, libraryName);
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv);
	I_Method1(void, setReferenceFrame, IN, osg::TexGenNode::ReferenceFrame, rf);
	I_Method0(osg::TexGenNode::ReferenceFrame, getReferenceFrame);
	I_Method1(void, setTextureUnit, IN, unsigned int, textureUnit);
	I_Method0(unsigned int, getTextureUnit);
	I_Method1(void, setTexGen, IN, osg::TexGen *, texgen);
	I_Method0(osg::TexGen *, getTexGen);
	I_Method0(const osg::TexGen *, getTexGen);
	I_Property(osg::TexGenNode::ReferenceFrame, ReferenceFrame);
	I_Property(osg::TexGen *, TexGen);
	I_Property(unsigned int, TextureUnit);
END_REFLECTOR
