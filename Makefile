# base options only
#no-deprecated is for SHA1 hash

include Makefile.in

GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -DNDEBUG -O3 -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6 -Wno-deprecated-declarations -Wa,-q
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

GenDVH: DVH.o GenDVH.o Parallelepiped.o io_timos.o linefile.o
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

%VTK.o: %VTK.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -I/usr/local/include/vtk -fPIC -c $< -o $@

%.o: %.cpp *.hpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) -fPIC -c $*.cpp -o $@

montecarlo: montecarlo.o mainloop.o random.o FullMonte.o OStreamObserver.o PGObserver.o fmdb.o LocalObserver.o
	$(GXX) $(GCC_OPTS) $^ -L$(BOOST_LIB) $(LIBS) -lmontecarlo -lFullMonteGeometry $(LIBDIRS) -o $@
	
simlocal: simlocal.o RandomAVX.o OStreamObserver.o
	$(GXX) $(GXX_OPTS) -L$(BOOST_LIB) -I$(BOOST_INCLUDE) -L. -LSFMT -lFullMonteGeometry -lboost_program_options -lboost_system -lboost_timer -lboost_chrono -lmontecarlo -o $@ $^

libmontecarlo.so: helpers.o SourceDescription.o LoggerSurface.o io_timos.o progress.o linefile.o fluencemap.o blob.o sse.o RandomAVX.o LoggerConservation.o LoggerEvent.o LoggerVolume.o FullMonte.o
	$(GXX) -shared -fPIC $(GXX_OPTS) $^ -L$(BOOST_LIB) -LSFMT -L. -lpq -lboost_program_options -lboost_system -lboost_timer -lFullMonteGeometry -lboost_chrono -Lfm-postgres -lSFMT -o $@

rletrace: rletrace.cpp progress.cpp
	$(GXX) -Wall -std=c++0x -lrt -DPOSIX_TIMER -O3 -o $@ $^

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

Test_Adaptors: Test_Adaptors.cpp Adaptors.hpp
	$(GXX) -g -Wall -I/usr/local/include -O3 -std=c++11 -o $@ $<


#### WRAPPING FUNCTIONS

TetraMeshTCL_wrap.cxx: TetraMeshTCL.i
	$(SWIG) -c++ -tcl -o $@ $^
	
TetraMeshTCL.o: TetraMeshTCL.cpp
	$(GXX) -g -c -fPIC -Wall -std=c++11 -I. -I/usr/local/include/pgsql -I/usr/local/include/vtk $^
	
TetraMeshTCL_wrap.o: TetraMeshTCL_wrap.cxx
	$(GXX) -g -c -fPIC -Wall -std=c++11 -DUSE_TCL_STUBS -I/usr/local/include/pgsql -I/usr/local/include/vtk $^
	
libFullMonteGeometry.so: TetraMeshBase.o newgeom.o graph.o face.o helpers.o
	$(GXX) -fPIC -shared -Wall -o $@ $^

libFullMonteVTK.so: TetraMeshBaseVTK.o VTKInterface.o
	$(GXX) -fPIC -shared -Wall -lFullMonteGeometry -L.			\
		-lvtkCommonDataModel-6.1 								\
		-lvtkCommonCore-6.1										\
		-o $@ $^
	
TetraMeshTCL.so: TetraMeshTCL_wrap.o TetraMeshTCL.o	Parallelepiped.o
	$(GXX) -g -Wall -fPIC -shared -L/usr/local/lib -Lfm-postgres	\
		-lvtkCommonCore-6.1				\
		-lvtkRenderingCoreTCL-6.1		\
		-lvtkCommonDataModelTCL-6.1		\
		-lvtkCommonCoreTCL-6.1			\
		-lvtkRenderingCore-6.1			\
		-lvtkCommonDataModel-6.1		\
		-ltclstub8.5					\
		-lFullMonteGeometry				\
		-lFullMonteVTK					\
        -lmontecarlo                    \
		-lfmpg							\
		-L.								\
		-I$(VTK_INCLUDE)				\
		-o $@ $^
