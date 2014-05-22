GCC_OPTS=-Wall -O3 -I/usr/include/antlr3.x.x
ANTLR=java -jar /usr/local/antlrv3/antlr-3.5.2-complete-no-st3.jar

default: test

test: parser
	parser stub.txt

bigtest: parser
	parser plan_HN.roi

pinnacleLexer.c pinnacleLexer.h pinnacleParser.c pinnacleParser.h: pinnacle.g
	$(ANTLR) $^

pinnacleLexer.o: pinnacleLexer.c pinnacleLexer.h
	g++ $(GCC_OPTS) -D_empty=NULL $< -c 

pinnacleParser.o: pinnacleParser.c pinnacleParser.h
	g++ $(GCC_OPTS) $< -c

parser_main.o: parser_main.cpp pinnacleLexer.h pinnacleParser.h
	g++ $(GCC_OPTS) $< -c

parser: pinnacleLexer.o pinnacleParser.o parser_main.o
	g++ -o $@ $^ -lantlr3c

clean:
	rm -f parser pinnacleParser.h pinnacleParser pinnacleLexer.c pinnacleLexer.h *.o 
