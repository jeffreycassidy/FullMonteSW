CXX_OPTS=-Wall -std=c++11 -O3 -fPIC -g
CXX_LIB_OPTS=-Wall -fPIC -shared -O3
CXX=g++

default: test

include ../CommonParser/Makefile.in

ANTLRTreeWalker.o: ANTLRTreeWalker.cpp
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) -c $<

Test.o: Test.cpp
	$(CXX) $(CXX_OPTS) -I$(ANTLR_INCLUDE) -c $<
	
libTIMOS.so: TIMOSParser.o TIMOSLexer.o
	$(CXX) $(CXX_LIB_OPTS) -L$(ANTLR_LIB) -lantlr3c -o $@ $^

test: Test.o libTIMOS.so ../../SourceDescription.o ../../Material.o
	$(CXX) $(CXX_LINK_OPTS) -L. -lantlr3c -lTIMOS ../../SourceDescription.o ../../Material.o ../../newgeom.o -o $@ Test.o
	#g++ -Wall -std=c++11 -O3 -L$(ANTLR_LIB) -lantlr3c -o $@ $^

clean: parser-clean