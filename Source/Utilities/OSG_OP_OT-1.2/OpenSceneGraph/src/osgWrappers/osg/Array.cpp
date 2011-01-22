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

#include <osg/Array>
#include <osg/CopyOp>
#include <osg/Object>
#include <osg/Vec2>
#include <osg/Vec2b>
#include <osg/Vec2s>
#include <osg/Vec3>
#include <osg/Vec3b>
#include <osg/Vec3s>
#include <osg/Vec4>
#include <osg/Vec4b>
#include <osg/Vec4s>
#include <osg/Vec4ub>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

BEGIN_ENUM_REFLECTOR(osg::Array::Type)
	I_EnumLabel(osg::Array::ArrayType);
	I_EnumLabel(osg::Array::ByteArrayType);
	I_EnumLabel(osg::Array::ShortArrayType);
	I_EnumLabel(osg::Array::IntArrayType);
	I_EnumLabel(osg::Array::UByteArrayType);
	I_EnumLabel(osg::Array::UShortArrayType);
	I_EnumLabel(osg::Array::UIntArrayType);
	I_EnumLabel(osg::Array::Vec4ubArrayType);
	I_EnumLabel(osg::Array::FloatArrayType);
	I_EnumLabel(osg::Array::Vec2ArrayType);
	I_EnumLabel(osg::Array::Vec3ArrayType);
	I_EnumLabel(osg::Array::Vec4ArrayType);
	I_EnumLabel(osg::Array::Vec2sArrayType);
	I_EnumLabel(osg::Array::Vec3sArrayType);
	I_EnumLabel(osg::Array::Vec4sArrayType);
	I_EnumLabel(osg::Array::Vec2bArrayType);
	I_EnumLabel(osg::Array::Vec3bArrayType);
	I_EnumLabel(osg::Array::Vec4bArrayType);
END_REFLECTOR

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::Array)
	I_BaseType(osg::Object);
	I_ConstructorWithDefaults3(IN, osg::Array::Type, arrayType, osg::Array::ArrayType, IN, GLint, dataSize, 0, IN, GLenum, dataType, 0);
	I_ConstructorWithDefaults2(IN, const osg::Array &, array, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method0(const char *, libraryName);
	I_Method0(const char *, className);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, x);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, x);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, x);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, x);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(osg::Array::Type, getType);
	I_Method0(GLint, getDataSize);
	I_Method0(GLenum, getDataType);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method0(void, trim);
	I_Method0(void, dirty);
	I_Method1(void, setModifiedCount, IN, unsigned int, value);
	I_Method0(unsigned int, getModifiedCount);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(GLint, DataSize);
	I_ReadOnlyProperty(GLenum, DataType);
	I_Property(unsigned int, ModifiedCount);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
	I_ReadOnlyProperty(osg::Array::Type, Type);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ArrayVisitor)
	I_Constructor0();
	I_Method1(void, apply, IN, osg::Array &, x);
	I_Method1(void, apply, IN, osg::ByteArray &, x);
	I_Method1(void, apply, IN, osg::ShortArray &, x);
	I_Method1(void, apply, IN, osg::IntArray &, x);
	I_Method1(void, apply, IN, osg::UByteArray &, x);
	I_Method1(void, apply, IN, osg::UShortArray &, x);
	I_Method1(void, apply, IN, osg::UIntArray &, x);
	I_Method1(void, apply, IN, osg::FloatArray &, x);
	I_Method1(void, apply, IN, osg::Vec2Array &, x);
	I_Method1(void, apply, IN, osg::Vec3Array &, x);
	I_Method1(void, apply, IN, osg::Vec4Array &, x);
	I_Method1(void, apply, IN, osg::Vec4ubArray &, x);
	I_Method1(void, apply, IN, osg::Vec2bArray &, x);
	I_Method1(void, apply, IN, osg::Vec3bArray &, x);
	I_Method1(void, apply, IN, osg::Vec4bArray &, x);
	I_Method1(void, apply, IN, osg::Vec2sArray &, x);
	I_Method1(void, apply, IN, osg::Vec3sArray &, x);
	I_Method1(void, apply, IN, osg::Vec4sArray &, x);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ConstArrayVisitor)
	I_Constructor0();
	I_Method1(void, apply, IN, const osg::Array &, x);
	I_Method1(void, apply, IN, const osg::ByteArray &, x);
	I_Method1(void, apply, IN, const osg::ShortArray &, x);
	I_Method1(void, apply, IN, const osg::IntArray &, x);
	I_Method1(void, apply, IN, const osg::UByteArray &, x);
	I_Method1(void, apply, IN, const osg::UShortArray &, x);
	I_Method1(void, apply, IN, const osg::UIntArray &, x);
	I_Method1(void, apply, IN, const osg::FloatArray &, x);
	I_Method1(void, apply, IN, const osg::Vec2Array &, x);
	I_Method1(void, apply, IN, const osg::Vec3Array &, x);
	I_Method1(void, apply, IN, const osg::Vec4Array &, x);
	I_Method1(void, apply, IN, const osg::Vec4ubArray &, x);
	I_Method1(void, apply, IN, const osg::Vec2bArray &, x);
	I_Method1(void, apply, IN, const osg::Vec3bArray &, x);
	I_Method1(void, apply, IN, const osg::Vec4bArray &, x);
	I_Method1(void, apply, IN, const osg::Vec2sArray &, x);
	I_Method1(void, apply, IN, const osg::Vec3sArray &, x);
	I_Method1(void, apply, IN, const osg::Vec4sArray &, x);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ConstValueVisitor)
	I_Constructor0();
	I_Method1(void, apply, IN, const GLbyte &, x);
	I_Method1(void, apply, IN, const GLshort &, x);
	I_Method1(void, apply, IN, const GLint &, x);
	I_Method1(void, apply, IN, const GLushort &, x);
	I_Method1(void, apply, IN, const GLubyte &, x);
	I_Method1(void, apply, IN, const GLuint &, x);
	I_Method1(void, apply, IN, const GLfloat &, x);
	I_Method1(void, apply, IN, const osg::Vec4ub &, x);
	I_Method1(void, apply, IN, const osg::Vec2 &, x);
	I_Method1(void, apply, IN, const osg::Vec3 &, x);
	I_Method1(void, apply, IN, const osg::Vec4 &, x);
	I_Method1(void, apply, IN, const osg::Vec2b &, x);
	I_Method1(void, apply, IN, const osg::Vec3b &, x);
	I_Method1(void, apply, IN, const osg::Vec4b &, x);
	I_Method1(void, apply, IN, const osg::Vec2s &, x);
	I_Method1(void, apply, IN, const osg::Vec3s &, x);
	I_Method1(void, apply, IN, const osg::Vec4s &, x);
END_REFLECTOR

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::IndexArray)
	I_BaseType(osg::Array);
	I_ConstructorWithDefaults3(IN, osg::Array::Type, arrayType, osg::Array::ArrayType, IN, GLint, dataSize, 0, IN, GLenum, dataType, 0);
	I_ConstructorWithDefaults2(IN, const osg::Array &, array, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Method1(bool, isSameKindAs, IN, const osg::Object *, obj);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
END_REFLECTOR

BEGIN_VALUE_REFLECTOR(osg::ValueVisitor)
	I_Constructor0();
	I_Method1(void, apply, IN, GLbyte &, x);
	I_Method1(void, apply, IN, GLshort &, x);
	I_Method1(void, apply, IN, GLint &, x);
	I_Method1(void, apply, IN, GLushort &, x);
	I_Method1(void, apply, IN, GLubyte &, x);
	I_Method1(void, apply, IN, GLuint &, x);
	I_Method1(void, apply, IN, GLfloat &, x);
	I_Method1(void, apply, IN, osg::Vec2 &, x);
	I_Method1(void, apply, IN, osg::Vec3 &, x);
	I_Method1(void, apply, IN, osg::Vec4 &, x);
	I_Method1(void, apply, IN, osg::Vec4ub &, x);
	I_Method1(void, apply, IN, osg::Vec2b &, x);
	I_Method1(void, apply, IN, osg::Vec3b &, x);
	I_Method1(void, apply, IN, osg::Vec4b &, x);
	I_Method1(void, apply, IN, osg::Vec2s &, x);
	I_Method1(void, apply, IN, osg::Vec3s &, x);
	I_Method1(void, apply, IN, osg::Vec4s &, x);
END_REFLECTOR

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLbyte COMMA  osg::Array::ByteArrayType COMMA  1 COMMA  GL_BYTE >, osg::ByteArray);

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLshort COMMA  osg::Array::ShortArrayType COMMA  1 COMMA  GL_SHORT >, osg::ShortArray);

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLint COMMA  osg::Array::IntArrayType COMMA  1 COMMA  GL_INT >, osg::IntArray);

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLubyte COMMA  osg::Array::UByteArrayType COMMA  1 COMMA  GL_UNSIGNED_BYTE >, osg::UByteArray);

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLushort COMMA  osg::Array::UShortArrayType COMMA  1 COMMA  GL_UNSIGNED_SHORT >, osg::UShortArray);

TYPE_NAME_ALIAS(osg::TemplateIndexArray< GLuint COMMA  osg::Array::UIntArrayType COMMA  1 COMMA  GL_UNSIGNED_INT >, osg::UIntArray);

TYPE_NAME_ALIAS(osg::TemplateArray< GLfloat COMMA  osg::Array::FloatArrayType COMMA  1 COMMA  GL_FLOAT >, osg::FloatArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec2 COMMA  osg::Array::Vec2ArrayType COMMA  2 COMMA  GL_FLOAT >, osg::Vec2Array);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec3 COMMA  osg::Array::Vec3ArrayType COMMA  3 COMMA  GL_FLOAT >, osg::Vec3Array);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec4 COMMA  osg::Array::Vec4ArrayType COMMA  4 COMMA  GL_FLOAT >, osg::Vec4Array);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec4ub COMMA  osg::Array::Vec4ubArrayType COMMA  4 COMMA  GL_UNSIGNED_BYTE >, osg::Vec4ubArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec2s COMMA  osg::Array::Vec2sArrayType COMMA  2 COMMA  GL_SHORT >, osg::Vec2sArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec3s COMMA  osg::Array::Vec3sArrayType COMMA  3 COMMA  GL_SHORT >, osg::Vec3sArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec4s COMMA  osg::Array::Vec4sArrayType COMMA  4 COMMA  GL_SHORT >, osg::Vec4sArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec2b COMMA  osg::Array::Vec2bArrayType COMMA  2 COMMA  GL_BYTE >, osg::Vec2bArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec3b COMMA  osg::Array::Vec3bArrayType COMMA  3 COMMA  GL_BYTE >, osg::Vec3bArray);

TYPE_NAME_ALIAS(osg::TemplateArray< osg::Vec4b COMMA  osg::Array::Vec4bArrayType COMMA  4 COMMA  GL_BYTE >, osg::Vec4bArray);

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< GLfloat COMMA  osg::Array::FloatArrayType COMMA  1 COMMA  GL_FLOAT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<GLfloat>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< GLfloat COMMA  osg::Array::FloatArrayType COMMA  1 COMMA  GL_FLOAT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLfloat *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec2 COMMA  osg::Array::Vec2ArrayType COMMA  2 COMMA  GL_FLOAT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec2>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec2 COMMA  osg::Array::Vec2ArrayType COMMA  2 COMMA  GL_FLOAT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec2 *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec2b COMMA  osg::Array::Vec2bArrayType COMMA  2 COMMA  GL_BYTE >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec2b>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec2b COMMA  osg::Array::Vec2bArrayType COMMA  2 COMMA  GL_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec2b *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec2s COMMA  osg::Array::Vec2sArrayType COMMA  2 COMMA  GL_SHORT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec2s>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec2s COMMA  osg::Array::Vec2sArrayType COMMA  2 COMMA  GL_SHORT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec2s *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec3 COMMA  osg::Array::Vec3ArrayType COMMA  3 COMMA  GL_FLOAT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec3>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec3 COMMA  osg::Array::Vec3ArrayType COMMA  3 COMMA  GL_FLOAT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec3 *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec3b COMMA  osg::Array::Vec3bArrayType COMMA  3 COMMA  GL_BYTE >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec3b>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec3b COMMA  osg::Array::Vec3bArrayType COMMA  3 COMMA  GL_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec3b *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec3s COMMA  osg::Array::Vec3sArrayType COMMA  3 COMMA  GL_SHORT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec3s>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec3s COMMA  osg::Array::Vec3sArrayType COMMA  3 COMMA  GL_SHORT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec3s *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec4 COMMA  osg::Array::Vec4ArrayType COMMA  4 COMMA  GL_FLOAT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec4>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec4 COMMA  osg::Array::Vec4ArrayType COMMA  4 COMMA  GL_FLOAT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec4 *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec4b COMMA  osg::Array::Vec4bArrayType COMMA  4 COMMA  GL_BYTE >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec4b>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec4b COMMA  osg::Array::Vec4bArrayType COMMA  4 COMMA  GL_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec4b *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec4s COMMA  osg::Array::Vec4sArrayType COMMA  4 COMMA  GL_SHORT >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec4s>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec4s COMMA  osg::Array::Vec4sArrayType COMMA  4 COMMA  GL_SHORT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec4s *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateArray< osg::Vec4ub COMMA  osg::Array::Vec4ubArrayType COMMA  4 COMMA  GL_UNSIGNED_BYTE >)
	I_BaseType(osg::Array);
	I_BaseType(std::vector<osg::Vec4ub>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateArray< osg::Vec4ub COMMA  osg::Array::Vec4ubArrayType COMMA  4 COMMA  GL_UNSIGNED_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, osg::Vec4ub *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLbyte COMMA  osg::Array::ByteArrayType COMMA  1 COMMA  GL_BYTE >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLbyte>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLbyte COMMA  osg::Array::ByteArrayType COMMA  1 COMMA  GL_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLbyte *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLint COMMA  osg::Array::IntArrayType COMMA  1 COMMA  GL_INT >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLint>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLint COMMA  osg::Array::IntArrayType COMMA  1 COMMA  GL_INT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLint *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLshort COMMA  osg::Array::ShortArrayType COMMA  1 COMMA  GL_SHORT >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLshort>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLshort COMMA  osg::Array::ShortArrayType COMMA  1 COMMA  GL_SHORT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLshort *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLubyte COMMA  osg::Array::UByteArrayType COMMA  1 COMMA  GL_UNSIGNED_BYTE >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLubyte>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLubyte COMMA  osg::Array::UByteArrayType COMMA  1 COMMA  GL_UNSIGNED_BYTE > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLubyte *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLuint COMMA  osg::Array::UIntArrayType COMMA  1 COMMA  GL_UNSIGNED_INT >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLuint>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLuint COMMA  osg::Array::UIntArrayType COMMA  1 COMMA  GL_UNSIGNED_INT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLuint *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::TemplateIndexArray< GLushort COMMA  osg::Array::UShortArrayType COMMA  1 COMMA  GL_UNSIGNED_SHORT >)
	I_BaseType(osg::IndexArray);
	I_BaseType(std::vector<GLushort>);
	I_Constructor0();
	I_ConstructorWithDefaults2(IN, const osg::TemplateIndexArray< GLushort COMMA  osg::Array::UShortArrayType COMMA  1 COMMA  GL_UNSIGNED_SHORT > &, ta, , IN, const osg::CopyOp &, copyop, osg::CopyOp::SHALLOW_COPY);
	I_Constructor1(IN, unsigned int, no);
	I_Constructor2(IN, unsigned int, no, IN, GLushort *, ptr);
	I_Method0(osg::Object *, cloneType);
	I_Method1(osg::Object *, clone, IN, const osg::CopyOp &, copyop);
	I_Method1(void, accept, IN, osg::ArrayVisitor &, av);
	I_Method1(void, accept, IN, osg::ConstArrayVisitor &, av);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ValueVisitor &, vv);
	I_Method2(void, accept, IN, unsigned int, index, IN, osg::ConstValueVisitor &, vv);
	I_Method2(int, compare, IN, unsigned int, lhs, IN, unsigned int, rhs);
	I_Method0(void, trim);
	I_Method0(const GLvoid *, getDataPointer);
	I_Method0(unsigned int, getTotalDataSize);
	I_Method0(unsigned int, getNumElements);
	I_Method1(unsigned int, index, IN, unsigned int, pos);
	I_ReadOnlyProperty(const GLvoid *, DataPointer);
	I_ReadOnlyProperty(unsigned int, TotalDataSize);
END_REFLECTOR


	STD_VECTOR_REFLECTOR(std::vector<osg::Vec2>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec3>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec4>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec2b>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec3b>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec4b>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec2s>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec3s>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec4s>);
	STD_VECTOR_REFLECTOR(std::vector<osg::Vec4ub>);
	STD_VECTOR_REFLECTOR(std::vector<GLubyte>);
	STD_VECTOR_REFLECTOR(std::vector<GLbyte>);
	STD_VECTOR_REFLECTOR(std::vector<GLushort>);
	STD_VECTOR_REFLECTOR(std::vector<GLshort>);
	STD_VECTOR_REFLECTOR(std::vector<GLuint>);
	STD_VECTOR_REFLECTOR(std::vector<GLint>);
	
