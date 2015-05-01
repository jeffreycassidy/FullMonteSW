include ../Makefile.in

CXX_LIB_OPTS=$(CXX_OPTS) -shared

all: libs

libs: libFullMonteGeometry.so

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o TetraMesh.o Face.o Tetra.o newgeom.o
	$(CXX) $(CXX_OPTS) -o $@ $^

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $*.cpp -o $@

libFullMonteGeometry.so: TetraMeshBase.o newgeom.o TetraMesh.o TetraMeshBase.o Face.o SourceDescription.o Material.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^