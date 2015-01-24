# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgViewer

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

SET(osgViewer_BUILD_IMPNAME_Debug "Debug/osgViewerd.lib")
SET(osgViewer_BUILD_IMPNAME_Release "Release/osgViewer.lib")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgViewer_BUILD_IMPNAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgViewer_BUILD_NAME_Debug "Debug/osgViewerd.dll")
SET(osgViewer_BUILD_NAME_Release "Release/osgViewer.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY COMPONENTS "libopenscenegraph" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgViewer_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgViewer_REMAPPED_NAME_Debug "osgViewerd.dll")
SET(osgViewer_REMAPPED_NAME_Release "osgViewer.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/CompositeViewer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Export")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/GraphicsWindow")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Renderer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Scene")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Version")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/View")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Viewer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/ViewerBase")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/ViewerEventHandlers")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/api/Win32/GraphicsWindowWin32")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/api/Win32/PixelBufferWin32")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/CompositeViewer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Export")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/GraphicsWindow")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Renderer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Scene")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Version")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/View")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/Viewer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/ViewerBase")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/ViewerEventHandlers")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer/api/Win32" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/api/Win32/GraphicsWindowWin32")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgViewer/api/Win32" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgViewer/api/Win32/PixelBufferWin32")
