#ifndef E_PBKDF2_HMAC_H__
#define E_PBKDF2_HMAC_H__ 1
#include "common.h"
ERL_NIF_TERM pbkdf2_hmac_nif(ErlNifEnv* env, int argc,
                             const ERL_NIF_TERM argv[]);
#endif