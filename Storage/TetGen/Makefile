include ../../Makefile.in

libs: libFullMonteTetGen.so libFullMonteTetGen_TCL.so

libFullMonteTetGen.so: TetGenReader.o TetGenWriter.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^ -lFullMonteGeometry 

libFullMonteTetGen_TCL.so: FullMonteTetGen_wrap.o
    $(CXX) $(CXX_LIB_OPTS) -o $@ $^ -ltclstub8.5
