include ../Makefile.in

all: libs

libs: libFullMonteGeometry.so libFullMonteGeometry_TCL.so

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o TetraMesh.o Face.o Tetra.o newgeom.o
	$(CXX) $(CXX_OPTS) -o $@ $^

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $*.cpp -o $@

libFullMonteGeometry.so: TetraMeshBase.o newgeom.o Face.o SourceDescription.o Material.o TetraMesh.o
	$(CXX) $(CXX_LIB_OPTS) -o  $@ $^

Test_Perm: Test_Perm.cpp TetraMesh.hpp
	$(CXX) $(CXX_OPTS) -o $@ $<
	
RayWalk: RayWalk.cpp
	$(CXX) $(CXX_OPTS) -o $@ $< -lFullMonteGeometry -lFullMonteTIMOS
	
ever_circle: ever_circle.cpp
	$(CXX) $(CXX_OPTS) -o $@ $<