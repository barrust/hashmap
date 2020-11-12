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
*   Testsuite
*******************************************************************************/
MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    /* setup */
    MU_RUN_TEST(test_default_setup);
    MU_RUN_TEST(test_non_default_setup);
}


int main() {
    // we want to ignore stderr print statements
    freopen("/dev/null", "w", stderr);

    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    printf("Number failed tests: %d\n", minunit_fail);
    return minunit_fail;
}
