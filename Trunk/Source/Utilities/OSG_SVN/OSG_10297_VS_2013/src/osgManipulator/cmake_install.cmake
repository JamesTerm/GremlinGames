# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/osgManipulator

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

SET(osgManipulator_BUILD_IMPNAME_Debug "Debug/osgManipulatord.lib")
SET(osgManipulator_BUILD_IMPNAME_Release "Release/osgManipulator.lib")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgManipulator_BUILD_IMPNAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgManipulator_BUILD_NAME_Debug "Debug/osgManipulatord.dll")
SET(osgManipulator_BUILD_NAME_Release "Release/osgManipulator.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY COMPONENTS "libopenscenegraph" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${osgManipulator_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(osgManipulator_REMAPPED_NAME_Debug "osgManipulatord.dll")
SET(osgManipulator_REMAPPED_NAME_Release "osgManipulator.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/AntiSquish")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Command")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/CommandManager")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Constraint")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Dragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Export")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Projector")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/RotateCylinderDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/RotateSphereDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Scale1DDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Scale2DDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/ScaleAxisDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Selection")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TabBoxDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TabPlaneDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TabPlaneTrackballDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TrackballDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Translate1DDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Translate2DDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TranslateAxisDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/TranslatePlaneDragger")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/osgManipulator" TYPE FILE COMPONENTS "libopenscenegraph-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/osgManipulator/Version")