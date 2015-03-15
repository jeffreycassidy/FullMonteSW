# base options only

default: libs

include Makefile.in

SUBDIRS=Geometry Storage VTK SFMT Software

Geometry/%:
	make -C Geometry $*
	
Storage/TIMOS/%:
	make -C Storage/TIMOS $*
	
VTK/%:
	make -C VTK $*
	
SFMT/%:
	make -C SFMT $*
	
Kernels/Software/%:
	make -C Kernels/Software $* 

libs: Geometry/libFullMonteGeometry.so Storage/TIMOS/libFullMonteTIMOS.so Storage/TIMOS/libFullMonteTIMOS_TCL.so SFMT/libSFMT.a \
	VTK/libFullMonteVTK.so Kernels/Software/libFullMonteCore.so libFullMonteBLI.so libFullMonteBLI_TCL.so

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $< -o $@
	

##### BLI-kernel related items (surface logging only)

libFullMonteBLI.so: BLIKernel.o
	$(CXX) $(CXX_OPTS) -shared -L. -lFullMonteGeometry -LKernels/Software -LGeometry -lboost_system -lboost_chrono -lFullMonteCore -o $@ $^


##### TCL bindings for BLI applications

BLIKernel_wrap.cxx: BLIKernel.i
	swig -tcl -c++ $<
	
BLIKernel_wrap.o: BLIKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -DUSE_TCL_STUBS -c $<

libFullMonteBLI_TCL.so: BLIKernel_wrap.o
	$(CXX) -shared $(CXX_OPTS) -ltclstub8.5 -L. -lFullMonteBLI -LGeometry -lFullMonteGeometry -o $@ $^
	
	
	
##### TCL bindings for volume simulation applications

VolumeKernel_wrap.cxx: VolumeKernel.i
	swig -tcl -c++ $<

VolumeKernel_wrap.o: VolumeKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -DUSE_TCL_STUBS -c $<

libFullMonteVolume_TCL.so: VolumeKernel_wrap.o VolumeKernel.o
	$(CXX) -shared $(CXX_OPTS) -ltclstub8.5 -L. -LGeometry -lFullMonteGeometry -LKernels/Software -lFullMonteCore -o $@ $^


#### Cleanup targets

veryclean: clean
	make -C Geometry clean
	make -C Storage/TIMOS clean
	make -C Storage/CommonParser clean
	make -C VTK clean
	make -C SFMT clean

clean:
	rm -rf *.o *.a *.so *.dSYM *_wrap.cxx
