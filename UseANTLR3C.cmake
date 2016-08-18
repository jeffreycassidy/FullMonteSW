FIND_PACKAGE(Java REQUIRED)

# Disable some harmless warnings thrown by Clang and G++
IF(CMAKE_COMPILER_IS_GNUCXX)
    LIST(APPEND ANTLR3C_C_FLAGS -Wno-unused-but-set-variable -Wno-unused-local-typedef)
ENDIF()

IF(CMAKE_COMPILER_IS_CLANG)
    LIST(APPEND ANTLR3C_C_FLAGS -Wno-unused-variable -Wno-unused-function -Wno-tautological-compare)
ENDIF()

SET(ANTLR3C_C_FLAGS         ${ANTLR3C_C_FLAGS}  CACHE STRING "Extra compile flags for ANTLR3 C runtime")
SET(ANTLR3C_DEFINITIONS     "_empty=NULL"       CACHE STRING "Extra compile definitions for ANTLR3 C runtime")

MARK_AS_ADVANCED(
    ANTLR3C_C_FLAGS
    ANTLR3C_C_DEFINITIONS
)

INCLUDE_DIRECTORIES(${ANTLR3C_INCLUDE_DIR})


FIND_PACKAGE(Perl REQUIRED)

#### ANTLR3 Parser targets
FUNCTION (ADD_ANTLR3C_PARSER GRAMMAR)
    STRING(REPLACE ";" " " FLAGSTR "${ANTLR3C_C_FLAGS}")
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_FLAGS "${FLAGSTR}")

    # Bugfix: missing definition of _empty
    STRING(REPLACE ";" " " DEFSTR ${ANTLR3C_DEFINITIONS})
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_DEFINITIONS "${DEFSTR}")

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c ${GRAMMAR}Parser.h ${GRAMMAR}Lexer.h ${GRAMMAR}.tokens
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR3_JAR} -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${GRAMMAR}.g 2>&1
# TODO: Block out warnings _without_ returning nonzero error code
#| grep -vi 'warning\(24\)'
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}        
        DEPENDS ${GRAMMAR}.g
        )

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}_tokens.h
        COMMAND ${PERL_EXECUTABLE} ${ANTLR3C_TOOLS_DIR}/mktok.pl ${GRAMMAR}.tokens > ${GRAMMAR}_tokens.h
        DEPENDS ${ANTLR3C_TOOLS_DIR}/mktok.pl ${GRAMMAR}.tokens
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

	ADD_CUSTOM_TARGET(${GRAMMAR}Parser DEPENDS ${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c ${GRAMMAR}_tokens.h)

    SET_TARGET_PROPERTIES(${GRAMMAR}Parser PROPERTIES INCLUDE_DIRECTORIES ${CMAKE_CURRENT_BINARY_DIR})

ENDFUNCTION()
