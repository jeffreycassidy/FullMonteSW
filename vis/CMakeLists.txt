cmake_minimum_required(VERSION 2.6)

LINK_DIRECTORIES( /usr/lib ../fm-postgres /usr/local/lib . )

ADD_DEFINITIONS (-DSUPPRESS_AVX )

SET(CMAKE_CXX_FLAGS "-O3 -g -Wall -std=c++11 -fabi-version=6 -msse4")

INCLUDE_DIRECTORIES(/usr/local/pgsql/include ../fm-postgres ..)

PROJECT(SurfaceRender)

FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

FIND_PACKAGE(Boost REQUIRED)
IF(Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
    LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF()

ADD_EXECUTABLE(RenderDiff RenderDiff.cpp  ../histogram.cpp ../blob.o ../source.o ../RandomAVX.o ../Notifier.o ../SFMT/SFMT.o ../FullMonte.o)
TARGET_LINK_LIBRARIES(RenderDiff
    vtkCommonCore
    vtkRenderingCore
    vtkRenderingFreeTypeOpenGL
    vtkRenderingAnnotation
    vtkRenderingOpenGL
    vtkInteractionStyle
    montecarlo
    fmpg
    pq
    crypto
    boost_program_options
    boost_timer
    boost_system
    ssl
    krb5)
