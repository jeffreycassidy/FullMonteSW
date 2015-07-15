include ../../Makefile.in

all: libs

libs: libFullMonteCore.so

##### Core functions (RNG, basic loggers, result types, and OStreamObserver)

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -c $< -o $@

random.o: random.cpp random.hpp
	$(CXX) -I$(SOURCE_ROOT) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@
	
	
Logger%.o: Logger/Logger%.cpp
	$(CXX) $(CXX_OPTS) -I$(SOURCE_ROOT) -c $^

libFullMonteCore.so: LoggerSurface.o LoggerConservation.o LoggerEvent.o sse.o  RandomAVX.o OStreamObserver.o FullMonte.o \
	LoggerEvent.o LoggerVolume.o FullMonte.o
	$(CXX) $(CXX_LIB_OPTS) -L$(BOOST_LIB) -shared -L. -lFullMonteGeometry -lFullMonteData -L../../SFMT -lSFMT -lboost_system -lboost_timer -lboost_chrono -o $@ $^
