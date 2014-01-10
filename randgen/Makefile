# @file  Makefile
# @brief Makefile
#
# @author Mutsuo Saito (Hiroshima University)
# @author Makoto Matsumoto (Hiroshima University)
#
# Copyright (C) 2007, 2008 Mutsuo Saito, Makoto Matsumoto and
# Hiroshima University. All rights reserved.
#
# The new BSD License is applied to this software.
# see LICENSE.txt
#
# @note
# We could comple test-sse2-Mxxx using gcc 3.4.4 of cygwin.
# We could comple test-sse2-Mxxx using gcc 4.0.1 of Linux.
# We coundn't comple test-sse2-Mxxx using gcc 3.3.2 of Linux.
# We could comple test-alti-Mxxx using gcc 3.3 of osx.
# We could comple test-alti-Mxxx using gcc 4.0 of osx.

#WARN = -Wmissing-prototypes -Wall #-Winline 
WARN = -Wall -Winline 
#WARN = -Wmissing-prototypes -Wall -W
OPTI = -O3 -finline-functions -fomit-frame-pointer -DNDEBUG \
-fno-strict-aliasing --param max-inline-insns-single=1800 
#--param inline-unit-growth=500 --param large-function-growth=900 #for gcc 4
#STD = -std=c99
CC = gcc
CCFLAGS = $(OPTI) $(WARN) $(STD)
SSE2FLAGS = -msse2 -DHAVE_SSE2

fsfmt_test: fSFMT.o fsfmt_test.o
	g++ ${CCFLAGS} ${SSE2FLAGS} -ggdb $^ -o $@

fsfmt_test.o: fsfmt_test.cpp
	g++ ${CCFLAGS} -ggdb ${SSE2FLAGS} -I/usr/local/boost/include -Wall -O3 -c $<

randgen: randgen.o dSFMT.o
	g++ ${CCFLAGS} $(SSE2FLAGS) -ggdb $^ -o $@

%.o: %.cpp *.h
	g++ ${CCFLAGS} -I/usr/local/boost/include -Wall -O3 -c $<

fSFMT.o: fSFMT.c dSFMT-params.h dSFMT-params19937.h
	gcc ${CCFLAGS} ${SSE2FLAGS} -DDSFMT_MEXP=19937 -c $<

dSFMT.o: dSFMT.c dSFMT-params.h dSFMT-params19937.h
	gcc ${CCFLAGS} ${SSE2FLAGS} -DDSFMT_MEXP=19937 -c $<

clean:
	rm -f *.o *~ test-*-M*
