include ../../Makefile.in

libs:

all: libs

libs: libFullMonteCore.so

##### Core functions (RNG, basic loggers, result types, and OStreamObserver)

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $< -o $@

random.o: random.cpp random.hpp
	$(CXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@
	
	
Logger%.o: Logger/Logger%.cpp
	$(CXX) $(CXX_OPTS) -c $^

libFullMonteCore.so: LoggerSurface.o LoggerConservation.o LoggerEvent.o sse.o  RandomAVX.o OStreamObserver.o FullMonte.o \
	LoggerEvent.o LoggerVolume.o FullMonte.o
	$(CXX) $(CXX_LIB_OPTS) -shared -L. -lFullMonteGeometry -lFullMonteData -L../../SFMT -lSFMT -lboost_system -lboost_timer -lboost_chrono -o $@ $^