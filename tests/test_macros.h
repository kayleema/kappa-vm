#ifndef VM_TEST_FRAMEWORK_H
#define VM_TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>

#define ASSERT_EQ(actual, expected, format) \
    if ((actual) != (expected)) { \
        fprintf(stderr, "Assertion failed: %s != %s\n", #actual, #expected); \
        fprintf(stderr, "  Actual:   " format "\n", actual); \
        fprintf(stderr, "  Expected: " format "\n", expected); \
        exit(1); \
    }

#define ASSERT_NE(actual, expected, format) \
    if ((actual) == (expected)) { \
        fprintf(stderr, "Assertion failed: (" #actual ") != (" #expected ")\n"); \
        fprintf(stderr, "  Actual:   " format "\n", actual); \
        fprintf(stderr, "  Expected: " format "\n", expected); \
        exit(1); \
    }

#define ASSERT_GT(actual, expected, format) \
    if ((actual) <= (expected)) { \
        fprintf(stderr, "Assertion failed: (" #actual ") > (" #expected ")\n"); \
        fprintf(stderr, "  Actual:   " format "\n", actual); \
        fprintf(stderr, "  Expected: " format "\n", expected); \
        exit(1); \
    }

#define TEST(name) void name(void)

#define RUN_TEST(name) \
    do { \
        printf("Running %s...\n", #name); \
        name(); \
        printf("✔︎ %s passed!\n", #name); \
    } while (0)

#endif // VM_TEST_FRAMEWORK_H
