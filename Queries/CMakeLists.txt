#ADD_LIBRARY(FullMonteQueries SHARED AbsorptionSum.cpp DirectionalSurface.cpp FluenceLineQuery.cpp  DoseVolumeHistogramGenerator.cpp  DoseSurfaceHistogramGenerator.cpp EnergyToFluence.cpp)
ADD_LIBRARY(FullMonteQueries SHARED DirectionalSurface.cpp EnergyToFluence.cpp DoseHistogramGenerator.cpp)
#ADD_LIBRARY(FullMonteQueries SHARED FluenceLineQuery.hpp)
TARGET_LINK_LIBRARIES(FullMonteQueries FullMonteGeometry FullMonteData)

SET_SOURCE_FILES_PROPERTIES(FullMonteQueries.i PROPERTIES CPLUSPLUS ON)

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SWIG_ADD_MODULE(FullMonteQueriesTCL tcl FullMonteQueries.i)
SWIG_LINK_LIBRARIES(FullMonteQueriesTCL FullMonteQueries tcl) 



#ADD_EXECUTABLE(Test_EmpiricalCDF Test_EmpiricalCDF.cpp)
#TARGET_LINK_LIBRARIES(Test_EmpiricalCDF ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

ADD_EXECUTABLE(Test_DrawDSH EmpiricalCDF.old.cpp)
TARGET_LINK_LIBRARIES(Test_DrawDSH ${VTK_LIBRARIES})

#ADD_EXECUTABLE(DSH EmpiricalCDF.cpp)
#TARGET_LINK_LIBRARIES(DSH ${VTK_LIBRARIES})

INSTALL(TARGETS FullMonteQueries LIBRARY DESTINATION lib)
INSTALL(TARGETS FullMonteQueriesTCL LIBRARY DESTINATION lib)