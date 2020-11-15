/*
	Default tests for using the hashmap utility functions
*/

#include <stdlib.h>         /* malloc, etc */
#include <stdio.h>          /* printf */

#include "timing.h"
#include "../src/hashmap.h"


// #define VERBOSE 0 // set to 1 for more output!
#define KEY_LEN 25 // much larger than it needs to be

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KCYN  "\x1B[36m"

// private functions
void success_or_failure(int res);



int main() {
    printf("Testing Hashmap version %s\n\n", hashmap_get_version());

    int num_els = 500000; //8000000;


    HashMap h;
    hashmap_init(&h);

    printf("HashMap: Add %d float elements into the hashmap: ", num_els);
    int i, res = 0;
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        float t = i * 1.3;
        float *v = hashmap_set_float(&h, key, t);
        if (v == NULL) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Check %d float elements in the hashmap: ", num_els);
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        float t = i * 1.3;
        float *v = hashmap_get(&h, key);
        if (v == NULL) {
            res = -1;
        } else if (*v != t) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Update %d float elements in the hashmap: ", num_els);
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        float t = i * 1.5;
        float *v = hashmap_get(&h, key);  // ensure this works correctly
        *v = i * 1.5;
        float *z = hashmap_set_float(&h, key, *v);
        if (z == NULL) {
            res = -1;
        } else if (*z != t) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Check %d updated float elements in the hashmap: ", num_els);
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        float t = i * 1.5;
        float *v = hashmap_get(&h, key);
        if (v == NULL) {
            res = -1;
        } else if (*v != t) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Update %d updated float elements to long elements in the hashmap: ", num_els);
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        long t = i * 25;
        long *v = hashmap_set_long(&h, key, t);
        if (v == NULL) {
            res = -1;
        } else if (*v != t) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Update %d updated long elements to double elements in the hashmap: ", num_els);
    for(i = 0; i < num_els; i++) {
        char key[KEY_LEN] = {0};
        sprintf(key, "%d", i);
        double t = i * 2.5;
        double *v = hashmap_set_double(&h, key, t);
        if (v == NULL) {
            res = -1;
        } else if (*v != t) {
            res = -1;
        }
    }
    success_or_failure(res);
    res = 0;

    printf("HashMap: Stats: \n");
    hashmap_stats(&h);

    printf("HashMap: Destroy: ");
    hashmap_destroy(&h);
    success_or_failure(0);

    return 0;
}



void success_or_failure(int res) {
    if (res == 0) {
        printf(KGRN "success!\n" KNRM);
    } else {
        printf(KRED "failure!\n" KNRM);
    }
}
