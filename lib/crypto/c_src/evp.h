#ifndef E_EVP_H__
#define E_EVP_H__ 1
#include "common.h"
ERL_NIF_TERM encapsulate_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM decapsulate_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM evp_compute_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM evp_generate_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif