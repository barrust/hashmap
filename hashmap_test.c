


#include "hashmap.h"


#define VERBOSE 0 // set to 1 for more output!

int main(int argc, char **argv) {
	printf("Testing Hashmap version %s\n", hashmap_get_version());

	int num_els = 3048;

	HashMap h;
	hashmap_init(&h);

	char *value = "this is a test of the system...";
	hashmap_set_string(&h, "test", value);
	int i;
	for(i = 0; i < num_els; i++) {
		char key[5] = {0};
		sprintf(key, "%d", i);
		int *v = hashmap_set_int(&h, key, i * 3);
		if (v == NULL) {
			printf("failed to insert the int...\n");
			break;
		}
	}

	printf("Modify elements in the hash\n");
	// now change it up some
	for(i = 0; i < num_els; i += 2) {
		char key[5] = {0};
		sprintf(key, "%d", i);
		int *v = (int*)hashmap_get(&h, key);
		if (v == NULL) {
			printf("%s is not in the hash\n", key);
		} else {
			(*v) = (*v) * 2;
		}
	}
	// get all the keys and print them out
	char** keys = hashmap_keys(&h);
	if (VERBOSE == 1) {
		printf("got all the keys!\n");
		for (i = 0; i < hashmap_number_keys(h); i++) {
			printf("key: %s\n", keys[i]);
		}
	}
	
	for (i = 0; i < hashmap_number_keys(h); i++) {
		free(keys[i]);
	}
	free(keys);

	char *tmp = (char *)hashmap_get(&h, "test");
	printf("key: test\tvalue: %s\n", tmp);

	for(i = 0; i < 15; i++) {
		char str[5] = {0};
		sprintf(str, "%d", i);
		int *ti = (int *)hashmap_get(&h, str);
		if (ti != NULL) {
			printf("key: %s\tvalue: %d\n", str, *ti);
		} else {
			printf("key: %s\tNot Found!\n", str);
		}
	}

	printf("Testing somethings that are not in the hash: \n");
	for(i = num_els; i < (num_els + 15); i++) {
		char str[5] = {0};
		sprintf(str, "%d", i);
		int *ti = (int *)hashmap_get(&h, str);
		if (ti != NULL) {
			printf("key: %s\tvalue: %d\n", str, *ti);
		} else {
			printf("key: %s\tNot Found!\n", str);
		}
	}

	hashmap_stats(&h);
	hashmap_destroy(&h);
}
