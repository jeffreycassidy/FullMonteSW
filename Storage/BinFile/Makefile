default: test

include ../../Makefile.in

test: TestBinFile
	./TestBinFile

libs: libFullMonteBinFile.so libFullMonteBinFile_TCL.so

TestBinFile: TestBinFile.o | libFullMonteBinFile.so
	$(CXX) $(CXX_OPTS) -L. -lFullMonteGeometry -lFullMonteTIMOS -lFullMonteBinFile $^ -o $@

libFullMonteBinFile.so: BinFile.o
	$(CXX) $(CXX_LIB_OPTS) -L$(BOOST_LIB) -lFullMonteGeometry -lboost_serialization -lFullMonteGeometry -o $@ $^
	
FullMonteBinFile_wrap.cxx: FullMonteBinFile.i
	swig -tcl -c++ -o $@ $<
	
FullMonteBinFile_wrap.o: FullMonteBinFile_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $^
	
libFullMonteBinFile_TCL.so: FullMonteBinFile_wrap.o | libFullMonteBinFile.so
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^  -ltclstub8.5 -lFullMonteBinFile -lFullMonteGeometry 
