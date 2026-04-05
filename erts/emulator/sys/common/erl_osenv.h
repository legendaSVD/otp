#ifndef __ERL_OSENV_H__
#define __ERL_OSENV_H__
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
typedef struct __erts_osenv_data_t erts_osenv_data_t;
struct __erts_osenv_data_t {
    Sint length;
    void *data;
};
void erts_osenv_init(erts_osenv_t *env);
void erts_osenv_clear(erts_osenv_t *env);
void erts_osenv_merge(erts_osenv_t *env, const erts_osenv_t *with, int overwrite);
int erts_osenv_get_term(const erts_osenv_t *env, struct process *process,
    Eterm key, Eterm *value);
int erts_osenv_put_term(erts_osenv_t *env, Eterm key, Eterm value);
int erts_osenv_unset_term(erts_osenv_t *env, Eterm key);
int erts_osenv_get_native(const erts_osenv_t *env, const erts_osenv_data_t *key,
    erts_osenv_data_t *value);
int erts_osenv_put_native(erts_osenv_t *env, const erts_osenv_data_t *key,
    const erts_osenv_data_t *value);
int erts_osenv_unset_native(erts_osenv_t *env, const erts_osenv_data_t *key);
typedef void (*erts_osenv_foreach_term_cb_t)(struct process *process,
    void *state, Eterm key, Eterm value);
typedef void (*erts_osenv_foreach_native_cb_t)(void *state,
    const erts_osenv_data_t *key,
    const erts_osenv_data_t *value);
void erts_osenv_foreach_term(const erts_osenv_t *env, struct process *process,
    void *state, erts_osenv_foreach_term_cb_t callback);
void erts_osenv_foreach_native(const erts_osenv_t *env, void *state,
    erts_osenv_foreach_native_cb_t callback);
#endif