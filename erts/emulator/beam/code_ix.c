#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "code_ix.h"
#include "global.h"
#include "beam_catches.h"
#include "erl_record.h"
#if 0
# define CIX_TRACE(text) erts_fprintf(stderr, "CIX_TRACE: " text " act=%u load=%u\r\n", erts_active_code_ix(), erts_staging_code_ix())
#else
# define CIX_TRACE(text)
#endif
#if defined(BEAMASM) && defined(ERTS_THR_INSTRUCTION_BARRIER)
#    define CODE_IX_ISSUE_INSTRUCTION_BARRIERS
#endif
erts_atomic32_t outstanding_blocking_code_barriers;
erts_atomic32_t the_active_code_index;
erts_atomic32_t the_staging_code_index;
struct code_permission {
    erts_mtx_t lock;
    Process *owner;
    void *aux_arg;
    int seized;
    struct code_permission_queue_item {
        Process *p;
        void (*aux_func)(void *);
        void *aux_arg;
        struct code_permission_queue_item *next;
    } *queue;
#ifdef ERTS_ENABLE_LOCK_CHECK
    int lc_soft_check;
#endif
};
static struct code_permission code_mod_permission;
static struct code_permission code_stage_permission;
#ifdef DEBUG
static erts_tsd_key_t needs_code_barrier;
#endif
void erts_code_ix_init(void)
{
    erts_atomic32_init_nob(&outstanding_blocking_code_barriers, 0);
    erts_atomic32_init_nob(&the_active_code_index, 0);
    erts_atomic32_init_nob(&the_staging_code_index, 0);
    erts_mtx_init(&code_mod_permission.lock,
        "code_mod_permission", NIL,
        ERTS_LOCK_FLAGS_PROPERTY_STATIC | ERTS_LOCK_FLAGS_CATEGORY_GENERIC);
    erts_mtx_init(&code_stage_permission.lock,
        "code_stage_permission", NIL,
        ERTS_LOCK_FLAGS_PROPERTY_STATIC | ERTS_LOCK_FLAGS_CATEGORY_GENERIC);
#ifdef DEBUG
    erts_tsd_key_create(&needs_code_barrier,
                        "erts_needs_code_barrier");
#endif
    CIX_TRACE("init");
}
void erts_start_staging_code_ix(int num_new)
{
    beam_catches_start_staging();
    erts_fun_start_staging();
    export_start_staging();
    erts_record_start_staging();
    module_start_staging();
    erts_start_staging_ranges(num_new);
    CIX_TRACE("start");
}
void erts_end_staging_code_ix(void)
{
    beam_catches_end_staging(1);
    erts_fun_end_staging(1);
    export_end_staging(1);
    erts_record_end_staging(1);
    module_end_staging(1);
    erts_end_staging_ranges(1);
    CIX_TRACE("end");
}
void erts_commit_staging_code_ix(void)
{
    extern void export_staged_write_lock(void);
    extern void export_staged_write_unlock(void);
    extern void fun_staged_write_lock(void);
    extern void fun_staged_write_unlock(void);
    extern void record_staged_write_lock(void);
    extern void record_staged_write_unlock(void);
    ErtsCodeIndex ix;
    export_staged_write_lock();
    fun_staged_write_lock();
    record_staged_write_lock();
    {
        ix = erts_staging_code_ix();
        erts_atomic32_set_nob(&the_active_code_index, ix);
        ix = (ix + 1) % ERTS_NUM_CODE_IX;
        erts_atomic32_set_nob(&the_staging_code_index, ix);
    }
    record_staged_write_unlock();
    fun_staged_write_unlock();
    export_staged_write_unlock();
    erts_tracer_nif_clear();
    CIX_TRACE("activate");
}
void erts_abort_staging_code_ix(void)
{
    beam_catches_end_staging(0);
    erts_fun_end_staging(0);
    export_end_staging(0);
    erts_record_end_staging(0);
    module_end_staging(0);
    erts_end_staging_ranges(0);
    CIX_TRACE("abort");
}
#if defined(DEBUG) || defined(ADDRESS_SANITIZER)
#    define CWP_DBG_FORCE_TRAP
#endif
#ifdef ERTS_ENABLE_LOCK_CHECK
static int has_code_permission(struct code_permission *lock);
#endif
static int try_seize_code_permission(struct code_permission *perm,
                                     Process* c_p,
                                     void (*aux_func)(void *),
                                     void *aux_arg)
{
    int success;
    ASSERT(!!c_p != !!aux_func);
    ASSERT(!erts_thr_progress_is_blocking());
    erts_mtx_lock(&perm->lock);
    success = !perm->seized;
    if (success) {
        perm->owner = c_p;
        perm->aux_arg = aux_arg;
        perm->seized = 1;
    } else {
        struct code_permission_queue_item* qitem;
        qitem = erts_alloc(ERTS_ALC_T_CODE_IX_LOCK_Q, sizeof(*qitem));
        if (c_p) {
            ERTS_LC_ASSERT(perm->owner != c_p);
            qitem->p = c_p;
            qitem->aux_func = NULL;
            qitem->aux_arg = NULL;
            erts_proc_inc_refc(c_p);
            erts_suspend(c_p, ERTS_PROC_LOCK_MAIN, NULL);
        } else {
            qitem->p = NULL;
            qitem->aux_func = aux_func;
            qitem->aux_arg = aux_arg;
        }
        qitem->next = perm->queue;
        perm->queue = qitem;
    }
    erts_mtx_unlock(&perm->lock);
    return success;
}
static void release_code_permission(struct code_permission *perm) {
    ERTS_LC_ASSERT(has_code_permission(perm));
    erts_mtx_lock(&perm->lock);
    while (perm->queue != NULL) {
        struct code_permission_queue_item* qitem = perm->queue;
        if (qitem->p) {
            erts_proc_lock(qitem->p, ERTS_PROC_LOCK_STATUS);
            if (!ERTS_PROC_IS_EXITING(qitem->p)) {
                erts_resume(qitem->p, ERTS_PROC_LOCK_STATUS);
            }
            erts_proc_unlock(qitem->p, ERTS_PROC_LOCK_STATUS);
            erts_proc_dec_refc(qitem->p);
        } else {
            ErtsSchedulerData *esdp = erts_get_scheduler_data();
            ASSERT(esdp && esdp->type == ERTS_SCHED_NORMAL);
            erts_schedule_misc_aux_work((int) esdp->no,
                                        qitem->aux_func,
                                        qitem->aux_arg);
        }
        perm->queue = qitem->next;
        erts_free(ERTS_ALC_T_CODE_IX_LOCK_Q, qitem);
    }
    perm->owner = NULL;
    perm->aux_arg = NULL;
    perm->seized = 0;
#ifdef ERTS_ENABLE_LOCK_CHECK
    perm->lc_soft_check = 0;
#endif
    erts_mtx_unlock(&perm->lock);
}
int erts_try_seize_code_mod_permission_aux(void (*aux_func)(void *),
                                           void *aux_arg)
{
    ASSERT(aux_func != NULL);
    return try_seize_code_permission(&code_mod_permission, NULL,
                                     aux_func, aux_arg);
}
#ifdef ERTS_ENABLE_LOCK_CHECK
void erts_lc_soften_code_mod_permission_check(void)
{
    code_mod_permission.lc_soft_check = 1;
}
#endif
int erts_try_seize_code_mod_permission(Process* c_p)
{
    ASSERT(c_p != NULL);
#ifdef CWP_DBG_FORCE_TRAP
    if (!(c_p->flags & F_DBG_FORCED_TRAP)) {
        c_p->flags |= F_DBG_FORCED_TRAP;
        return 0;
    } else {
        c_p->flags &= ~F_DBG_FORCED_TRAP;
    }
#endif
    return try_seize_code_permission(&code_mod_permission, c_p, NULL, NULL);
}
void erts_release_code_mod_permission(void)
{
    release_code_permission(&code_mod_permission);
}
int erts_try_seize_code_stage_permission(Process* c_p)
{
    ASSERT(c_p != NULL);
#ifdef CWP_DBG_FORCE_TRAP
    if (!(c_p->flags & F_DBG_FORCED_TRAP)) {
        c_p->flags |= F_DBG_FORCED_TRAP;
        return 0;
    } else {
        c_p->flags &= ~F_DBG_FORCED_TRAP;
    }
#endif
    return try_seize_code_permission(&code_stage_permission, c_p, NULL, NULL);
}
void erts_release_code_stage_permission(void) {
    release_code_permission(&code_stage_permission);
}
int erts_try_seize_code_load_permission(Process* c_p) {
    ASSERT(c_p != NULL);
#ifdef CWP_DBG_FORCE_TRAP
    if (!(c_p->flags & F_DBG_FORCED_TRAP)) {
        c_p->flags |= F_DBG_FORCED_TRAP;
        return 0;
    } else {
        c_p->flags &= ~F_DBG_FORCED_TRAP;
    }
#endif
    if (try_seize_code_permission(&code_stage_permission, c_p, NULL, NULL)) {
        if (try_seize_code_permission(&code_mod_permission, c_p, NULL, NULL)) {
            return 1;
        }
        erts_release_code_stage_permission();
    }
    return 0;
}
void erts_release_code_load_permission(void) {
    erts_release_code_mod_permission();
    erts_release_code_stage_permission();
}
#ifdef ERTS_ENABLE_LOCK_CHECK
static int has_code_permission(struct code_permission *perm)
{
    const ErtsSchedulerData *esdp = erts_get_scheduler_data();
    if (esdp && esdp->type == ERTS_SCHED_NORMAL) {
        int res;
        if (!perm->seized) {
            res = 0;
        }
        else if (esdp->current_process != NULL) {
            res = (perm->owner == esdp->current_process);
        } else {
            res = (perm->owner
        #ifdef ERTS_ENABLE_LOCK_CHECK
                   || perm->lc_soft_check
        #endif
                   || esdp->aux_work_data.lc_aux_arg == perm->aux_arg);
        }
        return res;
    }
    return 0;
}
int erts_has_code_load_permission(void) {
    return erts_has_code_stage_permission() && erts_has_code_mod_permission();
}
int erts_has_code_stage_permission(void) {
    return has_code_permission(&code_stage_permission);
}
int erts_has_code_mod_permission(void) {
    return has_code_permission(&code_mod_permission);
}
#endif
#ifdef DEBUG
void erts_debug_require_code_barrier(void) {
    erts_tsd_set(needs_code_barrier, (void*)(1));
}
void erts_debug_check_code_barrier(void) {
    ASSERT(erts_tsd_get(needs_code_barrier) == (void*)0);
}
static void erts_debug_unrequire_code_barrier(void) {
    erts_tsd_set(needs_code_barrier, (void*)(0));
}
#endif
static void schedule_code_barrier_later_op(void *barrier_) {
    ErtsCodeBarrier *barrier = (ErtsCodeBarrier*)barrier_;
    if (barrier->size == 0) {
        erts_schedule_thr_prgr_later_op(barrier->later_function,
                                        barrier->later_data,
                                        &barrier->later_op);
    } else {
        erts_schedule_thr_prgr_later_cleanup_op(barrier->later_function,
                                                barrier->later_data,
                                                &barrier->later_op,
                                                barrier->size);
    }
}
#ifdef CODE_IX_ISSUE_INSTRUCTION_BARRIERS
static void issue_instruction_barrier(void *barrier_) {
    ErtsCodeBarrier *barrier = (ErtsCodeBarrier*)barrier_;
    ERTS_THR_INSTRUCTION_BARRIER;
    if (erts_refc_dectest(&barrier->pending_schedulers, 0) == 0) {
        schedule_code_barrier_later_op(barrier);
    }
}
#endif
void erts_schedule_code_barrier(ErtsCodeBarrier *barrier,
                                void (*later_function)(void *),
                                void *later_data) {
    erts_schedule_code_barrier_cleanup(barrier, later_function, later_data, 0);
}
void erts_schedule_code_barrier_cleanup(ErtsCodeBarrier *barrier,
                                        void (*later_function)(void *),
                                        void *later_data,
                                        UWord size)
{
#ifdef DEBUG
    erts_debug_unrequire_code_barrier();
#endif
    barrier->later_function = later_function;
    barrier->later_data = later_data;
    barrier->size = size;
#ifdef CODE_IX_ISSUE_INSTRUCTION_BARRIERS
    erts_refc_init(&barrier->pending_schedulers,
                   (erts_aint_t)erts_no_schedulers);
    erts_schedule_multi_misc_aux_work(1, 1, erts_no_schedulers,
                                      issue_instruction_barrier,
                                      barrier);
    issue_instruction_barrier(barrier);
#else
    schedule_code_barrier_later_op(barrier);
#endif
}
#ifdef CODE_IX_ISSUE_INSTRUCTION_BARRIERS
static ErtsThrPrgrLaterOp global_code_barrier_lop;
static void decrement_blocking_code_barriers(void *ignored) {
    (void)ignored;
    if (erts_atomic32_dec_read_nob(&outstanding_blocking_code_barriers) > 0) {
        erts_atomic32_set_nob(&outstanding_blocking_code_barriers, 1);
        erts_schedule_thr_prgr_later_op(decrement_blocking_code_barriers,
                                        NULL,
                                        &global_code_barrier_lop);
    }
}
static void schedule_blocking_code_barriers(void *ignored) {
    ERTS_THR_INSTRUCTION_BARRIER;
    if (erts_atomic32_inc_read_nob(&outstanding_blocking_code_barriers) == 1) {
        erts_schedule_thr_prgr_later_op(decrement_blocking_code_barriers,
                                        NULL,
                                        &global_code_barrier_lop);
    }
}
#endif
void erts_blocking_code_barrier(void)
{
#ifdef DEBUG
    erts_debug_unrequire_code_barrier();
#endif
    ERTS_LC_ASSERT(erts_thr_progress_is_blocking());
#ifdef CODE_IX_ISSUE_INSTRUCTION_BARRIERS
    schedule_blocking_code_barriers(NULL);
#endif
}
void erts_code_ix_finalize_wait(void) {
#ifdef CODE_IX_ISSUE_INSTRUCTION_BARRIERS
    if (erts_atomic32_read_nob(&outstanding_blocking_code_barriers) != 0) {
        ERTS_THR_INSTRUCTION_BARRIER;
    }
#endif
}