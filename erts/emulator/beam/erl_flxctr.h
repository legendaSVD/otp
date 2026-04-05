#ifndef ERL_FLXCTR_H__
#define ERL_FLXCTR_H__
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "error.h"
#include "bif.h"
#include "big.h"
#include "erl_binary.h"
#include "bif.h"
#include <stddef.h>
#include <stdbool.h>
#define ERTS_MAX_FLXCTR_GROUPS 256
#define ERTS_FLXCTR_ATOMICS_PER_CACHE_LINE (ERTS_CACHE_LINE_SIZE / sizeof(erts_atomic_t))
typedef struct {
    int nr_of_counters;
    bool is_decentralized;
    union {
        erts_atomic_t counters_ptr;
        erts_atomic_t counters[1];
    } u;
} ErtsFlxCtr;
#define ERTS_FLXCTR_NR_OF_EXTRA_BYTES(NR_OF_COUNTERS)   \
    ((NR_OF_COUNTERS-1) * sizeof(erts_atomic_t))
void erts_flxctr_setup(int decentralized_counter_groups);
void erts_flxctr_init(ErtsFlxCtr* c,
                      bool is_decentralized,
                      Uint nr_of_counters,
                      ErtsAlcType_t alloc_type);
void erts_flxctr_destroy(ErtsFlxCtr* c, ErtsAlcType_t alloc_type);
ERTS_GLB_INLINE
void erts_flxctr_add(ErtsFlxCtr* c,
                     Uint counter_nr,
                     int to_add);
ERTS_GLB_INLINE
void erts_flxctr_inc(ErtsFlxCtr* c,
                     Uint counter_nr);
ERTS_GLB_INLINE
void erts_flxctr_dec(ErtsFlxCtr* c,
                     Uint counter_nr);
Sint erts_flxctr_read_approx(ErtsFlxCtr* c,
                             Uint counter_nr);
ERTS_GLB_INLINE
Sint erts_flxctr_inc_read_centralized(ErtsFlxCtr* c,
                                      Uint counter_nr);
ERTS_GLB_INLINE
Sint erts_flxctr_dec_read_centralized(ErtsFlxCtr* c,
                                      Uint counter_nr);
ERTS_GLB_INLINE
Sint erts_flxctr_read_centralized(ErtsFlxCtr* c,
                                  Uint counter_nr);
typedef enum {
    ERTS_FLXCTR_TRY_AGAIN_AFTER_TRAP,
    ERTS_FLXCTR_DONE,
    ERTS_FLXCTR_GET_RESULT_AFTER_TRAP
} ErtsFlxctrSnapshotResultType;
typedef struct {
    ErtsFlxctrSnapshotResultType type;
    Eterm trap_resume_state;
    Sint result[ERTS_FLXCTR_ATOMICS_PER_CACHE_LINE];
} ErtsFlxCtrSnapshotResult;
ErtsFlxCtrSnapshotResult
erts_flxctr_snapshot(ErtsFlxCtr* c,
                     ErtsAlcType_t alloc_type,
                     Process* p);
bool erts_flxctr_is_snapshot_result(Eterm term);
Sint erts_flxctr_get_snapshot_result_after_trap(Eterm trap_resume_state,
                                                Uint counter_nr);
void erts_flxctr_reset(ErtsFlxCtr* c,
                       Uint counter_nr);
bool erts_flxctr_is_snapshot_ongoing(ErtsFlxCtr* c);
int erts_flxctr_suspend_until_thr_prg_if_snapshot_ongoing(ErtsFlxCtr* c, Process* p);
size_t erts_flxctr_nr_of_allocated_bytes(ErtsFlxCtr* c);
Sint erts_flxctr_debug_memory_usage(void);
#define ERTS_FLXCTR_GET_CTR_ARRAY_PTR(C)                                \
    ((ErtsFlxCtrDecentralizedCtrArray*) erts_atomic_read_acqb(&(C)->u.counters_ptr))
#define ERTS_FLXCTR_GET_CTR_PTR(C, SCHEDULER_ID, COUNTER_ID)            \
    &(ERTS_FLXCTR_GET_CTR_ARRAY_PTR(C))->array[SCHEDULER_ID].counters[COUNTER_ID]
typedef union {
    erts_atomic_t counters[ERTS_FLXCTR_ATOMICS_PER_CACHE_LINE];
    char pad[ERTS_CACHE_LINE_SIZE];
} ErtsFlxCtrDecentralizedCtrArrayElem;
typedef struct ErtsFlxCtrDecentralizedCtrArray {
    void* block_start;
    erts_atomic_t snapshot_status;
    ErtsFlxCtrDecentralizedCtrArrayElem array[];
} ErtsFlxCtrDecentralizedCtrArray;
void erts_flxctr_set_slot(int group);
ERTS_GLB_INLINE
int erts_flxctr_get_slot_index(void);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
int erts_flxctr_get_slot_index(void)
{
    ErtsSchedulerData *esdp = erts_get_scheduler_data();
    ASSERT(esdp && !ERTS_SCHEDULER_IS_DIRTY(esdp));
    ASSERT(esdp->flxctr_slot_no > 0);
    return esdp->flxctr_slot_no;
}
ERTS_GLB_INLINE
void erts_flxctr_add(ErtsFlxCtr* c,
                     Uint counter_nr,
                     int to_add)
{
    ASSERT(counter_nr < c->nr_of_counters);
    if (c->is_decentralized) {
        erts_atomic_add_nob(ERTS_FLXCTR_GET_CTR_PTR(c,
                                                    erts_flxctr_get_slot_index(),
                                                    counter_nr),
                            to_add);
    } else {
        erts_atomic_add_nob(&c->u.counters[counter_nr], to_add);
    }
}
ERTS_GLB_INLINE
void erts_flxctr_inc(ErtsFlxCtr* c,
                     Uint counter_nr)
{
    ASSERT(counter_nr < c->nr_of_counters);
    if (c->is_decentralized) {
        erts_atomic_inc_nob(ERTS_FLXCTR_GET_CTR_PTR(c,
                                                    erts_flxctr_get_slot_index(),
                                                    counter_nr));
    } else {
        erts_atomic_inc_read_nob(&c->u.counters[counter_nr]);
    }
}
ERTS_GLB_INLINE
void erts_flxctr_dec(ErtsFlxCtr* c,
                     Uint counter_nr)
{
    ASSERT(counter_nr < c->nr_of_counters);
    if (c->is_decentralized) {
        erts_atomic_dec_nob(ERTS_FLXCTR_GET_CTR_PTR(c,
                                                    erts_flxctr_get_slot_index(),
                                                    counter_nr));
    } else {
        erts_atomic_dec_nob(&c->u.counters[counter_nr]);
    }
}
ERTS_GLB_INLINE
Sint erts_flxctr_inc_read_centralized(ErtsFlxCtr* c,
                                      Uint counter_nr)
{
    ASSERT(counter_nr < c->nr_of_counters);
    ASSERT(!c->is_decentralized);
    return erts_atomic_inc_read_nob(&c->u.counters[counter_nr]);
}
ERTS_GLB_INLINE
Sint erts_flxctr_dec_read_centralized(ErtsFlxCtr* c,
                                      Uint counter_nr)
{
    ASSERT(counter_nr < c->nr_of_counters);
    ASSERT(!c->is_decentralized);
    return erts_atomic_dec_read_nob(&c->u.counters[counter_nr]);
}
ERTS_GLB_INLINE
Sint erts_flxctr_read_centralized(ErtsFlxCtr* c,
                                  Uint counter_nr)
{
    ASSERT(counter_nr < c->nr_of_counters);
    ASSERT(!c->is_decentralized);
    return erts_atomic_read_nob(&((erts_atomic_t*)(c->u.counters))[counter_nr]);
}
#endif
#endif