all: clean
	gcc hashmap.c hashmap_test.c -lcrypto -o ./dist/hmt
clean:
	if [ -e ./dist/hmt ]; then rm ./dist/hmt; fi;
