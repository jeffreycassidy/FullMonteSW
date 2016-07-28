INCLUDE(${VTK_USE_FILE})

ADD_LIBRARY(FullMonteVTK SHARED vtkFullMonteTetraMeshBaseWrapper.cpp vtkFullMonteSpatialMapWrapperFU.cpp vtkFullMonteTetraMeshWrapper.cpp vtkFullMonteFluenceLineQueryWrapper.cpp SwigWrapping.cpp)
TARGET_LINK_LIBRARIES(FullMonteVTK ${VTK_LIBRARIES})

IF(WRAP_TCL)
    FIND_PACKAGE(VTKWrapTcl REQUIRED)
	INCLUDE(UseVTKWrapTcl.cmake)
	
	SET(MODULES vtkFullMonteTetraMeshWrapper;vtkFullMonteTetraMeshBaseWrapper;vtkFullMonteSpatialMapWrapperFU)
	
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
