MESSAGE("Searching for ANTLR3")

FIND_PACKAGE(Java REQUIRED)

FIND_FILE(
    ANTLR3_JAR
    antlr-3.5.2-complete.jar antlr-3.5.2-complete-no-st3.jar
    PATHS /usr/local/antlrv3 /usr/local/antlr3 /usr/local /usr/local/ant*
    PATH_SUFFIXES antlrv3 antlr3
    DOC "ANTLR3 jarfile"
    )

IF(NOT ANTLR3_JAR)
    MESSAGE("Failed to find ANTLR3 jarfile - please set ANTLR3_JAR")
ELSE()
    SET(ANTLR3_FOUND ON)

    MARK_AS_ADVANCED(
        ANTLR3_JAR
        )
ENDIF()
