default: test

include ../CommonParser/Makefile.in

CXX_OPTS=-Wall -fPIC -O3 -g -std=c++11 -c -I$(SOURCE_ROOT)

CXX_LIB_OPTS=-Wall -fPIC -shared -O3 -mavx -Wa,-q
CXX=g++

Test.o: Test.cpp TIMOSLexer.h TIMOSParser.h
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) $<
	
%.o: %.cpp TIMOS_tokens.h
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) $<

libFullMonteTIMOS.so: TIMOSParser.o TIMOSLexer.o TIMOSReader.o TIMOSWriter.o | TIMOS_tokens.h
	$(CXX) $(CXX_LIB_OPTS) -L$(ANTLR_LIB) -L../../Geometry -lantlr3c -lFullMonteGeometry -o $@ $^
	
TIMOSReader_wrap.cxx: TIMOSReader.i TIMOSReader.hpp
	swig -tcl -c++ $<
	
TIMOSReader_wrap.o: TIMOSReader_wrap.cxx
	$(CXX) $(CXX_OPTS) -DUSE_TCL_STUBS -c $^
	
libFullMonteTIMOS_TCL.so: TIMOSReader_wrap.o
	$(CXX) $(CXX_LIB_OPTS) -ltclstub8.5 -L../../Geometry -L. -lFullMonteTIMOS -lFullMonteGeometry -o $@ $^ 

test: Test.o libFullMonteTIMOS.so
	$(CXX) $(CXX_LINK_OPTS) -L$(ANTLR_LIB) -L../../Geometry -L. -lantlr3c -lFullMonteTIMOS -lFullMonteGeometry -o $@ Test.o

clean: parser-clean
	rm -f test *.so *.a *.o *_wrap.cxx *.dSYM
