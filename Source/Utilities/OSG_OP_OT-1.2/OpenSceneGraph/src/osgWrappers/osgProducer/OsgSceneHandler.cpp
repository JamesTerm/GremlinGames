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

#include <osg/DisplaySettings>
#include <osgProducer/OsgSceneHandler>
#include <osgUtil/SceneView>
#include <osgUtil/Statistics>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osgProducer::OsgSceneHandler)
	I_ConstructorWithDefaults1(IN, osg::DisplaySettings *, ds, NULL);
	I_Method1(void, setSceneView, IN, osgUtil::SceneView *, sceneView);
	I_Method0(osgUtil::SceneView *, getSceneView);
	I_Method0(const osgUtil::SceneView *, getSceneView);
	I_Method0(void, init);
	I_Method1(void, clear, IN, Producer::Camera &, camera);
	I_Method1(void, clearImplementation, IN, Producer::Camera &, camera);
	I_Method1(void, setClearCallback, IN, osgProducer::OsgSceneHandler::Callback *, callback);
	I_Method0(osgProducer::OsgSceneHandler::Callback *, getClearCallback);
	I_Method0(const osgProducer::OsgSceneHandler::Callback *, getClearCallback);
	I_Method1(void, cull, IN, Producer::Camera &, camera);
	I_Method1(void, cullImplementation, IN, Producer::Camera &, camera);
	I_Method1(void, setCullCallback, IN, osgProducer::OsgSceneHandler::Callback *, callback);
	I_Method0(osgProducer::OsgSceneHandler::Callback *, getCullCallback);
	I_Method0(const osgProducer::OsgSceneHandler::Callback *, getCullCallback);
	I_Method1(void, draw, IN, Producer::Camera &, camera);
	I_Method1(void, drawImplementation, IN, Producer::Camera &, camera);
	I_Method1(void, setDrawCallback, IN, osgProducer::OsgSceneHandler::Callback *, callback);
	I_Method0(osgProducer::OsgSceneHandler::Callback *, getDrawCallback);
	I_Method0(const osgProducer::OsgSceneHandler::Callback *, getDrawCallback);
	I_Method1(void, setCollectStats, IN, bool, collectStats);
	I_Method0(bool, getCollectStats);
	I_Method1(bool, getStats, IN, osgUtil::Statistics &, primStats);
	I_Method1(void, setContextID, IN, int, id);
	I_Method1(void, setFlushOfAllDeletedGLObjectsOnNextFrame, IN, bool, flag);
	I_Method0(bool, getFlushOfAllDeletedGLObjectsOnNextFrame);
	I_Method1(void, setCleanUpOnNextFrame, IN, bool, flag);
	I_Method0(bool, getCleanUpOnNextFrame);
	I_Property(bool, CleanUpOnNextFrame);
	I_Property(osgProducer::OsgSceneHandler::Callback *, ClearCallback);
	I_Property(bool, CollectStats);
	I_WriteOnlyProperty(int, ContextID);
	I_Property(osgProducer::OsgSceneHandler::Callback *, CullCallback);
	I_Property(osgProducer::OsgSceneHandler::Callback *, DrawCallback);
	I_Property(bool, FlushOfAllDeletedGLObjectsOnNextFrame);
	I_Property(osgUtil::SceneView *, SceneView);
END_REFLECTOR

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osgProducer::OsgSceneHandler::Callback)
	I_BaseType(osg::Referenced);
	I_Constructor0();
END_REFLECTOR
