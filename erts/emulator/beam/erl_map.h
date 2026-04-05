#ifndef __ERL_MAP_H__
#define __ERL_MAP_H__
#include "sys.h"
#include "erl_term_hashing.h"
#if ERTS_AT_LEAST_GCC_VSN__(3, 4, 0) || __has_builtin(__builtin_clz)
#  if defined(ARCH_64)
#    define hashmap_clz(x) \
    ((erts_ihash_t)__builtin_clzl((erts_ihash_t)(x)))
#  elif defined(ARCH_32)
#    define hashmap_clz(x) \
    ((erts_ihash_t)__builtin_clz((erts_ihash_t)(x)))
#  endif
#else
erts_ihash_t hashmap_clz(erts_ihash_t x);
#endif
#if ERTS_AT_LEAST_GCC_VSN__(3, 4, 0) || __has_builtin(__builtin_popcount)
#  if defined(ARCH_64)
#    define hashmap_bitcount(x) \
    ((erts_ihash_t)__builtin_popcountl((erts_ihash_t)(x)))
#  elif defined(ARCH_32)
#    define hashmap_bitcount(x) \
    ((erts_ihash_t)__builtin_popcount((erts_ihash_t)(x)))
#  endif
#else
erts_ihash_t hashmap_bitcount(erts_ihash_t x);
#endif
typedef struct flatmap_s {
    Eterm thing_word;
    Uint  size;
    Eterm keys;
} flatmap_t;
#define hashmap_size(x)               (((hashmap_head_t*) hashmap_val(x))->size)
#define hashmap_make_hash(Key)        erts_map_hash(Key)
#define hashmap_restore_hash(Lvl, Key)                                        \
    (ASSERT(Lvl < HAMT_MAX_LEVEL),                                            \
     hashmap_make_hash(Key) >> (4*(Lvl)))
#define hashmap_shift_hash(Hx, Lvl, Key)                                      \
    (++(Lvl), ASSERT(Lvl <= HAMT_MAX_LEVEL), \
     (Hx) >> 4)
#define flatmap_get_values(x)        (((Eterm *)(x)) + sizeof(flatmap_t)/sizeof(Eterm))
#define flatmap_get_keys(x)          (((Eterm *)tuple_val(((flatmap_t *)(x))->keys)) + 1)
#define flatmap_get_size(x)          (((flatmap_t*)(x))->size)
#ifdef DEBUG
#define MAP_SMALL_MAP_LIMIT    (3)
#else
#define MAP_SMALL_MAP_LIMIT    (32)
#endif
struct ErtsWStack_;
struct ErtsEStack_;
Eterm  erts_maps_put(Process *p, Eterm key, Eterm value, Eterm map);
int    erts_maps_update(Process *p, Eterm key, Eterm value, Eterm map, Eterm *res);
int    erts_maps_remove(Process *p, Eterm key, Eterm map, Eterm *res);
int    erts_maps_take(Process *p, Eterm key, Eterm map, Eterm *res, Eterm *value);
Eterm  erts_hashmap_insert(Process *p, erts_ihash_t hx, Eterm key, Eterm value,
			   Eterm node, int is_update);
int    erts_hashmap_insert_down(erts_ihash_t hx, Eterm key, Eterm value, Eterm node, Uint *sz,
			        Uint *upsz, struct ErtsEStack_ *sp, int is_update);
Eterm  erts_hashmap_insert_up(Eterm *hp, Eterm key, Eterm value,
			      Uint upsz, struct ErtsEStack_ *sp);
int    erts_validate_and_sort_flatmap(flatmap_t* map);
void   erts_usort_flatmap(flatmap_t* map);
void   hashmap_iterator_init(struct ErtsWStack_* s, Eterm node, int reverse);
Eterm* hashmap_iterator_next(struct ErtsWStack_* s);
Eterm* hashmap_iterator_prev(struct ErtsWStack_* s);
int    hashmap_key_hash_cmp(Eterm* ap, Eterm* bp);
Eterm  erts_hashmap_from_array(ErtsHeapFactory*, Eterm *leafs, Uint n, int reject_dupkeys);
#define erts_hashmap_from_ks_and_vs(F, KS, VS, N) \
    erts_hashmap_from_ks_and_vs_extra((F), (KS), (VS), (N), THE_NON_VALUE, THE_NON_VALUE);
Eterm erts_map_from_ks_and_vs(ErtsHeapFactory *factory, Eterm *ks, Eterm *vs, Uint n);
Eterm  erts_hashmap_from_ks_and_vs_extra(ErtsHeapFactory *factory,
                                         Eterm *ks, Eterm *vs, Uint n,
					 Eterm k, Eterm v);
const Eterm *erts_maps_get(Eterm key, Eterm map);
const Eterm *erts_hashmap_get(erts_ihash_t hx, Eterm key, Eterm map);
Sint erts_map_size(Eterm map);
typedef struct hashmap_head_s {
    Eterm thing_word;
    Uint size;
    Eterm items[1];
} hashmap_head_t;
#define _HEADER_MAP_SUBTAG_MASK       \
    ((3 << _HEADER_ARITY_OFFS) | _HEADER_SUBTAG_MASK)
#define _HEADER_MAP_HASHMAP_HEAD_MASK \
    (~(1 << _HEADER_ARITY_OFFS) & _HEADER_MAP_SUBTAG_MASK)
#define is_hashmap_header_head(x) (MAP_HEADER_TYPE(x) & (0x2))
#define is_hashmap_header_node(x) (MAP_HEADER_TYPE(x) == 1)
#define MAKE_MAP_HEADER(Type,Arity,Val) \
    (_make_header(((((Uint16)(Val)) << MAP_HEADER_ARITY_SZ) | \
     (Arity)) << MAP_HEADER_TAG_SZ | (Type) , _TAG_HEADER_MAP))
#define MAP_HEADER_FLATMAP \
    MAKE_MAP_HEADER(MAP_HEADER_TAG_FLATMAP_HEAD,0x1,0x0)
#define MAP_HEADER_HAMT_HEAD_ARRAY \
    MAKE_MAP_HEADER(MAP_HEADER_TAG_HAMT_HEAD_ARRAY,0x1,0xffff)
#define MAP_HEADER_HAMT_HEAD_BITMAP(Bmp) \
    MAKE_MAP_HEADER(MAP_HEADER_TAG_HAMT_HEAD_BITMAP,0x1,Bmp)
#define MAP_HEADER_HAMT_NODE_BITMAP(Bmp) \
    MAKE_MAP_HEADER(MAP_HEADER_TAG_HAMT_NODE_BITMAP,0x0,Bmp)
#define MAP_HEADER_HAMT_COLLISION_NODE(Arity) make_arityval(Arity)
#define MAP_HEADER_FLATMAP_SZ  (sizeof(flatmap_t) / sizeof(Eterm))
#define HAMT_NODE_ARRAY_SZ      (17)
#define HAMT_HEAD_ARRAY_SZ      (18)
#define HAMT_NODE_BITMAP_SZ(n)  (1 + n)
#define HAMT_HEAD_BITMAP_SZ(n)  (2 + n)
#define HAMT_COLLISION_NODE_SZ(n)     (1 + n)
#define HAMT_SUBTAG_NODE_BITMAP  ((MAP_HEADER_TAG_HAMT_NODE_BITMAP << _HEADER_ARITY_OFFS) | MAP_SUBTAG)
#define HAMT_SUBTAG_HEAD_ARRAY   ((MAP_HEADER_TAG_HAMT_HEAD_ARRAY  << _HEADER_ARITY_OFFS) | MAP_SUBTAG)
#define HAMT_SUBTAG_HEAD_BITMAP  ((MAP_HEADER_TAG_HAMT_HEAD_BITMAP << _HEADER_ARITY_OFFS) | MAP_SUBTAG)
#define HAMT_SUBTAG_HEAD_FLATMAP ((MAP_HEADER_TAG_FLATMAP_HEAD << _HEADER_ARITY_OFFS) | MAP_SUBTAG)
#define hashmap_index(hash)      ((hash) & 0xf)
#define HAMT_MAX_LEVEL ((sizeof(erts_ihash_t) * CHAR_BIT) / 4)
#define HASHMAP_WORDS_PER_KEY 3
#define HASHMAP_WORDS_PER_NODE 2
#ifdef DEBUG
#  define HASHMAP_ESTIMATED_TOT_NODE_SIZE(KEYS) \
    (HASHMAP_WORDS_PER_NODE * (KEYS) * 3/10)
#else
#  define HASHMAP_ESTIMATED_TOT_NODE_SIZE(KEYS) \
    (HASHMAP_WORDS_PER_NODE * (KEYS) * 4/10)
#endif
#define HASHMAP_ESTIMATED_HEAP_SIZE(KEYS) \
        ((KEYS)*HASHMAP_WORDS_PER_KEY + HASHMAP_ESTIMATED_TOT_NODE_SIZE(KEYS))
#endif