MESSAGE("Searching for ANTLR3 C runtime")

FIND_PACKAGE(ANTLR3 REQUIRED)

FIND_LIBRARY(
    ANTLR3C_LIBRARY
    antlr3c
    HINTS
    PATHS /usr/local /usr/local/lib /usr/local/antlr3c/lib /usr/local/antlr3c-3.4
    PATH_SUFFIXES lib
    DOC "ANTLR3 C runtime, typically libantlr3c.so"
    )

GET_FILENAME_COMPONENT(ANTLR3C_LIBRARY_DIR ${ANTLR3C_LIBRARY} DIRECTORY)

MESSAGE("Found antlr3c runtime at ${ANTLR3C_LIBRARY}, path ${ANTLR3C_LIBRARY_DIR}")

FIND_PATH(
    ANTLR3C_INCLUDE_DIR
    antlr3.h
    HINTS ${ANTLR3C_DIR} ${ANTLR3C_LIBRARY_DIR} ${ANTLR3C_LIBRARY_DIR}/../include
    PATHS /usr/local
    PATH_SUFFIXES include
    DOC "ANTLR3 C runtime, folder containing antlr3.h"
    )

IF(NOT ANTLR3C_LIBRARY)
    MESSAGE("Failed to find ANTLR3 C runtime library")
ELSEIF(NOT ANTLR3C_INCLUDE_DIR)
    MESSAGE("Failed to find ANTLR3 C runtime include files")
ELSE()
    SET(ANTLR3C_FOUND ON)

    SET(ANTLR3C_DEFINITIONS "_empty=NULL" CACHE STRING "Additional C compile flags for ANTLR3C")

## Use local version
    SET(ANTLR3C_USE_FILE "${CMAKE_SOURCE_DIR}/UseANTLR3C.cmake")
    SET(ANTLR3C_TOOLS_DIR "${CMAKE_SOURCE_DIR}/ANTLR3")
    MESSAGE("Using local version of UseANTLR3C.cmake at ${ANTLR3C_USE_FILE}")

    MARK_AS_ADVANCED(
        ANTLR3C_LIBRARY
        ANTLR3
        ANTLR3C_COMPILE_DEFINITIONS
        )
ENDIF()
