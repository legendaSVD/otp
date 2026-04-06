#ifndef E_API_NG_H__
#define E_API_NG_H__ 1
#include "common.h"
ERL_NIF_TERM ng_crypto_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ng_crypto_update_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ng_crypto_final_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ng_crypto_one_time_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ng_crypto_get_data_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif