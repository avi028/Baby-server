# -*- Makefile -*-

## Receipies
#targets : dependencies 
#<tab>actions


all: webServer.o utils.o sessions.o parsers.o responseHandler.o
	g++ webServer.o utils.o sessions.o parsers.o responseHandler.o -o server -lpthread

webServer.o:webServer.cpp def.h user_def.h includes.h structDef.h reflection.h utils.h sessions.h globals.h externs.h dynamicHTMLRenderer.h
	g++ -c webServer.cpp -o $@

# For testing the reflection logic 'refection.cpp not used in main code '
#reflection.o:reflection.cpp def.h user_def.h includes.h structDef.h reflection.h
#	g++ -c reflection.cpp -o $@

utils.o:utils.cpp includes.h utils.h def.h utils.cpp
	g++ -c utils.cpp -o $@

sessions.o:sessions.cpp includes.h def.h sessions.h externs.h sessions.cpp
	g++ -c sessions.cpp -o $@

parsers.o:parsers.cpp includes.h def.h parsers.h externs.h
	g++ -c parsers.cpp -o $@

responseHandler.o: user_def.h externs.h 
	g++ -c responseHandler.cpp -o $@

clean: 
	@rm -f *.o *.out server

run:
	@./server

.PHONY: clean all run