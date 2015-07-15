include ../Makefile.in

CXX_OPTS:=$(CXX_OPTS) -fPIC -I$(VTK_INCLUDE)
CXX_LIB_OPTS=$(CXX_OPTS) -shared

libs: libFullMonteVTK.so

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c -o $@ $<

FullMonteVTK_wrap.cxx: FullMonteVTK.hpp
	$(SWIG) -c++ -tcl -o $@ $^
	
FullMonteVTK_wrap.o: FullMonteVTK_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c -o $@ $<
	
libFullMonteVTK.so: FullMonteVTK.o FullMonteVTK_wrap.o TetraMeshTCL.o
	$(CXX) $(CXX_LIB_OPTS) -L../Geometry -L.		\
		-o $@ $^							\
		-ltclstub8.5							\
		-lvtkRenderingAnnotation-6.1					\
		-lvtkCommonDataModel-6.1 					\
		-lvtkCommonDataModelTCL-6.1					\
		-lvtkCommonCore-6.1						\
		-lvtkCommonCoreTCL-6.1						\
		-lvtkRenderingCore-6.1						\
		-lvtkInteractionWidgets-6.1					\
		-lvtkFiltersGeneral-6.1						\
		-lFullMonteGeometry 			
	
