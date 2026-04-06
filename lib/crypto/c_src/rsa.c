#include "rsa.h"
#include "bn.h"
static ERL_NIF_TERM rsa_generate_key(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#if !defined(HAS_3_0_API)
static ERL_NIF_TERM put_rsa_private_key(ErlNifEnv* env, const RSA *rsa);
#endif
#if !defined(HAS_3_0_API)
static int check_erlang_interrupt(int maj, int min, BN_GENCB *ctxt);
#endif
#define PUT1(env,bn,t) \
    if (bn) {if ((t = bin_from_bn(env, bn)) == atom_error) goto err;}   \
    else t = atom_undefined
#if !defined(HAS_3_0_API)
static ERL_NIF_TERM put_rsa_private_key(ErlNifEnv* env, const RSA *rsa)
{
    ERL_NIF_TERM result[8];
    const BIGNUM *n = NULL, *e = NULL, *d = NULL, *p = NULL, *q = NULL, *dmp1 = NULL, *dmq1 = NULL, *iqmp = NULL;
    RSA_get0_key(rsa, &n, &e, &d);
    if ((result[0] = bin_from_bn(env, e)) == atom_error)
        goto err;
    if ((result[1] = bin_from_bn(env, n)) == atom_error)
        goto err;
    if ((result[2] = bin_from_bn(env, d)) == atom_error)
        goto err;
    RSA_get0_factors(rsa, &p, &q);
    RSA_get0_crt_params(rsa, &dmp1, &dmq1, &iqmp);
    if (p && q && dmp1 && dmq1 && iqmp) {
        if ((result[3] = bin_from_bn(env, p)) == atom_error)
            goto err;
        if ((result[4] = bin_from_bn(env, q)) == atom_error)
            goto err;
        if ((result[5] = bin_from_bn(env, dmp1)) == atom_error)
            goto err;
        if ((result[6] = bin_from_bn(env, dmq1)) == atom_error)
            goto err;
        if ((result[7] = bin_from_bn(env, iqmp)) == atom_error)
            goto err;
	return enif_make_list_from_array(env, result, 8);
    } else {
	return enif_make_list_from_array(env, result, 3);
    }
 err:
    return enif_make_badarg(env);
}
static int check_erlang_interrupt(int maj, int min, BN_GENCB *ctxt)
{
    ErlNifEnv *env = BN_GENCB_get_arg(ctxt);
    if (!enif_is_current_process_alive(env)) {
	return 0;
    } else {
	return 1;
    }
}
int get_rsa_private_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey)
{
    ERL_NIF_TERM head, tail;
    BIGNUM *e = NULL, *n = NULL, *d = NULL;
    BIGNUM *p = NULL, *q = NULL;
    BIGNUM *dmp1 = NULL, *dmq1 = NULL, *iqmp = NULL;
    RSA *rsa = NULL;
    if (!enif_get_list_cell(env, key, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &e))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &n))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &d))
        goto bad_arg;
    if ((rsa = RSA_new()) == NULL)
        goto err;
    *pkey = EVP_PKEY_new();
    if (!RSA_set0_key(rsa, n, e, d))
        goto err;
    n = NULL;
    e = NULL;
    d = NULL;
    if (enif_is_empty_list(env, tail)) {
        if (EVP_PKEY_assign_RSA(*pkey, rsa) != 1)
            goto err;
        return 1;
    }
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &p))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &q))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &dmp1))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &dmq1))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &iqmp))
        goto bad_arg;
    if (!enif_is_empty_list(env, tail))
        goto bad_arg;
    if (!RSA_set0_factors(rsa, p, q))
        goto err;
    p = NULL;
    q = NULL;
    if (!RSA_set0_crt_params(rsa, dmp1, dmq1, iqmp))
        goto err;
    dmp1 = NULL;
    dmq1 = NULL;
    iqmp = NULL;
    if (EVP_PKEY_assign_RSA(*pkey, rsa) != 1)
        goto err;
    return 1;
 bad_arg:
 err:
    if (rsa)
        RSA_free(rsa);
    if (e)
        BN_free(e);
    if (n)
        BN_free(n);
    if (d)
        BN_free(d);
    if (p)
        BN_free(p);
    if (q)
        BN_free(q);
    if (dmp1)
        BN_free(dmp1);
    if (dmq1)
        BN_free(dmq1);
    if (iqmp)
        BN_free(iqmp);
    return 0;
}
int get_rsa_public_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey)
{
    ERL_NIF_TERM head, tail;
    BIGNUM *e = NULL, *n = NULL;
    RSA *rsa = NULL;
    if (!enif_get_list_cell(env, key, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &e))
        goto bad_arg;
    if (!enif_get_list_cell(env, tail, &head, &tail))
        goto bad_arg;
    if (!get_bn_from_bin(env, head, &n))
        goto bad_arg;
    if (!enif_is_empty_list(env, tail))
        goto bad_arg;
    if ((rsa = RSA_new()) == NULL)
        goto err;
    if (!RSA_set0_key(rsa, n, e, NULL))
        goto err;
    n = NULL;
    e = NULL;
    *pkey = EVP_PKEY_new();
    if (EVP_PKEY_assign_RSA(*pkey, rsa) != 1)
        goto err;
    return 1;
 bad_arg:
 err:
    if (rsa)
        RSA_free(rsa);
    if (e)
        BN_free(e);
    if (n)
        BN_free(n);
    return 0;
}
static ERL_NIF_TERM rsa_generate_key(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM ret;
    int modulus_bits;
    BIGNUM *pub_exp = NULL, *three = NULL;
    RSA *rsa = NULL;
    BN_GENCB *intr_cb = NULL;
#ifndef HAVE_OPAQUE_BN_GENCB
    BN_GENCB intr_cb_buf;
#endif
    if (!enif_get_int(env, argv[0], &modulus_bits))
        goto bad_arg;
    if (modulus_bits < 256)
        goto bad_arg;
    if (!get_bn_from_bin(env, argv[1], &pub_exp))
        goto bad_arg;
    if ((three = BN_new()) == NULL)
        goto err;
    if (!BN_set_word(three, 3))
        goto err;
    if (BN_cmp(pub_exp, three) < 0)
        goto err;
#ifdef HAVE_OPAQUE_BN_GENCB
    if ((intr_cb = BN_GENCB_new()) == NULL)
        goto err;
#else
    intr_cb = &intr_cb_buf;
#endif
    BN_GENCB_set(intr_cb, check_erlang_interrupt, env);
    if ((rsa = RSA_new()) == NULL)
        goto err;
    if (!RSA_generate_key_ex(rsa, modulus_bits, pub_exp, intr_cb))
        goto err;
    ret = put_rsa_private_key(env, rsa);
    goto done;
 bad_arg:
    return enif_make_badarg(env);
 err:
    ret = atom_error;
 done:
    if (pub_exp)
        BN_free(pub_exp);
    if (three)
        BN_free(three);
#ifdef HAVE_OPAQUE_BN_GENCB
    if (intr_cb)
        BN_GENCB_free(intr_cb);
#endif
    if (rsa)
        RSA_free(rsa);
    return ret;
}
int rsa_privkey_to_pubkey(ErlNifEnv* env,  EVP_PKEY *pkey, ERL_NIF_TERM *ret)
{
    const BIGNUM *n = NULL, *e = NULL, *d = NULL;
    ERL_NIF_TERM result[2];
    RSA *rsa = NULL;
    if ((rsa = EVP_PKEY_get1_RSA(pkey)) == NULL)
        goto err;
    RSA_get0_key(rsa, &n, &e, &d);
    if ((result[0] = bin_from_bn(env, e)) == atom_error)
        goto err;
    if ((result[1] = bin_from_bn(env, n)) == atom_error)
        goto err;
    *ret = enif_make_list_from_array(env, result, 2);
    RSA_free(rsa);
    return 1;
 err:
    if (rsa)
        RSA_free(rsa);
    return 0;
}
#else
int get_rsa_private_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey)
{
    ERL_NIF_TERM head, tail;
    OSSL_PARAM params[9];
    EVP_PKEY_CTX *ctx = NULL;
    int i = 0;
    head = key;
    if (!get_ossl_param_from_bin_in_list(env, "e", &head, &params[i++]) ||
        !get_ossl_param_from_bin_in_list(env, "n", &head, &params[i++]) ||
        !get_ossl_param_from_bin_in_list(env, "d", &head, &params[i++]))
        goto bad_arg;
    tail = head;
    if (!enif_is_empty_list(env, tail)) {
        if (!get_ossl_param_from_bin_in_list(env, "rsa-factor1", &head, &params[i++]) ||
            !get_ossl_param_from_bin_in_list(env, "rsa-factor2", &head, &params[i++]) ||
            !get_ossl_param_from_bin_in_list(env, "rsa-exponent1", &head, &params[i++]) ||
            !get_ossl_param_from_bin_in_list(env, "rsa-exponent2", &head, &params[i++]) ||
            !get_ossl_param_from_bin_in_list(env, "rsa-coefficient1", &head, &params[i++]) )
            goto bad_arg;
        tail = head;
        if (!enif_is_empty_list(env, tail))
            goto bad_arg;
    }
    params[i++] = OSSL_PARAM_construct_end();
    if ((ctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL)) == NULL)
        goto err;
    if (EVP_PKEY_fromdata_init(ctx) <= 0)
        goto err;
    if (EVP_PKEY_fromdata(ctx, pkey, EVP_PKEY_KEYPAIR, params) <= 0)
        goto bad_arg;
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return 1;
 bad_arg:
 err:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return 0;
}
int get_rsa_public_key(ErlNifEnv* env, ERL_NIF_TERM key, EVP_PKEY **pkey)
{
    ERL_NIF_TERM head, tail;
    OSSL_PARAM params[3];
    EVP_PKEY_CTX *ctx = NULL;
    head = key;
    if (!get_ossl_param_from_bin_in_list(env, "e", &head, &params[0]) )
        goto bad_arg;
    if (!get_ossl_param_from_bin_in_list(env, "n", &head, &params[1]) )
        goto bad_arg;
    tail = head;
    if (!enif_is_empty_list(env, tail))
        goto bad_arg;
    params[2] = OSSL_PARAM_construct_end();
    if ((ctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL)) == NULL)
        goto err;
    if (EVP_PKEY_fromdata_init(ctx) <= 0)
        goto err;
    if (EVP_PKEY_fromdata(ctx, pkey, EVP_PKEY_PUBLIC_KEY, params) <= 0)
        goto bad_arg;
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return 1;
 bad_arg:
 err:
    if (ctx) EVP_PKEY_CTX_free(ctx);
    return 0;
}
static ERL_NIF_TERM rsa_generate_key(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM ret;
    unsigned int msize;
    ErlNifBinary pub_exp;
    OSSL_PARAM params[3];
    EVP_PKEY *pkey = NULL;
    EVP_PKEY_CTX *pctx = NULL;
    if (!enif_get_uint(env, argv[0], &msize)) {
        ret = EXCP_BADARG_N(env, 0, "Can't get unsigned int");
        goto ret;
    }
    if (msize < 256) {
        ret = EXCP_BADARG_N(env, 0, "Can't be < 256");
        goto ret;
    }
    if (!enif_inspect_binary(env, argv[1], &pub_exp)) {
        ret = EXCP_BADARG_N(env, 1, "Can't get binary public exponent");
        goto ret;
    }
    pctx = EVP_PKEY_CTX_new_from_name(NULL, "RSA", NULL);
    if (!EVP_PKEY_keygen_init(pctx)) {
        ret = EXCP_ERROR(env, "Can't init RSA generation");
        goto ret;
    }
    params[0] = OSSL_PARAM_construct_uint("bits", &msize);
    params[1] = OSSL_PARAM_construct_BN("e", pub_exp.data, pub_exp.size);
    params[2] = OSSL_PARAM_construct_end();
    if (!EVP_PKEY_CTX_set_params(pctx, params))  {
        ret = EXCP_ERROR(env, "Can't set params");
        goto ret;
    }
    if (!EVP_PKEY_generate(pctx, &pkey)) {
        ret = EXCP_ERROR(env, "Can't generate RSA key-pair");
        goto ret;
    }
    {
        BIGNUM *e = NULL, *n = NULL, *d = NULL;
        BIGNUM *p = NULL, *q = NULL;
        BIGNUM *dmp1 = NULL, *dmq1 = NULL, *iqmp = NULL;
        ERL_NIF_TERM result[8];
        if (
            !EVP_PKEY_get_bn_param(pkey, "e", &e)
            || !EVP_PKEY_get_bn_param(pkey, "n", &n)
            || !EVP_PKEY_get_bn_param(pkey, "d", &d)
            || !EVP_PKEY_get_bn_param(pkey, "rsa-factor1", &p)
            || !EVP_PKEY_get_bn_param(pkey, "rsa-factor2", &q)
            || !EVP_PKEY_get_bn_param(pkey, "rsa-exponent1", &dmp1)
            || !EVP_PKEY_get_bn_param(pkey, "rsa-exponent2", &dmq1)
            || !EVP_PKEY_get_bn_param(pkey, "rsa-coefficient1", &iqmp)
            || ((result[0] = bin_from_bn(env, e)) == atom_error)
            || ((result[1] = bin_from_bn(env, n)) == atom_error)
            || ((result[2] = bin_from_bn(env, d)) == atom_error)
            || ((result[3] = bin_from_bn(env, p)) == atom_error)
            || ((result[4] = bin_from_bn(env, q)) == atom_error)
            || ((result[5] = bin_from_bn(env, dmp1)) == atom_error)
            || ((result[6] = bin_from_bn(env, dmq1)) == atom_error)
            || ((result[7] = bin_from_bn(env, iqmp)) == atom_error)
            ) {
            ret = EXCP_ERROR(env, "Can't get RSA keys");
            goto local_ret;
        }
        ret =  enif_make_list_from_array(env, result, 8);
    local_ret:
        if (e) BN_free(e);
        if (n) BN_free(n);
        if (d) BN_free(d);
        if (p) BN_free(p);
        if (q) BN_free(q);
        if (dmp1) BN_free(dmp1);
        if (dmq1) BN_free(dmq1);
        if (iqmp) BN_free(iqmp);
    }
 ret:
    if (pkey) EVP_PKEY_free(pkey);
    if (pctx) EVP_PKEY_CTX_free(pctx);
    return ret;
}
int rsa_privkey_to_pubkey(ErlNifEnv* env,  EVP_PKEY *pkey, ERL_NIF_TERM *ret)
{
    BIGNUM *e = NULL, *n = NULL;
    ERL_NIF_TERM result[2];
    if (
        !EVP_PKEY_get_bn_param(pkey, "e", &e)
        || !EVP_PKEY_get_bn_param(pkey, "n", &n)
        || ((result[0] = bin_from_bn(env, e)) == atom_error)
        || ((result[1] = bin_from_bn(env, n)) == atom_error)
        )
        goto err;
    *ret =  enif_make_list_from_array(env, result, 2);
    if (e) BN_free(e);
    if (n) BN_free(n);
    return 1;
 err:
    if (e) BN_free(e);
    if (n) BN_free(n);
    return 0;
}
#endif
ERL_NIF_TERM rsa_generate_key_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    return enif_schedule_nif(env, "rsa_generate_key",
			     ERL_NIF_DIRTY_JOB_CPU_BOUND,
			     rsa_generate_key, argc, argv);
}