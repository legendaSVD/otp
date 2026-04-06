#ifndef E_CMAC_H__
#define E_CMAC_H__ 1
#include "common.h"
#if defined(HAVE_CMAC) && !defined(HAVE_EVP_PKEY_new_CMAC_key)
int cmac_low_level(ErlNifEnv* env,
                   ErlNifBinary key_bin, const EVP_CIPHER* cipher, ErlNifBinary text,
                   ErlNifBinary *ret_bin, int *ret_bin_alloc, ERL_NIF_TERM *return_term);
#endif
#endif