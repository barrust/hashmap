CC=gcc
CCFLAGS=-Wall -Wpedantic -O3
DISTDIR=dist
SRCDIR=src
TESTDIR=tests

all: clean hashmap unified
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(CCFLAGS)
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/update_test.c -o ./dist/ut $(CCFLAGS)

omp: CCFLAGS+= -fopenmp
omp: hashmap
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(CCFLAGS)

hashmap:
	$(CC) -c $(SRCDIR)/hashmap.c -o $(DISTDIR)/hashmap.o $(CCFLAGS)

clean:
	rm -rf ./$(DISTDIR)/*

unified:
	python unified_header.py
