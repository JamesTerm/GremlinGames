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
#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/Object>
#include <osgSim/MultiSwitch>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::vector< bool >, osgSim::MultiSwitch::ValueList);

TYPE_NAME_ALIAS(std::vector< osgSim::MultiSwitch::ValueList >, osgSim::MultiSwitch::SwitchSetList);

BEGIN_OBJECT_REFLECTOR(osgSim::MultiSwitch)
	I_BaseType(osg::Group);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osgSim::MultiSwitch &, x, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, className);
	I_Method0(const char *, libraryName);
	I_Method1(void, accept, IN, osg::NodeVisitor &, nv);
	I_Method1(void, traverse, IN, osg::NodeVisitor &, nv);
	I_Method1(void, setNewChildDefaultValue, IN, bool, value);
	I_Method0(bool, getNewChildDefaultValue);
	I_Method1(bool, addChild, IN, osg::Node *, child);
	I_Method2(bool, insertChild, IN, unsigned int, index, IN, osg::Node *, child);
	I_Method1(bool, removeChild, IN, osg::Node *, child);
	I_Method3(void, setValue, IN, unsigned int, switchSet, IN, unsigned int, pos, IN, bool, value);
	I_Method2(bool, getValue, IN, unsigned int, switchSet, IN, unsigned int, pos);
	I_Method3(void, setChildValue, IN, const osg::Node *, child, IN, unsigned int, switchSet, IN, bool, value);
	I_Method2(bool, getChildValue, IN, const osg::Node *, child, IN, unsigned int, switchSet);
	I_Method1(bool, setAllChildrenOff, IN, unsigned int, switchSet);
	I_Method1(bool, setAllChildrenOn, IN, unsigned int, switchSet);
	I_Method2(bool, setSingleChildOn, IN, unsigned int, switchSet, IN, unsigned int, pos);
	I_Method1(void, setActiveSwitchSet, IN, unsigned int, switchSet);
	I_Method0(unsigned int, getActiveSwitchSet);
	I_Method1(void, setSwitchSetList, IN, const osgSim::MultiSwitch::SwitchSetList &, switchSetList);
	I_Method0(const osgSim::MultiSwitch::SwitchSetList &, getSwitchSetList);
	I_Method2(void, setValueList, IN, unsigned int, switchSet, IN, const osgSim::MultiSwitch::ValueList &, values);
	I_Method1(const osgSim::MultiSwitch::ValueList &, getValueList, IN, unsigned int, switchSet);
	I_Property(unsigned int, ActiveSwitchSet);
	I_WriteOnlyPropertyWithReturnType(unsigned int, AllChildrenOff, bool);
	I_WriteOnlyPropertyWithReturnType(unsigned int, AllChildrenOn, bool);
	I_IndexedProperty2(bool, ChildValue, const osg::Node *, child, unsigned int, switchSet);
	I_Property(bool, NewChildDefaultValue);
	I_Property(const osgSim::MultiSwitch::SwitchSetList &, SwitchSetList);
	I_IndexedProperty2(bool, Value, unsigned int, switchSet, unsigned int, pos);
	I_IndexedProperty1(const osgSim::MultiSwitch::ValueList &, ValueList, unsigned int, switchSet);
END_REFLECTOR

STD_VECTOR_REFLECTOR(std::vector< osgSim::MultiSwitch::ValueList >);

