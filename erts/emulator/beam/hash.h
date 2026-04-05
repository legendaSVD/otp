#ifndef __HASH_H__
#define __HASH_H__
#include "sys.h"
typedef UWord HashValue;
typedef struct hash Hash;
typedef int (*HCMP_FUN)(void*, void*);
typedef HashValue (*H_FUN)(void*);
typedef void* (*HALLOC_FUN)(void*);
typedef void (*HFREE_FUN)(void*);
typedef void* (*HMALLOC_FUN)(int,size_t);
typedef void (*HMFREE_FUN)(int,void*);
typedef int (*HMPRINT_FUN)(fmtfn_t,void*,const char*, ...);
typedef void (*HFOREACH_FUN)(void *, void *);
typedef struct hash_bucket
{
    struct hash_bucket* next;
    HashValue hvalue;
} HashBucket;
typedef struct hash_functions
{
    H_FUN hash;
    HCMP_FUN cmp;
    HALLOC_FUN alloc;
    HFREE_FUN free;
    HMALLOC_FUN meta_alloc;
    HMFREE_FUN meta_free;
    HMPRINT_FUN meta_print;
} HashFunctions;
typedef struct {
  char *name;
  int   size;
  int   used;
  int   objs;
  int   depth;
} HashInfo;
struct hash
{
    HashFunctions fun;
    int is_allocated;
    int meta_alloc_type;
    char* name;
    int shift;
    int max_shift;
    int shrink_threshold;
    int grow_threshold;
    int nobjs;
    HashBucket** bucket;
};
Hash* hash_new(int, char*, int, HashFunctions);
Hash* hash_init(int, Hash*, char*, int, HashFunctions);
void  hash_delete(Hash*);
void  hash_get_info(HashInfo*, Hash*);
void  hash_info(fmtfn_t, void *, Hash*);
int   hash_table_sz(Hash *);
void* hash_get(Hash*, void*);
void* hash_put(Hash*, void*);
void* hash_erase(Hash*, void*);
void* hash_remove(Hash*, void*);
void  hash_foreach(Hash*, HFOREACH_FUN, void *);
ERTS_GLB_INLINE Uint hash_get_slot(Hash *h, HashValue hv);
ERTS_GLB_INLINE void* hash_fetch(Hash *, void*, H_FUN, HCMP_FUN);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE Uint
hash_get_slot(Hash *h, HashValue hv)
{
    hv ^= hv >> h->shift;
#ifdef ARCH_64
    return (UWORD_CONSTANT(11400714819323198485) * hv) >> h->shift;
#else
    return (UWORD_CONSTANT(2654435769) * hv) >> h->shift;
#endif
}
ERTS_GLB_INLINE void* hash_fetch(Hash *h, void* tmpl, H_FUN hash, HCMP_FUN cmp)
{
    HashValue hval = hash(tmpl);
    Uint ix = hash_get_slot(h, hval);
    HashBucket* b = h->bucket[ix];
    ASSERT(h->fun.hash == hash);
    ASSERT(h->fun.cmp == cmp);
    while(b != (HashBucket*) 0) {
	if ((b->hvalue == hval) && (cmp(tmpl, (void*)b) == 0))
	    return (void*) b;
	b = b->next;
    }
    return (void*) 0;
}
#endif
#endif