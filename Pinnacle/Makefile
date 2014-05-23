GCC_OPTS=-Wall -std=c++11 -O0 -g -I/usr/include/antlr3.x.x
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
	
%.o: %.cpp
	g++ $(GCC_OPTS) $< -c
	
PinnacleFile.o: PinnacleFile.hpp

Test_Pinnacle: pinnacleLexer.o pinnacleParser.o PinnacleFile.o PinnacleObject.o Test_Pinnacle.o
	g++ -o $@ $^ -lantlr3c

clean:
	rm -f parser pinnacleParser.h pinnacleParser pinnacleLexer.c pinnacleLexer.h *.o 
