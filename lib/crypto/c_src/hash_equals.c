#include "common.h"
#include "hash_equals.h"
ERL_NIF_TERM hash_equals_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
#ifdef HAVE_OPENSSL_CRYPTO_MEMCMP
    ErlNifBinary s1, s2;
    ASSERT(argc == 2);
    if (!enif_inspect_binary(env, argv[0], &s1))
        goto bad_arg;
    if (!enif_inspect_binary(env, argv[1], &s2))
        goto bad_arg;
    if (s1.size != s2.size)
        goto err;
    if (CRYPTO_memcmp(s1.data, s2.data, s1.size) == 0)
        return enif_make_atom(env, "true");
    return enif_make_atom(env, "false");
 bad_arg:
 err:
    return enif_make_badarg(env);
#else
    return EXCP_NOTSUP(env, "Unsupported CRYPTO_memcmp");
#endif
}