ADD_LIBRARY(FullMonteGeometry SHARED TetraMeshBase.cpp TetraMesh.cpp Face.cpp newgeom.cpp Material.cpp)

IF (WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(FullMonteGeometry.i PROPERTIES CPLUSPLUS ON)
    SWIG_ADD_MODULE(FullMonteGeometryTCL tcl FullMonteGeometry.i)
    SWIG_LINK_LIBRARIES(FullMonteGeometryTCL ${TCL_STUB_LIBRARY} FullMonteGeometry)
ENDIF()



#### TESTS

FIND_PACKAGE(VTK REQUIRED)

INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${VTK_LIBRARY_DIRS} ${ANTLR3_LIB_DIR})
ADD_EXECUTABLE(Test_RayWalk Test_RayWalk.cpp RayWalk.cpp)
TARGET_LINK_LIBRARIES(Test_RayWalk ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} FullMonteGeometry FullMonteTIMOS vtkIOLegacy FullMonteVTKFile)
