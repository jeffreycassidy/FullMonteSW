default: libs

include Makefile.in

SUBDIRS=DVH Geometry GUI Kernels Storage VTK SFMT Test vis VTK

all:
	for d in $(SUBDIRS); do make -C $$d all; done; 

libs:
	for d in $(SUBDIRS); do make -C $$d libs; done;

#### Cleanup targets

veryclean: clean
	for d in $(SUBDIRS); do echo "*** Descending into $$d ***"; make -C $$d clean; done;
	

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c $<
	

##### TCL bindings for volume simulation applications

VolumeKernel_wrap.cxx: VolumeKernel.i
	swig -tcl -c++ $<

VolumeKernel_wrap.o: VolumeKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -DUSE_TCL_STUBS -c $<

libFullMonteVolume_TCL.so: VolumeKernel_wrap.o VolumeKernel.o
	$(CXX) $(CXX_LIB_OPTS) -ltclstub8.5 -L. -lFullMonteGeometry -lFullMonteCore -o $@ $^
	

##### TCL bindings for BLI applications

BLIKernel_wrap.cxx: BLIKernel.i
	swig -tcl -c++ $<
	
BLIKernel_wrap.o: BLIKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<
	
libFullMonteBLI.so: BLIKernel.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -lFullMonteCore -o $@ $^

libFullMonteBLI_TCL.so: BLIKernel_wrap.o | libFullMonteBLI.so
	$(CXX) -shared $(CXX_OPTS) -ltclstub8.5 -L. -lFullMonteBLI -lFullMonteData -o $@ $^



##### TCL bindings for tracer

Tracer_wrap.cxx: Tracer.i
	swig -tcl -c++ $<
	
Tracer_wrap.o: Tracer_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<
	
libFullMonteTracer.so: Tracer.o Tracer_wrap.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -lFullMonteCore -lFullMonteData -ltclstub8.5 -o $@ $^
