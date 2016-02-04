ADD_EXECUTABLE(Test_TetraEmitterFactory Test_TetraEmitterFactory.cpp TetraMeshEmitterFactory.cpp ../PacketDirection.cpp ../RandomAVX.cpp ../SSEMath.cpp)
TARGET_LINK_LIBRARIES(Test_TetraEmitterFactory ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} FullMonteGeometry FullMonteTIMOS SFMT)

ADD_SUBDIRECTORY(Test)
