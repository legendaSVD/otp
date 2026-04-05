#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "safe_hash.h"
#ifndef ERTS_SYS_CONTINOUS_FD_NUMBERS
static ERTS_INLINE void set_size(SafeHash* h, int size)
{
    ASSERT(size % SAFE_HASH_LOCK_CNT == 0);
    h->size_mask = size - 1;
    ASSERT((size & h->size_mask) == 0);
    h->grow_limit = size;
}
static ERTS_INLINE int align_up_pow2(int val)
{
    int x = val & (val-1);
    if (x==0) return val ? val : 1;
    do {
	val = x;
	x &= x - 1;
    }while (x);
    return val << 1;
}
static void rehash(SafeHash* h, int grow_limit)
{
    if (erts_atomic_xchg_acqb(&h->is_rehashing, 1) != 0) {
	return;
    }
    if (h->grow_limit == grow_limit) {
	int i, size, bytes;
	SafeHashBucket** new_tab;
	SafeHashBucket** old_tab = h->tab;
	int old_size = h->size_mask + 1;
	size = old_size * 2;
	bytes = size * sizeof(SafeHashBucket*);
	new_tab = (SafeHashBucket **) erts_alloc(h->type, bytes);
	sys_memzero(new_tab, bytes);
	for (i=0; i<SAFE_HASH_LOCK_CNT; i++) {
	    erts_mtx_lock(&h->lock_vec[i].mtx);
	}
	h->tab = new_tab;
	set_size(h, size);
	for (i = 0; i < old_size; i++) {
	    SafeHashBucket* b = old_tab[i];
	    while (b != NULL) {
		SafeHashBucket* b_next = b->next;
		int ix = b->hvalue & h->size_mask;
		b->next = new_tab[ix];
		new_tab[ix] = b;
		b = b_next;
	    }
	}
	for (i=0; i<SAFE_HASH_LOCK_CNT; i++) {
	    erts_mtx_unlock(&h->lock_vec[i].mtx);
	}
	erts_free(h->type, (void *) old_tab);
    }
    erts_atomic_set_relb(&h->is_rehashing, 0);
}
void safe_hash_get_info(SafeHashInfo *hi, SafeHash *h)
{
    int size;
    int i, lock_ix;
    int max_depth = 0;
    int objects = 0;
    for (lock_ix=0; lock_ix<SAFE_HASH_LOCK_CNT; lock_ix++) {
	erts_mtx_lock(&h->lock_vec[lock_ix].mtx);
	size = h->size_mask + 1;
	for (i = lock_ix; i < size; i += SAFE_HASH_LOCK_CNT) {
	    int depth = 0;
	    SafeHashBucket* b = h->tab[i];
	    while (b != NULL) {
		objects++;
		depth++;
		b = b->next;
	    }
	    if (depth > max_depth)
		max_depth = depth;
	}
	erts_mtx_unlock(&h->lock_vec[lock_ix].mtx);
    }
    hi->name  = h->name;
    hi->size  = size;
    hi->objs  = objects;
    hi->depth = max_depth;
}
int safe_hash_table_sz(SafeHash *h)
{
  int i, size;
  for(i=0; h->name[i]; i++);
  i++;
  erts_mtx_lock(&h->lock_vec[0].mtx);
  size = h->size_mask + 1;
  erts_mtx_unlock(&h->lock_vec[0].mtx);
  return sizeof(SafeHash) + size*sizeof(SafeHashBucket*) + i;
}
SafeHash* safe_hash_init(ErtsAlcType_t type, SafeHash* h, char* name, erts_lock_flags_t flags,
    int size, SafeHashFunctions fun)
{
    int i, bytes;
    size = align_up_pow2(size);
    bytes = size * sizeof(SafeHashBucket*);
    h->type = type;
    h->tab = (SafeHashBucket**) erts_alloc(h->type, bytes);
    sys_memzero(h->tab, bytes);
    h->name = name;
    h->fun = fun;
    set_size(h,size);
    erts_atomic_init_nob(&h->is_rehashing, 0);
    erts_atomic_init_nob(&h->nitems, 0);
    for (i=0; i<SAFE_HASH_LOCK_CNT; i++) {
        erts_mtx_init(&h->lock_vec[i].mtx, "safe_hash", NIL,
            flags);
    }
    return h;
}
void* safe_hash_get(SafeHash* h, void* tmpl)
{
    SafeHashValue hval = h->fun.hash(tmpl);
    SafeHashBucket* b;
    erts_mtx_t* lock = &h->lock_vec[hval % SAFE_HASH_LOCK_CNT].mtx;
    erts_mtx_lock(lock);
    b = h->tab[hval & h->size_mask];
    while(b != NULL) {
	if ((b->hvalue == hval) && (h->fun.cmp(tmpl, (void*)b) == 0))
	    break;
	b = b->next;
    }
    erts_mtx_unlock(lock);
    return (void*) b;
}
void* safe_hash_put(SafeHash* h, void* tmpl)
{
    int grow_limit;
    SafeHashValue hval = h->fun.hash(tmpl);
    SafeHashBucket* b;
    SafeHashBucket** head;
    erts_mtx_t* lock = &h->lock_vec[hval % SAFE_HASH_LOCK_CNT].mtx;
    erts_mtx_lock(lock);
    head = &h->tab[hval & h->size_mask];
    b = *head;
    while(b != NULL) {
	if ((b->hvalue == hval) && (h->fun.cmp(tmpl, (void*)b) == 0)) {
	    erts_mtx_unlock(lock);
	    return b;
	}
	b = b->next;
    }
    b = (SafeHashBucket*) h->fun.alloc(tmpl);
    b->hvalue = hval;
    b->next = *head;
    *head = b;
    grow_limit = h->grow_limit;
    erts_mtx_unlock(lock);
    if (erts_atomic_inc_read_nob(&h->nitems) > grow_limit) {
	rehash(h, grow_limit);
    }
    return (void*) b;
}
void* safe_hash_erase(SafeHash* h, void* tmpl)
{
    SafeHashValue hval = h->fun.hash(tmpl);
    SafeHashBucket* b;
    SafeHashBucket** prevp;
    erts_mtx_t* lock = &h->lock_vec[hval % SAFE_HASH_LOCK_CNT].mtx;
    erts_mtx_lock(lock);
    prevp = &h->tab[hval & h->size_mask];
    b = *prevp;
    while(b != NULL) {
	if ((b->hvalue == hval) && (h->fun.cmp(tmpl, (void*)b) == 0)) {
	    *prevp = b->next;
	    erts_mtx_unlock(lock);
	    erts_atomic_dec_nob(&h->nitems);
	    h->fun.free((void*)b);
	    return tmpl;
	}
	prevp = &b->next;
	b = b->next;
    }
    erts_mtx_unlock(lock);
    return NULL;
}
void safe_hash_for_each(SafeHash* h, void (*func)(void *, void *, void *),
                        void *func_arg2, void *func_arg3)
{
    int i;
    for (i = 0; i <= h->size_mask; i++) {
	SafeHashBucket* b = h->tab[i];
	while (b != NULL) {
	    (*func)((void *) b, func_arg2, func_arg3);
	    b = b->next;
	}
    }
}
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_enable_hash_lock_count(SafeHash *h, erts_lock_flags_t flags, int enable) {
    int i;
    for(i = 0; i < SAFE_HASH_LOCK_CNT; i++) {
        erts_mtx_t *lock = &h->lock_vec[i].mtx;
        if(enable) {
            erts_lcnt_install_new_lock_info(&lock->lcnt, "safe_hash", NIL,
                ERTS_LOCK_TYPE_MUTEX | flags);
        } else {
            erts_lcnt_uninstall(&lock->lcnt);
        }
    }
}
#endif
#endif