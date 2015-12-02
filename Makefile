default: libs

include Makefile.in

SUBDIRS=Geometry OutputTypes SFMT GUI Kernels Storage VTK

all:
	for d in $(SUBDIRS); do make -C $$d all; done;
	
liblinks:
	ln -sf SFMT/libSFMT.a
	ln -sf Kernels/libFullMonteKernel.so
	ln -sf Kernels/Software/libFullMonteCore.so
	ln -sf OutputTypes/libFullMonteData.so
	ln -sf Kernels/libFullMonteKernel_TCL.so
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
	ln -sf Storage/Matlab/libFullMonteMatlabText_TCL.so
	ln -sf Storage/Matlab/libFullMonteMatlabText.so
	ln -sf OutputTypes/libFullMonteData.dylib
	ln -sf OutputTypes/libFullMonteDataTCL.dylib
	ln -sf Storage/HexFile/libFullMonteHexFileTCL.dylib
	ln -sf Storage/HexFile/libFullMonteHexFile.dylib

libs: liblinks

	for d in $(SUBDIRS); do make -C $$d libs; done;

#### Cleanup targets

veryclean: clean
	for d in $(SUBDIRS); do echo "*** Descending into $$d ***"; make -C $$d clean; done;
