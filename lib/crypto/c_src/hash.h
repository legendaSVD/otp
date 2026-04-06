#ifndef E_HASH_H__
#define E_HASH_H__ 1
#include "common.h"
int init_hash_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
ERL_NIF_TERM hash_info_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hash_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hash_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hash_update_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hash_final_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hash_final_xof_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif