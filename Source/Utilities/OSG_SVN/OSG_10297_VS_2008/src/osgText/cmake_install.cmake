# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgText

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/INSTALL")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

SET(osgText_BUILD_IMPNAME_Debug "Debug/osgTextd.lib")
SET(osgText_BUILD_IMPNAME_Release "Release/osgText.lib")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgText_BUILD_IMPNAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgText_BUILD_NAME_Debug "Debug/osgTextd.dll")
SET(osgText_BUILD_NAME_Release "Release/osgText.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY COMPONENTS "libopenscenegraph" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgText_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgText_REMAPPED_NAME_Debug "osgTextd.dll")
SET(osgText_REMAPPED_NAME_Release "osgText.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Export")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Font")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Font3D")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/KerningType")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/String")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/TextBase")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Text")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Text3D")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/FadeText")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgText" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgText/Version")