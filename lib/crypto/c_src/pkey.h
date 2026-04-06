#ifndef E_PKEY_H__
#define E_PKEY_H__ 1
#include "common.h"
void prefetched_sign_algo_init(ErlNifEnv*);
enum pkey_format_t {
    PKEY_PUB  = 0,
    PKEY_PRIV = 1,
    PKEY_PRIV_SEED = 2
};
struct pkey_type_t {
    union {
        const char* atom_str;
        ERL_NIF_TERM atom;
    }name;
    const int evp_pkey_id;
    union {
        const char* alg_str;
#ifdef HAS_PREFETCH_SIGN_INIT
        EVP_SIGNATURE* alg;
#endif
    } sign;
    const bool allow_seed;
};
struct pkey_type_t* get_pkey_type(ERL_NIF_TERM alg_atom);
ERL_NIF_TERM build_pkey_type_list(ErlNifEnv* env, ERL_NIF_TERM tail, bool fips);
#ifdef HAS_3_0_API
int get_pkey_from_octet_string(ErlNifEnv*,
                               ERL_NIF_TERM alg_atom,
                               ERL_NIF_TERM key_bin,
                               enum pkey_format_t,
                               struct pkey_type_t *pkey_type,
                               EVP_PKEY **pkey_p,
                               ERL_NIF_TERM *ret_p);
#endif
ERL_NIF_TERM pkey_sign_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM pkey_sign_heavy_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM pkey_verify_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM pkey_crypt_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM privkey_to_pubkey_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif