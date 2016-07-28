MESSAGE("Searching for ANTLR3")

FIND_PACKAGE(Java REQUIRED)

FIND_FILE(
    ANTLR3_JAR
    antlr-3.5.2-complete.jar antlr-3.5.2-complete-no-st3.jar
    PATHS /usr/local/antlrv3 /usr/local/antlr3 /usr/local/ant*
    DOC "ANTLR3 jarfile"
    )

IF(ANTLR3_JAR-NOTFOUND)
    MESSAGE("Failed to find ANTLR3 jarfile - please set ANTLR3_JAR")
ELSE()
    SET(ANTLR3_FOUND ON)

    MARK_AS_ADVANCED(
        ANTLR3_JAR
        )
ENDIF()
