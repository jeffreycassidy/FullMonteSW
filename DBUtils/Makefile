GCC_OPTS=-Wall -mfpmath=sse -Wstrict-aliasing=2 -g -mavx -fpermissive -std=c++11 -fPIC -fabi-version=6 -DSQUELCH_AVX

include ../Makefile.in

INCLDIRS=-I/usr/local/include/pgsql -I/usr/local/include -I..
LIBDIRS=-L../fm-postgres -L.. -L$(BOOST_LIB) -L/usr/local/lib -L../fm-postgres
LIBS=-lfmpg -lmontecarlo -lboost_program_options -lboost_system -lboost_timer -lpq

all: GetFluence

exportResult: exportResult.cpp
	$(GXX) $(GCC_OPTS) $(INCLDIRS) $^ $(LIBS) $(LIBDIRS) -o $@
	
GetFluence: GetFluence.cpp
	$(GXX) -Wall -O3 -g -std=c++11 -mavx -fabi-version=6 -L. -L.. -L../fm-postgres -L/usr/local/lib/boost 	\
		-lfmpg 																								\
		-lpq 																								\
		-lboost_system -lboost_program_options -lboost_timer 												\
		-lFullMonteGeometry 																				\
		$(INCLDIRS) -Wa,-q $< -o $@

result2matlab: result2matlab.cpp source.o blob.o fluencemap.o
	$(GXX) $(GCC_OPTS) result2matlab.cpp MatFile.cpp source.o blob.o fluencemap.o newgeom.o face.o graph.o RandomAVX.o sse.o $(INCLDIRS) $(LIBDIRS) -Lfm-postgres -L. -lfmpg -lpq -lboost_program_options -o $@ 

MatFile_Test: MatFile_Test.cpp MatFile.cpp MatFile.hpp
	$(GXX) $(GCC_OPTS) MatFile_Test.cpp MatFile.cpp -o $@
	
exportmulti: MatFile.cpp exportmulti.cpp
	$(GXX) $(GCC_OPTS) $(LIBDIRS) $(LIBS) $(INCLDIRS)  $^ ../SFMT/SFMT.o -o $@

clean:
	rm -f *.so *.o GetFluence result2matlab a.out
