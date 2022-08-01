# -*- Makefile -*-

## Receipies
#targets : dependencies 
#<tab>actions


all: reflection.o webServer.o utils.o sessions.o parsers.o
	g++ webServer.o reflection.o utils.o sessions.o parsers.o -o server -lpthread

webServer.o: webServer.cpp def.h user_def.h includes.h structDef.h reflection.h utils.h sessions.h globals.h externs.h
	g++ -c webServer.cpp -o $@

reflection.o:reflection.cpp def.h user_def.h includes.h structDef.h reflection.h
	g++ -c reflection.cpp -o $@

utils.o:utils.cpp includes.h utils.h def.h utils.cpp
	g++ -c utils.cpp -o $@

sessions.o:sessions.cpp includes.h def.h sessions.h externs.h sessions.cpp
	g++ -c sessions.cpp -o $@

parsers.o:parsers.cpp includes.h def.h parsers.h externs.h
	g++ -c parsers.cpp -o $@

.PHONY: clean all

clean: 
	@rm -f *.o *.out

run:
	@./server