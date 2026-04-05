#ifndef ERL_MONITOR_LINK_H__
#define ERL_MONITOR_LINK_H__
#define ERTS_PROC_SIG_QUEUE_TYPE_ONLY
#include "erl_proc_sig_queue.h"
#undef ERTS_PROC_SIG_QUEUE_TYPE_ONLY
#define ERL_THR_PROGRESS_TSD_TYPE_ONLY
#include "erl_thr_progress.h"
#undef ERL_THR_PROGRESS_TSD_TYPE_ONLY
#include "erl_alloc.h"
#if defined(DEBUG) || 0
#  define ERTS_ML_DEBUG
#else
#  undef ERTS_ML_DEBUG
#endif
#ifdef ERTS_ML_DEBUG
#  define ERTS_ML_ASSERT ERTS_ASSERT
#else
#  define ERTS_ML_ASSERT(E) ((void) 1)
#endif
#define ERTS_MON_TYPE_MAX               ((Uint32) 9)
#define ERTS_MON_TYPE_PROC              ((Uint32) 0)
#define ERTS_MON_TYPE_PORT              ((Uint32) 1)
#define ERTS_MON_TYPE_TIME_OFFSET       ((Uint32) 2)
#define ERTS_MON_TYPE_DIST_PROC         ((Uint32) 3)
#define ERTS_MON_TYPE_DIST_PORT         ((Uint32) 4)
#define ERTS_MON_TYPE_RESOURCE          ((Uint32) 5)
#define ERTS_MON_TYPE_NODE              ((Uint32) 6)
#define ERTS_MON_TYPE_NODES             ((Uint32) 7)
#define ERTS_MON_TYPE_SUSPEND           ((Uint32) 8)
#define ERTS_MON_TYPE_ALIAS             ERTS_MON_TYPE_MAX
#define ERTS_MON_LNK_TYPE_MAX           (ERTS_MON_TYPE_MAX + ((Uint32) 4))
#define ERTS_LNK_TYPE_MAX               ERTS_MON_LNK_TYPE_MAX
#define ERTS_LNK_TYPE_PROC              (ERTS_MON_TYPE_MAX + ((Uint32) 1))
#define ERTS_LNK_TYPE_PORT              (ERTS_MON_TYPE_MAX + ((Uint32) 2))
#define ERTS_LNK_TYPE_DIST_PROC         (ERTS_MON_TYPE_MAX + ((Uint32) 3))
#define ERTS_LNK_TYPE_DIST_PORT         ERTS_LNK_TYPE_MAX
#define ERTS_ML_TYPE_BITS               5
#define ERTS_ML_TYPE_SHIFT              0
#define ERTS_ML_TYPE_MASK \
    (((((Uint32) 1) << ERTS_ML_TYPE_BITS) - 1) << ERTS_ML_TYPE_SHIFT)
#define ERTS_ML_GET_TYPE(MLN) \
    (((MLN)->flags & ERTS_ML_TYPE_MASK) >> ERTS_ML_TYPE_SHIFT)
#define ERTS_ML_SET_TYPE(MLN, T)                                        \
    do {                                                                \
        ASSERT(ERTS_ML_TYPE_MASK >= ((T) << ERTS_ML_TYPE_SHIFT));       \
        (MLN)->flags &= ~ERTS_ML_TYPE_MASK;                             \
        (MLN)->flags |= ((T) << ERTS_ML_TYPE_SHIFT);                    \
    } while (0)
#define ERTS_ML_STATE_ALIAS_BITS        2
#define ERTS_ML_STATE_ALIAS_SHIFT       ERTS_ML_TYPE_BITS
#define ERTS_ML_STATE_ALIAS_MASK \
    ((((Uint32) 1 << ERTS_ML_STATE_ALIAS_BITS) - 1) \
     << ERTS_ML_STATE_ALIAS_SHIFT)
#define ERTS_ML_STATE_ALIAS_NONE \
    (((Uint32) 0) << ERTS_ML_STATE_ALIAS_SHIFT)
#define ERTS_ML_STATE_ALIAS_UNALIAS \
    (((Uint32) 1) << ERTS_ML_STATE_ALIAS_SHIFT)
#define ERTS_ML_STATE_ALIAS_DEMONITOR \
    (((Uint32) 2) << ERTS_ML_STATE_ALIAS_SHIFT)
#define ERTS_ML_STATE_ALIAS_ONCE \
    (((Uint32) 3) << ERTS_ML_STATE_ALIAS_SHIFT)
#define ERTS_ML_F_BASE \
    (ERTS_ML_TYPE_BITS + ERTS_ML_STATE_ALIAS_BITS)
#define ERTS_ML_FLG_TARGET              (((Uint32) 1) << (ERTS_ML_F_BASE + 0))
#define ERTS_ML_FLG_IN_TABLE            (((Uint32) 1) << (ERTS_ML_F_BASE + 1))
#define ERTS_ML_FLG_IN_SUBTABLE         (((Uint32) 1) << (ERTS_ML_F_BASE + 2))
#define ERTS_ML_FLG_NAME                (((Uint32) 1) << (ERTS_ML_F_BASE + 3))
#define ERTS_ML_FLG_EXTENDED            (((Uint32) 1) << (ERTS_ML_F_BASE + 4))
#define ERTS_ML_FLG_SPAWN_PENDING       (((Uint32) 1) << (ERTS_ML_F_BASE + 5))
#define ERTS_ML_FLG_SPAWN_MONITOR       (((Uint32) 1) << (ERTS_ML_F_BASE + 6))
#define ERTS_ML_FLG_SPAWN_LINK          (((Uint32) 1) << (ERTS_ML_F_BASE + 7))
#define ERTS_ML_FLG_SPAWN_ABANDONED     (((Uint32) 1) << (ERTS_ML_F_BASE + 8))
#define ERTS_ML_FLG_SPAWN_NO_SMSG       (((Uint32) 1) << (ERTS_ML_F_BASE + 9))
#define ERTS_ML_FLG_SPAWN_NO_EMSG       (((Uint32) 1) << (ERTS_ML_F_BASE + 10))
#define ERTS_ML_FLG_TAG                 (((Uint32) 1) << (ERTS_ML_F_BASE + 11))
#define ERTS_ML_FLG_PRIO_ML             (((Uint32) 1) << (ERTS_ML_F_BASE + 12))
#define ERTS_ML_FLG_PRIO_ALIAS          (((Uint32) 1) << (ERTS_ML_F_BASE + 13))
#define ERTS_ML_FLG_SPAWN_LINK_PRIO     (((Uint32) 1) << (ERTS_ML_F_BASE + 14))
#define ERTS_ML_FLG_DBG_VISITED         (((Uint32) 1) << (ERTS_ML_F_BASE + 24))
#define ERTS_ML_FLGS_SPAWN              (ERTS_ML_FLG_SPAWN_PENDING      \
                                         | ERTS_ML_FLG_SPAWN_MONITOR    \
                                         | ERTS_ML_FLG_SPAWN_LINK       \
                                         | ERTS_ML_FLG_SPAWN_ABANDONED  \
                                         | ERTS_ML_FLG_SPAWN_NO_SMSG    \
                                         | ERTS_ML_FLG_SPAWN_NO_EMSG    \
                                         | ERTS_ML_FLG_SPAWN_LINK_PRIO)
#define ERTS_ML_FLGS_SAME \
    (ERTS_ML_FLG_EXTENDED|ERTS_ML_FLG_NAME)
typedef struct ErtsMonLnkNode__ ErtsMonLnkNode;
typedef int (*ErtsMonLnkNodeFunc)(ErtsMonLnkNode *, void *, Sint);
typedef struct {
    UWord parent;
    ErtsMonLnkNode *right;
    ErtsMonLnkNode *left;
} ErtsMonLnkTreeNode;
typedef struct {
    ErtsMonLnkNode *next;
    ErtsMonLnkNode *prev;
} ErtsMonLnkListNode;
struct ErtsMonLnkNode__ {
    union {
        ErtsNonMsgSignal signal;
        ErtsMonLnkTreeNode tree;
        ErtsMonLnkListNode list;
    } node;
    union {
        Eterm item;
        void *ptr;
    } other;
    Uint16 offset;
    Uint16 key_offset;
    Uint32 flags;
};
typedef struct ErtsMonLnkDist__ {
    Eterm nodename;
    Uint32 connection_id;
    erts_atomic_t refc;
    erts_mtx_t mtx;
    int alive;
    ErtsMonLnkNode *links;
    ErtsMonLnkNode *monitors;
    ErtsMonLnkNode *orig_name_monitors;
    ErtsMonLnkNode *dist_pend_spawn_exit;
    ErtsThrPrgrLaterOp cleanup_lop;
} ErtsMonLnkDist;
void erts_monitor_link_init(void);
ErtsMonLnkDist *erts_mon_link_dist_create(Eterm nodename);
ERTS_GLB_INLINE void erts_mon_link_dist_inc_refc(ErtsMonLnkDist *mld);
ERTS_GLB_INLINE void erts_mon_link_dist_dec_refc(ErtsMonLnkDist *mld);
ERTS_GLB_INLINE void erts_ml_dl_list_insert__(ErtsMonLnkNode **list,
                                              ErtsMonLnkNode *ml);
ERTS_GLB_INLINE void erts_ml_dl_list_delete__(ErtsMonLnkNode **list,
                                              ErtsMonLnkNode *ml);
ERTS_GLB_INLINE ErtsMonLnkNode *erts_ml_dl_list_first__(ErtsMonLnkNode *list);
ERTS_GLB_INLINE ErtsMonLnkNode *erts_ml_dl_list_last__(ErtsMonLnkNode *list);
void erts_schedule_mon_link_dist_destruction__(ErtsMonLnkDist *mld);
ERTS_GLB_INLINE void *erts_ml_node_to_main_struct__(ErtsMonLnkNode *mln);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE void
erts_mon_link_dist_inc_refc(ErtsMonLnkDist *mld)
{
    ERTS_ML_ASSERT(erts_atomic_read_nob(&mld->refc) > 0);
    erts_atomic_inc_nob(&mld->refc);
}
ERTS_GLB_INLINE void
erts_mon_link_dist_dec_refc(ErtsMonLnkDist *mld)
{
    ERTS_ML_ASSERT(erts_atomic_read_nob(&mld->refc) > 0);
    if (erts_atomic_dec_read_nob(&mld->refc) == 0)
        erts_schedule_mon_link_dist_destruction__(mld);
}
ERTS_GLB_INLINE void *
erts_ml_node_to_main_struct__(ErtsMonLnkNode *mln)
{
    return (void *) (((char *) mln) - ((size_t) mln->offset));
}
ERTS_GLB_INLINE void
erts_ml_dl_list_insert__(ErtsMonLnkNode **list, ErtsMonLnkNode *ml)
{
    ErtsMonLnkNode *first = *list;
    ERTS_ML_ASSERT(!(ml->flags & ERTS_ML_FLG_IN_TABLE));
    if (!first) {
        ml->node.list.next = ml->node.list.prev = ml;
        *list = ml;
    }
    else {
        ERTS_ML_ASSERT(first->node.list.prev->node.list.next == first);
        ERTS_ML_ASSERT(first->node.list.next->node.list.prev == first);
        ml->node.list.next = first;
        ml->node.list.prev = first->node.list.prev;
        first->node.list.prev = ml;
        ml->node.list.prev->node.list.next = ml;
    }
    ml->flags |= ERTS_ML_FLG_IN_TABLE;
}
ERTS_GLB_INLINE void
erts_ml_dl_list_delete__(ErtsMonLnkNode **list, ErtsMonLnkNode *ml)
{
    ERTS_ML_ASSERT(ml->flags & ERTS_ML_FLG_IN_TABLE);
    if (ml->node.list.next == ml) {
        ERTS_ML_ASSERT(ml->node.list.prev == ml);
        ERTS_ML_ASSERT(*list == ml);
        *list = NULL;
    }
    else {
        ERTS_ML_ASSERT(ml->node.list.prev->node.list.next == ml);
        ERTS_ML_ASSERT(ml->node.list.prev != ml);
        ERTS_ML_ASSERT(ml->node.list.next->node.list.prev == ml);
        ERTS_ML_ASSERT(ml->node.list.next != ml);
        if (*list == ml)
            *list = ml->node.list.next;
        ml->node.list.prev->node.list.next = ml->node.list.next;
        ml->node.list.next->node.list.prev = ml->node.list.prev;
    }
    ml->flags &= ~ERTS_ML_FLG_IN_TABLE;
}
ERTS_GLB_INLINE ErtsMonLnkNode *
erts_ml_dl_list_first__(ErtsMonLnkNode *list)
{
    return list;
}
ERTS_GLB_INLINE ErtsMonLnkNode *
erts_ml_dl_list_last__(ErtsMonLnkNode *list)
{
    if (!list)
        return NULL;
    return list->node.list.prev;
}
#endif
typedef struct ErtsMonLnkNode__ ErtsMonitor;
typedef int (*ErtsMonitorFunc)(ErtsMonitor *, void *, Sint);
typedef struct {
    ErtsMonitor origin;
    union {
        ErtsMonitor target;
        Eterm ref_heap[ERTS_MAX_INTERNAL_REF_SIZE];
    } u;
    Eterm ref;
    erts_atomic32_t refc;
} ErtsMonitorData;
typedef struct {
    ErtsMonitorData md;
    Eterm ref_heap[ERTS_MAX_INTERNAL_REF_SIZE];
} ErtsMonitorDataHeap;
typedef struct {
    ErtsMonitorData md;
    Eterm heap[1 + ERTS_MAX_INTERNAL_REF_SIZE];
} ErtsMonitorDataTagHeap;
typedef struct ErtsMonitorDataExtended__ ErtsMonitorDataExtended;
struct ErtsMonitorDataExtended__ {
    ErtsMonitorData md;
    union {
        Eterm name;
        Uint refc;
    } u;
    union {
        struct erl_off_heap_header *ohhp;
        ErtsMonitor *node_monitors;
    } uptr;
    ErtsMonLnkDist *dist;
    Eterm heap[1];
};
typedef struct ErtsMonitorSuspend__ ErtsMonitorSuspend;
struct ErtsMonitorSuspend__ {
    ErtsMonitorData md;
    ErtsMonitorSuspend *next;
    erts_atomic_t state;
};
#define ERTS_MSUSPEND_STATE_FLG_ACTIVE ((erts_aint_t) (((Uint) 1) << (sizeof(Uint)*8 - 1)))
#define ERTS_MSUSPEND_STATE_COUNTER_MASK (~ERTS_MSUSPEND_STATE_FLG_ACTIVE)
ErtsMonitor *erts_monitor_tree_lookup(ErtsMonitor *root, Eterm key);
ErtsMonitor *erts_monotor_tree_lookup_insert(ErtsMonitor **root,
                                             ErtsMonitor *mon);
ErtsMonitor *erts_monitor_tree_lookup_create(ErtsMonitor **root, int *created,
                                             Uint16 type, Eterm origin,
                                             Eterm target);
void erts_monitor_tree_insert(ErtsMonitor **root, ErtsMonitor *mon);
void erts_monitor_tree_replace(ErtsMonitor **root, ErtsMonitor *old,
                               ErtsMonitor *new_);
void erts_monitor_tree_delete(ErtsMonitor **root, ErtsMonitor *mon);
void erts_monitor_tree_foreach(ErtsMonitor *root,
                               ErtsMonitorFunc func,
                               void *arg);
int erts_monitor_tree_foreach_yielding(ErtsMonitor *root,
                                       ErtsMonitorFunc func,
                                       void *arg,
                                       void **vyspp,
                                       Sint reds);
void erts_monitor_tree_foreach_delete(ErtsMonitor **root,
                                      ErtsMonitorFunc func,
                                      void *arg);
int erts_monitor_tree_foreach_delete_yielding(ErtsMonitor **root,
                                              ErtsMonitorFunc func,
                                              void *arg,
                                              void **vyspp,
                                              Sint reds);
ERTS_GLB_INLINE void erts_monitor_list_insert(ErtsMonitor **list, ErtsMonitor *mon);
ERTS_GLB_INLINE void erts_monitor_list_delete(ErtsMonitor **list, ErtsMonitor *mon);
ERTS_GLB_INLINE ErtsMonitor *erts_monitor_list_first(ErtsMonitor *list);
ERTS_GLB_INLINE ErtsMonitor *erts_monitor_list_last(ErtsMonitor *list);
void erts_monitor_list_foreach(ErtsMonitor *list,
                               ErtsMonitorFunc func,
                               void *arg);
int erts_monitor_list_foreach_yielding(ErtsMonitor *list,
                                       ErtsMonitorFunc func,
                                       void *arg,
                                       void **vyspp,
                                       Sint reds);
void erts_monitor_list_foreach_delete(ErtsMonitor **list,
                                      ErtsMonitorFunc func,
                                      void *arg);
int erts_monitor_list_foreach_delete_yielding(ErtsMonitor **list,
                                              ErtsMonitorFunc func,
                                              void *arg,
                                              void **vyspp,
                                              Sint reds);
ErtsMonitorData *erts_monitor_create(Uint16 type, Eterm ref, Eterm origin,
                                     Eterm target, Eterm name, Eterm tag);
ERTS_GLB_INLINE ErtsMonitorData *erts_monitor_to_data(ErtsMonitor *mon);
ERTS_GLB_INLINE int erts_monitor_is_target(ErtsMonitor *mon);
ERTS_GLB_INLINE int erts_monitor_is_origin(ErtsMonitor *mon);
ERTS_GLB_INLINE int erts_monitor_is_in_table(ErtsMonitor *mon);
ERTS_GLB_INLINE void erts_monitor_release(ErtsMonitor *mon);
ERTS_GLB_INLINE void erts_monitor_release_both(ErtsMonitorData *mdp);
ERTS_GLB_INLINE int erts_monitor_dist_insert(ErtsMonitor *mon, ErtsMonLnkDist *dist);
ERTS_GLB_INLINE int erts_monitor_dist_delete(ErtsMonitor *mon);
void
erts_monitor_set_dead_dist(ErtsMonitor *mon, Eterm nodename);
Uint erts_monitor_size(ErtsMonitor *mon);
void erts_monitor_destroy__(ErtsMonitorData *mdp);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE int
erts_monitor_is_target(ErtsMonitor *mon)
{
    return !!(mon->flags & ERTS_ML_FLG_TARGET);
}
ERTS_GLB_INLINE int
erts_monitor_is_origin(ErtsMonitor *mon)
{
    return !(mon->flags & ERTS_ML_FLG_TARGET);
}
ERTS_GLB_INLINE int
erts_monitor_is_in_table(ErtsMonitor *mon)
{
    return !!(mon->flags & ERTS_ML_FLG_IN_TABLE);
}
ERTS_GLB_INLINE void
erts_monitor_list_insert(ErtsMonitor **list, ErtsMonitor *mon)
{
    erts_ml_dl_list_insert__((ErtsMonLnkNode **) list, (ErtsMonLnkNode *) mon);
}
ERTS_GLB_INLINE void
erts_monitor_list_delete(ErtsMonitor **list, ErtsMonitor *mon)
{
    erts_ml_dl_list_delete__((ErtsMonLnkNode **) list, (ErtsMonLnkNode *) mon);
}
ERTS_GLB_INLINE ErtsMonitor *
erts_monitor_list_first(ErtsMonitor *list)
{
    return (ErtsMonitor *) erts_ml_dl_list_first__((ErtsMonLnkNode *) list);
}
ERTS_GLB_INLINE ErtsMonitor *
erts_monitor_list_last(ErtsMonitor *list)
{
    return (ErtsMonitor *) erts_ml_dl_list_last__((ErtsMonLnkNode *) list);
}
#ifdef ERTS_ML_DEBUG
extern size_t erts_monitor_origin_offset;
extern size_t erts_monitor_origin_key_offset;
extern size_t erts_monitor_target_offset;
extern size_t erts_monitor_target_key_offset;
extern size_t erts_monitor_node_key_offset;
#endif
ERTS_GLB_INLINE ErtsMonitorData *
erts_monitor_to_data(ErtsMonitor *mon)
{
    ErtsMonitorData *mdp = (ErtsMonitorData *)erts_ml_node_to_main_struct__((ErtsMonLnkNode *) mon);
#ifdef ERTS_ML_DEBUG
    ERTS_ML_ASSERT(!(mdp->origin.flags & ERTS_ML_FLG_TARGET));
    ERTS_ML_ASSERT(erts_monitor_origin_offset == (size_t) mdp->origin.offset);
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_ALIAS
                   || !!(mdp->u.target.flags & ERTS_ML_FLG_TARGET));
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_ALIAS
                   || erts_monitor_target_offset == (size_t) mdp->u.target.offset);
    if (ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_NODE
        || ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_NODES
        || ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_SUSPEND) {
        ERTS_ML_ASSERT(erts_monitor_node_key_offset == (size_t) mdp->origin.key_offset);
        ERTS_ML_ASSERT(erts_monitor_node_key_offset == (size_t) mdp->u.target.key_offset);
    }
    else {
        ERTS_ML_ASSERT(erts_monitor_origin_key_offset == (size_t) mdp->origin.key_offset);
        ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_ALIAS
                       || erts_monitor_target_key_offset == (size_t) mdp->u.target.key_offset);
    }
#endif
    return mdp;
}
ERTS_GLB_INLINE void
erts_monitor_release(ErtsMonitor *mon)
{
    ErtsMonitorData *mdp = erts_monitor_to_data(mon);
    ERTS_ML_ASSERT(erts_atomic32_read_nob(&mdp->refc) > 0);
    if (erts_atomic32_dec_read_mb(&mdp->refc) == 0) {
        ERTS_ML_ASSERT(!(mdp->origin.flags & ERTS_ML_FLG_IN_TABLE));
        ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_ALIAS
                       || !(mdp->u.target.flags & ERTS_ML_FLG_IN_TABLE));
        erts_monitor_destroy__(mdp);
    }
}
ERTS_GLB_INLINE void
erts_monitor_release_both(ErtsMonitorData *mdp)
{
    ERTS_ML_ASSERT((mdp->origin.flags & ERTS_ML_FLGS_SAME)
                   == (mdp->u.target.flags & ERTS_ML_FLGS_SAME));
    ERTS_ML_ASSERT(erts_atomic32_read_nob(&mdp->refc) >= 2);
    if (erts_atomic32_add_read_mb(&mdp->refc, (erts_aint32_t) -2) == 0) {
        ERTS_ML_ASSERT(!(mdp->origin.flags & ERTS_ML_FLG_IN_TABLE));
        ERTS_ML_ASSERT(!(mdp->u.target.flags & ERTS_ML_FLG_IN_TABLE));
        erts_monitor_destroy__(mdp);
    }
}
ERTS_GLB_INLINE int
erts_monitor_dist_insert(ErtsMonitor *mon, ErtsMonLnkDist *dist)
{
    ErtsMonitorDataExtended *mdep;
    int insert;
    ERTS_ML_ASSERT(mon->flags & ERTS_ML_FLG_EXTENDED);
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_DIST_PROC
                   || ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_NODE);
    mdep = (ErtsMonitorDataExtended *) erts_monitor_to_data(mon);
    ERTS_ML_ASSERT(!mdep->dist);
    ERTS_ML_ASSERT(dist);
    erts_mtx_lock(&dist->mtx);
    insert = dist->alive;
    if (insert) {
        mdep->dist = dist;
        erts_mon_link_dist_inc_refc(dist);
        if ((mon->flags & (ERTS_ML_FLG_NAME
                           | ERTS_ML_FLG_TARGET)) == ERTS_ML_FLG_NAME)
            erts_monitor_tree_insert(&dist->orig_name_monitors, mon);
        else
            erts_monitor_list_insert(&dist->monitors, mon);
    }
    erts_mtx_unlock(&dist->mtx);
    return insert;
}
ERTS_GLB_INLINE int
erts_monitor_dist_delete(ErtsMonitor *mon)
{
    ErtsMonitorDataExtended *mdep;
    ErtsMonLnkDist *dist;
    Uint16 flags;
    int delete_;
    ERTS_ML_ASSERT(mon->flags & ERTS_ML_FLG_EXTENDED);
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_DIST_PROC
                   || ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_NODE);
    mdep = (ErtsMonitorDataExtended *) erts_monitor_to_data(mon);
    dist = mdep->dist;
    ERTS_ML_ASSERT(dist);
    erts_mtx_lock(&dist->mtx);
    flags = mon->flags;
    delete_ = !!dist->alive & !!(flags & ERTS_ML_FLG_IN_TABLE);
    if (delete_) {
        if ((flags & (ERTS_ML_FLG_NAME
                      | ERTS_ML_FLG_TARGET)) == ERTS_ML_FLG_NAME)
            erts_monitor_tree_delete(&dist->orig_name_monitors, mon);
        else
            erts_monitor_list_delete(&dist->monitors, mon);
    }
    erts_mtx_unlock(&dist->mtx);
    return delete_;
}
#endif
ErtsMonitorSuspend *erts_monitor_suspend_create(Eterm pid);
ErtsMonitorSuspend *erts_monitor_suspend_tree_lookup_create(ErtsMonitor **root,
                                                            int *created,
                                                            Eterm pid);
void erts_monitor_suspend_destroy(ErtsMonitorSuspend *msp);
ERTS_GLB_INLINE ErtsMonitorSuspend *erts_monitor_suspend(ErtsMonitor *mon);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE ErtsMonitorSuspend *erts_monitor_suspend(ErtsMonitor *mon)
{
    ERTS_ML_ASSERT(!mon || ERTS_ML_GET_TYPE(mon) == ERTS_MON_TYPE_SUSPEND);
    return (ErtsMonitorSuspend *) mon;
}
#endif
void
erts_debug_monitor_tree_destroying_foreach(ErtsMonitor *root,
                                           ErtsMonitorFunc func,
                                           void *arg,
                                           void *vysp);
void
erts_debug_monitor_list_destroying_foreach(ErtsMonitor *list,
                                           ErtsMonitorFunc func,
                                           void *arg,
                                           void *vysp);
typedef struct ErtsMonLnkNode__ ErtsLink;
typedef int (*ErtsLinkFunc)(ErtsLink *, void *, Sint);
typedef struct {
    ErtsLink link;
    Uint64 unlinking;
} ErtsILink;
typedef struct {
    ErtsLink proc;
    ErtsLink dist;
    erts_atomic32_t refc;
} ErtsLinkData;
typedef struct {
    ErtsLinkData ld;
    struct erl_off_heap_header *ohhp;
    ErtsMonLnkDist *dist;
    Uint64 unlinking;
    Eterm heap[1];
} ErtsELink;
ErtsLink *erts_link_tree_lookup(ErtsLink *root, Eterm item);
ErtsLink *erts_link_tree_lookup_insert(ErtsLink **root, ErtsLink *lnk);
ErtsLink *erts_link_external_tree_lookup_create(ErtsLink **root, int *created,
                                                Uint16 type, Eterm this_, Eterm other);
ErtsLink *erts_link_internal_tree_lookup_create(ErtsLink **root, int *created,
                                                Uint16 type, Eterm other);
void erts_link_tree_insert(ErtsLink **root, ErtsLink *lnk);
void erts_link_tree_replace(ErtsLink **root, ErtsLink *old, ErtsLink *new_);
ERTS_GLB_INLINE ErtsLink *erts_link_tree_insert_addr_replace(ErtsLink **root,
                                                             ErtsLink *lnk);
void erts_link_tree_delete(ErtsLink **root, ErtsLink *lnk);
ERTS_GLB_INLINE ErtsLink *erts_link_tree_key_delete(ErtsLink **root, ErtsLink *lnk);
void erts_link_tree_foreach(ErtsLink *root,
                            ErtsLinkFunc,
                            void *arg);
int erts_link_tree_foreach_yielding(ErtsLink *root,
                                    ErtsLinkFunc func,
                                    void *arg,
                                    void **vyspp,
                                    Sint reds);
void erts_link_tree_foreach_delete(ErtsLink **root,
                                   ErtsLinkFunc func,
                                   void *arg);
int erts_link_tree_foreach_delete_yielding(ErtsLink **root,
                                           ErtsLinkFunc func,
                                           void *arg,
                                           void **vyspp,
                                           Sint reds);
ERTS_GLB_INLINE void erts_link_list_insert(ErtsLink **list, ErtsLink *lnk);
ERTS_GLB_INLINE void erts_link_list_delete(ErtsLink **list, ErtsLink *lnk);
ERTS_GLB_INLINE ErtsLink *erts_link_list_first(ErtsLink *list);
ERTS_GLB_INLINE ErtsLink *erts_link_list_last(ErtsLink *list);
void erts_link_list_foreach(ErtsLink *list,
                            ErtsLinkFunc func,
                            void *arg);
int erts_link_list_foreach_yielding(ErtsLink *list,
                                    ErtsLinkFunc func,
                                    void *arg,
                                    void **vyspp,
                                    Sint reds);
void erts_link_list_foreach_delete(ErtsLink **list,
                                   ErtsLinkFunc func,
                                   void *arg);
int erts_link_list_foreach_delete_yielding(ErtsLink **list,
                                           ErtsLinkFunc func,
                                           void *arg,
                                           void **vyspp,
                                           Sint reds);
ErtsLinkData *erts_link_external_create(Uint16 type, Eterm this_, Eterm other);
ErtsLink *erts_link_internal_create(Uint16 type, Eterm id);
ERTS_GLB_INLINE ErtsELink *erts_link_to_elink(ErtsLink *lnk);
ERTS_GLB_INLINE ErtsLink *erts_link_to_other(ErtsLink *lnk, ErtsELink **elnkpp);
ERTS_GLB_INLINE int erts_link_is_in_table(ErtsLink *lnk);
ERTS_GLB_INLINE void erts_link_internal_release(ErtsLink *lnk);
ERTS_GLB_INLINE void erts_link_release(ErtsLink *lnk);
ERTS_GLB_INLINE void erts_link_release_both(ErtsLinkData *ldp);
ERTS_GLB_INLINE int erts_link_dist_insert(ErtsLink *lnk, ErtsMonLnkDist *dist);
ERTS_GLB_INLINE int erts_link_dist_delete(ErtsLink *lnk);
void
erts_link_set_dead_dist(ErtsLink *lnk, Eterm nodename);
Uint erts_link_size(ErtsLink *lnk);
void erts_link_destroy_elink__(ErtsELink *elnk);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
#ifdef ERTS_ML_DEBUG
extern size_t erts_link_proc_offset;
extern size_t erts_link_dist_offset;
extern size_t erts_link_key_offset;
#endif
ERTS_GLB_INLINE ErtsELink *
erts_link_to_elink(ErtsLink *lnk)
{
    ErtsELink *elnk;
    ERTS_ML_ASSERT(lnk->flags & ERTS_ML_FLG_EXTENDED);
    elnk = (ErtsELink *) erts_ml_node_to_main_struct__((ErtsMonLnkNode *) lnk);
#ifdef ERTS_ML_DEBUG
    ERTS_ML_ASSERT(erts_link_proc_offset == (size_t) elnk->ld.proc.offset);
    ERTS_ML_ASSERT(erts_link_key_offset == (size_t) elnk->ld.proc.key_offset);
    ERTS_ML_ASSERT(erts_link_dist_offset == (size_t) elnk->ld.dist.offset);
    ERTS_ML_ASSERT(erts_link_key_offset == (size_t) elnk->ld.dist.key_offset);
#endif
    return elnk;
}
ERTS_GLB_INLINE ErtsLink *
erts_link_to_other(ErtsLink *lnk, ErtsELink **elnkpp)
{
    ErtsELink *elnk = erts_link_to_elink(lnk);
    if (elnkpp)
        *elnkpp = elnk;
    return lnk == &elnk->ld.proc ? &elnk->ld.dist : &elnk->ld.proc;
}
ERTS_GLB_INLINE int
erts_link_is_in_table(ErtsLink *lnk)
{
    return !!(lnk->flags & ERTS_ML_FLG_IN_TABLE);
}
ERTS_GLB_INLINE void
erts_link_list_insert(ErtsLink **list, ErtsLink *lnk)
{
    erts_ml_dl_list_insert__((ErtsMonLnkNode **) list, (ErtsMonLnkNode *) lnk);
}
ERTS_GLB_INLINE void
erts_link_list_delete(ErtsLink **list, ErtsLink *lnk)
{
    erts_ml_dl_list_delete__((ErtsMonLnkNode **) list, (ErtsMonLnkNode *) lnk);
}
ERTS_GLB_INLINE ErtsLink *
erts_link_list_first(ErtsLink *list)
{
    return (ErtsLink *) erts_ml_dl_list_first__((ErtsMonLnkNode *) list);
}
ERTS_GLB_INLINE ErtsLink *
erts_link_list_last(ErtsLink *list)
{
    return (ErtsLink *) erts_ml_dl_list_last__((ErtsMonLnkNode *) list);
}
ERTS_GLB_INLINE void
erts_link_internal_release(ErtsLink *lnk)
{
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_PROC
                   || ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_PORT);
    ERTS_ML_ASSERT(!(lnk->flags & ERTS_ML_FLG_EXTENDED));
    erts_free(ERTS_ALC_T_LINK, lnk);
}
ERTS_GLB_INLINE void
erts_link_release(ErtsLink *lnk)
{
    if (!(lnk->flags & ERTS_ML_FLG_EXTENDED))
        erts_link_internal_release(lnk);
    else {
        ErtsELink *elnk = erts_link_to_elink(lnk);
        ERTS_ML_ASSERT(!(lnk->flags & ERTS_ML_FLG_IN_TABLE));
        ERTS_ML_ASSERT(erts_atomic32_read_nob(&elnk->ld.refc) > 0);
        if (erts_atomic32_dec_read_nob(&elnk->ld.refc) == 0)
            erts_link_destroy_elink__(elnk);
    }
}
ERTS_GLB_INLINE void
erts_link_release_both(ErtsLinkData *ldp)
{
    ERTS_ML_ASSERT(!(ldp->proc.flags & ERTS_ML_FLG_IN_TABLE));
    ERTS_ML_ASSERT(!(ldp->dist.flags & ERTS_ML_FLG_IN_TABLE));
    ERTS_ML_ASSERT(erts_atomic32_read_nob(&ldp->refc) >= 2);
    ERTS_ML_ASSERT(ldp->proc.flags & ERTS_ML_FLG_EXTENDED);
    ERTS_ML_ASSERT(ldp->dist.flags & ERTS_ML_FLG_EXTENDED);
    if (erts_atomic32_add_read_nob(&ldp->refc, (erts_aint32_t) -2) == 0)
        erts_link_destroy_elink__((ErtsELink *) ldp);
}
ERTS_GLB_INLINE ErtsLink *
erts_link_tree_insert_addr_replace(ErtsLink **root, ErtsLink *lnk)
{
    ErtsLink *lnk2 = erts_link_tree_lookup_insert(root, lnk);
    if (!lnk2)
        return NULL;
    if (lnk2 < lnk)
        return lnk;
    erts_link_tree_replace(root, lnk2, lnk);
    return lnk2;
}
ERTS_GLB_INLINE ErtsLink *
erts_link_tree_key_delete(ErtsLink **root, ErtsLink *lnk)
{
    ErtsLink *dlnk;
    if (erts_link_is_in_table(lnk))
        dlnk = lnk;
    else
        dlnk = erts_link_tree_lookup(*root, lnk->other.item);
    if (dlnk)
        erts_link_tree_delete(root, dlnk);
    return dlnk;
}
ERTS_GLB_INLINE int
erts_link_dist_insert(ErtsLink *lnk, ErtsMonLnkDist *dist)
{
    ErtsELink *elnk;
    int insert;
    ERTS_ML_ASSERT(lnk->flags & ERTS_ML_FLG_EXTENDED);
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_DIST_PROC);
    elnk = erts_link_to_elink(lnk);
    ERTS_ML_ASSERT(!elnk->dist);
    ERTS_ML_ASSERT(dist);
    erts_mtx_lock(&dist->mtx);
    insert = dist->alive;
    if (insert) {
        elnk->dist = dist;
        erts_mon_link_dist_inc_refc(dist);
        erts_link_list_insert(&dist->links, lnk);
    }
    erts_mtx_unlock(&dist->mtx);
    return insert;
}
ERTS_GLB_INLINE int
erts_link_dist_delete(ErtsLink *lnk)
{
    ErtsELink *elnk;
    ErtsMonLnkDist *dist;
    int delete_;
    ERTS_ML_ASSERT(lnk->flags & ERTS_ML_FLG_EXTENDED);
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_DIST_PROC
                   || ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_DIST_PORT);
    elnk = erts_link_to_elink(lnk);
    dist = elnk->dist;
    if (!dist)
        return -1;
    ERTS_ML_ASSERT(ERTS_ML_GET_TYPE(lnk) == ERTS_LNK_TYPE_DIST_PROC);
    erts_mtx_lock(&dist->mtx);
    delete_ = !!dist->alive & !!(lnk->flags & ERTS_ML_FLG_IN_TABLE);
    if (delete_)
        erts_link_list_delete(&dist->links, lnk);
    erts_mtx_unlock(&dist->mtx);
    return delete_;
}
#endif
void
erts_debug_link_tree_destroying_foreach(ErtsLink *root,
                                        ErtsLinkFunc func,
                                        void *arg,
                                        void *vysp);
#endif