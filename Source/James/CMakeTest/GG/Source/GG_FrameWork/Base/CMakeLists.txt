
IF   (DYNAMIC_NEWTEK)
    ADD_DEFINITIONS(-DFRAMEWORK_BASE_EXPORTS)
ELSE (DYNAMIC_NEWTEK)
    ADD_DEFINITIONS(-FRAMEWORK_BASE_STATIC)
ENDIF(DYNAMIC_NEWTEK)

SET(LIB_NAME FrameWork.Base)
GET_NATIVE_PRECOMPILED_HEADER(${LIB_NAME} stdafx.cpp)

SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    AlignedAlloc.h
    Callbacks.h
    Callbacks_Interface.h
    DebugOutput.h
    FrameWork_Base.h
    KVArray.h
    Misc.h
    stdafx.h
    Time_Type.h
)

ADD_LIBRARY(${LIB_NAME}
    ${NEWTEK2005_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Callbacks.cpp
    DebugOutput.cpp
    FrameWork_Base.cpp
    stdafx.cpp
    Time_Type.cpp
)

ADD_NATIVE_PRECOMPILED_HEADER(${LIB_NAME} stdafx.cpp)

LINK_INTERNAL(${LIB_NAME}     
    FrameWork.Threads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
