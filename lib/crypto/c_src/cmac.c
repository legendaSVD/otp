#include "common.h"
#if defined(HAVE_CMAC) && !defined(HAVE_EVP_PKEY_new_CMAC_key)
#include "cmac.h"
int cmac_low_level(ErlNifEnv* env,
                   ErlNifBinary key_bin, const EVP_CIPHER* cipher, ErlNifBinary text,
                   ErlNifBinary *ret_bin, int *ret_bin_alloc, ERL_NIF_TERM *return_term)
{
    CMAC_CTX *ctx = NULL;
    size_t size;
    if ((ctx = CMAC_CTX_new()) == NULL)
        goto local_err;
    if (!CMAC_Init(ctx, key_bin.data, key_bin.size, cipher, NULL))
        goto local_err;
    if (!CMAC_Update(ctx, text.data, text.size))
        goto local_err;
    if ((size = (size_t)EVP_CIPHER_block_size(cipher)) < 0)
        goto local_err;
    if (!enif_alloc_binary(size, ret_bin))
        goto local_err;
    *ret_bin_alloc = 1;
    if (!CMAC_Final(ctx, ret_bin->data, &ret_bin->size))
        goto local_err;
    CMAC_CTX_free(ctx);
    return 1;
 local_err:
    if (ctx)
        CMAC_CTX_free(ctx);
    *return_term = EXCP_ERROR(env,"Compat cmac");
    return 0;
}
#endif