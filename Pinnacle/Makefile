GCC_OPTS=-Wall -std=c++11 -O0 -g -I/usr/include/antlr3.x.x
Gxx=/sw/bin/g++-4
ANTLR=java -jar /usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar

all: Test_Pinnacle

test: Test_Pinnacle
	Test_Pinnacle stub.txt

bigtest: Test_Pinnacle
	Test_Pinnacle plan_HN.roi

pinnacleLexer.c pinnacleLexer.h pinnacleParser.c pinnacleParser.h: pinnacle.g
	$(ANTLR) $^

pinnacleLexer.o: pinnacleLexer.c pinnacleLexer.h
	$(Gxx) $(GCC_OPTS) -D_empty=NULL $< -c 

pinnacleParser.o: pinnacleParser.c pinnacleParser.h
	$(Gxx) $(GCC_OPTS) $< -c

parser_main.o: parser_main.cpp pinnacleLexer.h pinnacleParser.h
	$(Gxx) $(GCC_OPTS) $< -c
	
%.o: %.cpp
	$(Gxx) $(GCC_OPTS) $< -c
	
PinnacleROI.o: PinnacleROI.cpp PinnacleROI.hpp
	$(Gxx) $(GCC_OPTS) $< -c

PinnacleFile.o: PinnacleFile.hpp

Test_Pinnacle: pinnacleLexer.o pinnacleParser.o PinnacleFile.o PinnacleObject.o Test_Pinnacle.o PinnacleROI.o PinnacleCurve.o
	$(Gxx) -o $@ $^ -lantlr3c

clean:
	rm -f parser pinnacleParser.h pinnacleParser pinnacleLexer.c pinnacleLexer.h *.o 
