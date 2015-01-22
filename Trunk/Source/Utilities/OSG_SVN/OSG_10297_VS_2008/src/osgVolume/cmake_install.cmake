# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgVolume

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

SET(osgVolume_BUILD_IMPNAME_Debug "Debug/osgVolumed.lib")
SET(osgVolume_BUILD_IMPNAME_Release "Release/osgVolume.lib")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgVolume_BUILD_IMPNAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgVolume_BUILD_NAME_Debug "Debug/osgVolumed.dll")
SET(osgVolume_BUILD_NAME_Release "Release/osgVolume.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY COMPONENTS "libopenscenegraph" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgVolume_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgVolume_REMAPPED_NAME_Debug "osgVolumed.dll")
SET(osgVolume_REMAPPED_NAME_Release "osgVolume.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Export")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/FixedFunctionTechnique")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Layer")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Locator")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Property")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/RayTracedTechnique")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Version")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/Volume")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/VolumeTechnique")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgVolume" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgVolume/VolumeTile")
