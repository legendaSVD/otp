#ifndef _DB_TREE_H
#define _DB_TREE_H
#include "erl_db_util.h"
typedef struct tree_db_term {
    struct  tree_db_term *left, *right;
    int  balance;
    DbTerm dbterm;
} TreeDbTerm;
typedef struct {
    Uint pos;
    Uint slot;
    TreeDbTerm** array;
} DbTreeStack;
typedef struct db_table_tree {
    DbTableCommon common;
    TreeDbTerm *root;
    Uint deletion;
    erts_atomic_t is_stack_busy;
    DbTreeStack static_stack;
} DbTableTree;
void db_initialize_tree(void);
int db_create_tree(Process *p, DbTable *tbl);
void
erts_db_foreach_thr_prgr_offheap_tree(void (*func)(ErlOffHeap *, void *),
                                      void *arg);
#endif