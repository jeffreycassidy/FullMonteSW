GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -msse4 -fpermissive -std=c++11 -fPIC -fabi-version=6 -DSQUELCH_AVX

ifndef GXX
	GXX=g++
endif

INCLDIRS=-I/usr/local/include/pgsql -I/usr/local/include -I..
LIBDIRS=-L../fm-postgres -L.. -L/usr/local/lib/boost -L/usr/local/lib
LIBS=-lfmpg -lmontecarlo -lboost_program_options -lboost_system -lboost_timer -lpq

exportResult: exportResult.cpp libmontecarlo.so
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@
	
GetFluence: GetFluence.cpp
	$(GXX) -Wall -O3 -g -std=c++11 -mavx -fabi-version=6 -L. -L/usr/local/lib/boost -lfmpg -lpq -lboost_system -lboost_program_options -lboost_timer -lmontecarlo $(INCLDIRS) $< -o $@

result2matlab: result2matlab.cpp source.o blob.o fluencemap.o
	$(GXX) $(GCC_OPTS) result2matlab.cpp MatFile.cpp source.o blob.o fluencemap.o newgeom.o face.o graph.o RandomAVX.o sse.o $(INCLDIRS) $(LIBDIRS) -Lfm-postgres -L. -lfmpg -lpq -lboost_program_options -o $@ 

MatFile_Test: MatFile_Test.cpp MatFile.cpp MatFile.hpp
	$(GXX) $(GCC_OPTS) MatFile_Test.cpp MatFile.cpp -o $@
	
exportmulti: MatFile.cpp exportmulti.cpp
	$(GXX) $(GCC_OPTS) $(LIBDIRS) $(LIBS) $(INCLDIRS)  $^ ../SFMT/SFMT.o -o $@