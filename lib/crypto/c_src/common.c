#include "common.h"
#include <string.h>
#define MAX_CRYPTOLIB_ERR_SIZE 256
#define SEP ": "
ERL_NIF_TERM raise_exception(ErlNifEnv* env, ERL_NIF_TERM id, int arg_num, char* explanation, char* file, int line)
{
    ERL_NIF_TERM file_info, exception;
    char *error_msg;
#ifdef CRYPTO_DEVELOP_ERRORS
    char *p;
    error_msg = enif_alloc(strlen(explanation) + strlen(SEP) + MAX_CRYPTOLIB_ERR_SIZE);
    p = error_msg;
    strcpy(p, explanation);
    p += strlen(explanation);
    strcpy(p, SEP);
    p += strlen(SEP);
    ERR_error_string_n(ERR_peek_last_error(), p, MAX_CRYPTOLIB_ERR_SIZE);
#else
    error_msg = explanation;
#endif
    {
        ERL_NIF_TERM keys[3], vals[3];
        int ok;
        keys[0] = enif_make_atom(env,"c_file_name");
        vals[0] = enif_make_string(env, file, ERL_NIF_LATIN1);
        keys[1] = enif_make_atom(env,"c_file_line_num");
        vals[1] = enif_make_int(env, line);
        keys[2] = enif_make_atom(env,"c_function_arg_num");
        vals[2] = enif_make_int(env, arg_num);
        ok = enif_make_map_from_arrays(env, keys, vals, 3, &file_info);
        ASSERT(ok); (void)ok;
    }
    exception =
        enif_make_tuple3(env,
                         id,
                         file_info,
                         enif_make_string(env, error_msg, (ERL_NIF_LATIN1))
                         );
#ifdef CRYPTO_DEVELOP_ERRORS
    enif_free(error_msg);
#endif
    return enif_raise_exception(env, exception);
}