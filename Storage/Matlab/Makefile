include ../../Makefile.in

libs: libFullMonteMatlabWriter.so libFullMonteMatlabWriter_TCL.so

libFullMonteMatlabWriter.so: MatlabWriter.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@

