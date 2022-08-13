# -*- Makefile -*-

include server_config.mk 

IDIR =includes
CC=g++
ODIR=build/obj
LIBS=-lpthread
SDIR=src
TARGET=build/server

_DEPS=def.h user_def.h includes.h structDef.h reflection.h utils.h sessions.h globals.h externs.h dynamicHTMLRenderer.h
_OBJS=utils.o webServer.o sessions.o parsers.o responseHandler.o

DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

all:$(OBJS) $(DEPS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)
	@echo "#############################################################################"
	@echo "Build Successful"
	@echo "To run the server use - make run "

$(ODIR)/sessions.o:$(SDIR)/sessions.cpp $(DEPS)
	$(CC) -c $(SDIR)/sessions.cpp -o $@

$(ODIR)/parsers.o:$(SDIR)/parsers.cpp $(DEPS)
	$(CC) -c $(SDIR)/parsers.cpp -o $@

$(ODIR)/responseHandler.o:$(SDIR)/responseHandler.cpp $(DEPS) 
	$(CC) -c $(SDIR)/responseHandler.cpp -o $@

$(ODIR)/webServer.o:$(SDIR)/webServer.cpp $(DEPS)
	$(CC) -c $(SDIR)/webServer.cpp -o $@ $(LIBS)

$(ODIR)/utils.o:$(SDIR)/utils.cpp $(DEPS)
	$(CC) -c $(SDIR)/utils.cpp -o $@

.PHONY: clean

clean:
	@rm -f $(ODIR)/*.o $(TARGET)

run:
	@echo "Website Folder name -> $(WebSiteFolderName)  IP -> $(IP_ADDRESS):$(PORT)"
	@echo "------------------------------------------------------------------------"
	- @build/./server
