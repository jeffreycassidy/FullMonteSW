IF(WRAP_VTK)
    FIND_PACKAGE(VTK REQUIRED)
	INCLUDE_DIRECTORIES(${VTK_INCLUDE_DIRS})
    ADD_DEFINITIONS(-DVTK_OUTPUT)

    SET(EXTRA_SOURCES VTKPointCloud.cpp VTKLineCluster.cpp)
    SET(EXTRA_LIBS vtkIOLegacy vtkCommonDataModel)

ADD_EXECUTABLE(Test_Emitters Test_Emitters.cpp ${EXTRA_SOURCES} ../../PacketDirection.cpp ../../SSEMath.cpp ../TetraMeshEmitterFactory.cpp)
TARGET_LINK_LIBRARIES(Test_Emitters ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ${EXTRA_LIBS} SFMT FullMonteGeometry FullMonteTIMOS)

ELSE()
	MESSAGE("Test_Emitters cannot be built because VTK libraries are not provided")
ENDIF()

