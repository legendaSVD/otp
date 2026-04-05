#ifndef TESTCASE_DRIVER_H__
#define TESTCASE_DRIVER_H__
#include <erl_nif.h>
#include <stdlib.h>
typedef struct {
    ErlNifEnv* curr_env;
    char *testcase_name;
    int thr_nr;
    int free_mem;
    ERL_NIF_TERM build_type;
    void *extra;
} TestCaseState_t;
#define ASSERT(TCS, B) \
  ((void) ((B) ? 1 : testcase_assertion_failed((TCS), __FILE__, __LINE__, #B)))
int testcase_nif_init(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info);
void testcase_printf(TestCaseState_t *tcs, char *frmt, ...);
void testcase_succeeded(TestCaseState_t *tcs, char *frmt, ...);
void testcase_skipped(TestCaseState_t *tcs, char *frmt, ...);
void testcase_continue(TestCaseState_t *tcs);
void testcase_failed(TestCaseState_t *tcs, char *frmt, ...);
int testcase_assertion_failed(TestCaseState_t *tcs, char *file, int line,
			      char *assertion);
void *testcase_alloc(size_t size);
void *testcase_realloc(void *ptr, size_t size);
void testcase_free(void *ptr);
char *testcase_name(void);
void testcase_run(TestCaseState_t *tcs);
void testcase_cleanup(TestCaseState_t *tcs);
extern ErlNifFunc testcase_nif_funcs[3];
#endif