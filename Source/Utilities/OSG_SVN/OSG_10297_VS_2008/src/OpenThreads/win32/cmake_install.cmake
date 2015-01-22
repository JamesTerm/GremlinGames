# Install script for directory: $(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/src/OpenThreads/win32

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

SET(OpenThreads_BUILD_IMPNAME_Debug "Debug/OpenThreadsd.lib")
SET(OpenThreads_BUILD_IMPNAME_Release "Release/OpenThreads.lib")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${OpenThreads_BUILD_IMPNAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(OpenThreads_BUILD_NAME_Debug "Debug/OpenThreadsd.dll")
SET(OpenThreads_BUILD_NAME_Release "Release/OpenThreads.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY COMPONENTS "libopenthreads" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/lib/${OpenThreads_BUILD_NAME_${CMAKE_INSTALL_CONFIG_NAME}}")
SET(OpenThreads_REMAPPED_NAME_Debug "OpenThreadsd.dll")
SET(OpenThreads_REMAPPED_NAME_Release "OpenThreads.dll")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Atomic")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Barrier")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Block")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Condition")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Exports")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Mutex")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/ReadWriteMutex")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/ReentrantMutex")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/ScopedLock")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Thread")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OpenSceneGraph/include/OpenThreads/Version")
FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/OpenThreads" TYPE FILE COMPONENTS "libopenthreads-dev" FILES "$(GG_UTILITIES)/OSG_SVN/OSG_10297_VS_2008/include/OpenThreads/Config")
