include ../Makefile.in

CXX_OPTS:=$(CXX_OPTS) -I$(VTK_INCLUDE) -I$(TCL_INCLUDE)

libs: libFullMonteVTK.so

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c -o $@ $<

libFullMonteVTK.so: FullMonteVTK.o FullMonteVTK_wrap.o TetraMeshTCL.o
	$(CXX) $(CXX_LIB_OPTS) -L../Geometry -L$(VTK_LIB) -L.		\
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
	
