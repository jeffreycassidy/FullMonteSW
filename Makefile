# base options only
#no-deprecated is for SHA1 hash
GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6 -Wno-deprecated-declarations -Wa,-q
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lboost_chrono -lSFMT -lfmpg
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost -LSFMT -L. -Lfm-postgres
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I. -I/usr/local/include/boost -I/usr/local/include/pgsql -I..

SWIG=/sw/bin/swig

# Switch debug on/off
GCC_OPTS += -DNDEBUG -O3

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

ifndef GXX
	GXX=g++
endif

default: Test_LineSourceEmitter

Test_LineSourceEmitter: Test_LineSourceEmitter.cpp LineSourceEmitter.o newgeom.o RandomAVX.o graph.o Face.o Packet.o
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $(LIBDIRS) $(LIBS) -o $@ $^

all: montecarlo

docs: Doxyfile *.cpp *.hpp *.c *.h
	doxygen

Testing: Testing.cpp RandomAVX.hpp 
	$(GXX) -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

Test_AccumulationArray: Test_AccumulationArray.cpp AccumulationArray.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -o $@ $^

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

libmontecarlo.so: graph.o newgeom.o face.o helpers.o SourceDescription.o montecarlo.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o Notifier.o FullMonte.o
	$(GXX) -shared -fPIC $^ -LSFMT -lpq -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -Lfm-postgres -lfmpg -lSFMT -o $@

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
	
Test_RegionSet: Test_RegionSet.cpp RegionSet.cpp RegionSet.hpp
	$(GXX) -g -Wall -I/usr/local/include -O3 -std=c++11 -o $@ $^

TetraMeshTCL_wrap.cxx: TetraMeshTCL.i
	$(SWIG) -c++ -tcl -o $@ $^
	
TetraMeshTCL.so: TetraMeshTCL_wrap.cxx TetraMeshTCL.cpp TriSurf.cpp VTKInterface.cpp	
	$(GXX) -g -Wall -shared -I/usr/local/include -I/usr/local/include/vtk -L/usr/local/lib -L/usr/local/lib/tcltk -lvtkCommonCore-6.1 -lvtkRenderingCoreTCL-6.1 -lvtkCommonDataModelTCL-6.1 -lvtkCommonCoreTCL-6.1 -lvtkRenderingCore-6.1 -lvtkCommonDataModel-6.1 -I/usr/local/include/pgsql -fPIC -Wno-deprecated-declarations -std=c++11 -lmontecarlo -lfmpg -L. -Lfm-postgres -lboost_program_options -ltcl -lboost_system -DUSE_TCL_STUBS -ltclstub8.5 -o $@ $^
	