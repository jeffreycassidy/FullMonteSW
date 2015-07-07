include ../Makefile.in
#find the place for this -march=native (-msse4.1) 
CXX_LIB_OPTS=$(CXX_OPTS) -shared

all: libs

libs: libFullMonteGeometry.so

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o TetraMesh.o Face.o Tetra.o newgeom.o
	$(CXX) $(CXX_OPTS) -o $@ $^

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $*.cpp -o $@

libFullMonteGeometry.so: TetraMeshBase.o newgeom.o Face.o SourceDescription.o Material.o TetraMesh.o
	$(CXX) $(CXX_LIB_OPTS) -o  $@ $^
	
FullMonteGeometry_wrap.cxx: Geometry.i Geometry_types.i *.hpp
	$(SWIG) -tcl -c++ -o $@ $<
	
FullMonteGeometry_wrap.o: FullMonteGeometry_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<

libFullMonteGeometry_TCL.so: FullMonteGeometry_wrap.o | libFullMonteGeometry.so
	$(CXX) $(CXX_LIB_OPTS) -o $@ $< -lFullMonteGeometry -ltclstub8.5 

