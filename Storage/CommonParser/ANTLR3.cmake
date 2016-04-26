#### ANTLR config
SET(ANTLR_JAR "/usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar" CACHE PATH "Jar-file for ANTLR3")
SET(ANTLR3_LIBRARY antlr3c)
SET(ANTLR3_INCLUDE_DIR "/usr/local/antlr3c-3.4/include" CACHE PATH "ANTLR3 include path")
SET(ANTLR3_LIB_DIR "/usr/local/lib" CACHE PATH "ANTLR3 C runtime path")
SET(ANTLR3_FOUND ON)
SET(ANTLR3_DIR "/Users/jcassidy/src/FullMonteSW/FullMonte/Storage/CommonParser" CACHE PATH "Path to ANTLR3.cmake and related files")

SET(ANTLR3_COMPILE_FLAGS "")

# Disable some harmless warnings thrown by Clang and G++
IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(ANTLR3_COMPILE_FLAGS "${ANTLR3_COMPILE_FLAGS} -Wno-unused-but-set-variable")
ENDIF()

IF(CMAKE_COMPILER_IS_CLANG)
    SET(ANTLR3_COMPILE_FLAGS "${ANTLR3_COMPILE_FLAGS} -Wno-unused-variable -Wno-unused-function -Wno-tautological-compare")
ENDIF()

SET(ANTLR3_COMPILE_FLAGS "${ANTLR3_COMPILE_FLAGS}" CACHE STRING "")
SET(ANTLR3_COMPILE_DEFINITIONS "_empty=NULL" CACHE STRING "")
MARK_AS_ADVANCED(ANTLR3_COMPILE_FLAGS)


FIND_PACKAGE(Java REQUIRED)
FIND_PACKAGE(Perl REQUIRED)

#### ANTLR3 Parser targets
FUNCTION (ADD_ANTLR3_PARSER GRAMMAR)
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_FLAGS ${ANTLR3_COMPILE_FLAGS})

    # Bugfix: missing definition of _empty
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_DEFINITIONS "${ANTLR3_COMPILE_DEFINITIONS}")

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c ${GRAMMAR}Parser.h ${GRAMMAR}Lexer.h ${GRAMMAR}.tokens
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR} -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${GRAMMAR}.g 2>&1
# TODO: Block out warnings _without_ returning nonzero error code
#| grep -vi 'warning\(24\)'
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}        
        DEPENDS ${GRAMMAR}.g
        )

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}_tokens.h
        COMMAND ${PERL_EXECUTABLE} ${ANTLR3_DIR}/mktok.pl ${GRAMMAR}.tokens > ${GRAMMAR}_tokens.h
        DEPENDS ${ANTLR3_DIR}/mktok.pl ${GRAMMAR}.tokens
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

	ADD_CUSTOM_TARGET(${GRAMMAR}Parser DEPENDS ${GRAMMAR}_tokens.h)

ENDFUNCTION()
