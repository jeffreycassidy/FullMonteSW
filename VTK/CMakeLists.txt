INCLUDE(${VTK_USE_FILE})

#SET(MODULES vtkFullMonteTetraMeshWrapper;vtkFullMonteTetraMeshBaseWrapper;vtkFullMonteFluenceLineQueryWrapper)
SET(MODULES vtkFullMonteTetraMeshWrapper;vtkFullMonteTetraMeshBaseWrapper;vtkFullMonteFilterTovtkIdList;vtkFullMonteArrayAdaptor;vtkDoseHistogramWrapper;vtkSourceExporter)

FOREACH(MOD ${MODULES})
	LIST(APPEND MODULECPP "${MOD}.cpp")
ENDFOREACH()

MESSAGE("MODULECPP = ${MODULECPP}")

ADD_LIBRARY(FullMonteVTK SHARED ${MODULECPP} SwigWrapping.cpp)
TARGET_LINK_LIBRARIES(FullMonteVTK ${VTK_LIBRARIES} FullMonteData)

IF(WRAP_TCL)
    FIND_PACKAGE(VTKWrapTcl REQUIRED)
	INCLUDE(UseVTKWrapTcl.cmake)
	
	FOREACH(MOD ${MODULES})
		LIST(APPEND CXXFILES ${MOD}_wrap.cxx)
		VTK_TCL_WRAP(${MOD})
	ENDFOREACH()
	
	# Create the initialization wrapper		
	VTK_TCL_WRAP_INIT(${MODULES})
	
	# Add generated files to include path		
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
	
	# Compile it all		
	ADD_LIBRARY(FullMonteVTKTCL SHARED ${CXXFILES} vtkTCLInit.cxx)
	TARGET_LINK_LIBRARIES(FullMonteVTKTCL FullMonteVTK  ${VTK_LIBRARIES} ${TCL_LIBRARY} vtkCommonCoreTCL)

    INSTALL(TARGETS FullMonteVTKTCL
        DESTINATION lib)
ENDIF()

INSTALL (
    TARGETS FullMonteVTK
    DESTINATION lib)
