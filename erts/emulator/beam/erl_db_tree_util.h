#ifndef _DB_TREE_UTIL_H
#define _DB_TREE_UTIL_H
#if defined(ARCH_32)
#define STACK_NEED 50
#elif defined(ARCH_64)
#define STACK_NEED 90
#else
#error "Unsported architecture"
#endif
#define PUSH_NODE(Dtt, Tdt)                     \
    ((Dtt)->array[(Dtt)->pos++] = Tdt)
#define POP_NODE(Dtt)			\
     (((Dtt)->pos) ? 			\
      (Dtt)->array[--((Dtt)->pos)] : NULL)
#define TOP_NODE(Dtt)                   \
     ((Dtt->pos) ? 			\
      (Dtt)->array[(Dtt)->pos - 1] : NULL)
#define EMPTY_NODE(Dtt) (TOP_NODE(Dtt) == NULL)
#define DEC_NITEMS(DB)                                                  \
    erts_flxctr_dec(&(DB)->common.counters, ERTS_DB_TABLE_NITEMS_COUNTER_ID)
static ERTS_INLINE void free_term(DbTable *tb, TreeDbTerm* p)
{
    db_free_term(tb, p, offsetof(TreeDbTerm, dbterm));
}
#define DIR_LEFT 0
#define DIR_RIGHT 1
#define DIR_END 2
static ERTS_INLINE Sint cmp_key(DbTableCommon* tb, Eterm key, TreeDbTerm* obj) {
    return CMP(key, GETKEY(tb,obj->dbterm.tpl));
}
int tree_balance_left(TreeDbTerm **this);
int tree_balance_right(TreeDbTerm **this);
int db_first_tree_common(Process *p, DbTable *tbl, TreeDbTerm *root,
                         Eterm *ret, DbTableTree *stack_container,
                         Eterm (*func)(Process *, DbTable *, TreeDbTerm *));
int db_next_tree_common(Process *p, DbTable *tbl,
                        TreeDbTerm *root, Eterm key,
                        Eterm *ret, DbTreeStack* stack,
                        Eterm (*func)(Process *, DbTable *, TreeDbTerm *));
int db_last_tree_common(Process *p, DbTable *tbl, TreeDbTerm *root,
                        Eterm *ret, DbTableTree *stack_container,
                        Eterm (*func)(Process *, DbTable *, TreeDbTerm *));
int db_prev_tree_common(Process *p, DbTable *tbl, TreeDbTerm *root, Eterm key,
                        Eterm *ret, DbTreeStack* stack,
                        Eterm (*func)(Process *, DbTable *, TreeDbTerm *));
int db_put_tree_common(DbTableCommon *tb, TreeDbTerm **root, Eterm obj,
                       bool key_clash_fail, DbTableTree *stack_container);
int db_get_tree_common(Process *p, DbTableCommon *tb, TreeDbTerm *root, Eterm key,
                       Eterm *ret, DbTableTree *stack_container);
int db_get_element_tree_common(Process *p, DbTableCommon *tb, TreeDbTerm *root, Eterm key,
                               int ndex, Eterm *ret, DbTableTree *stack_container);
int db_member_tree_common(DbTableCommon *tb, TreeDbTerm *root, Eterm key, Eterm *ret,
                          DbTableTree *stack_container);
int db_erase_tree_common(DbTable *tbl, TreeDbTerm **root, Eterm key, Eterm *ret,
                         DbTreeStack *stack );
int db_erase_object_tree_common(DbTable *tbl, TreeDbTerm **root, Eterm object,
                                Eterm *ret, DbTableTree *stack_container);
int db_slot_tree_common(Process *p, DbTable *tbl, TreeDbTerm *root,
                        Eterm slot_term, Eterm *ret,
                        DbTableTree *stack_container,
                        CATreeRootIterator*);
int db_select_chunk_tree_common(Process *p, DbTable *tb,
                                Eterm tid, Eterm pattern, Sint chunk_size,
                                int reverse, Eterm *ret,
                                DbTableTree *stack_container,
                                CATreeRootIterator*);
int db_select_tree_common(Process *p, DbTable *tb,
                          Eterm tid, Eterm pattern, int reverse, Eterm *ret,
                          DbTableTree *stack_container,
                          CATreeRootIterator*);
int db_select_delete_tree_common(Process *p, DbTable *tbl,
                                 Eterm tid, Eterm pattern,
                                 Eterm *ret,
                                 DbTreeStack* stack,
                                 CATreeRootIterator* iter);
int db_select_continue_tree_common(Process *p,
                                   DbTableCommon *tb,
                                   Eterm continuation,
                                   Eterm *ret,
                                   DbTableTree *stack_container,
                                   CATreeRootIterator* iter);
int db_select_delete_continue_tree_common(Process *p,
                                          DbTable *tbl,
                                          Eterm continuation,
                                          Eterm *ret,
                                          DbTreeStack* stack,
                                          CATreeRootIterator* iter);
int db_select_count_tree_common(Process *p, DbTable *tb,
                                Eterm tid, Eterm pattern, Eterm *ret,
                                DbTableTree *stack_container,
                                CATreeRootIterator* iter);
int db_select_count_continue_tree_common(Process *p,
                                         DbTable *tb,
                                         Eterm continuation,
                                         Eterm *ret,
                                         DbTableTree *stack_container,
                                         CATreeRootIterator* iter);
int db_select_replace_tree_common(Process *p, DbTable*,
                                  Eterm tid, Eterm pattern, Eterm *ret,
                                  DbTableTree *stack_container,
                                  CATreeRootIterator* iter);
int db_select_replace_continue_tree_common(Process *p,
                                           DbTable*,
                                           Eterm continuation,
                                           Eterm *ret,
                                           DbTableTree *stack_container,
                                           CATreeRootIterator* iter);
int db_take_tree_common(Process *p, DbTable *tbl, TreeDbTerm **root,
                        Eterm key, Eterm *ret,
                        DbTreeStack *stack );
void db_print_tree_common(fmtfn_t to, void *to_arg,
                          int show, TreeDbTerm *root, DbTable *tbl);
void db_foreach_offheap_tree_common(TreeDbTerm *root,
                                    void (*func)(ErlOffHeap *, void *),
                                    void * arg);
bool db_lookup_dbterm_tree_common(Process *p, DbTable *tbl, TreeDbTerm **root,
                                 Eterm key, Eterm obj, DbUpdateHandle* handle,
                                 DbTableTree *stack_container);
void db_finalize_dbterm_tree_common(int cret,
                                    DbUpdateHandle *handle,
                                    TreeDbTerm **root,
                                    DbTableTree *stack_container);
void* db_eterm_to_dbterm_tree_common(bool compress, int keypos, Eterm obj);
void* db_dbterm_list_append_tree_common(void* last_term, void* db_term);
void* db_dbterm_list_remove_first_tree_common(void **list);
int db_put_dbterm_tree_common(DbTableCommon *tb, TreeDbTerm **root, TreeDbTerm *value_to_insert,
                              bool key_clash_fail, DbTableTree *stack_container);
void db_free_dbterm_tree_common(bool compressed, void* obj);
Eterm db_get_dbterm_key_tree_common(DbTable* tb, void* db_term);
Sint cmp_partly_bound(Eterm partly_bound_key, Eterm bound_key);
TreeDbTerm *db_find_tree_node_common(DbTableCommon*, TreeDbTerm *root,
                                     Eterm key);
Eterm db_binary_info_tree_common(Process*, TreeDbTerm*);
Eterm db_copy_key_tree(Process* p, DbTable* tbl, TreeDbTerm* node);
Eterm db_copy_key_and_object_tree(Process* p, DbTable* tbl, TreeDbTerm* node);
#endif