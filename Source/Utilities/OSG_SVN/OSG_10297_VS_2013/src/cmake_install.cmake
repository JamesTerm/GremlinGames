# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src

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

IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/OpenThreads/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osg/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgDB/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgUtil/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgGA/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgText/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgViewer/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgAnimation/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgFX/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgManipulator/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgParticle/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgShadow/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgSim/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgTerrain/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgWidget/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgVolume/cmake_install.cmake")
  INCLUDE("$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/src/osgPlugins/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)
