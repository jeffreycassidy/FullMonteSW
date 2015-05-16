include ../../Makefile.in

FullMonteSTL_wrap.cxx: STL.i
	swig -tcl -c++ -o $@ $< 
	
FullMonteSTL_wrap.o: FullMonteSTL_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $<
	
libFullMonteSTL.so: STLBinary.o STLText.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^

libFullMonteSTL_TCL.so: FullMonteSTL_wrap.o | libFullMonteSTL.so
	$(CXX) $(CXX_LIB_OPTS) -lFullMonteSTL -L. -ltclstub8.5 -o $@ $<
