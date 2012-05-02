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

#include <osg/FrameStamp>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_OBJECT_REFLECTOR(osg::FrameStamp)
	I_BaseType(osg::Referenced);
	I_Constructor0();
	I_Constructor1(IN, const osg::FrameStamp &, fs);
	I_Method1(void, setFrameNumber, IN, int, fnum);
	I_Method0(int, getFrameNumber);
	I_Method1(void, setReferenceTime, IN, double, refTime);
	I_Method0(double, getReferenceTime);
	I_Method1(void, setCalendarTime, IN, const tm &, calendarTime);
	I_Method1(void, getCalendarTime, IN, tm &, calendarTime);
	I_WriteOnlyProperty(const tm &, CalendarTime);
	I_Property(int, FrameNumber);
	I_Property(double, ReferenceTime);
END_REFLECTOR
