#ifndef __SAFE_HASH_H__
#define __SAFE_HASH_H__
#include "sys.h"
#include "erl_alloc.h"
#include "erl_lock_flags.h"
typedef unsigned long SafeHashValue;
typedef int (*SHCMP_FUN)(void*, void*);
typedef SafeHashValue (*SH_FUN)(void*);
typedef void* (*SHALLOC_FUN)(void*);
typedef void (*SHFREE_FUN)(void*);
typedef struct safe_hashbucket
{
    struct safe_hashbucket* next;
    SafeHashValue hvalue;
} SafeHashBucket;
typedef struct safe_hashfunctions
{
    SH_FUN hash;
    SHCMP_FUN cmp;
    SHALLOC_FUN alloc;
    SHFREE_FUN free;
} SafeHashFunctions;
typedef struct {
  char *name;
  int   size;
  int   used;
  int   objs;
  int   depth;
} SafeHashInfo;
#define SAFE_HASH_LOCK_CNT 16
typedef struct
{
    SafeHashFunctions fun;
    ErtsAlcType_t type;
    char* name;
    int size_mask;
    SafeHashBucket** tab;
    int grow_limit;
    erts_atomic_t nitems;
    erts_atomic_t is_rehashing;
    union {
	erts_mtx_t mtx;
	byte __cache_line__[64];
    }lock_vec[SAFE_HASH_LOCK_CNT];
} SafeHash;
SafeHash* safe_hash_init(ErtsAlcType_t, SafeHash*, char*, erts_lock_flags_t, int, SafeHashFunctions);
void  safe_hash_get_info(SafeHashInfo*, SafeHash*);
int   safe_hash_table_sz(SafeHash *);
void* safe_hash_get(SafeHash*, void*);
void* safe_hash_put(SafeHash*, void*);
void* safe_hash_erase(SafeHash*, void*);
void  safe_hash_for_each(SafeHash*, void (*func)(void *, void *, void *), void *, void *);
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_enable_hash_lock_count(SafeHash*, erts_lock_flags_t, int);
#endif
#endif