ADD_SUBDIRECTORY(Emitters)

ADD_LIBRARY(FullMonteSWKernel SHARED TetraVolumeKernel.cpp TetraSurfaceKernel.cpp SSEMath.cpp ThreadedMCKernelBase.cpp Material.cpp sse.cpp RandomAVX.cpp PacketDirection.cpp)
TARGET_LINK_LIBRARIES(FullMonteSWKernel SFMT FullMonteKernelBase FullMonteData)

#IF(ANTLR3_FOUND)
	LINK_DIRECTORIES(${ANTLR3_LIB_DIR})
	ADD_EXECUTABLE(Test_Mouse Test_Mouse.cpp)
	TARGET_LINK_LIBRARIES(Test_Mouse FullMonteSWKernel ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT FullMonteGeometry FullMonteTIMOS)
#ENDIF()
