
IF   (DYNAMIC_NEWTEK)
    ADD_DEFINITIONS(-DFRAMEWORK_THREADS_EXPORTS)
ELSE (DYNAMIC_NEWTEK)
    ADD_DEFINITIONS(-DFRAMEWORK_THREADS_STATIC)
ENDIF(DYNAMIC_NEWTEK)

SET(LIB_NAME FrameWork.Threads)
GET_NATIVE_PRECOMPILED_HEADER(${LIB_NAME} stdafx.cpp)

SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
	critical_section.h
	event.h
	file_observer.h
	FrameWork.Threads.h
	profile.h
	read_write_lock.h
	stdafx.h
	thread.h
	WorkerThread_Impl.h
)

ADD_LIBRARY(${LIB_NAME}
    ${NEWTEK2005_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    critical_section.cpp
    event.cpp
    file_observer.cpp
    profile.cpp
    read_write_lock.cpp
    stdafx.cpp
)

ADD_NATIVE_PRECOMPILED_HEADER(${LIB_NAME} stdafx.cpp)

LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
