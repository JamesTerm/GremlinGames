IF(NOT EXISTS "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/install_manifest.txt")
    MESSAGE(FATAL_ERROR "Cannot find install manifest: \"C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/install_manifest.txt\"")
ENDIF()

FILE(READ "C:/Stuff/gg/Source/Utilities/OSG_SVN/Visual_Studio_9_2008_Win32/install_manifest.txt" files)
STRING(REGEX REPLACE "\n" ";" files "${files}")

FOREACH(file ${files})
    MESSAGE(STATUS "Uninstalling \"${file}\"")
    IF(EXISTS "${file}")
        EXEC_PROGRAM(
            "C:/Program Files (x86)/cmake-2.4.8-win32-x86/bin/cmake.exe" ARGS "-E remove \"${file}\""
            OUTPUT_VARIABLE rm_out
            RETURN_VALUE rm_retval
            )
        IF(NOT "${rm_retval}" STREQUAL 0)
            MESSAGE(FATAL_ERROR "Problem when removing \"${file}\"")
        ENDIF()
    ELSE()
        MESSAGE(STATUS "File \"${file}\" does not exist.")
    ENDIF()
ENDFOREACH()
