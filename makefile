COMPILER=gcc
CFLAGS=-c -Wall -g 

all: pman.c build/list.o build/process.o build/utils.o
	$(COMPILER) $< build/*.o -o PMan -lreadline


build/process.o: list.h utils.h process.c process.h
	mkdir -p build
	$(COMPILER) $(CFLAGS) process.c -o $@

build/list.o: list.c list.h
	mkdir -p build
	$(COMPILER) $(CFLAGS) list.c -o $@
	

build/utils.o: utils.c utils.h
	mkdir -p build
	$(COMPILER) $(CFLAGS) utils.c -o $@
 	

clean: 
	rm -rf build/
	rm -f pman