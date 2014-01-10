cmake_minimum_required(VERSION 2.6)

LINK_DIRECTORIES( /usr/lib . )

SET(CMAKE_CXX_FLAGS "-O3 -g -Wall -std=c++0x -msse4 -I/usr/local/include/pgsql -lfmpg -lmontecarlo -I../fm-postgres -I/usr/local/include/boost -I.. -L/usr/local/lib/boost")

PROJECT(SurfaceRender)

ADD_DEFINITIONS(-DSSE)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(Boost REQUIRED)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

#ADD_EXECUTABLE(RenderDiff RenderDiff.cpp ../newgeom.cpp ../graph.cpp ../face.cpp ../helpers.cpp ../fluencemap.cpp ../histogram.cpp ../linefile.cpp ../fm-postgres/fm-postgres.cpp fm_vtk.cpp ../blob.cpp )
ADD_EXECUTABLE(RenderDiff RenderDiff.cpp  fm_vtk.cpp ../histogram.cpp)
TARGET_LINK_LIBRARIES(RenderDiff vtkGraphics vtkIO vtkCommon vtkImaging crypto vtkHybrid boost_program_options boost_timer boost_system /usr/lib/libpq.a ssl krb5)
