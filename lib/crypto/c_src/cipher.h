#ifndef E_CIPHER_H__
#define E_CIPHER_H__ 1
#include "common.h"
struct cipher_type_t {
    union {
	const char* str;
	ERL_NIF_TERM atom;
    }type;
     const char* str_v3;
    union {
	const EVP_CIPHER* (*funcp)(void);
	const EVP_CIPHER* p;
    }cipher;
    size_t key_len;
    unsigned flags;
    union {
        struct aead_ctrl {int ctx_ctrl_set_ivlen, ctx_ctrl_get_tag,  ctx_ctrl_set_tag;} aead;
    } extra;
};
#define NO_FIPS_CIPHER 1
#define AES_CFBx 2
#define ECB_BUG_0_9_8L 4
#define AEAD_CIPHER 8
#define NON_EVP_CIPHER 16
#define AES_CTR_COMPAT 32
#define CCM_MODE 64
#define GCM_MODE 128
#ifdef FIPS_SUPPORT
# define CIPHER_FORBIDDEN_IN_FIPS(P) (((P)->flags & NO_FIPS_CIPHER) && FIPS_MODE())
#else
# define CIPHER_FORBIDDEN_IN_FIPS(P) 0
#endif
extern ErlNifResourceType* evp_cipher_ctx_rtype;
struct evp_cipher_ctx {
    EVP_CIPHER_CTX* ctx;
    int iv_len;
    ERL_NIF_TERM padding;
    ErlNifBinary key_bin;
    int padded_size;
    int encflag;
    unsigned int size;
#if !defined(HAVE_EVP_AES_CTR)
    ErlNifEnv* env;
    ERL_NIF_TERM state;
#endif
};
ERL_NIF_TERM cipher_info_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
int init_cipher_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
void init_cipher_types(ErlNifEnv* env);
const struct cipher_type_t* get_cipher_type_no_key(ERL_NIF_TERM type);
const struct cipher_type_t* get_cipher_type(ERL_NIF_TERM type, size_t key_len);
int cmp_cipher_types(const void *keyp, const void *elemp);
int cmp_cipher_types_no_key(const void *keyp, const void *elemp);
ERL_NIF_TERM cipher_types_as_list(ErlNifEnv* env);
#endif