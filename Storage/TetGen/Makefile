include ../../Makefile.in

libs: libFullMonteTetGen.so libFullMonteTetGen_TCL.so

FullMonteTetGen_wrap.cxx: FullMonteTetGen.i
	$(SWIG) -tcl -c++ -o $@ $^
	
FullMonteTetGen_wrap.o: FullMonteTetGen_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<

libFullMonteTetGen.so: TetGenReader.o TetGenWriter.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^ -lFullMonteGeometry 

libFullMonteTetGen_TCL.so: FullMonteTetGen_wrap.o | libFullMonteTetGen.so
	$(CXX) $(CXX_LIB_OPTS) -o $@ $< -ltclstub8.5 -L . -lFullMonteTetGen -lFullMonteGeometry 
