GCC_OPTS=-Wall -std=c++11 -Wno-missing-braces -O0 -fPIC -g -I/usr/include/antlr3.x.x -I/usr/include/vtk -DvtkRenderingCore_AUTOINIT="1(vtkRenderingOpenGL)" -frounding-math

ifndef Gxx
Gxx=g++
endif

ANTLR=java -jar /usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar

default: PinnacleVTK.so
	echo 'package require vtk;  \
    load PinnacleVTK.so;        \
    printStr {hello world};     \
    set pf [readPinnacleFile plan_HN.roi]; \
    puts $$pf;\
    ' > test.tcl
	vtk test.tcl

pinnacleLexer.c pinnacleLexer.h pinnacleParser.c pinnacleParser.h: pinnacle.g
	$(ANTLR) $^

pinnacleLexer.o: pinnacleLexer.c pinnacleLexer.h
	$(Gxx) $(GCC_OPTS) -D_empty=NULL $< -c 

pinnacleParser.o: pinnacleParser.c pinnacleParser.h
	$(Gxx) $(GCC_OPTS) $< -c

parser_main.o: parser_main.cpp pinnacleLexer.h pinnacleParser.h
	$(Gxx) $(GCC_OPTS) $< -c
	
%.o: %.cpp *.hpp
	$(Gxx) $(GCC_OPTS) $< -c
	
PinnacleROI.o: PinnacleROI.cpp PinnacleROI.hpp
	$(Gxx) $(GCC_OPTS) $< -c
	
PinnacleVTK_wrap.cxx: PinnacleVTK.i
	swig -c++ -tcl -o $@ $^

PinnacleVTK.so: PinnacleVTK_wrap.cxx PinnacleVTK.cpp PinnacleFile.o PinnacleCurve.o PinnacleROI.o PinnacleObject.o pinnacleParser.o pinnacleLexer.o
	$(Gxx) -Wall -std=c++11 -shared -fPIC -I/usr/include/tcl -I/usr/local/include/vtk -DUSE_TCL_STUBS -lantlr3c -lvtkCommonCore-6.1 -lvtkCommonDataModel-6.1 -lvtkCommonCoreTCL-6.1 -lvtkCommonDataModelTCL-6.1 -ltclstub8.5 -L/usr/local/lib/vtk -o $@ $^

Test_Pinnacle: pinnacleLexer.o pinnacleParser.o PinnacleFile.o PinnacleObject.o Test_Pinnacle.o PinnacleROI.o PinnacleCurve.o PinnacleVTK.o PinnacleGraph.o
	g++ -o $@ $^ -lantlr3c -L/usr/local/lib -lvtkCommonCore-6.1 -lvtkRenderingCore-6.1 -lvtkCommonDataModel-6.1 -lvtkRenderingOpenGL-6.1 -lvtkInteractionStyle-6.1 -lvtkCommonExecutionModel-6.1 -lvtkFiltersCore-6.1 \
	-lvtkInteractionWidgets-6.1 -lvtkFiltersGeneral-6.1 -lvtkRenderingAnnotation-6.1 -lvtkRenderingFreeType-6.1 -lvtkfreetype-6.1 -lvtkRenderingFreeTypeOpenGL-6.1 \
	-lvtkCommonTransforms-6.1 -lvtkFiltersSources-6.1

Test_CGAL: pinnacleLexer.o pinnacleParser.o PinnacleFile.o PinnacleObject.o Test_CGAL.o PinnacleROI.o PinnacleCurve.o PinnacleVTK.o PinnacleGraph.o
	g++ -o $@ $^ -lantlr3c -L/usr/local/lib -lvtkCommonCore-6.1 -lvtkRenderingCore-6.1 -lvtkCommonDataModel-6.1 -lvtkRenderingOpenGL-6.1 -lvtkInteractionStyle-6.1 -lvtkCommonExecutionModel-6.1 -lvtkFiltersCore-6.1 \
	-lvtkInteractionWidgets-6.1 -lvtkFiltersGeneral-6.1 -lvtkRenderingAnnotation-6.1 -lvtkRenderingFreeType-6.1 -lvtkfreetype-6.1 -lvtkRenderingFreeTypeOpenGL-6.1 -lCGAL -lboost_thread -L/usr/lib/old_boost \
	-lvtkFiltersExtraction-6.1 -lvtkCommonTransforms-6.1

clean:
	rm -f parser pinnacleParser.h pinnacleParser pinnacleLexer.c pinnacleLexer.h *.o 
