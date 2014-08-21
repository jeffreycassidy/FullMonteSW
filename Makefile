# base options only
GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lSFMT
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost -LSFMT
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I. -I/usr/local/include/boost -I/usr/local/include/pgsql

# Switch debug on/off
GCC_OPTS += -DNDEBUG -O3

# Enable profiling
#GCC_OPTS += -pg

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

all: docs

docs: Doxyfile *.cpp *.hpp *.c *.h
	doxygen

Testing: Testing.cpp RandomAVX.hpp 
	g++ -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

Test_AccumulationArray: Test_AccumulationArray.cpp AccumulationArray.hpp
	g++ $(GCC_OPTS) $(INCLDIRS) -o $@ $^
	
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

meshquery: meshquery.cpp
	g++ -Wall -g -std=c++0x -O3 -mssse3 -msse4 -DSSE -I/usr/local/include -I. -I/usr/local/include/boost -L/usr/local/lib/boost -I/usr/local/include/pgsql -L. -Lfm-postgres -lfmpg -lpq -lboost_timer -lboost_system -lboost_program_options -lmontecarlo -o $@ $^

fm-postgres/%:
	make -C fm-postgres $*

random.o: random.cpp random.hpp
	g++ -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

%.o: %.cpp *.hpp
	g++ $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o random.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o FullMonte.o Notifier.o
	g++ $(GCC_OPTS) $^ $(LIBS) $(LIBDIRS) -o $@
	
montecarlo-tracer: graph.o newgeom.o face.o helpers.o source.o montecarlo.cpp LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o fm-postgres/fm-postgres.o blob.o fmdb.o fm-postgres/fmdbexportcase.o sse.o random.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o TracerStep.o
	g++ $(GCC_OPTS) -DTRACER $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@

libmontecarlo.so: graph.o newgeom.o face.o helpers.o source.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o random.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o
	g++ -shared -fPIC $^ -Lfm-postgres -lfmpg -o $@

ReadTracer: ReadTracer.cpp
	g++ -Wall -O3 -g -std=c++11 -mavx -lxerces-c $< Export_VTK_XML.cpp -o $@

clean: fm-postgres/clean
	rm -f *.o sse_int montecarlo blobmaint texwriter Test_VectorHG *.a *.so

# target to synchronize source files with a remote host	
sync-%:
	@echo "Synchronizing $*"
	rsync -crizt --delete --include 'Makefile' --include '*.c' --include '*.h' --include '*.?pp' --exclude '*' $*/ $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST):$(FULLMONTE_BUILD_PATH)/$*

# target to compile remotely; remote-XXX builds XXX on the remote machine
# user, host, path for build are specified by environment vars FULLMONTE_BUILD_USER/HOST/PATH
remote-%: sync-AVXMath sync-fm-postgres sync-. sync-SFMT sync-DBUtils
	echo "Building target $* remotely on $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST):$(FULLMONTE_BUILD_PATH)"
	ssh $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST) "cd $(FULLMONTE_BUILD_PATH); make $*"
	
remote-debug: remote-montecarlo
	ssh $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST)/$(FULLMONTE_BUILD_PATH)/$* .

rangeexprParser.o: output/rangeexprParser.c
	gcc -g -Wall -O3 -c output/rangeexprParser.c -o $@

rangeexprLexer.o: output/rangeexprLexer.c
	gcc -g -Wall -O3 -c output/rangeexprLexer.c -o $@
	
Test_RangeParse: Test_RangeParse.cpp rangeexprParser.o rangeexprLexer.o
	g++ -g -Wall -O3 -std=c++11 $^ -lantlr3c -lboost_program_options -o $@
	
timetest: timetest.cpp
	g++ -g -Wall -O3 -std=c++11 -I/usr/local/include $^ -o $@