/*
	Default tests for using the hashmap
*/

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


// #define VERBOSE 0 // set to 1 for more output!
#define KEY_LEN 25 // much larger than it needs to be

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

// private functions
void success_or_failure(int res);



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

    printf("HashMap: Add %d elements into the hashmap: ", num_els);
    int i, res = 0;
    PARRALLEL_FOR
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int *v = hashmap_set_int(&h, key, i * 3);
        if (v == NULL) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Correct number of elements: ");
    if (h.used_nodes == num_els) {
        success_or_failure(0);
    } else {
        success_or_failure(-1);
    }

    printf("HashMap: Correct keys found: ");
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int* v = (int*)hashmap_get(&h, key);
        if (v == NULL) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Correct values found: ");
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int* v = (int*)hashmap_get(&h, key);
        if (v == NULL && *v == 3 * i) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;


    printf("HashMap: Modify elements in the hash: ");
    // now change it up some
    PARRALLEL_FOR
    for(i = 0; i < num_els; i++) { // hit each one to ensure we found them all
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int *v = (int*)hashmap_get(&h, key);
        if (v == NULL) {
            res = -1;
        } else if (i % 2 == 0) {
            (*v) = (*v) * 2;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Check modified elements in the hash: ");
    PARRALLEL_FOR
    for(i = 0; i < num_els; i++) { // hit each one to ensure we found them all
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int *v = (int*)hashmap_get(&h, key);
        if (v == NULL) {
            res = -1;
        } else if (i % 2 == 0) {
            if ((*v) != i * 6) {
                res = -1;
            }
        } else if (*v != 3 * i) {
            printf("We had an odd that didn't match... i = %d\tval: %d\tmult: %d\n", i, *v, *v / i);
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Get array of keys: ");
    // get all the keys and print them out
    char** keys = hashmap_keys(&h);  // how should I test these???
    success_or_failure(0);

    printf("HashMap: Free the keys returned: ");
    for (i = 0; i < hashmap_number_keys(h); i++) {
        free(keys[i]);
    }
    free(keys);
    success_or_failure(0);

    /* TODO: fix this to make sure we actually removed them */
    printf("HashMap: Remove keys: ");
    fflush(stdout);
    PARRALLEL_FOR
    for(i = 0; i <= num_els; i += 500) { // lets remove 5 keys
        char str[KEY_LEN] = {0};
        sprintf(str, "%d", i);
        hashmap_remove(&h, str);
    }
    success_or_failure(0);

    printf("HashMap: Check removed keys: ");
    fflush(stdout);
    for (i = 0; i < num_els; i++) { // hit each one to ensure we found them all
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        int *v = (int*)hashmap_get(&h, key);
        if (v == NULL) {
            if (i % 500 != 0) {
                res = -1;
            }
        } else if (i % 2 == 0) {
            if ((*v) != i * 6) {
                res = -1;
            }
        } else if (*v != 3 * i) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: test elements not found (never added): ");
    for (i = num_els; i < num_els + 256; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        char *tmp = (char *)hashmap_get(&h, key);
        if (tmp != NULL) {
            res = -1;
            printf("%s: %s\n", key, tmp);
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Stats: ");
    if (h.used_nodes != 499000 || hashmap_get_fullness(&h) >= 25.0) {
        res = -1;
    }
    success_or_failure(res);
    res = 0;
    hashmap_stats(&h);

    printf("HashMap: Clear: ");
    hashmap_clear(&h);
    if (h.used_nodes != 0 || hashmap_get_fullness(&h) != 0.0) {
        res = -1;
    }
    success_or_failure(res);
    res = 0;
    hashmap_stats(&h);


    printf("HashMap: Destroy: ");
    hashmap_destroy(&h);
    success_or_failure(0);

    timing_end(&t);
    #if defined (_OPENMP)
    printf("Completed the multi-threaded Hashmap in %f seconds using %d threads!\n", timing_get_difference(t), THREADS);
    #else
    printf("Completed the Hashmap in %f seconds!\n", timing_get_difference(t));
    #endif
}


void success_or_failure(int res) {
    if (res == 0) {
        printf(KGRN "success!\n" KNRM);
    } else {
        printf(KRED "failure!\n" KNRM);
    }
}
