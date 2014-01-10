GCCFLAGS_COMMON=-Wall -O3 -I/usr/local/include -DSSE -DPLATFORM_DARWIN -Wstrict-aliasing=2
GCCFLAGS_SSE=$(GCCFLAGS_COMMON) -msse3 -mfpmath=sse -msse4 -DSSE3 -DSSE2 -DHAVE_SSE2 -ggdb
GCCFLAGS_NORMAL=$(GCCFLAGS_COMMON) -mfpmath=sse
GCCFLAGS_NOSSE=$(GCCFLAGS_COMMON) -mno-sse2 -mno-sse3 -mno-mmx -mfpmath=sse

GCCFLAGS=$(GCCFLAGS_SSE)

LDFLAGS=-lm -lboost_timer -lboost_system -L/usr/local/lib/boost

sortspeed: sortspeed.cpp
	g++ -Wall -O3 -g -std=c++0x $^ -L/usr/local/lib/boost -lboost_timer -o $@

setinsert: setinsert.cpp
	g++ -Wall -O3 -g -std=c++0x $^ -L/usr/local/lib/boost -lboost_timer -o $@

quicksgn: quicksgn.cpp
	g++ -Wall -O3 -I/usr/local/boost/include -L/usr/local/lib/boost -lboost_timer $^ ../sse.o -o $@

first: sse_intersection

timos_spin_icc: timos_spin.cpp
	icpc -O3 -Wall -msse4 -L/usr/local/lib/boost -lboost_timer -lboost_system timos_spin.cpp -o $@

timos_spin_gcc: timos_spin.cpp
	g++ -O3 -Wall -msse4 -L/usr/local/lib/boost -lboost_timer -lboost_system timos_spin.cpp -o $@

optics: sseoptics.cpp ../helpers.cpp timebench.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^


all: sse_intersection fastmin fastspin rand_printer rand_save

fastrand: fastrand.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^ ../random.o ../SFMT.o ../sse.o
#	g++ $(GCCFLAGS_SSE) -finline-functions -fomit-frame-pointer -fno-strict-aliasing --param max-inline-insns-single=1800 $(LDFLAGS) -o $@ $^ ../random.o ../SFMT.o ../sse.o

rand_printer: rand_printer.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^ ../random.o ../SFMT.o ../sse.o

rand_save: rand_save.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^ ../random.o ../SFMT.o ../sse.o

timebench_test: timebench.cpp timebench_test.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

fastminidx: fastminidx.cpp ../sse.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

fastspin: fastspin.cpp ../newgeom.cpp ../helpers.cpp timebench.cpp ../sse.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

fastmin: fastmin.cpp timebench.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

sse_intersection: sse_intersection.cpp ../face.cpp ../newgeom.cpp ../helpers.cpp ../graph.cpp ../source.cpp ../random.cpp ../sse.cpp ../SFMT.c timebench.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

sseutil: sseutil.cpp
	g++ $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

ssetrans: ssetrans.c
	gcc $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

nonsse: sse.c
	gcc $(GCCFLAGS_NOSSE) $(LDFLAGS) -o $@ $^

withsse: sse.c
	gcc $(GCCFLAGS_SSE) $(LDFLAGS) -o $@ $^

%.o: %.c
	gcc $(GCCFLAGS) -c $^

%.o: %.cpp
	g++ $(GCCFLAGS) -c $^

mmtest: timebench.o mm.o
	g++ $(LDFLAGS) $^ -o $@
