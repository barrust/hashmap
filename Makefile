CC=gcc
CCFLAGS=-Wall -Wpedantic -g
DISTDIR=dist
SRCDIR=src
TESTDIR=tests

all: clean hashmap unified
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(CCFLAGS)
omp:
	$(CC) $(SRCDIR)/hashmap.c $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(CCFLAGS) -fopenmp
hashmap:
	$(CC) -c $(SRCDIR)/hashmap.c -o $(DISTDIR)/hashmap.o $(CCFLAGS)
clean:
	rm -rf ./$(DISTDIR)/*

unified:
	python unified_header.py
