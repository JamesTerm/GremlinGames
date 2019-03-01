# Install script for directory: C:/Stuff/gg/Source/Utilities/OSG_SVN/OpenSceneGraph

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

FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osg.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgDB.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgFX.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgGA.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgParticle.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgSim.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgText.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgUtil.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgTerrain.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgManipulator.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgViewer.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgWidget.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgShadow.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgAnimation.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgVolume.pc")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/packaging/pkgconfig/openscenegraph-osgIntrospection.pc")
IF(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  INCLUDE("C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/src/cmake_install.cmake")
  INCLUDE("C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/applications/cmake_install.cmake")

ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)
IF(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
ELSE(CMAKE_INSTALL_COMPONENT)
  SET(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
ENDIF(CMAKE_INSTALL_COMPONENT)
FILE(WRITE "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/${CMAKE_INSTALL_MANIFEST}" "")
FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})
  FILE(APPEND "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/${CMAKE_INSTALL_MANIFEST}" "${file}\n")
ENDFOREACH(file)