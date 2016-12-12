#ADD_LIBRARY(FullMonteQueries SHARED AbsorptionSum.cpp DirectionalSurface.cpp FluenceLineQuery.cpp  DoseVolumeHistogramGenerator.cpp  DoseSurfaceHistogramGenerator.cpp EnergyToFluence.cpp)
ADD_LIBRARY(FullMonteQueries SHARED DirectionalSurface.cpp EnergyToFluence.cpp DoseHistogramGenerator.cpp)
TARGET_LINK_LIBRARIES(FullMonteQueries FullMonteGeometry FullMonteData)

IF(WRAP_TCL)
	SET_SOURCE_FILES_PROPERTIES(FullMonteQueries.i PROPERTIES CPLUSPLUS ON)

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

	SWIG_ADD_MODULE(FullMonteQueriesTCL tcl FullMonteQueries.i)
	SWIG_LINK_LIBRARIES(FullMonteQueriesTCL FullMonteQueries ${TCL_LIBRARY})
	INSTALL(TARGETS FullMonteQueriesTCL LIBRARY DESTINATION lib)
ENDIF() 

INSTALL(TARGETS FullMonteQueries LIBRARY DESTINATION lib)
	
