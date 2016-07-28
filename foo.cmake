MESSAGE("Boing")

#FIND_FILE(
#    VTKTclWrap_FILE
#    VTKTclWrapConfig.cmake
#    HINTS ${CMAKE_SOURCE_DIR}/VTK
#    )
#
#IF (NOT VTKTclWrap_FILE)
#    MESSAGE("Failed to find VTKTclWrapConfig.cmake")
#
#    FIND_PROGRAM(
#        VTKTclWrap_EXECUTABLE
#        NAMES vtkTclWrap vtkTclWrap-6.1 vtkTclWrap-6.3
#        PATHS /usr/local/bin /usr/bin
#        )
#
#    FIND_PROGRAM(
#        VTKTclWrap_INIT_EXECUTABLE
#        NAMES vtkTclWrapInit vtkTclWrapInit-6.1 vtkTclWrapInit-6.3
#        PATHS /usr/local/bin /usr/bin
#        )
#
#    IF (VTKTclWrap_EXECUTABLE AND VTKTclWrap_INIT_EXECUTABLE)
#        MESSAGE("Found VTK TCL wrapping files at ${VTKTclWrap_EXECUTABLE} and ${VTKTclWrap_INIT_EXECUTABLE}")
#        SET(VTKTclWrap_FOUND ON)
#    ELSE()
#        MESSAGE("Failed to locate VTK TCL wrapping utilities")
#    ENDIF()
#ELSE()
#    SET(VTKTclWrap_FOUND ON)
#    SET(VTKTclWrap_DIR ${VTKTclWrap_FILE} DIRECTORY CACHE "")
#    MESSAGE("Found VTKTclWrapConfig.cmake in ${VTKTclWrap_DIR}")
#ENDIF()

#UNSET(VTKTclWrap_FILE)
