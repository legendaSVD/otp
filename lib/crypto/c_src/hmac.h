#ifndef E_HMAC_H__
#define E_HMAC_H__ 1
#include "common.h"
#if !defined(HAS_EVP_PKEY_CTX) || DISABLE_EVP_HMAC
int init_hmac_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
ERL_NIF_TERM hmac_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hmac_update_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM hmac_final_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
int hmac_low_level(ErlNifEnv* env, const EVP_MD *md,
                   ErlNifBinary key_bin, ErlNifBinary text,
                   ErlNifBinary *ret_bin, int *ret_bin_alloc, ERL_NIF_TERM *return_term);
#endif
#endif