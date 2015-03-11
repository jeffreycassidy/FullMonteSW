# base options only
#no-deprecated is for SHA1 hash

include Makefile.in

GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -DNDEBUG -O3 -mavx -fpermissive -std=c++11 -fPIC -Wno-deprecated-declarations -Wa,-q -fabi-version=6
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lboost_chrono -lSFMT -lfmpg
LIBDIRS=-L/usr/local/lib -L$(BOOST_LIB) -LSFMT -L. -Lfm-postgres
INCLDIRS=-I$(BOOST_INCLUDE) -I/usr/local/include -I. -I/usr/local/include/pgsql -I$(VTK_INCLUDE) -I..

SWIG=swig

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

ifndef GXX
	GXX=g++
endif

default: GenDVHs

libs: libFullMonteGeometry.so libmontecarlo.so libFullMonteVTK.so
	make -C fm-postgres libfmpg.so

all: libs GenDVH montecarlo TetraMeshTCL.so
	make -C fm-postgres all
	make -C DBUtils all

GenDVH: DVH.o GenDVH.o Parallelepiped.o
	$(GXX) -L. -Lfm-postgres -L$(BOOST_LIB) 						\
	-lFullMonteGeometry												\
	-lmontecarlo													\
	-lboost_program_options -lboost_system -lboost_serialization 	\
	-lfmpg -o $@ $^

Test_TetraMeshBase: Test_TetraMeshBase.o TetraMeshBase.o newgeom.o
	$(GXX) $(GCC_OPTS) -o $@ $^

Test_LineSourceEmitter: Test_LineSourceEmitter.cpp LineSourceEmitter.o newgeom.o RandomAVX.o graph.o Face.o Packet.o
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $(LIBDIRS) $(LIBS) -o $@ $^

all: montecarlo

docs: Doxyfile *.cpp *.hpp *.c *.h
	doxygen

Testing: Testing.cpp RandomAVX.hpp 
	$(GXX) -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

Test_AccumulationArray: Test_AccumulationArray.cpp AccumulationArray.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -o $@ $^

tracelocal.o: simlocal.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -DTRACE_ENABLE -fPIC -c $< -o $@

%.o: %.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@
	

montecarlo: montecarlo.o mainloop.o random.o FullMonte.o OStreamObserver.o PGObserver.o fmdb.o LocalObserver.o
	$(GXX) $(GCC_OPTS) $^ -L$(BOOST_LIB) $(LIBS) -lmontecarlo -lFullMonteGeometry $(LIBDIRS) -o $@
	
simlocal: simlocal.o RandomAVX.o OStreamObserver.o
	$(GXX) $(GXX_OPTS) -L$(BOOST_LIB) -I$(BOOST_INCLUDE) -LStorage/TIMOS -L. -LSFMT -lFullMonteTIMOS -LGeometry -L. -lFullMonteGeometry -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -lmontecarlo -o $@ $^

tracelocal: tracelocal.o RandomAVX.o OStreamObserver.o
	$(GXX) $(GXX_OPTS) -L$(BOOST_LIB) -I$(BOOST_INCLUDE) -L. -LSFMT -lFullMonteGeometry -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -lmontecarlo -o $@ $^

libmontecarlo.so: helpers.o LoggerSurface.o progress.o fluencemap.o blob.o sse.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o FullMonte.o

	$(GXX) -shared -fPIC $(GXX_OPTS) $^ -L$(BOOST_LIB) -LSFMT -L. -lpq -lboost_program_options -lboost_system -lboost_timer -LGeometry -lFullMonteGeometry -lboost_chrono -Lfm-postgres -lSFMT -o $@

rletrace: rletrace.cpp
	$(GXX) -Wall -std=c++11 -O3 -o $@ $^

meshquery: meshquery.cpp
	$(GXX) -Wall -g -std=c++0x -O3 -mssse3 -msse4 -DSSE -I/usr/local/include -I. -I$(BOOST_INCLUDE) -L$(BOOST_LIB) -I/usr/local/include/pgsql -L. -Lfm-postgres -lfmpg -lpq -lboost_timer -lboost_system -lboost_program_options -lmontecarlo -o $@ $^

random.o: random.cpp random.hpp
	$(GXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

Test_Parallelepiped: Test_Parallelepiped.o Parallelepiped.o newgeom.o
	$(GXX) -Wall -o $@ $^

ReadTracer: ReadTracer.cpp
	$(GXX) -Wall -O3 -g -std=c++11 -mavx -lxerces-c $< Export_VTK_XML.cpp -o $@

clean:
	make -C DBUtils clean
	make -C fm-postgres clean
	rm -f *.o sse_int montecarlo blobmaint texwriter Test_VectorHG *.a *.so
	
Test_RegionSet: Test_RegionSet.cpp RegionSet.cpp RegionSet.hpp
	$(GXX) -g -Wall -I/usr/local/include -O3 -std=c++11 -o $@ $^

Test_Adaptors: Test_Adaptors.cpp Adaptors.hpp
	$(GXX) -g -Wall -I/usr/local/include -O3 -std=c++11 -o $@ $<


BGLMesh: BGLMesh.cpp
	$(GXX) -g -Wall -O3 -std=c++11 -lFullMonteGeometry -o $@ $^