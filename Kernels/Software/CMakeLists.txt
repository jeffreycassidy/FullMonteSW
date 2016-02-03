ADD_SUBDIRECTORY(Emitters)

ADD_LIBRARY(FullMonteSWKernel SHARED TetraVolumeKernel.cpp TetraSurfaceKernel.cpp SSEMath.cpp ThreadedMCKernelBase.cpp Material.cpp Logger/LoggerSurface.cpp Logger/LoggerVolume.cpp Logger/LoggerConservation.cpp Logger/LoggerEvent.cpp sse.cpp RandomAVX.cpp FullMonte.cpp PacketDirection.cpp)
TARGET_LINK_LIBRARIES(FullMonteSWKernel SFMT FullMonteKernelBase)

ADD_EXECUTABLE(Test_Mouse Test_Mouse.cpp)
TARGET_LINK_LIBRARIES(Test_Mouse FullMonteSWKernel ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} SFMT FullMonteGeometry FullMonteTIMOS)