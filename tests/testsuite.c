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
    char* t = hashmap_set_string(&h, "key", "another value");
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
    mu_assert_string_eq(v, t);

    char q[] = "THIS IS STILL A TEST";
    t = hashmap_set(&h, "test", q);
    mu_assert_string_eq(v, t);  // it gives you the old value back!
}

MU_TEST(test_hashmap_set_alt) {
    // hashmap_set_alt auto tries to free memory when it is cleaned up!
    char* v = calloc(25, sizeof(char));
    sprintf(v, "this is a test");
    void* t = hashmap_set_alt(&h, "test", v);
    mu_assert_string_eq(v, t);

    char* q = calloc(25, sizeof(char));
    sprintf(q, "THIS IS STILL A TEST");
    t = hashmap_set_alt(&h, "test", q);
    mu_assert_string_eq(q, t);  // it gives you the old value back!
}


/*******************************************************************************
*   Test Getters
*******************************************************************************/
MU_TEST(test_hashmap_get) {
    for (int i = 0; i < 3000; i++) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    /* now test things out! */
    int errors = 0;
    for (int i = 0; i < 3000; i++) {
        char key[15] = {0};
        char tmp[15] = {0};
        sprintf(key, "%d", i);
        sprintf(tmp, "%d-v", i);
        void* val = hashmap_get(&h, key);
        errors += (strcmp(tmp, val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_hashmap_get_changed) {
    for (int i = 0; i < 3000; i++) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i);
        hashmap_set(&h, key, val);
    }

    for (int i = 0; i < 3000; i++) {
        char key[15] = {0};
        char val[15] = {0};
        sprintf(key, "%d", i);
        sprintf(val, "%d-v", i + 100);
        hashmap_set(&h, key, val);
    }

    /* now test things out! */
    int errors = 0;
    for (int i = 0; i < 3000; i++) {
        char key[15] = {0};
        char tmp[15] = {0};
        sprintf(key, "%d", i);
        sprintf(tmp, "%d-v", i + 100);  // this should be what we pull out
        void* val = hashmap_get(&h, key);
        errors += (strcmp(tmp, val) == 0) ? 0 : 1;
    }
    mu_assert_int_eq(0, errors);
}

MU_TEST(test_hashmap_get_not_found) {

    for (int i = 0; i < 3000; i++) {
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
}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
