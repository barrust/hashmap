

#include <stdlib.h>         /* malloc, etc */
#include <stdio.h>          /* printf */

#if defined (_OPENMP)
#include <omp.h>
#endif

#include "timing.h"
#include "../src/hashmap.h"


#if defined (_OPENMP)
#define PARRALLEL_FOR _Pragma ("omp parallel for private(i)")
#else
#define PARRALLEL_FOR
#endif


#define VERBOSE 0 // set to 1 for more output!
#define KEY_LEN 25 // much larger than it needs to be

int main(int argc, char **argv) {
	printf("Testing Hashmap version %s\n\n", hashmap_get_version());

	int num_els = 500000; //8000000;

	#if defined (_OPENMP)
	printf("OpenMP Enabled\n\n");
	int THREADS =  omp_get_max_threads();
	printf("setting the number of threads to use to: %d\n", THREADS);
	omp_set_num_threads(THREADS);
	#endif

	Timing t;
	timing_start(&t);

	HashMap h;
	hashmap_init(&h);

	printf("Add strings into the hashmap\n");
	char *value = "this is a test of the system...";
	hashmap_set_string(&h, "test", value);
	int i;
	PARRALLEL_FOR
	for(i = 0; i < num_els; i++) {
		// printf("Thread: %d\n", omp_get_thread_num());
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		int *v = hashmap_set_int(&h, key, i * 3);
		if (v == NULL) {
			printf("failed to insert the int: %d\n", i);
		}
	}
	printf("Completed adding strings into the hashmap\n\n");

	printf("Modify elements in the hash\n");
	// now change it up some
	PARRALLEL_FOR
	for(i = 0; i < num_els; i++) { // hit each one to ensure we found them all
		char key[KEY_LEN] = {0};
		sprintf(key, "%d", i);
		int *v = (int*)hashmap_get(&h, key);
		if (v == NULL) {
			printf("%s is not in the hash\n", key);
		} else if (i % 2 == 0) {
			(*v) = (*v) * 2;
		}
	}
	printf("Completed modifing elements\n\n");

	// get all the keys and print them out
	char** keys = hashmap_keys(&h);
	if (VERBOSE == 1) {
		printf("Retrieved Keys: printing out\n");
		for (i = 0; i < hashmap_number_keys(h); i++) {
			printf("key: %s\n", keys[i]);
		}
	}
	printf("Free the keys returned\n");
	for (i = 0; i < hashmap_number_keys(h); i++) {
		free(keys[i]);
	}
	free(keys);
	printf("Completed freeing keys\n\n");

	PARRALLEL_FOR
	for(i = 5; i <= 25; i+=5) { // lets remove 5 keys
		char str[KEY_LEN] = {0};
		sprintf(str, "%d", i);
		hashmap_remove(&h, str);
	}
	printf("Test retrieving elements\n");
	char *tmp = (char *)hashmap_get(&h, "test");
	printf("key: test\tvalue: %s\n", tmp);

	for(i = 0; i < 30; i++) {
		char str[KEY_LEN] = {0};
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
		char str[KEY_LEN] = {0};
		sprintf(str, "%d", i);
		int *ti = (int *)hashmap_get(&h, str);
		if (ti != NULL) {
			printf("key: %s\tvalue: %d\n", str, *ti);
		} else {
			printf("key: %s\tNot Found!\n", str);
		}
	}

	hashmap_stats(&h);

	printf("\nFree memory\n");
	hashmap_destroy(&h);
	printf("Completed freeing memory\n");
	timing_end(&t);
	#if defined (_OPENMP)
	printf("Completed the multi-threaded Hashmap in %f seconds using %d threads!\n", timing_get_difference(t), THREADS);
	#else
	printf("Completed the Hashmap in %f seconds!\n", timing_get_difference(t));
	#endif
}
