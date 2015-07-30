default: test

include ../../Makefile.in

test: TestBinFile
	./TestBinFile

libs: libFullMonteBinFile.so libFullMonteBinFile_TCL.so

libFullMonteBinFile.so: BinFile.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@

libFullMonteBinFile_TCL.so: FullMonteBinFile_wrap.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@ -L$(BOOST_LIB) -lboost_serialization -ltclstub8.5 -lFullMonteBinFile

TestBinFile: TestBinFile.o | libFullMonteBinFile.so
	$(CXX) $(CXX_OPTS) -L. -lFullMonteGeometry -lFullMonteTIMOS -lFullMonteBinFile $^ -o $@
	
libFullMonteBinFile_TCL.so: FullMonteBinFile_wrap.o | libFullMonteBinFile.so
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^  -ltclstub8.5 -lFullMonteBinFile -lFullMonteGeometry 
