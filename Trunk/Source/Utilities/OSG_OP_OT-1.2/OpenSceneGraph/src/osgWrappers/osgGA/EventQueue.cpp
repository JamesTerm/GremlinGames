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

#include <osg/Timer>
#include <osgGA/EventQueue>
#include <osgGA/GUIEventAdapter>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::list< osg::ref_ptr< osgGA::GUIEventAdapter > >, osgGA::EventQueue::Events);

BEGIN_OBJECT_REFLECTOR(osgGA::EventQueue)
	I_BaseType(osg::Referenced);
	I_ConstructorWithDefaults1(IN, osgGA::GUIEventAdapter::MouseYOrientation, mouseYOrientation, osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS);
	I_Method1(void, setEvents, IN, osgGA::EventQueue::Events &, events);
	I_Method1(bool, takeEvents, IN, osgGA::EventQueue::Events &, events);
	I_Method1(bool, copyEvents, IN, osgGA::EventQueue::Events &, events);
	I_Method1(void, appendEvents, IN, osgGA::EventQueue::Events &, events);
	I_Method1(void, addEvent, IN, osgGA::GUIEventAdapter *, event);
	I_Method4(void, windowResize, IN, float, Xmin, IN, float, Ymin, IN, float, Xmax, IN, float, Ymax);
	I_Method1(void, mouseScroll, IN, osgGA::GUIEventAdapter::ScrollingMotion, sm);
	I_Method2(void, mouseScroll2D, IN, float, x, IN, float, y);
	I_Method1(void, penPressure, IN, float, pressure);
	I_Method2(void, penProximity, IN, osgGA::GUIEventAdapter::TabletPointerType, pt, IN, bool, isEntering);
	I_Method2(void, mouseWarp, IN, float, x, IN, float, y);
	I_Method2(void, mouseMotion, IN, float, x, IN, float, y);
	I_Method3(void, mouseButtonPress, IN, float, x, IN, float, y, IN, unsigned int, button);
	I_Method3(void, mouseDoubleButtonPress, IN, float, x, IN, float, y, IN, unsigned int, button);
	I_Method3(void, mouseButtonRelease, IN, float, x, IN, float, y, IN, unsigned int, button);
	I_Method1(void, keyPress, IN, osgGA::GUIEventAdapter::KeySymbol, key);
	I_Method1(void, keyRelease, IN, osgGA::GUIEventAdapter::KeySymbol, key);
	I_Method1(void, frame, IN, double, t);
	I_Method1(void, setStartTick, IN, osg::Timer_t, tick);
	I_Method0(osg::Timer_t, getStartTick);
	I_Method0(double, getTime);
	I_Method0(osgGA::GUIEventAdapter *, createEvent);
	I_Method0(osgGA::GUIEventAdapter *, getCurrentEventState);
	I_Method0(const osgGA::GUIEventAdapter *, getCurrentEventState);
	I_ReadOnlyProperty(osgGA::GUIEventAdapter *, CurrentEventState);
	I_WriteOnlyProperty(osgGA::EventQueue::Events &, Events);
	I_Property(osg::Timer_t, StartTick);
	I_ReadOnlyProperty(double, Time);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ref_ptr< osgGA::GUIEventAdapter >)
	I_Constructor0();
	I_Constructor1(IN, osgGA::GUIEventAdapter *, ptr);
	I_Constructor1(IN, const osg::ref_ptr< osgGA::GUIEventAdapter > &, rp);
	I_Method0(osgGA::GUIEventAdapter *, get);
	I_Method0(bool, valid);
	I_Method0(osgGA::GUIEventAdapter *, release);
	I_Method1(void, swap, IN, osg::ref_ptr< osgGA::GUIEventAdapter > &, rp);
	I_ReadOnlyProperty(osgGA::GUIEventAdapter *, );
END_REFLECTOR

STD_LIST_REFLECTOR(std::list< osg::ref_ptr< osgGA::GUIEventAdapter > >);
