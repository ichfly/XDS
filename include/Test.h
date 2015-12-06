#include <stdio.h>

#define TEST_START(test_name)                     \
    u32 __num_passed = 0;                         \
    u32 __num_failed = 0;                         \
    const char* __test_name = test_name;

#define TEST_END()                                                   \
    printf("Total: %d, Passed: %d, Failed: %d\n",                    \
        __num_passed+__num_failed, __num_passed, __num_failed);      \
    return __num_failed != 0;

#define EXPECT(statement, desc)                                    \
    if(!(statement)) {                                             \
        fprintf(stderr, "%s::" desc " FAIL\n", __test_name);       \
        __num_failed++;                                            \
    }                                                              \
    else {                                                         \
        fprintf(stderr, "%s::" desc " PASS\n", __test_name);       \
        __num_passed++;                                            \
    }
