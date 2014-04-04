# base options only
GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -msse4 -DSVNVERSION=\"$(VERSION)\" -DSSE -mtune=core2 -march=core2 -fpermissive -std=c++0x -fPIC
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I. -I/usr/local/include/boost -I/usr/local/include/pgsql

# Switch debug on/off
#GCC_OPTS += -O0
GCC_OPTS += -DNDEBUG -O3 #-DVERBOSE_PRINT

# Enable profiling
#GCC_OPTS += -pg

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
VERSION:=$(shell svnversion -n 2>/dev/null )
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

test-ExtractBoundary: Test_ExtractBoundary
	./Test_ExtractBoundary

Test_ExtractBoundary: Test_ExtractBoundary.cpp
	g++ -Wall -O3 -std=c++11 -mavx -msse4 -I/usr/local/include -I/usr/local/include/pgsql -I. -I/usr/local/include/boost -lboost_program_options -lboost_system -lpq -lmontecarlo -lfmpg -g -L/usr/local/lib/boost -L. $^ -o $@

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

XMLExport: XMLExport.cpp
	g++ -Wall -O3 -std=c++11 -I/usr/local/include/pgsql -I/usr/local/include/boost -I. -mavx -g -L. -lpq -lcrypto -lmontecarlo -lfmpg -lxerces-c $^ -o $@


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
	g++ -O1 -msse4 -g -Wall -msse3 -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

SFMT.o: SFMT.c SFMT*.h
	gcc -O1 -c -g -Wall -Wmissing-prototypes -Winline -fPIC -finline-functions -fno-strict-aliasing --param max-inline-insns-single=1800 -msse2 -DHAVE_SSE2 -DSFMT_MEXP=19937 -DNDEBUG $< -o $@

%.o: %.cpp *.hpp
	g++ $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o utils/writeFileVTK.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o random.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ $(GCC_OPTS) $^ $(LIBS) $(LIBDIRS) -o $@

montecarlo-trace: graph.o newgeom.o face.o helpers.o source.o montecarlo.cpp LoggerSurface.o io_timos.o progress.o utils/writeFileVTK.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o random.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ $(GCC_OPTS) -DLOG_MEMTRACE $^ $(INCLDIRS) $(LIBS) $(LIBDIRS) -o $@

libmontecarlo.so: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o utils/writeFileVTK.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o random.o SFMT.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ -shared -fPIC $^ -Lfm-postgres -lfmpg -o $@
#	g++ -shared -fPIC $(LIBS) $(LIBDIRS) $^ -o $@

exportResult: exportResult.cpp libmontecarlo.so
	g++ $(GCC_OPTS) $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@

clean: fm-postgres/clean
	rm -f *.o sse_int montecarlo blobmaint texwriter
