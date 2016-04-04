ADD_SUBDIRECTORY(Placement)
ADD_SUBDIRECTORY(Modifiers)

ADD_LIBRARY(FullMonteGeometry SHARED TetraMeshBase.cpp TetraMesh.cpp Face.cpp newgeom.cpp Sources/Print.cpp Filters/TetraFilterByRegion.cpp Filters/TetraMeshBaseFilter.cpp Filters/TetraMeshFilter.cpp RayWalk.cpp Filters/TriFilterRegionBounds.cpp Basis.cpp MeshChecker.cpp)

ADD_EXECUTABLE(CheckMesh CheckMesh.cpp)
TARGET_LINK_LIBRARIES(CheckMesh FullMonteGeometry FullMonteVTKFile)

IF (WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(FullMonteGeometry.i PROPERTIES CPLUSPLUS ON)
    SWIG_ADD_MODULE(FullMonteGeometryTCL tcl FullMonteGeometry.i)
    SWIG_LINK_LIBRARIES(FullMonteGeometryTCL ${TCL_STUB_LIBRARY} FullMonteGeometry)
ENDIF()



#### TESTS

FIND_PACKAGE(VTK REQUIRED)

INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
LINK_DIRECTORIES(${VTK_LIBRARY_DIRS} ${ANTLR3_LIB_DIR})
