#ifndef ERTS_DB_SCHED_SPEC_TYPES__
#define ERTS_DB_SCHED_SPEC_TYPES__
union db_table;
typedef union db_table DbTable;
typedef struct ErtsEtsAllReq_ ErtsEtsAllReq;
typedef struct {
    ErtsEtsAllReq *next;
    ErtsEtsAllReq *prev;
} ErtsEtsAllReqList;
typedef struct {
    ErtsEtsAllReq *ongoing;
    ErlHeapFragment *hfrag;
    DbTable *tab;
    ErtsEtsAllReq *queue;
} ErtsEtsAllYieldData;
typedef struct {
    erts_atomic_t count;
    DbTable *clist;
} ErtsEtsTables;
#endif
#ifndef ERTS_ONLY_SCHED_SPEC_ETS_DATA
#ifndef ERL_DB_H__
#define ERL_DB_H__
#include "sys.h"
#undef ERL_THR_PROGRESS_TSD_TYPE_ONLY
#define ERL_THR_PROGRESS_TSD_TYPE_ONLY
#include "erl_thr_progress.h"
#undef ERL_THR_PROGRESS_TSD_TYPE_ONLY
#include "bif.h"
#include "erl_db_util.h"
#include "erl_db_hash.h"
#include "erl_db_tree.h"
#include "erl_db_catree.h"
Uint erts_get_ets_misc_mem_size(void);
Uint erts_ets_table_count(void);
typedef struct {
    DbTableCommon common;
    ErtsThrPrgrLaterOp data;
} DbTableRelease;
struct ErtsSchedulerData_;
int erts_handle_yielded_ets_all_request(ErtsAuxWorkData *awdp);
void erts_ets_sched_spec_data_init(struct ErtsSchedulerData_ *esdp);
union db_table {
    DbTableCommon common;
    DbTableHash hash;
    DbTableTree tree;
    DbTableCATree catree;
    DbTableRelease release;
};
#define DB_DEF_MAX_TABS 8192
#define ERL_MAX_ETS_TABLES_ENV "ERL_MAX_ETS_TABLES"
typedef enum {
    ERTS_DB_SPNCNT_NONE,
    ERTS_DB_SPNCNT_VERY_LOW,
    ERTS_DB_SPNCNT_LOW,
    ERTS_DB_SPNCNT_NORMAL,
    ERTS_DB_SPNCNT_HIGH,
    ERTS_DB_SPNCNT_VERY_HIGH,
    ERTS_DB_SPNCNT_EXTREMELY_HIGH
} ErtsDbSpinCount;
void init_db(ErtsDbSpinCount);
int erts_db_process_exiting(Process *, ErtsProcLocks, void **);
int erts_db_execute_free_fixation(Process*, DbFixation*);
void db_info(fmtfn_t, void *, bool);
void erts_db_foreach_table(void (*)(DbTable *, void *), void *, bool);
void erts_db_foreach_offheap(DbTable *,
			     void (*func)(ErlOffHeap *, void *),
			     void *);
void erts_db_foreach_thr_prgr_offheap(void (*func)(ErlOffHeap *, void *),
                                      void *);
extern int erts_ets_rwmtx_spin_count;
extern int user_requested_db_max_tabs;
extern bool erts_ets_realloc_always_moves;
extern bool erts_ets_always_compress;
extern Export ets_select_delete_continue_exp;
extern Export ets_select_count_continue_exp;
extern Export ets_select_replace_continue_exp;
extern Export ets_select_continue_exp;
extern erts_atomic_t erts_ets_misc_mem_size;
Eterm erts_ets_colliding_names(Process*, Eterm name, Uint cnt);
int erts_ets_force_split(Eterm tid, int on);
int erts_ets_debug_random_split_join(Eterm tid, int on);
Uint erts_db_get_max_tabs(void);
Eterm erts_db_make_tid(Process *c_p, DbTableCommon *tb);
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_enable_db_lock_count(DbTable *tb, int enable);
void erts_lcnt_update_db_locks(int enable);
#endif
#ifdef ETS_DBG_FORCE_TRAP
extern int erts_ets_dbg_force_trap;
#endif
#endif
#if defined(ERTS_WANT_DB_INTERNAL__) && !defined(ERTS_HAVE_DB_INTERNAL__)
#define ERTS_HAVE_DB_INTERNAL__
#include "erl_alloc.h"
#define ERTS_DB_ALC_MEM_UPDATE_(TAB, FREE_SZ, ALLOC_SZ)			\
do {                                                                    \
    if ((TAB) != NULL) {                                                \
        erts_aint_t sz__ = (((erts_aint_t) (ALLOC_SZ))			\
                            - ((erts_aint_t) (FREE_SZ)));               \
        ASSERT((TAB));							\
        erts_flxctr_add(&(TAB)->common.counters,                        \
                        ERTS_DB_TABLE_MEM_COUNTER_ID,                   \
                        sz__);                                          \
    }                                                                   \
} while (0)
#define ERTS_ETS_MISC_MEM_ADD(SZ) \
  erts_atomic_add_nob(&erts_ets_misc_mem_size, (SZ));
ERTS_GLB_INLINE void *erts_db_alloc(ErtsAlcType_t type,
				    DbTable *tab,
				    Uint size) ERTS_ATTR_MALLOC_US(3);
ERTS_GLB_INLINE void *erts_db_alloc_fnf(ErtsAlcType_t type,
					DbTable *tab,
					Uint size) ERTS_ATTR_MALLOC_US(3);
ERTS_GLB_INLINE void*
erts_db_alloc_nt(ErtsAlcType_t type, Uint size) ERTS_ATTR_MALLOC_US(2);
ERTS_GLB_INLINE void*
erts_db_alloc_fnf_nt(ErtsAlcType_t type, Uint size) ERTS_ATTR_MALLOC_US(2);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void *
erts_db_alloc(ErtsAlcType_t type, DbTable *tab, Uint size)
{
    void *res = erts_alloc(type, size);
    ERTS_DB_ALC_MEM_UPDATE_(tab, 0, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_alloc_fnf(ErtsAlcType_t type, DbTable *tab, Uint size)
{
    void *res = erts_alloc_fnf(type, size);
    if (!res)
	return NULL;
    ERTS_DB_ALC_MEM_UPDATE_(tab, 0, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_alloc_nt(ErtsAlcType_t type, Uint size)
{
    void *res = erts_alloc(type, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_alloc_fnf_nt(ErtsAlcType_t type, Uint size)
{
    void *res = erts_alloc_fnf(type, size);
    if (!res)
	return NULL;
    return res;
}
#endif
ERTS_GLB_INLINE void *erts_db_realloc(ErtsAlcType_t type,
				      DbTable *tab,
				      void *ptr,
				      Uint old_size,
				      Uint size);
ERTS_GLB_INLINE void *erts_db_realloc_fnf(ErtsAlcType_t type,
					  DbTable *tab,
					  void *ptr,
					  Uint old_size,
					  Uint size);
ERTS_GLB_INLINE void *erts_db_realloc_nt(ErtsAlcType_t type,
					 void *ptr,
					 Uint old_size,
					 Uint size);
ERTS_GLB_INLINE void *erts_db_realloc_fnf_nt(ErtsAlcType_t type,
					     void *ptr,
					     Uint old_size,
					     Uint size);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void *
erts_db_realloc(ErtsAlcType_t type, DbTable *tab, void *ptr,
		Uint old_size, Uint size)
{
    void *res;
    ASSERT(!ptr || old_size == ERTS_ALC_DBG_BLK_SZ(ptr));
    res = erts_realloc(type, ptr, size);
    ERTS_DB_ALC_MEM_UPDATE_(tab, old_size, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_realloc_fnf(ErtsAlcType_t type, DbTable *tab, void *ptr,
		    Uint old_size, Uint size)
{
    void *res;
    ASSERT(!ptr || old_size == ERTS_ALC_DBG_BLK_SZ(ptr));
    res = erts_realloc_fnf(type, ptr, size);
    if (!res)
	return NULL;
    ERTS_DB_ALC_MEM_UPDATE_(tab, old_size, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_realloc_nt(ErtsAlcType_t type, void *ptr,
		   Uint old_size, Uint size)
{
    void *res;
    ASSERT(!ptr || old_size == ERTS_ALC_DBG_BLK_SZ(ptr));
    res = erts_realloc(type, ptr, size);
    return res;
}
ERTS_GLB_INLINE void *
erts_db_realloc_fnf_nt(ErtsAlcType_t type, void *ptr,
		       Uint old_size, Uint size)
{
    void *res;
    ASSERT(!ptr || old_size == ERTS_ALC_DBG_BLK_SZ(ptr));
    res = erts_realloc_fnf(type, ptr, size);
    if (!res)
	return NULL;
    return res;
}
#endif
ERTS_GLB_INLINE void erts_db_free(ErtsAlcType_t type,
				  DbTable *tab,
				  void *ptr,
				  Uint size);
ERTS_GLB_INLINE void erts_schedule_db_free(DbTableCommon* tab,
                                           void (*free_func)(void *),
                                           void *ptr,
                                           ErtsThrPrgrLaterOp *lop,
                                           Uint size);
ERTS_GLB_INLINE void erts_db_free_nt(ErtsAlcType_t type,
				     void *ptr,
				     Uint size);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void
erts_db_free(ErtsAlcType_t type, DbTable *tab, void *ptr, Uint size)
{
    ASSERT(ptr != 0);
    ASSERT(size == ERTS_ALC_DBG_BLK_SZ(ptr));
    ERTS_DB_ALC_MEM_UPDATE_(tab, size, 0);
    ASSERT(tab == NULL ||
           ((void *) tab) != ptr ||
           tab->common.counters.is_decentralized ||
           0 == erts_flxctr_read_centralized(&tab->common.counters,
                                             ERTS_DB_TABLE_MEM_COUNTER_ID));
    erts_free(type, ptr);
}
ERTS_GLB_INLINE void
erts_schedule_db_free(DbTableCommon* tab,
                      void (*free_func)(void *),
                      void *ptr,
                      ErtsThrPrgrLaterOp *lop,
                      Uint size)
{
    ASSERT(ptr != 0);
    ASSERT(((void *) tab) != ptr);
    ASSERT(size == ERTS_ALC_DBG_BLK_SZ(ptr));
    ERTS_DB_ALC_MEM_UPDATE_((DbTable*)tab, size, 0);
    erts_schedule_thr_prgr_later_cleanup_op(free_func, ptr, lop, size);
}
ERTS_GLB_INLINE void
erts_db_free_nt(ErtsAlcType_t type, void *ptr, Uint size)
{
    ASSERT(ptr != 0);
    ASSERT(size == ERTS_ALC_DBG_BLK_SZ(ptr));
    erts_free(type, ptr);
}
#endif
#endif
#endif