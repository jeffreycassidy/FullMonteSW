default: test

include ../../Makefile.in

test: TestBinFile
	./TestBinFile

libs: libFullMonteBinFile.so libFullMonteBinFile_TCL.so

TestBinFile: TestBinFile.o | libFullMonteBinFile.so
	$(CXX) $(CXX_OPTS) -L. -lFullMonteGeometry -lFullMonteTIMOS -lFullMonteBinFile $^ -o $@
	
libFullMonteBinFile_TCL.so: FullMonteBinFile_wrap.o | libFullMonteBinFile.so
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^  -ltclstub8.5 -lFullMonteBinFile -lFullMonteGeometry 
