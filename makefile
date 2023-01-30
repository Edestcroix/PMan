COMPILER=gcc
CFLAGS=-c -Wall -g 
COMPILE = $(COMPILER) $(CFLAGS)

all: pman.c build/list.o build/process.o build/utils.o
	$(COMPILER) $< build/*.o -o PMan

build/process.o: list.h utils.h process.c process.h
	mkdir -p build
	$(COMPILE) process.c -o $@

build/list.o: list.c list.h
	mkdir -p build
	$(COMPILE) list.c -o $@

build/utils.o: utils.c utils.h
	mkdir -p build
	$(COMPILE) utils.c -o $@

clean: 
	rm -rf build/
	rm -f pman