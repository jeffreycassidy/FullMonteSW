include ../Makefile.in

all: libs

libs: libFullMonteGeometry.so libFullMonteGeometry_TCL.so

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o TetraMesh.o Face.o Tetra.o newgeom.o
	$(CXX) $(CXX_OPTS) -o $@ $^

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $*.cpp -o $@

libFullMonteGeometry.so: TetraMeshBase.o newgeom.o Face.o SourceDescription.o Material.o TetraMesh.o
	$(CXX) $(CXX_LIB_OPTS) -o  $@ $^
