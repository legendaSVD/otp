#ifndef _DB_UTIL_H
#define _DB_UTIL_H
#include <stdbool.h>
#include "erl_flxctr.h"
#include "global.h"
#include "erl_message.h"
#include "erl_bif_unique.h"
#ifdef DEBUG
#define DMC_DEBUG 1
#define ETS_DBG_FORCE_TRAP 1
#endif
#define DB_ERROR_NONE_FALSE 1
#define DB_ERROR_NONE       0
#define DB_ERROR_BADITEM   -1
#define DB_ERROR_BADTABLE  -2
#define DB_ERROR_SYSRES    -3
#define DB_ERROR_BADKEY    -4
#define DB_ERROR_BADPARAM  -5
#define DB_ERROR_UNSPEC    -10
typedef struct db_term {
    struct erl_off_heap_header* first_oh;
    Uint size;
#ifdef DEBUG_CLONE
    Eterm* debug_clone;
#endif
    Eterm tpl[];
} DbTerm;
#define ERTS_SIZEOF_DBTERM(WORDS) (offsetof(DbTerm,tpl) + sizeof(Eterm)*(WORDS))
#define DB_MUST_RESIZE 1
#define DB_NEW_OBJECT 2
#define DB_INC_TRY_GROW 4
typedef struct {
    DbTable* tb;
    DbTerm* dbterm;
    void** bp;
    Uint new_size;
    int flags;
    union {
        struct {
            struct DbTableHashLockAndCounter* lck_ctr;
        } hash;
        struct {
            struct DbTableCATreeNode* base_node;
            struct DbTableCATreeNode* parent;
            int current_level;
        } catree;
    } u;
    Eterm* old_tpl;
#ifdef DEBUG
    Eterm old_tpl_dflt[2];
#else
    Eterm old_tpl_dflt[8];
#endif
} DbUpdateHandle;
enum DbIterSafety {
    ITER_UNSAFE,
    ITER_SAFE_LOCKED,
    ITER_SAFE
};
typedef struct db_table_method
{
    int (*db_create)(Process *p, DbTable* tb);
    int (*db_first)(Process* p,
		    DbTable* tb,
		    Eterm* ret   );
    int (*db_next)(Process* p,
		   DbTable* tb,
		   Eterm key,
		   Eterm* ret );
    int (*db_last)(Process* p,
		   DbTable* tb,
		   Eterm* ret   );
    int (*db_prev)(Process* p,
		   DbTable* tb,
		   Eterm key,
		   Eterm* ret);
    int (*db_put)(DbTable* tb,
		  Eterm obj,
                  bool key_clash_fail,
                  SWord *consumed_reds_p);
    int (*db_get)(Process* p,
		  DbTable* tb,
		  Eterm key,
		  Eterm* ret);
    int (*db_get_element)(Process* p,
			  DbTable* tb,
			  Eterm key,
			  int index,
			  Eterm* ret);
    int (*db_member)(DbTable* tb,
		     Eterm key,
		     Eterm* ret);
    int (*db_erase)(DbTable* tb,
		    Eterm key,
		    Eterm* ret);
    int (*db_erase_object)(DbTable* tb,
			   Eterm obj,
			   Eterm* ret);
    int (*db_slot)(Process* p,
		   DbTable* tb,
		   Eterm slot,
		   Eterm* ret);
    int (*db_select_chunk)(Process* p,
			   DbTable* tb,
                           Eterm tid,
			   Eterm pattern,
			   Sint chunk_size,
			   int reverse,
			   Eterm* ret,
                           enum DbIterSafety);
    int (*db_select)(Process* p,
		     DbTable* tb,
                     Eterm tid,
		     Eterm pattern,
		     int reverse,
		     Eterm* ret,
                     enum DbIterSafety);
    int (*db_select_delete)(Process* p,
			    DbTable* tb,
                            Eterm tid,
			    Eterm pattern,
			    Eterm* ret,
                            enum DbIterSafety);
    int (*db_select_continue)(Process* p,
			      DbTable* tb,
			      Eterm continuation,
			      Eterm* ret,
                              enum DbIterSafety*);
    int (*db_select_delete_continue)(Process* p,
				     DbTable* tb,
				     Eterm continuation,
				     Eterm* ret,
                                     enum DbIterSafety*);
    int (*db_select_count)(Process* p,
			   DbTable* tb,
                           Eterm tid,
			   Eterm pattern,
			   Eterm* ret,
                           enum DbIterSafety);
    int (*db_select_count_continue)(Process* p,
				    DbTable* tb,
				    Eterm continuation,
				    Eterm* ret,
                                    enum DbIterSafety*);
    int (*db_select_replace)(Process* p,
            DbTable* tb,
            Eterm tid,
            Eterm pattern,
            Eterm* ret,
            enum DbIterSafety);
    int (*db_select_replace_continue)(Process* p,
            DbTable* tb,
            Eterm continuation,
            Eterm* ret,
            enum DbIterSafety*);
    int (*db_take)(Process *, DbTable *, Eterm, Eterm *);
    SWord (*db_delete_all_objects)(Process* p,
                                   DbTable* db,
                                   SWord reds,
                                   Eterm* nitems_holder_wb);
    Eterm (*db_delete_all_objects_get_nitems_from_holder)(Process* p,
                                                          Eterm nitems_holder);
    int (*db_free_empty_table)(DbTable* db);
    SWord (*db_free_table_continue)(DbTable* db, SWord reds);
    void (*db_print)(fmtfn_t to,
		     void* to_arg,
                     bool show,
		     DbTable* tb  );
    void (*db_foreach_offheap)(DbTable* db,
			       void (*func)(ErlOffHeap *, void *),
			       void *arg);
    bool (*db_lookup_dbterm)(Process *, DbTable *, Eterm key, Eterm obj,
                            DbUpdateHandle* handle);
    void (*db_finalize_dbterm)(int cret, DbUpdateHandle* handle);
    void* (*db_eterm_to_dbterm)(bool compress, int keypos, Eterm obj);
    void* (*db_dbterm_list_append)(void* last_term, void* db_term);
    void* (*db_dbterm_list_remove_first)(void** list);
    int (*db_put_dbterm)(DbTable* tb,
                         void* obj,
                         bool key_clash_fail,
                         SWord *consumed_reds_p);
    void (*db_free_dbterm)(bool compressed, void* obj);
    Eterm (*db_get_dbterm_key)(DbTable* tb, void* db_term);
    int (*db_get_binary_info)(Process*, DbTable* tb, Eterm key, Eterm* ret);
    int (*db_raw_first)(Process*, DbTable*, Eterm* ret);
    int (*db_raw_next)(Process*, DbTable*, Eterm key, Eterm* ret);
    int (*db_first_lookup)(Process* p,
		    DbTable* tb,
		    Eterm* ret   );
    int (*db_next_lookup)(Process* p,
		   DbTable* tb,
		   Eterm key,
		   Eterm* ret );
    int (*db_last_lookup)(Process* p,
		   DbTable* tb,
		   Eterm* ret   );
    int (*db_prev_lookup)(Process* p,
		   DbTable* tb,
		   Eterm key,
		   Eterm* ret);
} DbTableMethod;
typedef struct db_fixation {
    struct {
        struct db_fixation *next, *prev;
        Binary* btid;
    } tabs;
    struct {
        struct db_fixation *left, *right, *parent;
        bool is_red;
        Process* p;
    } procs;
    Uint counter;
} DbFixation;
typedef struct {
    DbTable *next;
    DbTable *prev;
} DbTableList;
#define ERTS_DB_TABLE_NITEMS_COUNTER_ID 0
#define ERTS_DB_TABLE_MEM_COUNTER_ID 1
typedef struct db_table_common {
    erts_refc_t refc;
    erts_refc_t fix_count;
    DbTableList all;
    DbTableList owned;
    erts_rwmtx_t rwlock;
    erts_mtx_t fixlock;
    bool is_thread_safe;
    Uint32 type;
    Eterm owner;
    Eterm heir;
    Eterm heir_data;
    Uint64 heir_started_interval;
    Eterm the_name;
    Binary *btid;
    DbTableMethod* meth;
    ErtsFlxCtr counters;
    char extra_for_flxctr[ERTS_FLXCTR_NR_OF_EXTRA_BYTES(2)];
    struct {
	ErtsMonotonicTime monotonic;
	ErtsMonotonicTime offset;
    } time;
    DbFixation* fixing_procs;
    Uint32 status;
    int keypos;
    bool compress;
    struct ets_insert_2_list_info* continuation_ctx;
#ifdef ETS_DBG_FORCE_TRAP
    int dbg_force_trap;
#endif
} DbTableCommon;
#define DB_PRIVATE        (1 << 0)
#define DB_PROTECTED      (1 << 1)
#define DB_PUBLIC         (1 << 2)
#define DB_DELETE         (1 << 3)
#define DB_SET            (1 << 4)
#define DB_BAG            (1 << 5)
#define DB_DUPLICATE_BAG  (1 << 6)
#define DB_ORDERED_SET    (1 << 7)
#define DB_CA_ORDERED_SET (1 << 8)
#define DB_FINE_LOCKED    (1 << 9)
#define DB_FREQ_READ      (1 << 10)
#define DB_NAMED_TABLE    (1 << 11)
#define DB_BUSY           (1 << 12)
#define DB_EXPLICIT_LOCK_GRANULARITY  (1 << 13)
#define DB_FINE_LOCKED_AUTO (1 << 14)
#define DB_CATREE_FORCE_SPLIT (1 << 31)
#define DB_CATREE_DEBUG_RANDOM_SPLIT_JOIN (1 << 30)
#define IS_HASH_TABLE(Status) (!!((Status) &                       \
                                  (DB_BAG | DB_SET | DB_DUPLICATE_BAG)))
#define IS_HASH_WITH_AUTO_TABLE(Status) \
    (((Status) &                                                        \
      (DB_ORDERED_SET | DB_CA_ORDERED_SET | DB_FINE_LOCKED_AUTO)) == DB_FINE_LOCKED_AUTO)
#define IS_TREE_TABLE(Status) (!!((Status) & \
				  DB_ORDERED_SET))
#define IS_CATREE_TABLE(Status) (!!((Status) & \
                                    DB_CA_ORDERED_SET))
#define NFIXED(T) (erts_refc_read(&(T)->common.fix_count,0))
#define IS_FIXED(T) (NFIXED(T) != 0)
#define META_DB_LOCK_FREE() (erts_no_schedulers == 1)
#define DB_LOCK_FREE(T) META_DB_LOCK_FREE()
#define GETKEY(dth, tplp)   (*((tplp) + ((DbTableCommon*)(dth))->keypos))
ERTS_GLB_INLINE Eterm db_copy_key(Process* p, DbTable* tb, DbTerm* obj);
Eterm db_copy_from_comp(DbTableCommon* tb, DbTerm* bp, Eterm** hpp,
			ErlOffHeap* off_heap);
bool db_eq_comp(DbTableCommon* tb, Eterm a, DbTerm* b);
DbTerm* db_alloc_tmp_uncompressed(DbTableCommon* tb, DbTerm* org);
void db_free_tmp_uncompressed(DbTerm* obj);
ERTS_GLB_INLINE Eterm db_copy_object_from_ets(DbTableCommon* tb, DbTerm* bp,
					      Eterm** hpp, ErlOffHeap* off_heap);
ERTS_GLB_INLINE bool db_eq(DbTableCommon* tb, Eterm a, DbTerm* b);
Eterm db_do_read_element(DbUpdateHandle* handle, Sint position);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE Eterm db_copy_key(Process* p, DbTable* tb, DbTerm* obj)
{
    Eterm key = GETKEY(tb, obj->tpl);
    if is_immed(key) return key;
    else {
	Uint size = size_object(key);
	Eterm* hp = HAlloc(p, size);
	Eterm res = copy_struct(key, size, &hp, &MSO(p));
	ASSERT(EQ(res,key));
	return res;
    }
}
ERTS_GLB_INLINE Eterm db_copy_object_from_ets(DbTableCommon* tb, DbTerm* bp,
					      Eterm** hpp, ErlOffHeap* off_heap)
{
    if (tb->compress) {
        return db_copy_from_comp(tb, bp, hpp, off_heap);
    }
    else {
        return make_tuple(copy_shallow(bp->tpl, bp->size, hpp, off_heap));
    }
}
ERTS_GLB_INLINE bool db_eq(DbTableCommon* tb, Eterm a, DbTerm* b)
{
    if (!tb->compress) {
	return EQ(a, make_tuple(b->tpl));
    }
    else {
	return db_eq_comp(tb, a, b);
    }
}
#endif
#define DB_READ  (DB_PROTECTED|DB_PUBLIC)
#define DB_WRITE DB_PUBLIC
#define DB_INFO  (DB_PROTECTED|DB_PUBLIC|DB_PRIVATE)
#define DB_READ_TBL_STRUCT (DB_PROTECTED|DB_PUBLIC|DB_PRIVATE|DB_BUSY)
#define ONLY_WRITER(P,T) (((T)->common.status & (DB_PRIVATE|DB_PROTECTED)) \
			  && (T)->common.owner == (P)->common.id)
#define ONLY_READER(P,T) (((T)->common.status & DB_PRIVATE) && \
(T)->common.owner == (P)->common.id)
BIF_RETTYPE db_get_trace_control_word(Process* p);
BIF_RETTYPE db_set_trace_control_word(Process* p, Eterm tcw);
BIF_RETTYPE db_get_trace_control_word_0(BIF_ALIST_0);
BIF_RETTYPE db_set_trace_control_word_1(BIF_ALIST_1);
void db_initialize_util(void);
Eterm db_getkey(int keypos, Eterm obj);
void db_cleanup_offheap_comp(DbTerm* p);
void db_free_term(DbTable *tb, void* basep, Uint offset);
void db_free_term_no_tab(bool compress, void* basep, Uint offset);
Uint db_term_size(DbTable *tb, void* basep, Uint offset);
void* db_store_term(DbTableCommon *tb, DbTerm* old, Uint offset, Eterm obj);
void* db_store_term_comp(DbTableCommon *tb,
                         int keypos,
                         DbTerm* old,
                         Uint offset,Eterm obj);
Eterm db_copy_element_from_ets(DbTableCommon* tb, Process* p, DbTerm* obj,
			       Uint pos, Eterm** hpp, Uint extra);
int db_has_map(Eterm obj);
bool db_is_fully_bound(Eterm obj);
int db_is_variable(Eterm obj);
void db_do_update_element(DbUpdateHandle* handle,
			  Sint position,
			  Eterm newval);
void db_finalize_resize(DbUpdateHandle* handle, Uint offset);
Eterm db_add_counter(Eterm** hpp, Eterm counter, Eterm incr);
Binary *db_match_set_compile(Process *p, Eterm matchexpr,
			     Uint flags, Uint *freasonp);
bool db_match_keeps_key(int keypos, Eterm match, Eterm guard, Eterm body);
int erts_db_match_prog_destructor(Binary *);
typedef struct match_prog {
    ErlHeapFragment *term_save;
    int num_bindings;
    struct erl_heap_fragment *saved_program_buf;
    Eterm saved_program;
    Uint heap_size;
    Uint stack_offset;
    struct ErtsTraceSession* trace_session;
#ifdef DMC_DEBUG
    UWord* prog_end;
#endif
    UWord text[1];
} MatchProg;
#define DMC_ERR_STR_LEN 100
typedef enum { dmcWarning, dmcError} DMCErrorSeverity;
typedef struct dmc_error {
    char error_string[DMC_ERR_STR_LEN + 1];
    int variable;
    struct dmc_error *next;
    DMCErrorSeverity severity;
} DMCError;
typedef struct dmc_err_info {
    unsigned int *var_trans;
    int num_trans;
    int error_added;
    DMCError *first;
} DMCErrInfo;
#define DCOMP_TABLE ((Uint) 1)
#define DCOMP_TRACE ((Uint) 4)
#define DCOMP_DIALECT_MASK ((Uint) 0x7)
#define DCOMP_FAKE_DESTRUCTIVE ((Uint) 8)
#define DCOMP_ALLOW_TRACE_OPS ((Uint) 0x10)
#define DCOMP_CALL_TRACE ((Uint) 0x20)
Binary *db_match_compile(Eterm *matchexpr, Eterm *guards,
			 Eterm *body, int num_matches,
			 Uint flags,
			 DMCErrInfo *err_info,
                         Uint *freasonp,
                         const bool *is_prefix);
Eterm db_match_dbterm_uncompressed(DbTableCommon* tb, Process* c_p, Binary* bprog,
                                   DbTerm* obj, enum erts_pam_run_flags);
Eterm db_match_dbterm(DbTableCommon* tb, Process* c_p, Binary* bprog,
                      DbTerm *obj, enum erts_pam_run_flags);
Eterm db_prog_match(Process *p, Process *self,
                    Binary *prog, Eterm term,
		    Eterm *termp, int arity,
		    enum erts_pam_run_flags in_flags,
		    Uint32 *return_flags );
DMCErrInfo *db_new_dmc_err_info(void);
Eterm db_format_dmc_err_info(Process *p, DMCErrInfo *ei);
void db_free_dmc_err_info(DMCErrInfo *ei);
ERTS_GLB_INLINE Eterm erts_db_make_match_prog_ref(Process *p, Binary *mp, Eterm **hpp);
ERTS_GLB_INLINE Binary *erts_db_get_match_prog_binary(Eterm term);
ERTS_GLB_INLINE Binary *erts_db_get_match_prog_binary_unchecked(Eterm term);
union erts_tmp_aligned_offheap
{
    BinRef proc_bin;
    ErtsMRefThing mref_thing;
};
ERTS_GLB_INLINE void erts_align_offheap(union erl_off_heap_ptr*,
                                        union erts_tmp_aligned_offheap* tmp);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE Eterm erts_db_make_match_prog_ref(Process *p, Binary *mp, Eterm **hpp)
{
    return erts_mk_magic_ref(hpp, &MSO(p), mp);
}
ERTS_GLB_INLINE Binary *
erts_db_get_match_prog_binary_unchecked(Eterm term)
{
    Binary *bp = erts_magic_ref2bin(term);
    ASSERT(bp->intern.flags & BIN_FLAG_MAGIC);
    ASSERT((ERTS_MAGIC_BIN_DESTRUCTOR(bp) == erts_db_match_prog_destructor));
    return bp;
}
ERTS_GLB_INLINE Binary *
erts_db_get_match_prog_binary(Eterm term)
{
    Binary *bp;
    if (!is_internal_magic_ref(term))
	return NULL;
    bp = erts_magic_ref2bin(term);
    ASSERT(bp->intern.flags & BIN_FLAG_MAGIC);
    if (ERTS_MAGIC_BIN_DESTRUCTOR(bp) != erts_db_match_prog_destructor)
	return NULL;
    return bp;
}
ERTS_GLB_INLINE void
erts_align_offheap(union erl_off_heap_ptr* ohp,
                   union erts_tmp_aligned_offheap* tmp)
{
    if ((UWord)ohp->voidp % sizeof(UWord) != 0) {
        sys_memcpy(tmp, ohp->voidp, sizeof(Eterm));
        if (tmp->proc_bin.thing_word == HEADER_BIN_REF) {
            sys_memcpy(tmp, ohp->voidp, sizeof(tmp->proc_bin));
            ohp->br = &tmp->proc_bin;
        }
        else {
            sys_memcpy(tmp, ohp->voidp, sizeof(tmp->mref_thing));
            ASSERT(is_magic_ref_thing(&tmp->mref_thing));
            ohp->mref = &tmp->mref_thing;
        }
    }
}
#endif
#define IsMatchProgBinary(BP) \
  (((BP)->intern.flags & BIN_FLAG_MAGIC) \
   && ERTS_MAGIC_BIN_DESTRUCTOR((BP)) == erts_db_match_prog_destructor)
#define Binary2MatchProg(BP) \
  (ASSERT(IsMatchProgBinary((BP))), \
   ((MatchProg *) ERTS_MAGIC_BIN_DATA((BP))))
#endif