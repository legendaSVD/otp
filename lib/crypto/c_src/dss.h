#ifndef E_DSS_H__
#define E_DSS_H__ 1
#include "common.h"
#ifdef HAVE_DSA
int get_dss_private_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey);
int get_dss_public_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey);
int dss_privkey_to_pubkey(ErlNifEnv* env, EVP_PKEY *pkey, ERL_NIF_TERM *ret);
#endif
#endif