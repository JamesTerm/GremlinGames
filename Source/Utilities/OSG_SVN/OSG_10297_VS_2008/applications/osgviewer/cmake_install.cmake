# Install script for directory: C:/Stuff/gg/Source/Utilities/OSG_SVN/OpenSceneGraph/applications/osgviewer

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/INSTALL")
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

SET(application_osgviewer_BUILD_NAME_Debug "Debug/osgviewerd.exe")
SET(application_osgviewer_BUILD_NAME_Release "Release/osgviewer.exe")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE COMPONENTS "openscenegraph" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/bin/${application_osgviewer_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(application_osgviewer_REMAPPED_NAME_Debug "osgviewerd.exe")
SET(application_osgviewer_REMAPPED_NAME_Release "osgviewer.exe")
