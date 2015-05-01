default: libs 

all: libs trace2vtk

include ../Makefile.in

libs: libFullMonteData.so

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c $<

libFullMonteData.so: fluencemap.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -o $@ $^ 


trace2vtk: trace2vtk.cpp
	$(CXX) $(CXX_OPTS) -I/usr/local/include/vtk -L/usr/local/lib/vtk -lvtkCommonCore-6.1 -lvtkIOLegacy-6.1 -lvtkCommonDataModel-6.1 -lvtkIOCore-6.1 -o $@ $^