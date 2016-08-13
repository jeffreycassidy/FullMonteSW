ADD_LIBRARY(FullMonteData SHARED FluenceConverter.cpp OutputDataSummarize.cpp MCConservationCounts.cpp OutputData.cpp MCEventCounts.cpp AbsorptionSum.cpp DirectionalSurface.cpp FluenceLineQuery.cpp)
#DoseSurfaceHistogramGenerator.cpp)
TARGET_LINK_LIBRARIES(FullMonteData FullMonteGeometry)

IF (WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(FullMonteData.i PROPERTIES CPLUSPLUS ON)

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SWIG_ADD_MODULE(FullMonteDataTCL tcl FullMonteData.i)
    SWIG_LINK_LIBRARIES(FullMonteDataTCL FullMonteData tcl)

    INSTALL(TARGETS FullMonteDataTCL LIBRARY DESTINATION lib)
ENDIF()

#ADD_EXECUTABLE(Test_EmpiricalCDF Test_EmpiricalCDF.cpp)
#TARGET_LINK_LIBRARIES(Test_EmpiricalCDF ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

ADD_EXECUTABLE(Test_Containers Test_Containers.cpp)
TARGET_LINK_LIBRARIES(Test_Containers ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} FullMonteData)

ADD_EXECUTABLE(Test_DrawDSH EmpiricalCDF.cpp)
TARGET_LINK_LIBRARIES(Test_DrawDSH ${VTK_LIBRARIES})

ADD_EXECUTABLE(Test_OutputTypes Test_OutputTypes.cpp OutputDataSummarize.cpp MCConservationCounts.cpp OutputData.cpp MCEventCounts.cpp)
TARGET_LINK_LIBRARIES(Test_OutputTypes ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

INSTALL(TARGETS FullMonteData
	LIBRARY
	DESTINATION lib)
