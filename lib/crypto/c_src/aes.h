#ifndef E_AES_H__
#define E_AES_H__ 1
#include "common.h"
#if !defined(HAVE_EVP_AES_CTR)
ERL_NIF_TERM aes_ctr_stream_encrypt_compat(ErlNifEnv* env, const ERL_NIF_TERM state_arg, const ERL_NIF_TERM data_arg);
#endif
#ifdef HAVE_GCM_EVP_DECRYPT_BUG
ERL_NIF_TERM aes_gcm_decrypt_NO_EVP(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif
#endif