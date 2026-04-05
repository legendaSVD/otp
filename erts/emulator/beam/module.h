#ifndef __MODULE_H__
#define __MODULE_H__
#include "index.h"
#include "beam_code.h"
struct erl_module_instance {
    const BeamCodeHeader* code_hdr;
    int code_length;
    unsigned catches;
    struct erl_module_nif* nif;
    int num_breakpoints;
    int num_traced_exports;
    const void *executable_region;
    void *writable_region;
    void *metadata;
    int unsealed;
};
typedef struct erl_module {
    IndexSlot slot;
    int module;
    int seen;
    struct erl_module_instance curr;
    struct erl_module_instance old;
    struct erl_module_instance* on_load;
} Module;
void erts_module_instance_init(struct erl_module_instance* modi);
Module* erts_get_module(Eterm mod, ErtsCodeIndex code_ix);
Module* erts_put_module(Eterm mod);
void *erts_writable_code_ptr(struct erl_module_instance* modi,
                             const void *ptr);
void erts_unseal_module(struct erl_module_instance *modi);
void erts_seal_module(struct erl_module_instance *modi);
void init_module_table(void);
void module_start_staging(void);
void module_end_staging(int commit);
void module_info(fmtfn_t, void *);
Module *module_code(int, ErtsCodeIndex);
int module_code_size(ErtsCodeIndex);
int module_table_sz(void);
ERTS_GLB_INLINE void erts_rwlock_old_code(ErtsCodeIndex);
ERTS_GLB_INLINE void erts_rwunlock_old_code(ErtsCodeIndex);
ERTS_GLB_INLINE void erts_rlock_old_code(ErtsCodeIndex);
ERTS_GLB_INLINE void erts_runlock_old_code(ErtsCodeIndex);
#ifdef ERTS_ENABLE_LOCK_CHECK
int erts_is_old_code_rlocked(ErtsCodeIndex);
#endif
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
extern erts_rwmtx_t the_old_code_rwlocks[ERTS_NUM_CODE_IX];
ERTS_GLB_INLINE void erts_rwlock_old_code(ErtsCodeIndex code_ix)
{
    erts_rwmtx_rwlock(&the_old_code_rwlocks[code_ix]);
}
ERTS_GLB_INLINE void erts_rwunlock_old_code(ErtsCodeIndex code_ix)
{
    erts_rwmtx_rwunlock(&the_old_code_rwlocks[code_ix]);
}
ERTS_GLB_INLINE void erts_rlock_old_code(ErtsCodeIndex code_ix)
{
    erts_rwmtx_rlock(&the_old_code_rwlocks[code_ix]);
}
ERTS_GLB_INLINE void erts_runlock_old_code(ErtsCodeIndex code_ix)
{
    erts_rwmtx_runlock(&the_old_code_rwlocks[code_ix]);
}
#ifdef ERTS_ENABLE_LOCK_CHECK
ERTS_GLB_INLINE int erts_is_old_code_rlocked(ErtsCodeIndex code_ix)
{
    return erts_lc_rwmtx_is_rlocked(&the_old_code_rwlocks[code_ix]);
}
#endif
#endif
#endif