# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/applications/osgfilecache

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

SET(application_osgfilecache_BUILD_NAME_Debug "Debug/osgfilecached.exe")
SET(application_osgfilecache_BUILD_NAME_Release "Release/osgfilecache.exe")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE COMPONENTS "openscenegraph" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/bin/${application_osgfilecache_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(application_osgfilecache_REMAPPED_NAME_Debug "osgfilecached.exe")
SET(application_osgfilecache_REMAPPED_NAME_Release "osgfilecache.exe")
