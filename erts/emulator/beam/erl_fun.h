#ifndef __ERLFUNTABLE_H__
#define __ERLFUNTABLE_H__
#include "erl_threads.h"
typedef struct erl_fun_entry {
    ErtsDispatchable dispatch;
    ErtsCodePtr pend_purge_address;
    Eterm module;
    byte uniq[16];
    int arity;
    int index;
    int old_uniq;
    int old_index;
} ErlFunEntry;
typedef struct erl_fun_thing {
    Eterm thing_word;
    union {
        const ErtsDispatchable *disp;
        const ErlFunEntry *fun;
        const Export *exp;
    } entry;
    Eterm env[];
} ErlFunThing;
#define is_external_fun(FunThing)                                             \
    (!!((FunThing)->thing_word >> FUN_HEADER_KIND_OFFS))
#define is_local_fun(FunThing)                                                \
    (!is_external_fun(FunThing))
#define fun_arity(FunThing)                                                   \
    (((FunThing)->thing_word >> FUN_HEADER_ARITY_OFFS) & 0xFF)
#define fun_num_free(FunThing)                                                \
    (((FunThing)->thing_word >> FUN_HEADER_ENV_SIZE_OFFS) & 0xFF)
#define ERL_FUN_SIZE ((sizeof(ErlFunThing)/sizeof(Eterm)))
void erts_init_fun_table(void);
void erts_fun_info(fmtfn_t, void *);
int erts_fun_table_sz(void);
int erts_fun_entries_sz(void);
ErlFunEntry *erts_fun_entry_put(Eterm mod, int old_uniq, int old_index,
                                const byte* uniq, int index, int arity);
const ErlFunEntry *erts_fun_entry_get_or_make_stub(Eterm mod,
                                                   int old_uniq,
                                                   int old_index,
                                                   const byte* uniq,
                                                   int index,
                                                   int arity);
const ErtsCodeMFA *erts_get_fun_mfa(const ErlFunEntry *fe, ErtsCodeIndex ix);
void erts_set_fun_code(ErlFunEntry *fe, ErtsCodeIndex ix, ErtsCodePtr address);
ERTS_GLB_INLINE
ErtsCodePtr erts_get_fun_code(ErlFunEntry *fe, ErtsCodeIndex ix);
int erts_is_fun_loaded(const ErlFunEntry* fe, ErtsCodeIndex ix);
struct erl_module_instance;
void erts_fun_purge_prepare(struct erl_module_instance* modi);
void erts_fun_purge_abort_prepare(ErlFunEntry **funs, Uint no);
void erts_fun_purge_abort_finalize(ErlFunEntry **funs, Uint no);
void erts_fun_purge_complete(ErlFunEntry **funs, Uint no);
void erts_dump_fun_entries(fmtfn_t, void *);
void erts_fun_start_staging(void);
void erts_fun_end_staging(int commit);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
ErtsCodePtr erts_get_fun_code(ErlFunEntry *fe, ErtsCodeIndex ix) {
    return fe->dispatch.addresses[ix];
}
#endif
#endif