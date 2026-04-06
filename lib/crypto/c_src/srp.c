#include "srp.h"
#include "bn.h"
ERL_NIF_TERM srp_value_B_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    BIGNUM *bn_verifier = NULL;
    BIGNUM *bn_exponent = NULL, *bn_generator = NULL, *bn_prime = NULL, *bn_multiplier = NULL, *bn_result = NULL;
    BN_CTX *bn_ctx = NULL;
    unsigned char* ptr;
    int dlen;
    ERL_NIF_TERM ret;
    if (FIPS_MODE()) return RAISE_NOTSUP(env);
    ASSERT(argc == 5);
    if (!get_bn_from_bin(env, argv[0], &bn_multiplier))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[1], &bn_verifier))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[2], &bn_generator))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[3], &bn_exponent))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[4], &bn_prime))
        goto bad_arg;
    if ((bn_result = BN_new()) == NULL)
        goto err;
    if ((bn_ctx = BN_CTX_new()) == NULL)
        goto err;
    if (!BN_mod_mul(bn_multiplier, bn_multiplier, bn_verifier, bn_prime, bn_ctx))
        goto err;
    BN_set_flags(bn_exponent, BN_FLG_CONSTTIME);
    if (!BN_mod_exp(bn_result, bn_generator, bn_exponent, bn_prime, bn_ctx))
        goto err;
    if (!BN_mod_add(bn_result, bn_result, bn_multiplier, bn_prime, bn_ctx))
        goto err;
    if (!BN_nnmod(bn_multiplier, bn_result, bn_prime, bn_ctx))
        goto err;
    if (BN_is_zero(bn_multiplier))
        goto err;
    if ((dlen = BN_num_bytes(bn_result)) < 0)
        goto err;
    if ((ptr = enif_make_new_binary(env, (size_t)dlen, &ret)) == NULL)
        goto err;
    if (BN_bn2bin(bn_result, ptr) < 0)
        goto err;
    goto done;
 bad_arg:
    ret = enif_make_badarg(env);
    goto done;
 err:
    ret = atom_error;
 done:
    if (bn_multiplier)
        BN_free(bn_multiplier);
    if (bn_verifier)
        BN_free(bn_verifier);
    if (bn_generator)
        BN_free(bn_generator);
    if (bn_exponent)
        BN_free(bn_exponent);
    if (bn_prime)
        BN_free(bn_prime);
    if (bn_result)
        BN_free(bn_result);
    if (bn_ctx)
        BN_CTX_free(bn_ctx);
    return ret;
}
ERL_NIF_TERM srp_user_secret_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    BIGNUM *bn_exponent = NULL, *bn_a = NULL;
    BIGNUM *bn_u = NULL, *bn_multiplier = NULL, *bn_exp2 = NULL;
    BIGNUM *bn_base = NULL, *bn_prime = NULL, *bn_generator = NULL;
    BIGNUM *bn_B = NULL, *bn_result = NULL;
    BN_CTX *bn_ctx = NULL;
    unsigned char *ptr;
    int dlen;
    ERL_NIF_TERM ret;
    if (FIPS_MODE()) return RAISE_NOTSUP(env);
    ASSERT(argc == 7);
    if (!get_bn_from_bin(env, argv[0], &bn_a))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[1], &bn_u))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[2], &bn_B))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[3], &bn_multiplier))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[4], &bn_generator))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[5], &bn_exponent))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[6], &bn_prime))
        goto bad_arg;
    if ((bn_ctx = BN_CTX_new()) == NULL)
        goto err;
    if ((bn_result = BN_new()) == NULL)
        goto err;
    if (!BN_nnmod(bn_result, bn_B, bn_prime, bn_ctx))
        goto err;
    if (BN_is_zero(bn_result))
        goto err;
    if ((bn_base = BN_new()) == NULL)
        goto err;
    BN_set_flags(bn_exponent, BN_FLG_CONSTTIME);
    if (!BN_mod_exp(bn_result, bn_generator, bn_exponent, bn_prime, bn_ctx))
        goto err;
    if (!BN_mod_mul(bn_result, bn_multiplier, bn_result, bn_prime, bn_ctx))
        goto err;
    if (!BN_mod_sub(bn_base, bn_B, bn_result, bn_prime, bn_ctx))
        goto err;
    if ((bn_exp2 = BN_new()) == NULL)
        goto err;
    if (!BN_mul(bn_result, bn_u, bn_exponent, bn_ctx))
        goto err;
    if (!BN_add(bn_exp2, bn_a, bn_result))
        goto err;
    BN_set_flags(bn_exp2, BN_FLG_CONSTTIME);
    if (!BN_mod_exp(bn_result, bn_base, bn_exp2, bn_prime, bn_ctx))
        goto err;
    if ((dlen = BN_num_bytes(bn_result)) < 0)
        goto err;
    if ((ptr = enif_make_new_binary(env, (size_t)dlen, &ret)) == NULL)
        goto err;
    if (BN_bn2bin(bn_result, ptr) < 0)
        goto err;
    goto done;
 bad_arg:
    ret = enif_make_badarg(env);
    goto done;
 err:
    ret = atom_error;
 done:
    if (bn_a)
        BN_free(bn_a);
    if (bn_u)
        BN_free(bn_u);
    if (bn_B)
        BN_free(bn_B);
    if (bn_multiplier)
        BN_free(bn_multiplier);
    if (bn_generator)
        BN_free(bn_generator);
    if (bn_exponent)
        BN_free(bn_exponent);
    if (bn_prime)
        BN_free(bn_prime);
    if (bn_ctx)
        BN_CTX_free(bn_ctx);
    if (bn_result)
        BN_free(bn_result);
    if (bn_base)
        BN_free(bn_base);
    if (bn_exp2)
        BN_free(bn_exp2);
    return ret;
}
ERL_NIF_TERM srp_host_secret_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    BIGNUM *bn_b = NULL, *bn_verifier = NULL;
    BIGNUM *bn_prime = NULL, *bn_A = NULL, *bn_u = NULL, *bn_base = NULL, *bn_result = NULL;
    BN_CTX *bn_ctx = NULL;
    unsigned char *ptr;
    int dlen;
    ERL_NIF_TERM ret;
    if (FIPS_MODE()) return RAISE_NOTSUP(env);
    ASSERT(argc == 5);
    if (!get_bn_from_bin(env, argv[0], &bn_verifier))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[1], &bn_b))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[2], &bn_u))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[3], &bn_A))
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[4], &bn_prime))
        goto bad_arg;
    if ((bn_ctx = BN_CTX_new()) == NULL)
        goto err;
    if ((bn_result = BN_new()) == NULL)
        goto err;
    if (!BN_nnmod(bn_result, bn_A, bn_prime, bn_ctx))
        goto err;
    if (BN_is_zero(bn_result))
        goto err;
    if ((bn_base = BN_new()) == NULL)
        goto err;
    BN_set_flags(bn_u, BN_FLG_CONSTTIME);
    if (!BN_mod_exp(bn_base, bn_verifier, bn_u, bn_prime, bn_ctx))
        goto err;
    if (!BN_mod_mul(bn_base, bn_A, bn_base, bn_prime, bn_ctx))
        goto err;
    BN_set_flags(bn_b, BN_FLG_CONSTTIME);
    if (!BN_mod_exp(bn_result, bn_base, bn_b, bn_prime, bn_ctx))
        goto err;
    if ((dlen = BN_num_bytes(bn_result)) < 0)
        goto err;
    if ((ptr = enif_make_new_binary(env, (size_t)dlen, &ret)) == NULL)
        goto err;
    if (BN_bn2bin(bn_result, ptr) < 0)
        goto err;
    goto done;
 bad_arg:
    ret = enif_make_badarg(env);
    goto done;
 err:
    ret = atom_error;
 done:
    if (bn_verifier)
        BN_free(bn_verifier);
    if (bn_b)
        BN_free(bn_b);
    if (bn_u)
        BN_free(bn_u);
    if (bn_A)
        BN_free(bn_A);
    if (bn_prime)
        BN_free(bn_prime);
    if (bn_ctx)
        BN_CTX_free(bn_ctx);
    if (bn_result)
        BN_free(bn_result);
    if (bn_base)
        BN_free(bn_base);
    return ret;
}