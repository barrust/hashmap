all: clean
	gcc hashmap.c hashmap_test.c -o ./dist/hmt
clean:
	if [ -e ./dist/hmt ]; then rm ./dist/hmt; fi;
unified:
	cat hashmap.h | head -n -2 > ./dist/hashmap.h
	cat hashmap.c | tail -n -$(shell expr $(shell wc -l < hashmap.c) - 15) >> ./dist/hashmap.h
	cat hashmap.h | tail -n 3 >> ./dist/hashmap.h
