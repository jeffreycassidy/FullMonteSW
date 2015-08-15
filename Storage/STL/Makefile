include ../../Makefile.in

libs: libFullMonteSTL_TCL.so 

libFullMonteSTL.so: STLBinary.o STLText.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^
