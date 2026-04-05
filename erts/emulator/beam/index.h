#ifndef __INDEX_H__
#define __INDEX_H__
#include "hash.h"
#include "erl_alloc.h"
typedef struct index_slot
{
    HashBucket bucket;
    int index;
} IndexSlot;
typedef struct index_table
{
    Hash htable;
    ErtsAlcType_t type;
    int size;
    int limit;
    int entries;
    IndexSlot*** seg_table;
} IndexTable;
#define INDEX_PAGE_SHIFT 10
#define INDEX_PAGE_SIZE (1 << INDEX_PAGE_SHIFT)
#define INDEX_PAGE_MASK ((1 << INDEX_PAGE_SHIFT)-1)
IndexTable *erts_index_init(ErtsAlcType_t,IndexTable*,char*,int,int,HashFunctions);
void index_info(fmtfn_t, void *, IndexTable*);
int index_table_sz(IndexTable *);
int index_get(IndexTable*, void*);
IndexSlot* index_put_entry(IndexTable*, void*);
void index_erase_latest_from(IndexTable*, Uint ix);
ERTS_GLB_INLINE int index_put(IndexTable*, void*);
ERTS_GLB_INLINE IndexSlot* erts_index_lookup(IndexTable*, Uint);
ERTS_GLB_INLINE int erts_index_num_entries(IndexTable* t);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE int index_put(IndexTable* t, void* tmpl)
{
    return index_put_entry(t, tmpl)->index;
}
ERTS_GLB_INLINE IndexSlot*
erts_index_lookup(IndexTable* t, Uint ix)
{
    return t->seg_table[ix>>INDEX_PAGE_SHIFT][ix&INDEX_PAGE_MASK];
}
ERTS_GLB_INLINE int erts_index_num_entries(IndexTable* t)
{
    int ret = t->entries;
    ERTS_THR_READ_MEMORY_BARRIER;
    return ret;
}
#endif
#endif