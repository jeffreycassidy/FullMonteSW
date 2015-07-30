include ../../Makefile.in

libs: libFullMonteMatlabWriter.so libFullMonteMatlabWriter_TCL.so

libFullMonteMatlabWriter.so: MatlabWriter.o
	$(CXX) $(CXX_LIB_OPTS) $^ -o $@


## Unit tests

Test_Permutation: Test_Permutation.cpp Permutation.hpp
	g++ -fPIC -O3 -g -std=c++11 -I/usr/local/include/boost -L/usr/local/lib -lboost_unit_test_framework -o $@ $<