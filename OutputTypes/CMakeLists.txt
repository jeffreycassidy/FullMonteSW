ADD_LIBRARY(FullMonteData SHARED AbstractSpatialMap.cpp OutputDataSummarize.cpp MCConservationCounts.cpp PacketPositionTrace.cpp PacketPositionTraceSet.cpp OutputData.cpp MCEventCounts.cpp)
TARGET_LINK_LIBRARIES(FullMonteData FullMonteGeometry)

IF (WRAP_TCL)
    SET_SOURCE_FILES_PROPERTIES(FullMonteData.i PROPERTIES CPLUSPLUS ON)

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SWIG_ADD_MODULE(FullMonteDataTCL tcl FullMonteData.i)
    SWIG_LINK_LIBRARIES(FullMonteDataTCL FullMonteData ${TCL_LIBRARY})

    INSTALL(TARGETS FullMonteDataTCL LIBRARY DESTINATION lib)
ENDIF()

INSTALL(TARGETS FullMonteData
	LIBRARY
	DESTINATION lib)