all:
	gcc hashmap.c hashmap_test.c -lcrypto -o ./dist/hmt 
