#ifndef E_DIGEST_H__
#define E_DIGEST_H__ 1
#include "common.h"
struct digest_type_t {
    const char*  str;
    const char* str_v3;
    ERL_NIF_TERM atom;
    unsigned flags;
    struct {
        const EVP_MD* (*funcp)(void);
        const EVP_MD* p;
    }md;
    unsigned int xof_default_length;
};
#define NO_FIPS_DIGEST 1
#define PBKDF2_ELIGIBLE_DIGEST 2
#ifdef FIPS_SUPPORT
# define DIGEST_FORBIDDEN_IN_FIPS(P) (((P)->flags & NO_FIPS_DIGEST) && FIPS_MODE())
#else
# define DIGEST_FORBIDDEN_IN_FIPS(P) 0
#endif
void init_digest_types(ErlNifEnv* env);
struct digest_type_t* get_digest_type(ERL_NIF_TERM type);
#ifdef HAS_3_0_API
ERL_NIF_TERM digest_types_as_list(ErlNifEnv* env);
#endif
#endif