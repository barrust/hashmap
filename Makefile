all: clean unified
	gcc hashmap.c hashmap_test.c -o ./dist/hmt
clean:
	if [ -e ./dist/hmt ]; then rm ./dist/hmt; fi;
	if [ -e ./dist/hashmap.h ]; then rm ./dist/hashmap.h; fi;
unified:
	python unified_header.py
