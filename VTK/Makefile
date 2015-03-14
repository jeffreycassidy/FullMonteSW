include ../Makefile.in

CXX_OPTS:=$(CXX_OPTS) -fPIC -I/usr/local/include/vtk
CXX_LIB_OPTS=$(CXX_OPTS) -shared

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c -o $@ $<

FullMonteVTK_wrap.cxx: FullMonteVTK.hpp
	$(SWIG) -c++ -tcl -o $@ $^
	
FullMonteVTK_wrap.o: FullMonteVTK_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c -o $@ $<
	
libFullMonteVTK.so: FullMonteVTK.o FullMonteVTK_wrap.o TetraMeshTCL.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -L../Geometry -L.			\
		-ltclstub8.5											\
		-lvtkRenderingAnnotation-6.1							\
		-lvtkCommonDataModel-6.1 								\
		-lvtkCommonDataModelTCL-6.1								\
		-lvtkCommonCore-6.1										\
		-lvtkCommonCoreTCL-6.1									\
		-lvtkRenderingCore-6.1									\
		-o $@ $^

clean:
	rm -rf *.dSYM
	rm -f *.so *.o *.a *_wrap.cxx
