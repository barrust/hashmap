CC=gcc
COMPFLAGS=-Wall -Wpedantic -Winline -Wextra -Wno-long-long
DISTDIR=dist
SRCDIR=src
TESTDIR=tests

all: hashmap
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/hashmap_test.c -o ./dist/hmt $(COMPFLAGS) $(CCFLAGS)
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/update_test.c -o ./dist/ut $(COMPFLAGS) $(CCFLAGS)

hashmap:
	$(CC) -c $(SRCDIR)/hashmap.c -o $(DISTDIR)/hashmap.o $(COMPFLAGS) $(CCFLAGS)

debug: COMPFLAGS += -g
debug: all

release: COMPFLAGS += -O3
release: all

sanitize: COMPFLAGS += -fsanitize=undefined
sanitize: test

test: COMPFLAGS += --coverage
test: hashmap
	$(CC) $(DISTDIR)/hashmap.o $(TESTDIR)/testsuite.c $(CCFLAGS) $(COMPFLAGS) -o ./$(DISTDIR)/test -g

runtests:
	@ if [ -f "./$(DISTDIR)/test" ]; then ./$(DISTDIR)/test; fi

clean:
	if [ -f "./$(DISTDIR)/hashmap.o" ]; then rm -r ./$(DISTDIR)/hashmap.o; fi
	if [ -f "./$(DISTDIR)/ut" ]; then rm -r ./$(DISTDIR)/ut; fi
	if [ -f "./$(DISTDIR)/hmt" ]; then rm -r ./$(DISTDIR)/hmt; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcno; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -rf ./$(DISTDIR)/*.gcda; fi
	if [ -f "./$(DISTDIR)/test" ]; then rm -r ./$(DISTDIR)/test; fi
	rm -f ./*.gcno
	rm -f ./*.gcda
	rm -f ./*.gcov
