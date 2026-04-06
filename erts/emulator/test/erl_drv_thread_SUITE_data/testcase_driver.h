#ifndef TESTCASE_DRIVER_H__
#define TESTCASE_DRIVER_H__
#include "erl_driver.h"
#include <stdlib.h>
typedef struct {
    char *testcase_name;
    char *command;
    int command_len;
    void *extra;
} TestCaseState_t;
#define ASSERT_CLNUP(TCS, B, CLN)					\
do {									\
    if (!(B)) {								\
	CLN;								\
	testcase_assertion_failed((TCS), __FILE__, __LINE__, #B);	\
    }									\
} while (0)
#define ASSERT(TCS, B) ASSERT_CLNUP(TCS, B, (void) 0)
void testcase_printf(TestCaseState_t *tcs, char *frmt, ...);
void testcase_succeeded(TestCaseState_t *tcs, char *frmt, ...);
void testcase_skipped(TestCaseState_t *tcs, char *frmt, ...);
void testcase_failed(TestCaseState_t *tcs, char *frmt, ...);
int testcase_assertion_failed(TestCaseState_t *tcs, char *file, int line,
			      char *assertion);
void *testcase_alloc(size_t size);
void *testcase_realloc(void *ptr, size_t size);
void testcase_free(void *ptr);
char *testcase_name(void);
void testcase_run(TestCaseState_t *tcs);
void testcase_cleanup(TestCaseState_t *tcs);
#endif