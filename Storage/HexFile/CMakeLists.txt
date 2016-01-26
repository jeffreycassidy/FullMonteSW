cmake_minimum_required(VERSION 3.0)

find_package(VTK REQUIRED)

include_directories(/Users/jcassidy/src)
link_directories(/Users/jcassidy/src/FullMonteSW/FullMonte /Users/jcassidy/src/FullMonteSW/FullMonte/VTK)

include(${VTK_USE_FILE})

add_definitions(-std=c++11 -Wall -O3 -DUSE_TCL_STUBS)

add_executable(Test_HexFile Test_HexFile.cpp HexFileReader.cpp)
target_link_libraries(Test_HexFile boost_unit_test_framework gmp)

add_executable(Test_LineFile Test_LineFile.cpp)
target_link_libraries(Test_LineFile boost_unit_test_framework)

add_executable(HexConv HexConv.cpp HexFileReader.cpp)
target_link_libraries(HexConv FullMonteGeometry FullMonteVTK FullMonteTIMOS vtkCommonDataModel-6.1 vtkIOLegacy)

set(TCLMODULE HexFile)

ADD_CUSTOM_COMMAND(OUTPUT ${TCLMODULE}_wrap.cxx COMMAND swig -c++ -tcl ${TCLMODULE}.hpp
	DEPENDS ${TCLMODULE}.hpp)
	
add_library(FullMonteHexFile SHARED HexFileReader.cpp)

add_library(FullMonteHexFileTCL SHARED HexFile_wrap.cxx)
target_link_libraries(FullMonteHexFileTCL FullMonteHexFile libtclstub8.5.a)