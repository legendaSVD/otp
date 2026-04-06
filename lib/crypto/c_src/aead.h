#ifndef E_AEAD_H__
#define E_AEAD_H__ 1
#include "common.h"
int init_aead_cipher_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
ERL_NIF_TERM aead_cipher_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM aead_cipher_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif