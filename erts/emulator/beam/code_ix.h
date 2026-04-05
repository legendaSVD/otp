#ifndef __CODE_IX_H__
#define __CODE_IX_H__
#ifndef __SYS_H__
#  ifdef HAVE_CONFIG_H
#    include "config.h"
#  endif
#  include "sys.h"
#endif
#include "beam_opcodes.h"
#undef ERL_THR_PROGRESS_TSD_TYPE_ONLY
#define ERL_THR_PROGRESS_TSD_TYPE_ONLY
#include "erl_thr_progress.h"
#undef ERL_THR_PROGRESS_TSD_TYPE_ONLY
struct process;
#define ERTS_NUM_CODE_IX 3
#ifdef BEAMASM
#define ERTS_ADDRESSV_SIZE (ERTS_NUM_CODE_IX + 1)
#define ERTS_SAVE_CALLS_CODE_IX (ERTS_ADDRESSV_SIZE - 1)
#else
#define ERTS_ADDRESSV_SIZE ERTS_NUM_CODE_IX
#endif
typedef struct ErtsDispatchable_ {
    ErtsCodePtr addresses[ERTS_ADDRESSV_SIZE];
} ErtsDispatchable;
typedef unsigned ErtsCodeIndex;
typedef struct ErtsCodeMFA_ {
    Eterm module;
    Eterm function;
    Uint arity;
} ErtsCodeMFA;
typedef struct ErtsCodeInfo_ {
    struct {
#ifndef BEAMASM
        BeamInstr op;
#else
        struct {
            char raise_function_clause[sizeof(BeamInstr) - 1];
            char breakpoint_flag;
        } metadata;
#endif
    } u;
    struct GenericBp *gen_bp;
    ErtsCodeMFA mfa;
} ErtsCodeInfo;
typedef struct {
    erts_refc_t pending_schedulers;
    ErtsThrPrgrLaterOp later_op;
    UWord size;
    void (*later_function)(void *);
    void *later_data;
} ErtsCodeBarrier;
ERTS_GLB_INLINE
ErtsCodePtr erts_codeinfo_to_code(const ErtsCodeInfo *ci);
ERTS_GLB_INLINE
const ErtsCodeInfo *erts_code_to_codeinfo(ErtsCodePtr I);
ERTS_GLB_INLINE
ErtsCodePtr erts_codemfa_to_code(const ErtsCodeMFA *mfa);
ERTS_GLB_INLINE
const ErtsCodeMFA *erts_code_to_codemfa(ErtsCodePtr I);
void erts_code_ix_init(void);
ERTS_GLB_INLINE
ErtsCodeIndex erts_active_code_ix(void);
ERTS_GLB_INLINE
ErtsCodeIndex erts_staging_code_ix(void);
int erts_try_seize_code_load_permission(struct process* c_p);
void erts_release_code_load_permission(void);
int erts_try_seize_code_stage_permission(struct process* c_p);
void erts_release_code_stage_permission(void);
int erts_try_seize_code_mod_permission(struct process* c_p);
int erts_try_seize_code_mod_permission_aux(void (*func)(void *),
                                           void *arg);
#ifdef ERTS_ENABLE_LOCK_CHECK
void erts_lc_soften_code_mod_permission_check(void);
#else
# define erts_lc_soften_code_mod_permission_check() ((void)0)
#endif
void erts_release_code_mod_permission(void);
void erts_start_staging_code_ix(int num_new);
void erts_end_staging_code_ix(void);
void erts_commit_staging_code_ix(void);
void erts_abort_staging_code_ix(void);
#ifdef DEBUG
void erts_debug_require_code_barrier(void);
void erts_debug_check_code_barrier(void);
#endif
void erts_schedule_code_barrier(ErtsCodeBarrier *barrier,
                                void (*later_function)(void *),
                                void *later_data);
void erts_schedule_code_barrier_cleanup(ErtsCodeBarrier *barrier,
                                        void (*later_function)(void *),
                                        void *later_data,
                                        UWord size);
void erts_blocking_code_barrier(void);
void erts_code_ix_finalize_wait(void);
#ifdef ERTS_ENABLE_LOCK_CHECK
int erts_has_code_load_permission(void);
int erts_has_code_stage_permission(void);
int erts_has_code_mod_permission(void);
#endif
#define ASSERT_MFA(MFA)                                                 \
    ASSERT(is_atom((MFA)->module) && is_atom((MFA)->function))
extern erts_atomic32_t the_active_code_index;
extern erts_atomic32_t the_staging_code_index;
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
ErtsCodePtr erts_codeinfo_to_code(const ErtsCodeInfo *ci)
{
#ifndef BEAMASM
    ASSERT(BeamIsOpCode(ci->u.op, op_i_func_info_IaaI) || !ci->u.op);
#endif
    ASSERT_MFA(&ci->mfa);
    return (ErtsCodePtr)&ci[1];
}
ERTS_GLB_INLINE
const ErtsCodeInfo *erts_code_to_codeinfo(ErtsCodePtr I)
{
    const ErtsCodeInfo *ci = &((const ErtsCodeInfo *)I)[-1];
#ifndef BEAMASM
    ASSERT(BeamIsOpCode(ci->u.op, op_i_func_info_IaaI) || !ci->u.op);
#endif
    ASSERT_MFA(&ci->mfa);
    return ci;
}
ERTS_GLB_INLINE
ErtsCodePtr erts_codemfa_to_code(const ErtsCodeMFA *mfa)
{
    ASSERT_MFA(mfa);
    return (ErtsCodePtr)&mfa[1];
}
ERTS_GLB_INLINE
const ErtsCodeMFA *erts_code_to_codemfa(ErtsCodePtr I)
{
    const ErtsCodeMFA *mfa = &((const ErtsCodeMFA *)I)[-1];
    ASSERT_MFA(mfa);
    return mfa;
}
ERTS_GLB_INLINE ErtsCodeIndex erts_active_code_ix(void)
{
    return erts_atomic32_read_nob(&the_active_code_index);
}
ERTS_GLB_INLINE ErtsCodeIndex erts_staging_code_ix(void)
{
    return erts_atomic32_read_nob(&the_staging_code_index);
}
#endif
#endif