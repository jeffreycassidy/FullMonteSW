ADD_LIBRARY(FullMonteData SHARED OutputDataSummarize.cpp MCConservationCounts.cpp OutputData.cpp MCEventCounts.cpp)
#TARGET_LINK_LIBRARIES(FullMonteData FullMonteGeometry)

#IF (WRAP_TCL)
#    SET_SOURCE_FILES_PROPERTIES(FullMonteData.i PROPERTIES CPLUSPLUS ON)

#    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

#    SWIG_ADD_MODULE(FullMonteDataTCL tcl FullMonteData.i)
#    SWIG_LINK_LIBRARIES(FullMonteDataTCL FullMonteData ${TCL_STUB_LIBRARY})
#ENDIF()

#ADD_EXECUTABLE(Test_OutputTypes Test_OutputTypes.cpp)
#TARGET_LINK_LIBRARIES(Test_OutputTypes boost_unit_test_framework)

ADD_EXECUTABLE(Test_Containers Test_Containers.cpp)
TARGET_LINK_LIBRARIES(Test_Containers ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

ADD_EXECUTABLE(Test_OutputTypes Test_OutputTypes.cpp OutputDataSummarize.cpp MCConservationCounts.cpp OutputData.cpp MCEventCounts.cpp)
TARGET_LINK_LIBRARIES(Test_OutputTypes ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})