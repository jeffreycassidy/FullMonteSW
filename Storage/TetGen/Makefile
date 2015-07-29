include ../../Makefile.in

libs: libFullMonteTetGen.so libFullMonteTetGen_TCL.so

libFullMonteTetGen.so: TetGenReader.o TetGenWriter.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^ -lFullMonteGeometry 
