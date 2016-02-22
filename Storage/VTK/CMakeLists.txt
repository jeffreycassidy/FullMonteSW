FIND_PACKAGE(VTK REQUIRED)

INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIR})
INCLUDE(${VTK_USE_FILE})

ADD_LIBRARY(FullMonteVTKFile SHARED VTKLegacyReader.cpp VTKLegacyWriter.cpp)
TARGET_LINK_LIBRARIES(FullMonteVTKFile FullMonteGeometry vtkCommonCore vtkIOCore vtkIOLegacy vtkCommonDataModel)
	
IF(WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(FullMonteVTKFile.i PROPERTIES CPLUSPLUS ON)
	
	SWIG_ADD_MODULE(FullMonteVTKFileTCL tcl FullMonteVTKFile.i)
	SWIG_LINK_LIBRARIES(FullMonteVTKFileTCL FullMonteVTKFile ${TCL_STUB_LIBRARY})
ENDIF()


ADD_CUSTOM_TARGET(Tests)

## Test cases

#ADD_SUBDIRECTORY(Tests)

# Load and re-save knee mesh
ADD_CUSTOM_TARGET(TestKneeMeshTCL
    COMMAND tclsh ${CMAKE_CURRENT_SOURCE_DIR}/Tests/test_knee.tcl ${CMAKE_CURRENT_SOURCE_DIR}/Tests/knee.mesh.vtk knee.out.vtk
    DEPENDS FullMonteVTKFileTCL )
ADD_DEPENDENCIES(Tests TestKneeMeshTCL)
