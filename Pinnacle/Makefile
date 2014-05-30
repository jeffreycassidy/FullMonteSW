GCC_OPTS=-Wall -std=c++11 -Wno-missing-braces -O0 -fPIC -g -I/usr/include/antlr3.x.x -I/usr/include/vtk -DvtkRenderingCore_AUTOINIT="1(vtkRenderingOpenGL)"
ANTLR=java -jar /usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar

all: Test_Pinnacle

test: Test_Pinnacle
	Test_Pinnacle stub.txt

bigtest: Test_Pinnacle
	Test_Pinnacle plan_HN.roi

pinnacleLexer.c pinnacleLexer.h pinnacleParser.c pinnacleParser.h: pinnacle.g
	$(ANTLR) $^

pinnacleLexer.o: pinnacleLexer.c pinnacleLexer.h
	g++ $(GCC_OPTS) -D_empty=NULL $< -c 

pinnacleParser.o: pinnacleParser.c pinnacleParser.h
	g++ $(GCC_OPTS) $< -c

parser_main.o: parser_main.cpp pinnacleLexer.h pinnacleParser.h
	g++ $(GCC_OPTS) $< -c
	
%.o: %.cpp *.hpp
	g++ $(GCC_OPTS) $< -c
	
PinnacleROI.o: PinnacleROI.cpp PinnacleROI.hpp
	g++ $(GCC_OPTS) $< -c

Test_Pinnacle: pinnacleLexer.o pinnacleParser.o PinnacleFile.o PinnacleObject.o Test_Pinnacle.o PinnacleROI.o PinnacleCurve.o PinnacleVTK.o PinnacleGraph.o
	g++ -o $@ $^ -lantlr3c -L/usr/local/lib -lvtkCommonCore-6.1 -lvtkRenderingCore-6.1 -lvtkCommonDataModel-6.1 -lvtkRenderingOpenGL-6.1 -lvtkInteractionStyle-6.1 -lvtkCommonExecutionModel-6.1 -lvtkFiltersCore-6.1

clean:
	rm -f parser pinnacleParser.h pinnacleParser pinnacleLexer.c pinnacleLexer.h *.o 
