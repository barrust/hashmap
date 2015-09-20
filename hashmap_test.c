


#include "hashmap.h"


int main(int argc, char **argv) {
	printf("Testing Hashmap version %s\n", hashmap_get_version());


	HashMap h;
	hashmap_init(&h);

	char *value = "this is a test of the system...";
	hashmap_set(&h, "test", value);
	int i;
	for(i = 0; i < 2048; i++) {
		char key[5] = {0};
		sprintf(key, "%d", i);
		int *v = hashmap_set_int(&h, key, i * 3);
		if (v == NULL) {
			printf("failed to insert the int...\n");
			break;
		}
	}

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
	for(i = 2048; i < 2048 + 15; i++) {
		char str[5] = {0};
		sprintf(str, "%d", i);
		int *ti = (int *)hashmap_get(&h, str);
		if (ti != NULL) {
			printf("key: %s\tvalue: %d\n", str, *ti);
		} else {
			printf("key: %s\tNot Found!\n", str);
		}
	}

	hashmap_destroy(&h);
}
