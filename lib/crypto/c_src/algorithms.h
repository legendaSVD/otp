#ifndef E_ALGORITHMS_H__
#define E_ALGORITHMS_H__ 1
#include "common.h"
int create_curve_mutex(void);
void destroy_curve_mutex(void);
void init_algorithms_types(ErlNifEnv* env);
ERL_NIF_TERM hash_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM pubkey_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM kem_algorithms_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM cipher_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM mac_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM curve_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM rsa_opts_algorithms(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif