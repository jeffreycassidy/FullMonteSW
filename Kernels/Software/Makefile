include ../../Makefile.in

all: libFullMonteCore.so

##### Core functions (RNG, basic loggers, result types, and OStreamObserver)

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -c $< -o $@

random.o: random.cpp random.hpp
	$(CXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

libFullMonteCore.so: LoggerSurface.o LoggerConservation.o LoggerEvent.o sse.o  RandomAVX.o OStreamObserver.o FullMonte.o \
	fluencemap.o LoggerEvent.o LoggerVolume.o FullMonte.o
	$(CXX) $(CXX_OPTS) -shared -L. -lFullMonteGeometry -L../../SFMT -lSFMT -L../../Geometry -lboost_system -lboost_timer -lboost_chrono -o $@ $^
