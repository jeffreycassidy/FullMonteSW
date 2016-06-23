default: test

include ../../Makefile.in

test: TestBinFile
	./TestBinFile

libs: libFullMonteBinFile.so libFullMonteBinFile_TCL.so

libFullMonteBinFile.so: BinFile.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@ -lboost_serialization -lFullMonteGeometry

libFullMonteBinFile_TCL.so: FullMonteBinFile_wrap.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@ -L$(BOOST_LIB) -ltclstub8.5 -lFullMonteBinFile -lFullMonteGeometry

TestBinFile: TestBinFile.o | libFullMonteBinFile.so
	$(CXX) $(CXX_OPTS) -L. -lFullMonteGeometry -lFullMonteTIMOS -lFullMonteBinFile $^ -o $@
