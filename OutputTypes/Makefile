default: libs 



include ../Makefile.in

all: libs trace2vtk

libs: libFullMonteData.so

%.o: %.cpp
	$(CXX) $(CXX_OPTS)  -c $<

libFullMonteData.so: fluencemap.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -o $@ $^ 


trace2vtk: trace2vtk.cpp
	$(CXX) $(CXX_OPTS) -I$(VTK_INCLUDE) -L$(VTK_LIB) -lvtkCommonCore-6.1 -lvtkIOLegacy-6.1 -lvtkCommonDataModel-6.1 -lvtkIOCore-6.1 -o $@ $^
