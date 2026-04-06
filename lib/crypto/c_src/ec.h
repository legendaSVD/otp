#ifndef E_EC_H__
#define E_EC_H__ 1
#include "common.h"
#if defined(HAVE_EC)
# if defined(HAS_3_0_API)
struct get_curve_def_ctx
{
    char curve_name[20];
    int use_curve_name;
};
int get_curve_definition(ErlNifEnv* env, ERL_NIF_TERM *ret, ERL_NIF_TERM def,
                         OSSL_PARAM params[], int *i,
                         size_t *order_size,
                         struct get_curve_def_ctx*);
# endif
# if ! defined(HAS_3_0_API)
int get_ec_key_sz(ErlNifEnv* env, ERL_NIF_TERM curve, ERL_NIF_TERM priv, ERL_NIF_TERM pub,
                  EC_KEY** res, size_t* size);
# endif
int get_ec_public_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey);
int get_ec_private_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey);
int term2point(ErlNifEnv* env, ERL_NIF_TERM term, EC_GROUP *group, EC_POINT **pptr);
#endif
ERL_NIF_TERM ec_generate_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif