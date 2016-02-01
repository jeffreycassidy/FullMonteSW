ADD_EXECUTABLE(Test_EmitterFactory Test_EmitterFactory.cpp EmitterFactory.cpp ../Packet.cpp ../RandomAVX.cpp ../SSEMath.cpp)
TARGET_LINK_LIBRARIES(Test_EmitterFactory ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} FullMonteGeometry FullMonteTIMOS SFMT)

ADD_SUBDIRECTORY(Test)
