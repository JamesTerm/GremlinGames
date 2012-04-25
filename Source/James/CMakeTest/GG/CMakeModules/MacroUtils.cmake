
#######################################################################################################
#  macro for linking libraries that come from Findxxxx commands, so there is a variable that contains the
#  full path of the library name. in order to differentiate release and debug, this macro get the
#  NAME of the variables, so the macro gets as arguments the target name and the following list of parameters
#  is intended as a list of variable names each one containing  the path of the libraries to link to
#  The existance of a varibale name with _DEBUG appended is tested and, in case it' s value is used
#  for linking to when in debug mode 
#  the content of this library for linking when in debugging
#######################################################################################################


MACRO(LINK_WITH_VARIABLES TRGTNAME)
    FOREACH(varname ${ARGN})
        IF(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${${varname}}" debug "${${varname}_DEBUG}")
        ELSE(${varname}_DEBUG)
            TARGET_LINK_LIBRARIES(${TRGTNAME} "${${varname}}" )
        ENDIF(${varname}_DEBUG)
    ENDFOREACH(varname)
ENDMACRO(LINK_WITH_VARIABLES TRGTNAME)

MACRO(LINK_INTERNAL TRGTNAME)
    FOREACH(LINKLIB ${ARGN})
        IF(MSVC AND OSG_MSVC_VERSIONED_DLL)
            #when using versioned names, the .dll name differ from .lib name, there is a problem with that:
            #CMake 2.4.7, at least seem to use PREFIX instead of IMPORT_PREFIX  for computing linkage info to use into projects,
            # so we full path name to specify linkage, this prevent automatic inferencing of dependencies, so we add explicit depemdencies
            #to library targets used
        IF(NOT MSVC_IDE)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${OUTPUT_LIBDIR}/${LINKLIB}.lib" debug "${OUTPUT_LIBDIR}/${LINKLIB}${CMAKE_DEBUG_POSTFIX}.lib")
        ELSE(NOT MSVC_IDE)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${OUTPUT_LIBDIR}/${LINKLIB}" debug "${OUTPUT_LIBDIR}/${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
        ENDIF(NOT MSVC_IDE)
            ADD_DEPENDENCIES(${TRGTNAME} ${LINKLIB})
        ELSE(MSVC AND OSG_MSVC_VERSIONED_DLL)
            TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${LINKLIB}" debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
        ENDIF(MSVC AND OSG_MSVC_VERSIONED_DLL)
    ENDFOREACH(LINKLIB)
ENDMACRO(LINK_INTERNAL TRGTNAME)

MACRO(LINK_EXTERNAL TRGTNAME)
    FOREACH(LINKLIB ${ARGN})
        TARGET_LINK_LIBRARIES(${TRGTNAME} "${LINKLIB}" )
    ENDFOREACH(LINKLIB)
ENDMACRO(LINK_EXTERNAL TRGTNAME)


#######################################################################################################
#  macro for common setup of core libraries: it links OPENGL_LIBRARIES in undifferentiated mode
#######################################################################################################

MACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)
    LINK_EXTERNAL(${CORELIB_NAME} ${OPENGL_LIBRARIES}) 
    LINK_WITH_VARIABLES(${CORELIB_NAME} OPENTHREADS_LIBRARY)
    IF(NEWTEK_SONAMES)
      SET_TARGET_PROPERTIES(${CORELIB_NAME} PROPERTIES VERSION ${NEWTEK_VERSION} SOVERSION ${NEWTEK_SOVERSION})
    ENDIF(NEWTEK_SONAMES)
ENDMACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)


#######################################################################################################
#  macro for common setup of plugins, examples and applications it expect some variables to be set:
#  either within the local CMakeLists or higher in hierarchy
#  TARGET_NAME is the name of the folder and of the actually .exe or .so or .dll
#  TARGET_TARGETNAME  is the name of the target , this get buit out of a prefix, if present and TARGET_TARGETNAME
#  TARGET_SRC  are the sources of the target
#  TARGET_H are the eventual headers of the target
#  TARGET_LIBRARIES are the libraries to link to that are internal to the project and have d suffix for debug
#  TARGET_EXTERNAL_LIBRARIES are external libraries and are not differentiated with d suffix
#  TARGET_LABEL is the label IDE should show up for targets
##########################################################################################################

MACRO(SETUP_LINK_LIBRARIES)
    ######################################################################
    #
    # This set up the libraries to link to, it assumes there are two variable: one common for a group of examples or plagins
    # kept in the variable TARGET_COMMON_LIBRARIES and an example or plugin specific kept in TARGET_ADDED_LIBRARIES 
    # they are combined in a single list checked for unicity 
    # the suffix ${CMAKE_DEBUG_POSTFIX} is used for differentiating optimized and debug
    #
    # a second variable TARGET_EXTERNAL_LIBRARIES hold the list of  libraries not differentiated between debug and optimized 
    ##################################################################################
    SET(TARGET_LIBRARIES ${TARGET_COMMON_LIBRARIES})

    FOREACH(LINKLIB ${TARGET_ADDED_LIBRARIES})
      SET(TO_INSERT TRUE)
      FOREACH (value ${TARGET_COMMON_LIBRARIES})
            IF (${value} STREQUAL ${LINKLIB})
                  SET(TO_INSERT FALSE)
            ENDIF (${value} STREQUAL ${LINKLIB})
        ENDFOREACH (value ${TARGET_COMMON_LIBRARIES})
      IF(TO_INSERT)
          LIST(APPEND TARGET_LIBRARIES ${LINKLIB})
      ENDIF(TO_INSERT)
    ENDFOREACH(LINKLIB)

#    FOREACH(LINKLIB ${TARGET_LIBRARIES})
#            TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} optimized ${LINKLIB} debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
#    ENDFOREACH(LINKLIB)
        LINK_INTERNAL(${TARGET_TARGETNAME} ${TARGET_LIBRARIES})
    FOREACH(LINKLIB ${TARGET_EXTERNAL_LIBRARIES})
            TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} ${LINKLIB})
    ENDFOREACH(LINKLIB)
        IF(TARGET_LIBRARIES_VARS)
            LINK_WITH_VARIABLES(${TARGET_TARGETNAME} ${TARGET_LIBRARIES_VARS})
        ENDIF(TARGET_LIBRARIES_VARS)
    IF(MSVC  AND OSG_MSVC_VERSIONED_DLL)
        #when using full path name to specify linkage, it seems that already linked libs must be specified
            LINK_EXTERNAL(${TARGET_TARGETNAME} ${OPENGL_LIBRARIES}) 
    ENDIF(MSVC AND OSG_MSVC_VERSIONED_DLL)

ENDMACRO(SETUP_LINK_LIBRARIES)

############################################################################################
# this is the common set of command for all the plugins
#

MACRO(SETUP_PLUGIN PLUGIN_NAME)

        SET(TARGET_NAME ${PLUGIN_NAME} )

    #MESSAGE("in -->SETUP_PLUGIN<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")

    ## we have set up the target label and targetname by taking into account global prfix (osgdb_)

    IF(NOT TARGET_TARGETNAME)
            SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    ENDIF(NOT TARGET_TARGETNAME)
    IF(NOT TARGET_LABEL)
            SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    ENDIF(NOT TARGET_LABEL)
    
# here we use the command to generate the library    

    IF   (DYNAMIC_NEWTEK)
        ADD_LIBRARY(${TARGET_TARGETNAME} MODULE ${TARGET_SRC} ${TARGET_H})
    ELSE (DYNAMIC_NEWTEK)
        ADD_LIBRARY(${TARGET_TARGETNAME} STATIC ${TARGET_SRC} ${TARGET_H})
    ENDIF(DYNAMIC_NEWTEK)
    
    #not sure if needed, but for plugins only Msvc need the d suffix
    IF(NOT MSVC)
        SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_POSTFIX "")
    ELSE(NOT MSVC)
        IF(OSG_MSVC_VERSIONED_DLL) 
                       
                #this is a hack... the build place is set to lib/<debug or release> by LIBARARY_OUTPUT_PATH equal to OUTPUT_LIBDIR
                #the .lib will be crated in ../ so going straight in lib by the IMPORT_PREFIX property
                #because we want dll placed in OUTPUT_BINDIR ie the bin folder sibling of lib, we can use ../../bin to go there,
                #it is hardcoded, we should compute OUTPUT_BINDIR position relative to OUTPUT_LIBDIR ... to be implemented
                #changing bin to something else breaks this hack
                #the dll are placed in bin/${OSG_PLUGINS} 
                
                IF(NOT MSVC_IDE)
                    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PREFIX "../bin/${OSG_PLUGINS}/")                     
                ELSE(NOT MSVC_IDE)
                    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PREFIX "../../bin/${OSG_PLUGINS}/" IMPORT_PREFIX "../")
                ENDIF(NOT MSVC_IDE)

        ELSE(OSG_MSVC_VERSIONED_DLL)
            
            #in standard mode (unversioned) the .lib and .dll are placed in lib/<debug or release>/${OSG_PLUGINS}.
            #here the PREFIX property has been used, the same result would be accomplidhe by prepending ${OSG_PLUGINS}/ to OUTPUT_NAME target property
            
            SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PREFIX "${OSG_PLUGINS}/")
        ENDIF(OSG_MSVC_VERSIONED_DLL)
    ENDIF(NOT MSVC)
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")
 
    SETUP_LINK_LIBRARIES()

#the installation path are differentiated for win32 that install in bib versus other architecture that install in lib${LIB_POSTFIX}/${OSG_PLUGINS}
    IF(WIN32)
        INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib/${OSG_PLUGINS} LIBRARY DESTINATION bin/${OSG_PLUGINS} )
    ELSE(WIN32)
        INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} LIBRARY DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} )
    ENDIF(WIN32)
ENDMACRO(SETUP_PLUGIN)


#################################################################################################################
# this is the macro for example and application setup
###########################################################

MACRO(SETUP_EXE IS_COMMANDLINE_APP)
    #MESSAGE("in -->SETUP_EXE<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")
    IF(NOT TARGET_TARGETNAME)
            SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
    ENDIF(NOT TARGET_TARGETNAME)
    IF(NOT TARGET_LABEL)
            SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
    ENDIF(NOT TARGET_LABEL)

    IF(${IS_COMMANDLINE_APP})
    
        ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC} ${TARGET_H})
        
    ELSE(${IS_COMMANDLINE_APP})
    
        IF(APPLE)
            # SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${NEWTEK_MAJOR_VERSION}.${NEWTEK_MINOR_VERSION}.${NEWTEK_PATCH_VERSION}")
            # Short Version is the "marketing version". It is the version
            # the user sees in an information panel.
            SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${NEWTEK_MAJOR_VERSION}.${NEWTEK_MINOR_VERSION}.${NEWTEK_PATCH_VERSION}")
            # Bundle version is the version the OS looks at.
            SET(MACOSX_BUNDLE_BUNDLE_VERSION "${NEWTEK_MAJOR_VERSION}.${NEWTEK_MINOR_VERSION}.${NEWTEK_PATCH_VERSION}")
            SET(MACOSX_BUNDLE_GUI_IDENTIFIER "org.NEWTEK.${TARGET_TARGETNAME}" )
            SET(MACOSX_BUNDLE_BUNDLE_NAME "${TARGET_NAME}" )
            # SET(MACOSX_BUNDLE_ICON_FILE "myicon.icns")
            # SET(MACOSX_BUNDLE_COPYRIGHT "")
            # SET(MACOSX_BUNDLE_INFO_STRING "Info string, localized?")
        ENDIF(APPLE)

        IF(WIN32)
            IF (REQUIRE_WINMAIN_FLAG)
                SET(PLATFORM_SPECIFIC_CONTROL WIN32)
            ENDIF(REQUIRE_WINMAIN_FLAG)
        ENDIF(WIN32)

        IF(APPLE)
            IF(OSG_BUILD_APPLICATION_BUNDLES)
                SET(PLATFORM_SPECIFIC_CONTROL MACOSX_BUNDLE)
            ENDIF(OSG_BUILD_APPLICATION_BUNDLES)
        ENDIF(APPLE)

        ADD_EXECUTABLE(${TARGET_TARGETNAME} ${PLATFORM_SPECIFIC_CONTROL} ${TARGET_SRC} ${TARGET_H})
        
    ENDIF(${IS_COMMANDLINE_APP})
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
    SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    IF(MSVC_IDE AND OSG_MSVC_VERSIONED_DLL)
            SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PREFIX "../")    
    ENDIF(MSVC_IDE AND OSG_MSVC_VERSIONED_DLL)
    SETUP_LINK_LIBRARIES()    

ENDMACRO(SETUP_EXE)

# Takes optional second argument (is_commandline_app?) in ARGV1
MACRO(SETUP_APPLICATION APPLICATION_NAME)

        SET(TARGET_NAME ${APPLICATION_NAME} )

        IF(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP ${ARGV1})
        ELSE(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP 0)
        ENDIF(${ARGC} GREATER 1)
            
        SETUP_EXE(${IS_COMMANDLINE_APP})
        
    INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin  )

ENDMACRO(SETUP_APPLICATION)

MACRO(SETUP_COMMANDLINE_APPLICATION APPLICATION_NAME)

    SETUP_APPLICATION(${APPLICATION_NAME} 1)

ENDMACRO(SETUP_COMMANDLINE_APPLICATION)

# Takes optional second argument (is_commandline_app?) in ARGV1
MACRO(SETUP_EXAMPLE EXAMPLE_NAME)

        SET(TARGET_NAME ${EXAMPLE_NAME} )

        IF(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP ${ARGV1})
        ELSE(${ARGC} GREATER 1)
            SET(IS_COMMANDLINE_APP 0)
        ENDIF(${ARGC} GREATER 1)
            
        SETUP_EXE(${IS_COMMANDLINE_APP})
        
    INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION share/NEWTEK/bin  )            

ENDMACRO(SETUP_EXAMPLE)


MACRO(SETUP_COMMANDLINE_EXAMPLE EXAMPLE_NAME)

    SETUP_EXAMPLE(${EXAMPLE_NAME} 1)

ENDMACRO(SETUP_COMMANDLINE_EXAMPLE)

# Takes two optional arguments -- osg prefix and osg version
MACRO(HANDLE_MSVC_DLL)
	#this is a hack... the build place is set to lib/<debug or release> by LIBARARY_OUTPUT_PATH equal to OUTPUT_LIBDIR
	#the .lib will be crated in ../ so going straight in lib by the IMPORT_PREFIX property
	#because we want dll placed in OUTPUT_BINDIR ie the bin folder sibling of lib, we can use ../../bin to go there,
	#it is hardcoded, we should compute OUTPUT_BINDIR position relative to OUTPUT_LIBDIR ... to be implemented
	#changing bin to something else breaks this hack
	#the dll are versioned by prefixing the name with nt_${NEWTEK2005_SOVERSION}-

        # LIB_PREFIX: use "osg" by default, else whatever we've been given.
        IF(${ARGC} GREATER 0)
                SET(LIB_PREFIX ${ARGV0})
        ELSE(${ARGC} GREATER 0)
                SET(LIB_PREFIX osg)
        ENDIF(${ARGC} GREATER 0)

        # LIB_SOVERSION: use OSG's soversion by default, else whatever we've been given
        IF(${ARGC} GREATER 1)
                SET(LIB_SOVERSION ${ARGV1})
        ELSE(${ARGC} GREATER 1)
                SET(LIB_SOVERSION ${NEWTEK_SOVERSION})
        ENDIF(${ARGC} GREATER 1)
    
        IF(NOT MSVC_IDE) 
            SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../bin/${LIB_PREFIX}${LIB_SOVERSION}-")
            SET(NEW_LIB_NAME "${OUTPUT_BINDIR}/${LIB_PREFIX}${LIB_SOVERSION}-${LIB_NAME}")
            ADD_CUSTOM_COMMAND(
                TARGET ${LIB_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy "${NEW_LIB_NAME}.lib"  "${OUTPUT_LIBDIR}/${LIB_NAME}.lib"
                COMMAND ${CMAKE_COMMAND} -E copy "${NEW_LIB_NAME}.exp"  "${OUTPUT_LIBDIR}/${LIB_NAME}.exp"
                COMMAND ${CMAKE_COMMAND} -E remove "${NEW_LIB_NAME}.lib"
                COMMAND ${CMAKE_COMMAND} -E remove "${NEW_LIB_NAME}.exp"
                )
        ELSE(NOT MSVC_IDE)
            SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../../bin/${LIB_PREFIX}${LIB_SOVERSION}-" IMPORT_PREFIX "../")
        ENDIF(NOT MSVC_IDE)

#     SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../../bin/nt_${NEWTEK2005_SOVERSION}-")
#     SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
ENDMACRO(HANDLE_MSVC_DLL)