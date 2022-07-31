# -*- Makefile -*-

## Receipies
#targets : dependencies 
#<tab>actions


all: reflection webServer utils sessions
	g++ webServer.o reflection.o utils.o sessions.o -o server -lpthread

webServer: def.h user_def.h includes.h structDef.h reflection.h utils.h sessions.h globals.h externs.h
	g++ -c webServer.cpp

reflection: def.h user_def.h includes.h structDef.h reflection.h
	g++ -c reflection.cpp

utils: includes.h utils.h def.h
	g++ -c utils.cpp

sessions: includes.h def.h sessions.h externs.h
	g++ -c sessions.cpp

clean: 
	rm -f *.o server