ADD_LIBRARY(FullMonteKernels SHARED Kernel.cpp Software/Logger/LoggerSurface.cpp Software/Logger/LoggerVolume.cpp Software/Logger/LoggerConservation.cpp Software/Logger/LoggerEvent.cpp Software/sse.cpp Software/RandomAVX.cpp Software/OStreamObserver.cpp Software/FullMonte.cpp)
TARGET_LINK_LIBRARIES(FullMonteKernels FullMonteGeometry FullMonteData ${Boost_LIBRARIES} SFMT)

IF(WRAP_TCL)
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SET_SOURCE_FILES_PROPERTIES(FullMonteKernels.i PROPERTIES CPLUSPLUS ON)
    SWIG_ADD_MODULE(FullMonteKernelsTCL tcl FullMonteKernels.i)
    SWIG_LINK_LIBRARIES(FullMonteKernelsTCL FullMonteKernels ${TCL_STUB_LIBRARY})	
ENDIF()


ADD_SUBDIRECTORY(Software)