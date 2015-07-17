default: libs

include Makefile.in

SUBDIRS= Geometry SFMT GUI Kernels Storage VTK 

all:
	for d in $(SUBDIRS); do make -C $$d all; done;
	
ubuntu12:
	ssh jcassidy@10.211.55.7 "cd src/FullMonte; make SOURCE_ROOT=/home/jcassidy/src ANTLR_DIR=/usr/local/antlr3 libs" 

liblinks:
	ln -sf SFMT/libSFMT.a
	ln -sf Kernels/libFullMonteKernels.so
	ln -sf Kernels/Software/libFullMonteCore.so
	ln -sf OutputTypes/libFullMonteData.so
	ln -sf Kernels/libFullMonteKernels_TCL.so
	ln -sf Geometry/libFullMonteGeometry.so
	ln -sf Geometry/libFullMonteGeometry_TCL.so
	ln -sf Storage/TIMOS/libFullMonteTIMOS.so
	ln -sf Storage/TIMOS/libFullMonteTIMOS_TCL.so
	ln -sf VTK/libFullMonteVTK.so
	ln -sf Storage/BinFile/libFullMonteBinFile.so
	ln -sf Storage/BinFile/libFullMonteBinFile_TCL.so
	ln -sf Storage/STL/libFullMonteSTL.so
	ln -sf Storage/STL/libFullMonteSTL_TCL.so
	ln -sf Storage/TetGen/libFullMonteTetGen.so
	ln -sf Storage/TetGen/libFullMonteTetGen_TCL.so
	ln -sf Storage/Matlab/libFullMonteMatlabWriter_TCL.so
	ln -sf Storage/Matlab/libFullMonteMatlabWriter.so

libs: liblinks

	for d in $(SUBDIRS); do make -C $$d libs; done;

#### Cleanup targets

veryclean: clean
	for d in $(SUBDIRS); do echo "*** Descending into $$d ***"; make -C $$d clean; done;
	

%.o: %.cpp
	$(CXX) $(CXX_OPTS) -c $<
	

##### TCL bindings for volume simulation applications

VolumeKernel_wrap.cxx: VolumeKernel.i
	$(SWIG) -tcl -c++ $<

VolumeKernel_wrap.o: VolumeKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -DUSE_TCL_STUBS -c $<

libFullMonteVolume_TCL.so: VolumeKernel_wrap.o VolumeKernel.o
	$(CXX) $(CXX_LIB_OPTS) -ltclstub8.5 -L. -lFullMonteGeometry -lFullMonteCore -o $@ $^
	

##### TCL bindings for BLI applications

BLIKernel_wrap.cxx: BLIKernel.i
	$(SWIG) -tcl -c++ $<
	
BLIKernel_wrap.o: BLIKernel_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<
	
libFullMonteBLI.so: BLIKernel.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -lFullMonteCore -o $@ $^

libFullMonteBLI_TCL.so: BLIKernel_wrap.o | libFullMonteBLI.so
	$(CXX) -shared $(CXX_OPTS) -ltclstub8.5 -L. -lFullMonteBLI -lFullMonteData -o $@ $^



##### TCL bindings for tracer

Tracer_wrap.cxx: Tracer.i
	$(SWIG) -tcl -c++ $<
	
Tracer_wrap.o: Tracer_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<
	
libFullMonteTracer.so: Tracer.o Tracer_wrap.o
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteGeometry -lFullMonteCore -lFullMonteData -ltclstub8.5 -o $@ $^
