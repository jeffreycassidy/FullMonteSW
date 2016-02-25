INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
ADD_EXECUTABLE(UnitTests UnitTests.cpp)
TARGET_LINK_LIBRARIES(UnitTests ${Boost_unit_test_framework_LIBRARIES} FullMonteGeometry FullMonteVTKFile)

ADD_CUSTOM_TARGET(VTKFileTests)
ADD_DEPENDENCIES(VTKFileTests RunUnitTests)

ADD_DEPENDENCIES(Tests VTKFileTests)

CONFIGURE_FILE(TestConfig.h.in TestConfig.h)

ADD_CUSTOM_TARGET(RunUnitTests
    COMMAND ./UnitTests
    DEPENDS knee.mesh.vtk UnitTests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
