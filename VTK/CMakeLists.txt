IF(WRAP_TCL)
	SET(VTK_INSTALL_RUNTIME_DIR ${CMAKE_INSTALL_PREFIX}/bin)
	SET(VTK_INSTALL_LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/lib)
	SET(VTK_INSTALL_ARCHIVE_DIR ${CMAKE_INSTALL_PREFIX}/lib)
	
	INCLUDE(vtkModuleMacros)
	INCLUDE(vtkTclWrapping)
	
	SET(BUILD_SHARED_LIBS_PREV ${BUILD_SHARED_LIBS})
	SET(BUILD_SHARED_LIBS ON)
	
	SET(vtkFullMonte_TCL_NAME vtkFullMonte)	
ENDIF()

SET(MODULES vtkFullMonteTetraMeshWrapper;vtkFullMonteTetraMeshBaseWrapper;vtkFullMonteFilterTovtkIdList;vtkFullMonteArrayAdaptor;vtkDoseHistogramWrapper;vtkSourceExporter;vtkFullMontePacketPositionTraceSetToPolyData)

FOREACH(MOD ${MODULES})
	LIST(APPEND MODULECPP "${MOD}.cpp")
ENDFOREACH()

ADD_LIBRARY(vtkFullMonte SHARED ${MODULECPP} SwigWrapping.cpp)
TARGET_LINK_LIBRARIES(vtkFullMonte ${VTK_LIBRARIES} FullMonteData)

IF(WRAP_TCL)
	FOREACH(MOD ${MODULES})
		LIST(APPEND HFILES ${MOD}.h)
		LIST(APPEND CXXSRC ${MOD}.cpp)
	ENDFOREACH()

	vtk_add_tcl_wrapping(vtkFullMonte "${HFILES}" "")
ENDIF()

INSTALL (
    TARGETS vtkFullMonte
    DESTINATION lib)

SET(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_PREV})