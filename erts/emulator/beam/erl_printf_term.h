#ifndef ERL_PRINTF_TERM_H__
#define ERL_PRINTF_TERM_H__
#include "erl_printf_format.h"
int erts_printf_term(fmtfn_t fn, void* arg, ErlPfEterm term, long precision);
#endif