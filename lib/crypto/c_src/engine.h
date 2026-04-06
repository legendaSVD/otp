#ifndef E_ENGINE_H__
#define E_ENGINE_H__ 1
#include "common.h"
#ifdef HAS_ENGINE_SUPPORT
int get_engine_and_key_id(ErlNifEnv *env, ERL_NIF_TERM key, char ** id, ENGINE **e);
char *get_key_password(ErlNifEnv *env, ERL_NIF_TERM key);
#endif
int init_engine_ctx(ErlNifEnv *env, ErlNifBinary* rt_buf);
int create_engine_mutex(ErlNifEnv *env);
void destroy_engine_mutex(ErlNifEnv *env);
ERL_NIF_TERM engine_by_id_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_init_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_finish_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_free_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_load_dynamic_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_ctrl_cmd_strings_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_register_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_unregister_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_add_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_remove_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_get_first_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_get_next_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_get_id_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_get_name_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM engine_get_all_methods_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ensure_engine_loaded_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
ERL_NIF_TERM ensure_engine_unloaded_nif(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
#endif