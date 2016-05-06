ADD_SUBDIRECTORY(Emitters)

ADD_LIBRARY(FullMonteSWKernel SHARED TetraVolumeKernel.cpp TetraSurfaceKernel.cpp TetraSVKernel.cpp SSEMath.cpp ThreadedMCKernelBase.cpp PacketDirection.cpp HenyeyGreenstein.cpp)
TARGET_LINK_LIBRARIES(FullMonteSWKernel SFMT FullMonteKernelBase FullMonteData ${Boost_LIBRARIES})

## Require ANTLR3 parser if using TIM-OS file format

if (ANTLR3_INCLUDE_DIR)
	LINK_DIRECTORIES(${ANTLR3_LIB_DIR})
	
	ADD_EXECUTABLE(Test_Mouse Test_Mouse.cpp)
	TARGET_LINK_LIBRARIES(Test_Mouse FullMonteSWKernel ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT FullMonteGeometry FullMonteTIMOS)
	IF (WRAP_VTK)
		INCLUDE(${VTK_USE_FILE})
		TARGET_LINK_LIBRARIES(Test_Mouse FullMonteVTK ${VTK_LIBRARIES})

        ## Paraview visualization file
        FILE(COPY VisualizeMouse.pvsm DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	ENDIF()
	
	IF(WRAP_TCL)
		# Set up file with data path hardwired
		CONFIGURE_FILE(test_config.tcl.in test_config.tcl)
		CONFIGURE_FILE(tclmonte.sh.in tclmonte.sh)
		
		SET_SOURCE_FILES_PROPERTIES(FullMonteSWKernel.i PROPERTIES CPLUSPLUS ON)
    	SWIG_ADD_MODULE(FullMonteSWKernelTCL tcl FullMonteSWKernel.i)
    	SWIG_LINK_LIBRARIES(FullMonteSWKernelTCL ${TCL_STUB_LIBRARY} FullMonteGeometry FullMonteSWKernel)
		
#		FILE(COPY test_mouse.tcl
#			DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	ENDIF()
ENDIF()

ADD_EXECUTABLE(Test_BlockRandomDistribution Test_BlockRandomDistribution.cpp HenyeyGreenstein.cpp)
TARGET_LINK_LIBRARIES(Test_BlockRandomDistribution ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

ADD_EXECUTABLE(Test_RNG Test_RNG.cpp)
TARGET_LINK_LIBRARIES(Test_RNG ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT)
