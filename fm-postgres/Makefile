# base options only
ifndef GXX
    GXX=g++
endif

#GXX=/sw/bin/g++-4
GCC_OPTS=-Wall -mfpmath=sse -fPIC -g -std=c++11 -msse4 $(EXTRA_OPTS) -fabi-version=6 -DSQUELCH_AVX
LIBS=-lboost_program_options -lboost_system -lboost_iostreams -lpq -lcrypto
LIBDIRS=-L/usr/local/lib -L/usr/local/lib/boost -L. -L..
INCLDIRS=-I/usr/local/boost -I/usr/local/include -I/usr/local/include/boost -I/usr/local/include/pgsql -I..

ifdef FULLMONTE_DEBUG
	GCC_OPTS += -O0 -g
else
	GCC_OPTS += -DNDEBUG -O3
endif

ifdef FULLMONTE_PROF
    GCC_OPTS += -pg
endif

OS:=$(shell uname)
VERSION:=$(shell svnversion -n 2>/dev/null )

ifeq ($(OS),Darwin)
GCC_OPTS += -DPLATFORM_DARWIN
endif

#all: fmdbimportmesh fmdb_export_mesh tupletest
all: tupletest fmdbexportcase fmdbimportmesh libfmpg.so

getMaterials: getMaterials.cpp libfmpg.so
	g++ $(GCC_OPTS) -lfmpg -lmontecarlo $(LIBS) $(LIBDIRS) $(INCLDIRS) $< -o $@

libfmpg.so: fm-postgres.o fmdbexportcase.o ../SourceDescription.o
	$(GXX) $(LIBDIRS) $(LIBS) -Wall -fPIC -shared -o $@ $^

fmdbexportcase: ../newgeom.o ../face.o fmdbexportcase.cpp exportcase_test.cpp ../graph.o fm-postgres.cpp ../helpers.cpp ../io_timos.cpp ../source.cpp fm-postgres.cpp ../linefile.cpp
	$(GXX) -ggdb -Wall $(GCC_OPTS) $(LIBS) $(INCLDIRS) -o $@ $^

fmdbimportmesh: ../newgeom.o ../face.o fmdbimportmesh.cpp ../graph.o fm-postgres.cpp ../helpers.cpp ../io_timos.cpp ../source.cpp ../linefile.cpp
	g++ -ggdb -Wall $(LIBS) $(INCLDIRS) -o $@ $^

libdyn.so: dyn.cpp
	g++ -Wall -fPIC -shared  -o $@ $^

dyn_main: dyn_main.cpp
	g++ $(GCC_OPTS) $(LIBS) $(LIBDIRS) $^ -o $@ -ldl

%.o: %.cpp *.hpp
	$(GXX) -fPIC $(GCC_OPTS) $(INCLDIRS) -c $*.cpp

clean:
	rm -f *.o *.so pgload *.so dyn_main fmdbimportmesh fmdb_export_mesh fmdbexportcase getMaterials
