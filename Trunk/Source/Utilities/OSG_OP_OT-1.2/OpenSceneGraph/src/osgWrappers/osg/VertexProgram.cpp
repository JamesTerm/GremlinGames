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
#include <osg/Matrix>
#include <osg/Object>
#include <osg/State>
#include <osg/StateAttribute>
#include <osg/Vec4>
#include <osg/VertexProgram>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(std::map< GLuint COMMA  osg::Vec4 >, osg::VertexProgram::LocalParamList);

TYPE_NAME_ALIAS(std::map< GLenum COMMA  osg::Matrix >, osg::VertexProgram::MatrixList);

BEGIN_OBJECT_REFLECTOR(osg::VertexProgram)
	I_BaseType(osg::StateAttribute);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::VertexProgram &, vp, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method0(osg::StateAttribute::Type, getType);
	I_Method1(int, compare, IN, const osg::StateAttribute &, sa);
	I_Method1(bool, getModeUsage, IN, osg::StateAttribute::ModeUsage &, usage);
	I_Method1(GLuint &, getVertexProgramID, IN, unsigned int, contextID);
	I_Method1(void, setVertexProgram, IN, const char *, program);
	I_Method1(void, setVertexProgram, IN, const std::string &, program);
	I_Method0(const std::string &, getVertexProgram);
	I_Method2(void, setProgramLocalParameter, IN, const GLuint, index, IN, const osg::Vec4 &, p);
	I_Method1(void, setLocalParameters, IN, const osg::VertexProgram::LocalParamList &, lpl);
	I_Method0(osg::VertexProgram::LocalParamList &, getLocalParameters);
	I_Method0(const osg::VertexProgram::LocalParamList &, getLocalParameters);
	I_Method2(void, setMatrix, IN, const GLenum, mode, IN, const osg::Matrix &, matrix);
	I_Method1(void, setMatrices, IN, const osg::VertexProgram::MatrixList &, matrices);
	I_Method0(osg::VertexProgram::MatrixList &, getMatrices);
	I_Method0(const osg::VertexProgram::MatrixList &, getMatrices);
	I_Method0(void, dirtyVertexProgramObject);
	I_Method1(void, apply, IN, osg::State &, state);
	I_Method1(void, compileGLObjects, IN, osg::State &, state);
	I_MethodWithDefaults1(void, releaseGLObjects, IN, osg::State *, state, 0);
	I_StaticMethod2(void, deleteVertexProgramObject, IN, unsigned int, contextID, IN, GLuint, handle);
	I_StaticMethod3(void, flushDeletedVertexProgramObjects, IN, unsigned int, contextID, IN, double, currentTime, IN, double &, availableTime);
	I_StaticMethod2(osg::VertexProgram::Extensions *, getExtensions, IN, unsigned int, contextID, IN, bool, createIfNotInitalized);
	I_StaticMethod2(void, setExtensions, IN, unsigned int, contextID, IN, osg::VertexProgram::Extensions *, extensions);
	I_Property(const osg::VertexProgram::LocalParamList &, LocalParameters);
	I_Property(const osg::VertexProgram::MatrixList &, Matrices);
	I_ReadOnlyProperty(osg::StateAttribute::Type, Type);
	I_Property(const std::string &, VertexProgram);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::VertexProgram::Extensions)
	I_BaseType(osg::Referenced);
	I_Constructor1(IN, unsigned int, contextID);
	I_Constructor1(IN, const osg::VertexProgram::Extensions &, rhs);
	I_Method1(void, lowestCommonDenominator, IN, const osg::VertexProgram::Extensions &, rhs);
	I_Method1(void, setupGLExtenions, IN, unsigned int, contextID);
	I_Method1(void, setVertexProgramSupported, IN, bool, flag);
	I_Method0(bool, isVertexProgramSupported);
	I_Method2(void, glBindProgram, IN, GLenum, target, IN, GLuint, id);
	I_Method2(void, glGenPrograms, IN, GLsizei, n, IN, GLuint *, programs);
	I_Method2(void, glDeletePrograms, IN, GLsizei, n, IN, GLuint *, programs);
	I_Method4(void, glProgramString, IN, GLenum, target, IN, GLenum, format, IN, GLsizei, len, IN, const void *, string);
	I_Method3(void, glProgramLocalParameter4fv, IN, GLenum, target, IN, GLuint, index, IN, const GLfloat *, params);
	I_WriteOnlyProperty(bool, VertexProgramSupported);
	I_WriteOnlyProperty(unsigned int, upGLExtenions);
END_REFLECTOR
