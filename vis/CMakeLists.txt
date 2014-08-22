cmake_minimum_required(VERSION 2.6)

LINK_DIRECTORIES( /usr/lib ../fm-postgres )

SET(CMAKE_CXX_FLAGS "-O3 -g -Wall -std=c++11 -fabi-version=6 -msse4 -I/usr/local/include/pgsql -lfmpg -lmontecarlo -I../fm-postgres -I/usr/local/include/boost -I..")

INCLUDE_DIRECTORIES("/usr/local/include/pgsql")

PROJECT(SurfaceRender)

#ADD_DEFINITIONS(-DSSE)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(Boost REQUIRED)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
ENDIF()

ADD_EXECUTABLE(RenderDiff RenderDiff.cpp  ../histogram.cpp)
TARGET_LINK_LIBRARIES(RenderDiff vtkGraphics vtkIO vtkCommon vtkImaging crypto vtkHybrid boost_program_options boost_timer boost_system /usr/lib/libpq.a ssl krb5)
