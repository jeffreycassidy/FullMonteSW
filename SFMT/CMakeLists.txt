SET(CMAKE_C_FLAGS "-O1 -c -g -Wall -Wmissing-prototypes -Winline -fPIC -finline-functions -fno-strict-aliasing --param max-inline-insns-single=1800 -msse2 -DHAVE_SSE2 -DSFMT_MEXP=19937 -DNDEBUG")

ADD_LIBRARY(SFMT SFMT.c)
