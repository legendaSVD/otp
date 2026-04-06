#ifndef E_MAC_H__
#define E_MAC_H__ 1
#include "common.h"
int init_mac_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
void init_mac_types(ErlNifEnv* env);
ERL_NIF_TERM mac_types_as_list(ErlNifEnv* env);
ERL_NIF_TERM mac_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM mac_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM mac_update_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM mac_final_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif