CC=gcc
CCFLAGS=-Wall -Wpedantic -O3
DISTDIR=dist
SRCDIR=src
TESTDIR=tests

all: hashmap unified
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(CCFLAGS)
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/update_test.c -o ./dist/ut $(CCFLAGS)

hashmap:
	$(CC) -c $(SRCDIR)/hashmap.c -o $(DISTDIR)/hashmap.o $(CCFLAGS)

clean:
	if [ -d "./$(DISTDIR)/" ]; then rm -rf ./$(DISTDIR)/*; fi

unified:
	python unified_header.py
