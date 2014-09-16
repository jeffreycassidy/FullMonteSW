# base options only
#no-deprecated is for SHA1 hash

ifndef BOOST_INCLUDE
	BOOST_INCLUDE=-I/usr/local/include
endif

ifndef BOOST_LIB
	BOOST_LIB=-L/usr/local/lib/boost
endif

GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6 -Wno-deprecated-declarations -Wa,-q
LIBS=-lboost_program_options -lboost_timer -lpq -lcrypto -lboost_system -lboost_chrono -lSFMT -lfmpg
LIBDIRS=-L/usr/local/lib $(BOOST_LIB) -LSFMT -L. -Lfm-postgres
INCLDIRS=$(BOOST_INCLUDE) -I/usr/local/include -I. -I/usr/local/include/pgsql -I..

SWIG=swig

# Switch debug on/off
#GCC_OPTS += -DNDEBUG -O3

# Check for Mac OS (no POSIX timer)
OS:=$(shell uname)
ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

ifndef GXX
	GXX=g++
endif

all: libFullMonteGeometry.so libFullMonteVTK.so TetraMeshTCL.so

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

all: montecarlo

%VTK.o: %VTK.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -I/usr/local/include/vtk -fPIC -c $< -o $@

%.o: %.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: montecarlo.o mainloop.o random.o FullMonte.o OStreamObserver.o PGObserver.o
	$(GXX) $(GCC_OPTS) $^ $(BOOST_LIB) $(LIBS) -lmontecarlo $(LIBDIRS) -o $@
	
simlocal: simlocal.o RandomAVX.o OStreamObserver.o libmontecarlo.so
	$(GXX) $(BOOST_LIB) $(BOOST_INCLUDE) -LSFMT -lpq -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -o $@ $^

libmontecarlo.so: graph.o newgeom.o face.o helpers.o SourceDescription.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o mainloop.o blob.o fmdb.o sse.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o FullMonte.o
	$(GXX) -shared -fPIC $^ $(BOOST_LIB) -LSFMT -lpq -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -Lfm-postgres -lfmpg -lSFMT -o $@

rletrace: rletrace.cpp progress.cpp
	$(GXX) -Wall -std=c++0x -lrt -DPOSIX_TIMER -O3 -o $@ $^

meshquery: meshquery.cpp
	$(GXX) -Wall -g -std=c++0x -O3 -mssse3 -msse4 -DSSE -I/usr/local/include -I. -I/usr/local/include/boost -L/usr/local/lib/boost -I/usr/local/include/pgsql -L. -Lfm-postgres -lfmpg -lpq -lboost_timer -lboost_system -lboost_program_options -lmontecarlo -o $@ $^

fm-postgres/%:
	make -C fm-postgres $*

random.o: random.cpp random.hpp
	$(GXX) -O1 -msse4 -g -Wall -mavx -DNDEBUG -DPLATFORM_DARWIN $< -fPIC -c -o $@

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
#remote-%: sync-AVXMath sync-fm-postgres sync-. sync-SFMT sync-DBUtils
#	echo "Building target $* remotely on $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST):$(FULLMONTE_BUILD_PATH)"
#	ssh $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST) "cd $(FULLMONTE_BUILD_PATH); make $*"
	
#remote-debug: remote-montecarlo
#	ssh $(FULLMONTE_BUILD_USER)@$(FULLMONTE_BUILD_HOST)/$(FULLMONTE_BUILD_PATH)/$* .
	
Test_RegionSet: Test_RegionSet.cpp RegionSet.cpp RegionSet.hpp
	$(GXX) -g -Wall -I/usr/local/include -O3 -std=c++11 -o $@ $^




#### WRAPPING FUNCTIONS

TetraMeshTCL_wrap.cxx: TetraMeshTCL.i
	$(SWIG) -c++ -tcl -o $@ $^
	
TetraMeshTCL.o: TetraMeshTCL.cpp
	$(GXX) -g -c -Wall -std=c++11 -I/usr/local/include/vtk $^
	
TetraMeshTCL_wrap.o: TetraMeshTCL_wrap.cxx
	$(GXX) -g -c -Wall -std=c++11 -DUSE_TCL_STUBS -I/usr/local/include/vtk $^
	
libFullMonteGeometry.so: TetraMeshBase.o newgeom.o
	$(GXX) -fPIC -shared -Wall -o $@ $^

libFullMonteVTK.so: TetraMeshBaseVTK.o
	$(GXX) -fPIC -shared -Wall -lFullMonteGeometry -L.			\
		-lvtkCommonDataModel-6.1 								\
		-lvtkCommonCore-6.1										\
		-o $@ $^
	
TetraMeshTCL.so: TetraMeshTCL_wrap.o TetraMeshTCL.o	
	$(GXX) -g -Wall -fPIC -shared -L/usr/local/lib	\
		-lvtkCommonCore-6.1				\
		-lvtkRenderingCoreTCL-6.1		\
		-lvtkCommonDataModelTCL-6.1		\
		-lvtkCommonCoreTCL-6.1			\
		-lvtkRenderingCore-6.1			\
		-lvtkCommonDataModel-6.1		\
		-ltclstub8.5					\
		-lFullMonteGeometry				\
		-lFullMonteVTK					\
		-L.								\
		-I/usr/local/include/vtk		\
		-o $@ $^