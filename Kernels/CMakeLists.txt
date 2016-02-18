ADD_LIBRARY(FullMonteKernelBase SHARED Kernel.cpp OStreamObserver.cpp TetraKernelBase.cpp)
 
TARGET_LINK_LIBRARIES(FullMonteKernelBase FullMonteGeometry FullMonteData ${Boost_SYSTEM_LIBRARY})

IF(WRAP_TCL)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SET_SOURCE_FILES_PROPERTIES(FullMonteKernels.i PROPERTIES CPLUSPLUS ON)
    SWIG_ADD_MODULE(FullMonteKernelsTCL tcl FullMonteKernels.i)
    SWIG_LINK_LIBRARIES(FullMonteKernelsTCL FullMonteKernelBase ${TCL_STUB_LIBRARY})	
ENDIF()

ADD_SUBDIRECTORY(Software)