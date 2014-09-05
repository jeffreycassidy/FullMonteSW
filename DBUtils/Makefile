GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -msse -fpermissive -std=c++11 -fPIC -fabi-version=6 -DSQUELCH_AVX
GXX=/sw/bin/g++-4
LIBDIRS=-L../fm-postgres -L..
LIBS=-lfmpg -lmontecarlo

exportResult: exportResult.cpp libmontecarlo.so
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@
	
GetFluence: GetFluence.cpp
	$(GXX) -Wall -O3 -g -std=c++11 -mavx -fabi-version=6 -L. -L/usr/local/lib/boost -lfmpg -lpq -lboost_system -lboost_program_options -lboost_timer -lmontecarlo $(INCLDIRS) $< -o $@

result2matlab: result2matlab.cpp source.o blob.o fluencemap.o
	$(GXX) $(GCC_OPTS) result2matlab.cpp MatFile.cpp source.o blob.o fluencemap.o newgeom.o face.o graph.o RandomAVX.o sse.o $(INCLDIRS) $(LIBDIRS) -Lfm-postgres -L. -lfmpg -lpq -lboost_program_options -o $@ 

MatFile_Test: MatFile_Test.cpp MatFile.cpp MatFile.hpp
	$(GXX) $(GCC_OPTS) MatFile_Test.cpp MatFile.cpp -o $@
	
exportmulti: MatFile.cpp exportmulti.cpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $(LIBDIRS) $(LIBS) $^ -o $@