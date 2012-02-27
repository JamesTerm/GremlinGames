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
#include <osg/Object>
#include <osg/Shader>
#include <osg/State>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ENUM_REFLECTOR(osg::Shader::Type)
	I_EnumLabel(osg::Shader::VERTEX);
	I_EnumLabel(osg::Shader::FRAGMENT);
	I_EnumLabel(osg::Shader::UNDEFINED);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::Shader)
	I_BaseType(osg::Object);
	I_ConstructorWithDefaults1(IN, osg::Shader::Type, type, osg::Shader::UNDEFINED);
	I_Constructor2(IN, osg::Shader::Type, type, IN, const std::string &, source);
	I_ConstructorWithDefaults2(IN, const osg::Shader &, rhs, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method1(int, compare, IN, const osg::Shader &, rhs);
	I_Method1(bool, setType, IN, osg::Shader::Type, t);
	I_Method1(void, setShaderSource, IN, const std::string &, sourceText);
	I_Method1(bool, loadShaderSourceFromFile, IN, const std::string &, fileName);
	I_Method0(const std::string &, getShaderSource);
	I_Method0(osg::Shader::Type, getType);
	I_Method0(const char *, getTypename);
	I_MethodWithDefaults1(void, releaseGLObjects, IN, osg::State *, state, 0);
	I_Method0(void, dirtyShader);
	I_Method1(void, compileShader, IN, unsigned int, contextID);
	I_Method2(void, attachShader, IN, unsigned int, contextID, IN, GLuint, program);
	I_Method2(bool, getGlShaderInfoLog, IN, unsigned int, contextID, IN, std::string &, log);
	I_StaticMethod2(osg::Shader *, readShaderFile, IN, osg::Shader::Type, type, IN, const std::string &, fileName);
	I_StaticMethod2(void, deleteGlShader, IN, unsigned int, contextID, IN, GLuint, shader);
	I_StaticMethod3(void, flushDeletedGlShaders, IN, unsigned int, contextID, IN, double, currentTime, IN, double &, availableTime);
	I_StaticMethod1(osg::Shader::Type, getTypeId, IN, const std::string &, tname);
	I_Property(const std::string &, ShaderSource);
	I_PropertyWithReturnType(osg::Shader::Type, Type, bool);
	I_ReadOnlyProperty(const char *, Typename);
END_REFLECTOR
