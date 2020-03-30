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
	if [ -f "./$(DISTDIR)/hashmap.o" ]; then rm -r ./$(DISTDIR)/hashmap.o; fi
	if [ -f "./$(DISTDIR)/ut" ]; then rm -r ./$(DISTDIR)/ut; fi
	if [ -f "./$(DISTDIR)/hmt" ]; then rm -r ./$(DISTDIR)/hmt; fi
	if [ -f "./$(DISTDIR)/hashmap.h" ]; then rm -r ./$(DISTDIR)/hashmap.h; fi

unified:
	python unified_header.py
