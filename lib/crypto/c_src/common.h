#ifndef E_COMMON_H__
#define E_COMMON_H__ 1
#ifdef __WIN32__
#  include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <stdbool.h>
#include <erl_nif.h>
#include "openssl_config.h"
#include "atoms.h"
ERL_NIF_TERM raise_exception(ErlNifEnv* env, ERL_NIF_TERM id, int arg_num, char* explanation, char* file, int Line);
#define EXCP_ERROR(Env,  Str)           raise_exception((Env), atom_error,  -1,       (Str), __FILE__, __LINE__)
#define EXCP_NOTSUP(Env,  Str)          raise_exception((Env), atom_notsup, -1,       (Str), __FILE__, __LINE__)
#define EXCP_ERROR_N(Env, ArgNum, Str)  raise_exception((Env), atom_error,  (ArgNum), (Str), __FILE__, __LINE__)
#define EXCP_NOTSUP_N(Env, ArgNum, Str) raise_exception((Env), atom_notsup, (ArgNum), (Str), __FILE__, __LINE__)
#define EXCP_BADARG_N(Env, ArgNum, Str) raise_exception((Env), atom_badarg, (ArgNum), (Str), __FILE__, __LINE__)
#define RAISE_NOTSUP(Env) enif_raise_exception((Env), atom_notsup)
#define assign_goto(Var, Goto, CALL) {Var = (CALL); goto Goto;}
#endif