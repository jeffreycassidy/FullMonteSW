include ../../Makefile.in

libs: libFullMonteMatlabWriter.so libFullMonteMatlabWriter_TCL.so

libFullMonteMatlabWriter.so: MatlabWriter.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@

libFullMonteMatlabWriter_TCL.so: MatlabWriter_wrap.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@ -ltclstub8.5 -lFullMonteGeometry -lFullMonteMatlabWriter

