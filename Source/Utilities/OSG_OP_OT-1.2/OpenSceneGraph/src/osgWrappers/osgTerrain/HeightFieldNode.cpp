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
#include <osg/Image>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osg/Shape>
#include <osgTerrain/HeightFieldNode>
#include <osgTerrain/HeightFieldRenderer>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgTerrain::HeightFieldNode)
	I_BaseType(osg::Group);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osgTerrain::HeightFieldNode &, x, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, className);
	I_Method0(const char *, libraryName);
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv);
	I_Method1(void, traverse, IN, osg::NodeVisitor &, nv);
	I_Method1(void, setHeightField, IN, osg::HeightField *, heightField);
	I_Method0(osg::HeightField *, getHeightField);
	I_Method0(const osg::HeightField *, getHeightField);
	I_Method0(void, heightFieldHasBeenModified);
	I_Method1(void, setRenderer, IN, osgTerrain::HeightFieldRenderer *, renderer);
	I_Method0(osgTerrain::HeightFieldRenderer *, getRenderer);
	I_Method0(const osgTerrain::HeightFieldRenderer *, getRenderer);
	I_Method1(void, setBaseTextureImage, IN, osg::Image *, image);
	I_Method0(osg::Image *, getBaseTextureImage);
	I_Method0(const osg::Image *, getBaseTextureImage);
	I_Method1(void, setDetailTextureImage, IN, osg::Image *, image);
	I_Method0(osg::Image *, getDetailTextureImage);
	I_Method0(const osg::Image *, getDetailTextureImage);
	I_Method1(void, setCloudShadowTextureImage, IN, osg::Image *, image);
	I_Method0(osg::Image *, getCloudShadowTextureImage);
	I_Method0(const osg::Image *, getCloudShadowTextureImage);
	I_Method1(void, setNormalMapImage, IN, osg::Image *, image);
	I_Method0(osg::Image *, getNormalMapImage);
	I_Method0(const osg::Image *, getNormalMapImage);
	I_Method0(void, computeNormalMap);
	I_Property(osg::Image *, BaseTextureImage);
	I_Property(osg::Image *, CloudShadowTextureImage);
	I_Property(osg::Image *, DetailTextureImage);
	I_Property(osg::HeightField *, HeightField);
	I_Property(osg::Image *, NormalMapImage);
	I_Property(osgTerrain::HeightFieldRenderer *, Renderer);
END_REFLECTOR
