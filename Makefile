# base options only
GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I. -I/usr/local/include/boost -I/usr/local/include/pgsql

# Switch debug on/off
#GCC_OPTS += -O0
GCC_OPTS += -DNDEBUG -O3

# Enable profiling
#GCC_OPTS += -pg

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

Testing: Testing.cpp SFMT.h SFMT.c RandomAVX.hpp 
	g++ -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

default: avx_mathfun_test

Test_VectorMatSpin: VectorMatSpin.cpp graph.cpp face.cpp newgeom.cpp helpers.cpp Packet.o
	g++ -Wall -g -std=c++11 -O0 -L/usr/local/lib -L/usr/local/lib/boost -lm -lboost_timer -lboost_system -mavx -I/usr/local/boost -I/usr/local/include -o $@ $^

Test_VectorHG: VectorHG.cpp graph.cpp face.cpp newgeom.cpp helpers.cpp Material.o
	g++ -Wall -g -std=c++11 -O3 -L/usr/local/lib -L/usr/local/lib/boost -lm -lboost_timer -lboost_system -mavx -I/usr/local/boost -I/usr/local/include -o $@ $^

Test_AccumulationArray: Test_AccumulationArray.cpp AccumulationArray.hpp
	g++ $(GCC_OPTS) $(INCLDIRS) -o $@ $^

avx_random: avx_random.cpp RandomAVX.hpp SFMT.h SFMT.c random.cpp
	g++ -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<
	
avx_compare: avx_compare.cpp avx_mathfun.h
	g++ -Wall -std=c++11 -mavx -O3 -lboost_system -lboost_timer -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

avx_mathfun_test: avx_mathfun.h avx_mathfun_test.c
	g++ -Wall -std=c++11 -mavx -O3 -o $@ $^
	
test-ExtractBoundary: Test_ExtractBoundary
	./Test_ExtractBoundary

Test_ExtractBoundary: Test_ExtractBoundary.cpp Export_VTK_XML.cpp Export_VTK_XML.hpp MeshMapper.hpp
	g++ -Wall -O3 -std=c++11 -mavx -msse4 -I/usr/local/include -I/usr/local/include/pgsql -I. -I/usr/local/include/boost -lboost_program_options -lboost_timer -lboost_system -lpq -lmontecarlo -lfmpg -g -L/usr/local/lib/boost -lxerces-c -L. \
		Test_ExtractBoundary.cpp Export_VTK_XML.cpp -o $@

test-PointMapper: Test_PointMapper
	./Test_PointMapper

Test_PointMapper: Test_PointMapper.cpp
	g++ -Wall -O3 -std=c++11 -mavx -I/usr/local/include -g $^ -o $@

test: XMLExport
	./XMLExport
	vtk vtk_testXML.tcl

testXML: XMLWriter
	./XMLWriter
	vtk vtk_testXML.tcl

XMLWriter: XMLWriter.cpp
	g++ -Wall -O3 -std=c++11 -g $^ -lxerces-c -o $@

XMLExport: XMLExport.cpp Export_VTK_XML.cpp
	g++ -Wall -O3 -std=c++11 -I/usr/local/include/pgsql -I/usr/local/include/boost -I. -mavx -g -L. \
		-lpq -lcrypto -lmontecarlo -lfmpg -L/usr/local/boost/lib -lboost_system -lboost_timer -lboost_program_options -lxerces-c \
		XMLExport.cpp Export_VTK_XML.cpp -o $@

all: montecarlo

rletrace: rletrace.cpp progress.cpp
	g++ -Wall -std=c++0x -lrt -DPOSIX_TIMER -O3 -o $@ $^

#fm-postgres/libfmpg.so
meshquery: meshquery.cpp
	g++ -Wall -g -std=c++0x -O3 -mssse3 -msse4 -DSSE -I/usr/local/include -I. -I/usr/local/include/boost -L/usr/local/lib/boost -I/usr/local/include/pgsql -L. -Lfm-postgres -lfmpg -lpq -lboost_timer -lboost_system -lboost_program_options -lmontecarlo -o $@ $^
#	blob.o source.o sse.o graph.o newgeom.o face.o random.o SFMT.o

fm-postgres/%:
	make -C fm-postgres $*

random.o: random.cpp random.hpp
	g++ -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

SFMT.o: SFMT.c SFMT*.h
	gcc -O1 -c -g -Wall -Wmissing-prototypes -Winline -fPIC -finline-functions -fno-strict-aliasing --param max-inline-insns-single=1800 -msse2 -DHAVE_SSE2 -DSFMT_MEXP=19937 -DNDEBUG $< -o $@

%.o: %.cpp *.hpp
	g++ $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o random.o RandomAVX.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ $(GCC_OPTS) $^ $(LIBS) $(LIBDIRS) -o $@

montecarlo-trace: graph.o newgeom.o face.o helpers.o source.o montecarlo.cpp LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o RandomAVX.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ $(GCC_OPTS) -DLOG_MEMTRACE $^ $(INCLDIRS) $(LIBS) $(LIBDIRS) -o $@

libmontecarlo.so: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o random.o RandomAVX.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ -shared -fPIC $^ -Lfm-postgres -lfmpg -o $@

exportResult: exportResult.cpp libmontecarlo.so
	g++ $(GCC_OPTS) $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@

clean: fm-postgres/clean
	rm -f *.o sse_int montecarlo blobmaint texwriter Test_VectorHG *.a *.so
