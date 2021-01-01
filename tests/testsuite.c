#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "minunit.h"
#include "../src/hashmap.h"


// the basic set to use!
HashMap h;


void test_setup(void) {
    hashmap_init(&h);
}

void test_teardown(void) {
    hashmap_destroy(&h);
}


/*******************************************************************************
*   Test the setup
*******************************************************************************/
MU_TEST(test_default_setup) {
    mu_assert_int_eq(1024, h.number_nodes);  /* 1024 is the INITIAL_NUM_ELEMENTS definition! */
    mu_assert_int_eq(0, h.used_nodes);
}

MU_TEST(test_non_default_setup) {
    HashMap q;
    hashmap_init_alt(&q, 500, NULL); // for now, ignore the hash function
    mu_assert_int_eq(500, q.number_nodes);
    mu_assert_int_eq(0, q.used_nodes);
    hashmap_destroy(&q);
}

/*******************************************************************************
*   Test Utility Setters
*******************************************************************************/
MU_TEST(test_hashmap_set_int) {
    int* t = hashmap_set_int(&h, "100", 100);
    mu_assert_int_eq(100, *t);
    mu_assert_int_eq(1, hashmap_number_keys(h));
}

MU_TEST(test_hashmap_set_long) {
    long* t = hashmap_set_long(&h, "100", 100);
    mu_assert_int_eq(100, *t);
    mu_assert_int_eq(1, hashmap_number_keys(h));
}

MU_TEST(test_hashmap_set_float) {
    float v = 0.111;
    float* t = hashmap_set_float(&h, "100", v);
    mu_assert_double_eq(v, *t);
    mu_assert_int_eq(1, hashmap_number_keys(h));
}

MU_TEST(test_hashmap_set_double) {
    double v = 0.1110000833869;
    double* t = hashmap_set_double(&h, "100", v);
    mu_assert_double_eq(v, *t);
    mu_assert_int_eq(1, hashmap_number_keys(h));
}

MU_TEST(test_hashmap_set_string) {
    char* t = hashmap_set_string(&h, "key", (char*)"another value");
    mu_assert_string_eq(t, "another value");
    mu_assert_int_eq(1, hashmap_number_keys(h));
}


/*******************************************************************************
*   Test Setters
*******************************************************************************/
MU_TEST(test_hashmap_set) {
    // hashmap_set does not auto remove the item from the stack (free)
    char v[] = "this is a test";
    void* t = hashmap_set(&h, "test", v);
    mu_assert_string_eq(v, (char*)t);

    char q[] = "THIS IS STILL A TEST";
    t = hashmap_set(&h, "test", q);
    mu_assert_string_eq(v, (char*)t);  // it gives you the old value back!
}

MU_TEST(test_hashmap_set_alt) {
    // hashmap_set_alt auto tries to free memory when it is cleaned up!
    char* v = (char*)calloc(25, sizeof(char));
    sprintf(v, "this is a test");
    void* t = hashmap_set_alt(&h, "test", v);
    mu_assert_string_eq(v, (char*)t);

    char* q = (char*)calloc(25, sizeof(char));
    sprintf(q, "THIS IS STILL A TEST");
    t = hashmap_set_alt(&h, "test", q);
    mu_assert_string_eq(q, (char*)t);  // it gives you the old value back!
}


/*******************************************************************************
*   Test Getters
*******************************************************************************/
MU_TEST(test_hashmap_get) {
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    /* now test things out! */
    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char tmp[15] = {0};
        sprintf(key, "%d", i);
        sprintf(tmp, "%d-v", i);
        void* val = hashmap_get(&h, key);
        errors += (strcmp(tmp, (char*)val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_hashmap_get_changed) {
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i + 100);
        hashmap_set(&h, key, val);
    }

    /* now test things out! */
    int errors = 0;
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char tmp[15] = {0};
        sprintf(key, "%d", i);
        sprintf(tmp, "%d-v", i + 100);  // this should be what we pull out
        void* val = hashmap_get(&h, key);
        errors += (strcmp(tmp, (char*)val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_hashmap_get_not_found) {

    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    mu_assert_null(hashmap_get(&h, "a"));
    mu_assert_null(hashmap_get(&h, "3001"));
    mu_assert_not_null(hashmap_get(&h, "2999"));
}

/*******************************************************************************
*   Test Removal
*******************************************************************************/
MU_TEST(test_hashmap_remove) {
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    // test getting back what we expected
    int errors = 0;
    for (int i = 0; i < 3000; i+=2) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        void* v = hashmap_remove(&h, key);
        errors += (strcmp(val, (char*)v) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    // test not present what was removed
    errors = 0;
    for (int i = 0; i < 3000; i+=2) {
        char key[15] = {0};
        sprintf(key, "%d", i);
        void* v = hashmap_remove(&h, key);
        errors += (v == NULL) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    // test present what was left
    errors = 0;
    for (int i = 1; i < 3000; i+=2) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        void* v = hashmap_get(&h, key);
        errors += (v != NULL && strcmp((char*)v, val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_hashmap_remove_mallocd) {
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char* val = (char*)calloc(15, sizeof(char));
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set_alt(&h, key, val);
    }

    // test getting back NULL since it will try to free it for us
    int errors = 0;
    for (int i = 0; i < 3000; i+=2) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        void* v = hashmap_remove(&h, key);
        errors += (v == NULL) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    // test not present what was removed
    errors = 0;
    for (int i = 0; i < 3000; i+=2) {
        char key[15] = {0};
        sprintf(key, "%d", i);
        void* v = hashmap_remove(&h, key);
        errors += (v == NULL) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    // test present what was left
    errors = 0;
    for (int i = 1; i < 3000; i+=2) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        void* v = hashmap_get(&h, key);
        errors += (v != NULL && strcmp((char*)v, val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

/*******************************************************************************
*   Test Keys
*******************************************************************************/
MU_TEST(test_hashmap_keys) {
    // add some values
    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    char** keys = hashmap_keys(&h);
    // make some tests
    int errors = 0;
    unsigned s = hashmap_number_keys(h);
    for (unsigned int i = 0; i < s; ++i) {
        void* val = hashmap_get(&h, keys[i]);
        int key_i = atoi(keys[i]);
        char val_c[15] = {0};
        sprintf(val_c, "%d-v", key_i);
        errors += (val != NULL && strcmp((char*)val, val_c) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);

    // free the data
    for (unsigned int i = 0; i < hashmap_number_keys(h); ++i) {
        free(keys[i]);
    }
    free(keys);

}

/*******************************************************************************
*   Test Statistics
*******************************************************************************/
MU_TEST(test_hashmap_stat) {
    for (int i = 0; i < 55000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    /* save the printout to a buffer */
    int stdout_save;
    char buffer[2046] = {0};
    fflush(stdout); //clean everything first
    stdout_save = dup(STDOUT_FILENO); //save the stdout state
    freopen("output_file", "a", stdout); //redirect stdout to null pointer
    setvbuf(stdout, buffer, _IOFBF, 1024); //set buffer to stdout

    hashmap_stats(&h);

    /* reset stdout */
    freopen("output_file", "a", stdout); //redirect stdout to null again
    dup2(stdout_save, STDOUT_FILENO); //restore the previous state of stdout
    setvbuf(stdout, NULL, _IONBF, 0); //disable buffer to print to screen instantly

    // Not sure this is necessary, but it cleans it up
    remove("output_file");

    mu_assert_not_null(buffer);
    mu_assert_string_eq("HashMap:\n\
    Number Nodes: 262144\n\
    Used Nodes: 55000\n\
    Fullness: 20.980835%\n\
    Average O(n): 1.038040\n\
    Average Used O(n): 1.181309\n\
    Max O(n): 7\n\
    Max Consecutive Buckets Used: 10\n\
    Number Hash Collisions: 0\n\
    Number Index Collisions: 7328\n\
    Size on disk (bytes): 3857184\n", buffer);
}

MU_TEST(test_hashmap_fullness) {
    /* on empty */
    mu_assert_double_eq(0.0, hashmap_get_fullness(&h));

    for (int i = 0; i < 3000; ++i) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    mu_assert_double_eq(18.310546875, hashmap_get_fullness(&h));
}

/*******************************************************************************
*   Testsuite
*******************************************************************************/
MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_default_setup);
    MU_RUN_TEST(test_non_default_setup);

    /* utility setters */
    MU_RUN_TEST(test_hashmap_set_int);
    MU_RUN_TEST(test_hashmap_set_long);
    MU_RUN_TEST(test_hashmap_set_float);
    MU_RUN_TEST(test_hashmap_set_double);
    MU_RUN_TEST(test_hashmap_set_string);

    /* setters */
    MU_RUN_TEST(test_hashmap_set);
    MU_RUN_TEST(test_hashmap_set_alt);

    /* getters */
    MU_RUN_TEST(test_hashmap_get);
    MU_RUN_TEST(test_hashmap_get_changed);
    MU_RUN_TEST(test_hashmap_get_not_found);

    /* remove */
    MU_RUN_TEST(test_hashmap_remove);
    MU_RUN_TEST(test_hashmap_remove_mallocd);

    /* keys */
    MU_RUN_TEST(test_hashmap_keys);

    /* statistics */
    MU_RUN_TEST(test_hashmap_stat);
    MU_RUN_TEST(test_hashmap_fullness);
}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
