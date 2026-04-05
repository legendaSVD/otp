#ifndef _DB_CATREE_H
#define _DB_CATREE_H
struct DbTableCATreeNode;
typedef struct {
    Eterm term;
    struct erl_off_heap_header* oh;
    Uint size;
    Eterm heap[1];
} DbRouteKey;
typedef struct {
    erts_rwmtx_t lock;
    erts_atomic_t lock_statistics;
    bool is_valid;
    TreeDbTerm *root;
    ErtsThrPrgrLaterOp free_item;
    char end_of_struct__;
} DbTableCATreeBaseNode;
typedef struct {
#ifdef ERTS_ENABLE_LOCK_CHECK
    Sint lc_order;
#endif
    ErtsThrPrgrLaterOp free_item;
    erts_mtx_t lock;
    bool is_valid;
    erts_atomic_t left;
    erts_atomic_t right;
    DbRouteKey key;
} DbTableCATreeRouteNode;
typedef struct DbTableCATreeNode {
    bool is_base_node;
    union {
        DbTableCATreeRouteNode route;
        DbTableCATreeBaseNode base;
    } u;
} DbTableCATreeNode;
typedef struct {
    Uint pos;
    Uint size;
    DbTableCATreeNode** array;
} CATreeNodeStack;
typedef struct db_table_catree {
    DbTableCommon common;
    erts_atomic_t root;
    bool deletion;
    Uint nr_of_deleted_items;
    Binary* nr_of_deleted_items_wb;
} DbTableCATree;
typedef struct {
    DbTableCATree* tb;
    Eterm next_route_key;
    DbTableCATreeNode* locked_bnode;
    DbTableCATreeNode* bnode_parent;
    int bnode_level;
    bool read_only;
    DbRouteKey* search_key;
} CATreeRootIterator;
void db_initialize_catree(void);
int db_create_catree(Process *p, DbTable *tbl);
TreeDbTerm** catree_find_root(Eterm key, CATreeRootIterator*);
TreeDbTerm** catree_find_next_from_pb_key_root(Eterm key, CATreeRootIterator*);
TreeDbTerm** catree_find_prev_from_pb_key_root(Eterm key, CATreeRootIterator*);
TreeDbTerm** catree_find_nextprev_root(CATreeRootIterator*, int next, Eterm* keyp);
TreeDbTerm** catree_find_next_root(CATreeRootIterator*, Eterm* keyp);
TreeDbTerm** catree_find_prev_root(CATreeRootIterator*, Eterm* keyp);
TreeDbTerm** catree_find_first_root(CATreeRootIterator*);
TreeDbTerm** catree_find_last_root(CATreeRootIterator*);
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_enable_db_catree_lock_count(DbTableCATree *tb, int enable);
#endif
void db_catree_force_split(DbTableCATree*, int on);
void db_catree_debug_random_split_join(DbTableCATree*, int on);
typedef struct {
    Uint route_nodes;
    Uint base_nodes;
    Uint max_depth;
} DbCATreeStats;
void db_calc_stats_catree(DbTableCATree*, DbCATreeStats*);
void
erts_db_foreach_thr_prgr_offheap_catree(void (*func)(ErlOffHeap *, void *),
                                        void *arg);
#endif