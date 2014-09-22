# base options only
ifndef GXX
    GXX=g++
endif

GCC_OPTS=-Wall -mfpmath=sse -fPIC -g -std=c++11 -mavx $(EXTRA_OPTS) -fabi-version=6 -Wno-deprecated-declarations -Wa,-q
LIBS=-lboost_program_options -lboost_system -lboost_iostreams -lpq -lcrypto -lmontecarlo
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


default: libfmpg.so

all: tupletest fmdbexportcase fmdbimportmesh libfmpg.so

getMaterials: getMaterials.cpp libfmpg.so
	g++ $(GCC_OPTS) -lfmpg -lmontecarlo $(LIBS) $(LIBDIRS) $(INCLDIRS) $< -o $@

libfmpg.so: fm-postgres.o fmdbexportcase.o 
	$(GXX) $(LIBDIRS) -Wall -lpq -lboost_program_options -lcrypto -lmontecarlo -L.. -fPIC -shared -o $@ $^

fmdbexportcase: fmdbexportcase.cpp exportcase_test.cpp
	$(GXX) -ggdb -Wall $(GCC_OPTS) $(LIBS) $(INCLDIRS) -o $@ $^

fmdbimportmesh: fmdbimportmesh.cpp
	g++ -ggdb -Wall $(LIBS) $(INCLDIRS) -o $@ $^

%.o: %.cpp *.hpp
	$(GXX) -fPIC $(GCC_OPTS) $(INCLDIRS) -c $*.cpp

clean:
	rm -f *.o *.so pgload *.so dyn_main fmdbimportmesh fmdb_export_mesh fmdbexportcase getMaterials
