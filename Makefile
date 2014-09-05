# base options only
GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lSFMT -lfmpg
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost -LSFMT -L. -Lfm-postgres
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I. -I/usr/local/include/boost -I/usr/local/include/pgsql -I..

# Switch debug on/off
GCC_OPTS += -DNDEBUG -O3

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

#GXX=/sw/bin/g++-4
GXX=g++

all: montecarlo

docs: Doxyfile *.cpp *.hpp *.c *.h
	doxygen

Testing: Testing.cpp RandomAVX.hpp 
	$(GXX) -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

Test_AccumulationArray: Test_AccumulationArray.cpp AccumulationArray.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -o $@ $^
	
test-ExtractBoundary: Test_ExtractBoundary
	./Test_ExtractBoundary

Test_ExtractBoundary: Test_ExtractBoundary.cpp Export_VTK_XML.cpp Export_VTK_XML.hpp MeshMapper.hpp
	$(GXX) -Wall -O3 -std=c++11 -mavx -msse4 -I/usr/local/include -I/usr/local/include/pgsql -I. -I/usr/local/include/boost -lboost_program_options -lboost_timer -lboost_system -lpq -lmontecarlo -lfmpg -g -L/usr/local/lib/boost -lxerces-c -L. \
		Test_ExtractBoundary.cpp Export_VTK_XML.cpp -o $@

test-PointMapper: Test_PointMapper
	./Test_PointMapper

Test_PointMapper: Test_PointMapper.cpp
	$(GXX) -Wall -O3 -std=c++11 -mavx -I/usr/local/include -g $^ -o $@

test: XMLExport
	./XMLExport
	vtk vtk_testXML.tcl

testXML: XMLWriter
	./XMLWriter
	vtk vtk_testXML.tcl

XMLWriter: XMLWriter.cpp
	$(GXX) -Wall -O3 -std=c++11 -g $^ -lxerces-c -o $@

XMLExport: XMLExport.cpp Export_VTK_XML.cpp
	$(GXX) -Wall -O3 -std=c++11 -I/usr/local/include/pgsql -I/usr/local/include/boost -I. -mavx -g -L. \
		-lpq -lcrypto -lmontecarlo -lfmpg -L/usr/local/boost/lib -lboost_system -lboost_timer -lboost_program_options -lxerces-c \
		XMLExport.cpp Export_VTK_XML.cpp -o $@

all: montecarlo

rletrace: rletrace.cpp progress.cpp
	$(GXX) -Wall -std=c++0x -lrt -DPOSIX_TIMER -O3 -o $@ $^

meshquery: meshquery.cpp
	$(GXX) -Wall -g -std=c++0x -O3 -mssse3 -msse4 -DSSE -I/usr/local/include -I. -I/usr/local/include/boost -L/usr/local/lib/boost -I/usr/local/include/pgsql -L. -Lfm-postgres -lfmpg -lpq -lboost_timer -lboost_system -lboost_program_options -lmontecarlo -o $@ $^

fm-postgres/%:
	make -C fm-postgres $*

random.o: random.cpp random.hpp
	$(GXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

%.o: %.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: graph.o newgeom.o face.o helpers.o SourceDescription.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o random.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o FullMonte.o Notifier.o
	$(GXX) $(GCC_OPTS) $^ $(LIBS) $(LIBDIRS) -o $@

libmontecarlo.so: graph.o newgeom.o face.o helpers.o SourceDescription.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o random.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o Notifier.o FullMonte.o
	$(GXX) -shared -fPIC $^ -Lfm-postgres -lfmpg -o $@

ReadTracer: ReadTracer.cpp
	$(GXX) -Wall -O3 -g -std=c++11 -mavx -lxerces-c $< Export_VTK_XML.cpp -o $@

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
	
timetest: timetest.cpp
	$(GXX) -g -Wall -O3 -std=c++11 -I/usr/local/include $^ -o $@
