GeompackFile.o: Geompack/GeompackFile.cpp Geompack/GeompackFile.hpp
	g++ -Wall -DBOOST_RESULT_OF_USE_DECLTYPE -g -std=c++11 -O3 -I/usr/local/boost -c -o $@ $<
	
Test_GeompackFile.o: Geompack/Test_GeompackFile.cpp Geompack/GeompackFile.hpp
	g++ -Wall -DBOOST_RESULT_OF_USE_DECLTYPE -g -std=c++11 -O3 -I/usr/local/boost -c -o $@ $<
	
Test_GeompackFile: GeompackFile.o Test_GeompackFile.o
	g++ -Wall -g -std=c++11 -O3 $^ -o $@