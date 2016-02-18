ADD_SUBDIRECTORY(Emitters)

ADD_LIBRARY(FullMonteSWKernel SHARED TetraVolumeKernel.cpp TetraSurfaceKernel.cpp SSEMath.cpp ThreadedMCKernelBase.cpp sse.cpp PacketDirection.cpp)
TARGET_LINK_LIBRARIES(FullMonteSWKernel SFMT FullMonteKernelBase FullMonteData)

if (ANTLR3_INCLUDE_DIR)
	LINK_DIRECTORIES(${ANTLR3_LIB_DIR})
	ADD_EXECUTABLE(Test_Mouse Test_Mouse.cpp)
	TARGET_LINK_LIBRARIES(Test_Mouse FullMonteSWKernel ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT FullMonteGeometry FullMonteTIMOS)
ENDIF()

ADD_DEFINITIONS(-mavx2)

ADD_EXECUTABLE(Test_BlockRandomDistribution Test_BlockRandomDistribution.cpp HenyeyGreenstein.cpp)
TARGET_LINK_LIBRARIES(Test_BlockRandomDistribution ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

ADD_EXECUTABLE(Test_RNG Test_RNG.cpp)
TARGET_LINK_LIBRARIES(Test_RNG ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT)