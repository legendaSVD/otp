#ifndef ERL_NODE_TABLES_BASIC__
#define ERL_NODE_TABLES_BASIC__
typedef struct dist_entry_ DistEntry;
typedef struct ErtsDistOutputBuf_ ErtsDistOutputBuf;
typedef struct ErtsDistOutputBufsContainer_ ErtsDistOutputBufsContainer;
void erts_ref_dist_entry(DistEntry *dep);
void erts_deref_dist_entry(DistEntry *dep);
#endif
#if !defined(ERL_NODE_TABLES_BASIC_ONLY) && !defined(ERL_NODE_TABLES_H__)
#define ERL_NODE_TABLES_H__
#include "sys.h"
#include "hash.h"
#include "erl_alloc.h"
#define ERTS_PORT_TASK_ONLY_BASIC_TYPES__
#include "erl_port_task.h"
#undef ERTS_PORT_TASK_ONLY_BASIC_TYPES__
#include "erl_process.h"
#define ERTS_BINARY_TYPES_ONLY__
#include "erl_binary.h"
#undef ERTS_BINARY_TYPES_ONLY__
#include "erl_monitor_link.h"
#define ERTS_NODE_TAB_DELAY_GC_DEFAULT (60)
#define ERTS_NODE_TAB_DELAY_GC_MAX (100*1000*1000)
#define ERTS_NODE_TAB_DELAY_GC_INFINITY (ERTS_NODE_TAB_DELAY_GC_MAX+1)
#define ERST_INTERNAL_CHANNEL_NO 0
enum dist_entry_state {
    ERTS_DE_STATE_IDLE,
    ERTS_DE_STATE_PENDING,
    ERTS_DE_STATE_CONNECTED,
    ERTS_DE_STATE_EXITING
};
#define ERTS_DE_QFLG_BUSY			(((erts_aint32_t) 1) <<  0)
#define ERTS_DE_QFLG_EXIT			(((erts_aint32_t) 1) <<  1)
#define ERTS_DE_QFLGS_ALL			(ERTS_DE_QFLG_BUSY \
						 | ERTS_DE_QFLG_EXIT)
#if defined(ARCH_64)
#define ERTS_DIST_OUTPUT_BUF_DBG_PATTERN ((Uint) 0xf713f713f713f713UL)
#else
#define ERTS_DIST_OUTPUT_BUF_DBG_PATTERN ((Uint) 0xf713f713)
#endif
struct ErtsDistOutputBuf_ {
#ifdef DEBUG
    Uint dbg_pattern;
#endif
    ErtsDistOutputBuf *next;
    Binary *bin;
    ErlIOVec *eiov;
    int ignore_busy;
};
struct ErtsDistOutputBufsContainer_ {
    Sint fragments;
    byte *extp;
    ErtsDistOutputBuf obuf[1];
};
typedef struct {
    ErtsDistOutputBuf *first;
    ErtsDistOutputBuf *last;
} ErtsDistOutputQueue;
struct ErtsProcList_;
struct dist_entry_ {
    HashBucket hash_bucket;
    struct dist_entry_ *next;
    struct dist_entry_ *prev;
    erts_rwmtx_t rwmtx;
    Eterm sysname;
    Uint32 creation;
    erts_atomic_t input_handler;
    Eterm cid;
    Uint32 connection_id;
    enum dist_entry_state state;
    int pending_nodedown;
    Process* suspended_nodeup;
    Uint64 dflags;
    Uint32 opts;
    ErtsMonLnkDist *mld;
    erts_mtx_t qlock;
    erts_atomic32_t qflgs;
    erts_atomic32_t notify;
    erts_atomic_t qsize;
    erts_atomic_t total_qsize;
    erts_atomic64_t in;
    erts_atomic64_t out;
    ErtsDistOutputQueue out_queue;
    struct ErtsProcList_ *suspended;
    ErtsDistOutputQueue tmp_out_queue;
    ErtsDistOutputQueue finalized_out_queue;
    erts_atomic_t dist_cmd_scheduled;
    ErtsPortTaskHandle dist_cmd;
    Uint (*send)(Port *prt, ErtsDistOutputBuf *obuf);
    struct cache* cache;
    ErtsThrPrgrLaterOp later_op;
    struct dist_sequences *sequences;
};
#ifdef ERL_NODE_BOOKKEEP
struct erl_node_bookkeeping {
    Eterm who;
    Eterm term;
    char *file;
    int line;
    enum { ERL_NODE_INC, ERL_NODE_DEC } what;
};
#define ERTS_BOOKKEEP_SIZE (1024)
#endif
typedef struct erl_node_ {
  HashBucket hash_bucket;
  erts_refc_t refc;
  Eterm	sysname;
  Uint32 creation;
  DistEntry *dist_entry;
#ifdef ERL_NODE_BOOKKEEP
  struct erl_node_bookkeeping books[ERTS_BOOKKEEP_SIZE];
  erts_atomic_t slot;
#endif
} ErlNode;
extern Hash erts_dist_table;
extern Hash erts_node_table;
extern erts_rwmtx_t erts_dist_table_rwmtx;
extern erts_rwmtx_t erts_node_table_rwmtx;
extern DistEntry *erts_hidden_dist_entries;
extern DistEntry *erts_visible_dist_entries;
extern DistEntry *erts_pending_dist_entries;
extern DistEntry *erts_not_connected_dist_entries;
extern Sint erts_no_of_hidden_dist_entries;
extern Sint erts_no_of_visible_dist_entries;
extern Sint erts_no_of_pending_dist_entries;
extern Sint erts_no_of_not_connected_dist_entries;
extern DistEntry *erts_this_dist_entry;
extern ErlNode *erts_this_node;
extern char *erts_this_node_sysname;
Uint erts_delayed_node_table_gc(void);
DistEntry *erts_channel_no_to_dist_entry(Uint);
DistEntry *erts_sysname_to_connected_dist_entry(Eterm);
DistEntry *erts_find_or_insert_dist_entry(Eterm);
DistEntry *erts_find_dist_entry(Eterm);
void erts_schedule_delete_dist_entry(DistEntry *);
Uint erts_dist_table_size(void);
void erts_dist_table_info(fmtfn_t, void *);
void erts_set_dist_entry_not_connected(DistEntry *);
void erts_set_dist_entry_pending(DistEntry *);
void erts_set_dist_entry_connected(DistEntry *, Eterm, Uint64);
ErlNode *erts_find_or_insert_node(Eterm, Uint32, Eterm);
ErlNode *erts_find_node(Eterm, Uint32);
void erts_schedule_delete_node(ErlNode *);
void erts_set_this_node(Eterm, Uint32);
Uint erts_node_table_size(void);
void erts_init_node_tables(int);
void erts_node_table_info(fmtfn_t, void *);
void erts_print_node_info(fmtfn_t, void *, Eterm, int*, int*);
Eterm erts_get_node_and_dist_references(struct process *);
#if defined(ERTS_ENABLE_LOCK_CHECK)
int erts_lc_is_de_rwlocked(DistEntry *);
int erts_lc_is_de_rlocked(DistEntry *);
#endif
int erts_dist_entry_destructor(Binary *bin);
DistEntry *erts_dhandle_to_dist_entry(Eterm dhandle, Uint32* connection_id);
#define ERTS_DHANDLE_SIZE (3+ERTS_MAGIC_REF_THING_SIZE)
Eterm erts_build_dhandle(Eterm **hpp, ErlOffHeap*, DistEntry*, Uint32 conn_id);
Eterm erts_make_dhandle(Process *c_p, DistEntry*, Uint32 conn_id);
ERTS_GLB_INLINE void erts_init_node_entry(ErlNode *np, erts_aint_t val);
#ifdef ERL_NODE_BOOKKEEP
#define erts_ref_node_entry(NP, MIN, T) erts_ref_node_entry__((NP), (MIN), (T), __FILE__, __LINE__)
#define erts_deref_node_entry(NP, T) erts_deref_node_entry__((NP), (T), __FILE__, __LINE__)
ERTS_GLB_INLINE erts_aint_t erts_ref_node_entry__(ErlNode *np, int min_val, Eterm term, char *file, int line);
ERTS_GLB_INLINE void erts_deref_node_entry__(ErlNode *np, Eterm term, char *file, int line);
#else
ERTS_GLB_INLINE erts_aint_t erts_ref_node_entry(ErlNode *np, int min_val, Eterm term);
ERTS_GLB_INLINE void erts_deref_node_entry(ErlNode *np, Eterm term);
#endif
ERTS_GLB_INLINE erts_aint_t erts_node_refc(ErlNode *np);
ERTS_GLB_INLINE void erts_de_rlock(DistEntry *dep);
ERTS_GLB_INLINE void erts_de_runlock(DistEntry *dep);
ERTS_GLB_INLINE void erts_de_rwlock(DistEntry *dep);
ERTS_GLB_INLINE void erts_de_rwunlock(DistEntry *dep);
#ifdef ERL_NODE_BOOKKEEP
void erts_node_bookkeep(ErlNode *, Eterm , int, char *file, int line);
#else
#define erts_node_bookkeep(...)
#endif
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void
erts_init_node_entry(ErlNode *np, erts_aint_t val)
{
    erts_refc_init(&np->refc, val);
}
#ifdef ERL_NODE_BOOKKEEP
ERTS_GLB_INLINE erts_aint_t
erts_ref_node_entry__(ErlNode *np, int min_val, Eterm term, char *file, int line)
{
    erts_node_bookkeep(np, term, ERL_NODE_INC, file, line);
    return erts_refc_inctest(&np->refc, min_val);
}
ERTS_GLB_INLINE void
erts_deref_node_entry__(ErlNode *np, Eterm term, char *file, int line)
{
    erts_node_bookkeep(np, term, ERL_NODE_DEC, file, line);
    if (erts_refc_dectest(&np->refc, 0) == 0)
	erts_schedule_delete_node(np);
}
#else
ERTS_GLB_INLINE erts_aint_t
erts_ref_node_entry(ErlNode *np, int min_val, Eterm term)
{
    return erts_refc_inctest(&np->refc, min_val);
}
ERTS_GLB_INLINE void
erts_deref_node_entry(ErlNode *np, Eterm term)
{
    if (erts_refc_dectest(&np->refc, 0) == 0)
	erts_schedule_delete_node(np);
}
ERTS_GLB_INLINE erts_aint_t
erts_node_refc(ErlNode *np)
{
    return erts_refc_read(&np->refc, 0);
}
#endif
ERTS_GLB_INLINE void
erts_de_rlock(DistEntry *dep)
{
    erts_rwmtx_rlock(&dep->rwmtx);
}
ERTS_GLB_INLINE void
erts_de_runlock(DistEntry *dep)
{
    erts_rwmtx_runlock(&dep->rwmtx);
}
ERTS_GLB_INLINE void
erts_de_rwlock(DistEntry *dep)
{
    erts_rwmtx_rwlock(&dep->rwmtx);
}
ERTS_GLB_INLINE void
erts_de_rwunlock(DistEntry *dep)
{
    erts_rwmtx_rwunlock(&dep->rwmtx);
}
#endif
void erts_debug_test_node_tab_delayed_delete(Sint64 millisecs);
void erts_lcnt_update_distribution_locks(int enable);
#endif