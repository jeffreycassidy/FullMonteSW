CXX_OPTS=-Wall -std=c++11 -O3 -fPIC -g -mavx
CXX_LIB_OPTS=-Wall -fPIC -shared -O3 -mavx
CXX=g++

default: test

include ../CommonParser/Makefile.in

Test.o: Test.cpp TIMOSLexer.h TIMOSParser.h
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) -c $<
	
TIMOSReader.o: TIMOSReader.cpp TIMOS_tokens.h
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) -c $<

	
libFullMonteTIMOS.so: TIMOSParser.o TIMOSLexer.o TIMOSReader.o
	$(CXX) $(CXX_LIB_OPTS) -L$(ANTLR_LIB) -lantlr3c -o $@ $^

test: Test.o libFullMonteTIMOS.so ../../libFullMonteGeometry.so
	$(CXX) $(CXX_LINK_OPTS) -L$(ANTLR_LIB) -L. -lantlr3c -lFullMonteTIMOS -lFullMonteGeometry -o $@ Test.o
	#g++ -Wall -std=c++11 -O3 -L$(ANTLR_LIB) -lantlr3c -o $@ $^

clean: parser-clean