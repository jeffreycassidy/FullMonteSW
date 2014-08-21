
Test_VectorMatSpin: VectorMatSpin.cpp graph.cpp face.cpp newgeom.cpp helpers.cpp Packet.o
	g++ -Wall -g -std=c++11 -O0 -L/usr/local/lib -L/usr/local/lib/boost -lm -lboost_timer -lboost_system -mavx -I/usr/local/boost -I/usr/local/include -o $@ $^

Test_VectorHG: VectorHG.cpp graph.cpp face.cpp newgeom.cpp helpers.cpp Material.o
	g++ -Wall -g -std=c++11 -O3 -L/usr/local/lib -L/usr/local/lib/boost -lm -lboost_timer -lboost_system -mavx -I/usr/local/boost -I/usr/local/include -o $@ $^

avx_random: avx_random.cpp RandomAVX.hpp random.cpp
	g++ -Wall -std=c++11 -mavx -g -O3 -lboost_system -lboost_timer -fabi-version=6 -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<
	
avx_compare: avx_compare.cpp avx_mathfun.h
	g++ -Wall -std=c++11 -mavx -O3 -lboost_system -lboost_timer -L/usr/local/lib -I/usr/local/include -DUSE_SSE2 -o $@ $<

avx_mathfun_test: avx_mathfun.h avx_mathfun_test.c
	g++ -Wall -std=c++11 -mavx -O3 -o $@ $^
