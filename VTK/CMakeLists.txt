IF (WRAP_VTK)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

ADD_LIBRARY(FullMonteVTK SHARED vtkFullMonteTetraMeshBaseWrapper.cpp vtkFullMonteSpatialMapWrapper.cpp vtkFullMonteTetraMeshWrapper.cpp vtkFullMonteFluenceLineQueryWrapper.cpp)
TARGET_LINK_LIBRARIES(FullMonteVTK vtkFiltersCore vtkIOLegacy vtkCommonDataModel)
# vtkFiltersSources vtkInteractionWidgets)

#SWIG_ADD_MODULE(FullMonteVTKTCL tcl FullMonteVTKTCLInit.cxx)
#SWIG_LINK_LIBRARIES(FullMonteVTKTCL FullMonteVTK vtkCommonCoreTCL vtkCommonDataModelTCL vtkFiltersSourcesTCL)

#FUNCTION(VTK_TCL_WRAP fn)
#	ADD_CUSTOM_COMMAND(
#		OUTPUT ${fn}_wrap.cxx
#		COMMAND /usr/local/bin/vtkWrapTcl-6.1 -o ${fn}_wrap.cxx ${CMAKE_CURRENT_SOURCE_DIR}/${fn}.h
#		DEPENDS ${fn}.h)
#ENDFUNCTION()

#VTK_TCL_WRAP(SparseVectorVTK)

ENDIF()
