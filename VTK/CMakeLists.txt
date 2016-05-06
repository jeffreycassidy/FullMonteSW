IF (WRAP_VTK)

	FIND_PACKAGE(VTK REQUIRED)
	INCLUDE(${VTK_USE_FILE})
## TODO: Remove SwigWrapping from here, move to SWIG-specific location
	ADD_LIBRARY(FullMonteVTK SHARED vtkFullMonteTetraMeshBaseWrapper.cpp vtkFullMonteSpatialMapWrapper.cpp vtkFullMonteTetraMeshWrapper.cpp vtkFullMonteFluenceLineQueryWrapper.cpp SwigWrapping.cpp)
	TARGET_LINK_LIBRARIES(FullMonteVTK vtkFiltersCore vtkIOLegacy vtkCommonDataModel)

# vtkFiltersSources vtkInteractionWidgets)

	#SWIG_ADD_MODULE(FullMonteVTKTCL tcl FullMonteVTKTCLInit.cxx)
	#SWIG_LINK_LIBRARIES(FullMonteVTKTCL FullMonteVTK vtkCommonCoreTCL vtkCommonDataModelTCL vtkFiltersSourcesTCL)
	
	IF(WRAP_TCL)
		FUNCTION(VTK_TCL_WRAP fn)
		
			FILE(COPY ${CMAKE_CURRENT_SOURCE_DIR}/${fn}.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
			
			ADD_CUSTOM_COMMAND(
				OUTPUT ${fn}_wrap.cxx
				COMMAND /usr/local/bin/vtkWrapTcl-6.3 -o ${CMAKE_CURRENT_BINARY_DIR}/${fn}_wrap.cxx ${CMAKE_CURRENT_SOURCE_DIR}/${fn}.h
				DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${fn}.h
				)
		ENDFUNCTION()
		
		FUNCTION(VTK_TCL_WRAP_INIT)
			STRING(REPLACE ";" "\\n" ARGNLIST "${ARGN}")
			ADD_CUSTOM_COMMAND(
				OUTPUT vtkTCLModuleList.txt
				COMMAND echo "FullMonteVTK ${ARGNLIST}" > vtkTCLModuleList.txt
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
				VERBATIM)

			ADD_CUSTOM_COMMAND(
				OUTPUT vtkTCLInit.cxx
				DEPENDS vtkTCLModuleList.txt
				COMMAND vtkWrapTclInit-6.3 vtkTCLModuleList.txt vtkTCLInit.cxx
				WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
		ENDFUNCTION()

		SET(MODULES vtkFullMonteTetraMeshWrapper;vtkFullMonteTetraMeshBaseWrapper)
		
		FOREACH(MOD ${MODULES})
			LIST(APPEND CXXFILES ${MOD}_wrap.cxx)
		ENDFOREACH()

		FOREACH(MOD ${MODULES})
			VTK_TCL_WRAP(${MOD})
		ENDFOREACH()
		
		VTK_TCL_WRAP_INIT(${MODULES})
		
		INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
		
		ADD_LIBRARY(FullMonteVTKTCL SHARED ${CXXFILES} vtkTCLInit.cxx)
		TARGET_LINK_LIBRARIES(FullMonteVTKTCL FullMonteVTK vtkCommonCoreTCL-6.3)
	ENDIF()
ENDIF()
