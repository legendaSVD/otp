#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "sys.h"
#include "erl_vm.h"
#include "global.h"
#include "index.h"
void index_info(fmtfn_t to, void *arg, IndexTable *t)
{
    hash_info(to, arg, &t->htable);
    erts_print(to, arg, "=index_table:%s\n", t->htable.name);
    erts_print(to, arg, "size: %d\n",	t->size);
    erts_print(to, arg, "limit: %d\n",	t->limit);
    erts_print(to, arg, "entries: %d\n",t->entries);
}
int
index_table_sz(IndexTable *t)
{
  return (sizeof(IndexTable)
          - sizeof(Hash)
          + t->size*sizeof(IndexSlot*)
          +  hash_table_sz(&(t->htable)));
}
IndexTable*
erts_index_init(ErtsAlcType_t type, IndexTable* t, char* name,
		int size, int limit, HashFunctions fun)
{
    Uint base_size = (((Uint)limit+INDEX_PAGE_SIZE-1)/INDEX_PAGE_SIZE)*sizeof(IndexSlot*);
    hash_init(type, &t->htable, name, 3*size/4, fun);
    t->size = 0;
    t->limit = limit;
    t->entries = 0;
    t->type = type;
    t->seg_table = (IndexSlot***) erts_alloc(type, base_size);
    return t;
}
IndexSlot*
index_put_entry(IndexTable* t, void* tmpl)
{
    int ix;
    IndexSlot* p = (IndexSlot*) hash_put(&t->htable, tmpl);
    if (p->index >= 0) {
	return p;
    }
    ix = t->entries;
    if (ix >= t->size) {
	Uint sz;
	if (ix >= t->limit) {
	    erts_exit(ERTS_DUMP_EXIT, "no more index entries in %s (max=%d)\n",
		     t->htable.name, t->limit);
	}
	sz = INDEX_PAGE_SIZE*sizeof(IndexSlot*);
	t->seg_table[ix>>INDEX_PAGE_SHIFT] = erts_alloc(t->type, sz);
	t->size += INDEX_PAGE_SIZE;
    }
    p->index = ix;
    t->seg_table[ix>>INDEX_PAGE_SHIFT][ix&INDEX_PAGE_MASK] = p;
    ERTS_THR_WRITE_MEMORY_BARRIER;
    t->entries++;
    return p;
}
int index_get(IndexTable* t, void* tmpl)
{
    IndexSlot* p = (IndexSlot*) hash_get(&t->htable, tmpl);
    if (p != NULL) {
	return p->index;
    }
    return -1;
}
void index_erase_latest_from(IndexTable* t, Uint from_ix)
{
    if(from_ix < (Uint)t->entries) {
	int ix;
	for (ix = from_ix; ix < t->entries; ix++)  {
	    IndexSlot* obj = t->seg_table[ix>>INDEX_PAGE_SHIFT][ix&INDEX_PAGE_MASK];
	    hash_erase(&t->htable, obj);
	}
	t->entries = from_ix;
    }
    else {
	ASSERT(from_ix == t->entries);
    }
}