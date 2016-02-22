#### ANTLR config
SET(ANTLR_JAR "/usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar" CACHE PATH "Jar-file for ANTLR3")
SET(ANTLR3_LIBRARIES antlr3c)
SET(ANTLR3_INCLUDE_DIR "/usr/local/antlr3c-3.4/include" CACHE PATH "ANTLR3 include path")
SET(ANTLR3_LIB_DIR "/usr/local/lib" CACHE PATH "ANTLR3 C runtime path")
SET(ANTLR3_FOUND ON)

#### ANTLR3 Parser targets
FUNCTION (ADD_ANTLR3_PARSER GRAMMAR)

	# Disable some harmless warnings thrown by Clang and G++
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_FLAGS "-Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-tautological-compare")
    
    # Bugfix: missing definition of _empty
    SET_SOURCE_FILES_PROPERTIES(${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c PROPERTIES COMPILE_DEFINITIONS "_empty=NULL")

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}Parser.c ${GRAMMAR}Lexer.c ${GRAMMAR}Parser.h ${GRAMMAR}Lexer.h ${GRAMMAR}.tokens
        COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR} -o ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${GRAMMAR}.g 2>&1 | grep -vi 'warning\(24\)'
        
        DEPENDS ${GRAMMAR}.g
        )

    ADD_CUSTOM_COMMAND(
        OUTPUT ${GRAMMAR}_tokens.h
        COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../CommonParser/mktok.pl ${CMAKE_CURRENT_BINARY_DIR}/${GRAMMAR}.tokens > ${GRAMMAR}_tokens.h
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../CommonParser/mktok.pl ${GRAMMAR}.tokens
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        )

	ADD_CUSTOM_TARGET(grammar DEPENDS ${GRAMMAR}_tokens.h)

ENDFUNCTION()