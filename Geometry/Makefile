CXX=g++
CXX_OPTS=-Wall -mfpmath=sse -g -DNDEBUG -O3 -mavx -std=c++11 -fPIC -Wa,-q -I/Users/jcassidy/src
CXX_LIB_OPTS=$(CXX_OPTS) -shared

# base options only
#no-deprecated is for SHA1 hash

#LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lboost_chrono -lSFMT -lfmpg
#LIBDIRS=-L/usr/local/lib -L$(BOOST_LIB) -LSFMT -L. -Lfm-postgres
#INCLDIRS=-I$(BOOST_INCLUDE) -I/usr/local/include -I. -I/usr/local/include/pgsql -I$(VTK_INCLUDE) -I..

#SWIG=swig

# Check for Mac OS (no POSIX timer)
#OS:=$(shell uname)
#ifeq ($(OS),Darwin)
#CXX_OPTS += -DPLATFORM_DARWIN
#endif

#ifndef CXX
#	CXX=g++
#endif

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o TetraMesh.o Face.o Tetra.o newgeom.o
	$(CXX) $(CXX_OPTS) -o $@ $^

%.o: %.cpp *.hpp
	$(CXX) $(CXX_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

libFullMonteGeometry.so: TetraMeshBase.o newgeom.o TetraMesh.o TetraMeshBase.o Face.o SourceDescription.o Material.o
	$(CXX) $(CXX_LIB_OPTS) -o $@ $^

clean:
	rm -f *.o *.so *.a