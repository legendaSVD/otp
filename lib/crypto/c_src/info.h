#ifndef E_INFO_H__
#define E_INFO_H__ 1
#include "common.h"
#ifdef HAVE_DYNAMIC_CRYPTO_LIB
extern char *crypto_callback_name;
int change_basename(ErlNifBinary* bin, char* buf, size_t bufsz, const char* newfile);
void error_handler(void* null, const char* errstr);
#endif
const char* resource_name(const char *name, ErlNifBinary* buf);
ERL_NIF_TERM info_lib(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM info_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif