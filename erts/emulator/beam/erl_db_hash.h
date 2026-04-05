#ifndef _DB_HASH_H
#define _DB_HASH_H
#include "erl_db_util.h"
typedef struct fixed_deletion {
    UWord slot : sizeof(UWord)*8 - 2;
    bool all : 1;
    bool trap : 1;
    struct fixed_deletion *next;
} FixedDeletion;
typedef Uint32 HashVal;
typedef struct hash_db_term {
    struct  hash_db_term* next;
    UWord hvalue : sizeof(UWord)*8 - 1;
    UWord pseudo_deleted : 1;
    DbTerm dbterm;
} HashDbTerm;
#ifdef ERTS_DB_HASH_LOCK_CNT
#define DB_HASH_LOCK_CNT ERTS_DB_HASH_LOCK_CNT
#else
#define DB_HASH_LOCK_CNT 64
#endif
typedef struct DbTableHashLockAndCounter {
    erts_atomic_t nitems;
    Sint lck_stat;
    erts_rwmtx_t lck;
} DbTableHashLockAndCounter;
typedef struct db_table_hash_fine_lock_slot {
    union {
	DbTableHashLockAndCounter lck_ctr;
	byte _cache_line_alignment[ERTS_ALC_CACHE_LINE_ALIGN_SIZE(sizeof(DbTableHashLockAndCounter))];
    } u;
} DbTableHashFineLockSlot;
typedef struct db_table_hash {
    DbTableCommon common;
    erts_atomic_t lock_array_resize_state;
    erts_atomic_t szm;
    erts_atomic_t nactive;
    erts_atomic_t shrink_limit;
    erts_atomic_t segtab;
    struct segment* first_segtab[1];
    UWord nlocks;
    UWord nslots;
    UWord nsegs;
    erts_atomic_t fixdel;
    erts_atomic_t is_resizing;
    DbTableHashFineLockSlot* locks;
} DbTableHash;
typedef enum {
    DB_HASH_LOCK_ARRAY_RESIZE_STATUS_NORMAL = 0,
    DB_HASH_LOCK_ARRAY_RESIZE_STATUS_GROW   = 1,
    DB_HASH_LOCK_ARRAY_RESIZE_STATUS_SHRINK = 2
} db_hash_lock_array_resize_state;
void db_hash_adapt_number_of_locks(DbTable* tb);
#define  DB_HASH_ADAPT_NUMBER_OF_LOCKS(TB)                                   \
    do {                                                                     \
        if (IS_HASH_WITH_AUTO_TABLE(TB->common.type)                         \
            && (erts_atomic_read_nob(&tb->hash.lock_array_resize_state)      \
                != DB_HASH_LOCK_ARRAY_RESIZE_STATUS_NORMAL)) {               \
            db_hash_adapt_number_of_locks(tb);                               \
        }                                                                    \
    }while(0)
void db_initialize_hash(void);
SWord db_unfix_table_hash(DbTableHash *tb);
Uint db_kept_items_hash(DbTableHash *tb);
int db_create_hash(Process *p,
		   DbTable *tbl );
int db_put_hash(DbTable *tbl, Eterm obj, bool key_clash_fail, SWord* consumed_reds_p);
int db_get_hash(Process *p, DbTable *tbl, Eterm key, Eterm *ret);
int db_erase_hash(DbTable *tbl, Eterm key, Eterm *ret);
typedef struct {
    float avg_chain_len;
    float std_dev_chain_len;
    float std_dev_expected;
    int max_chain_len;
    int min_chain_len;
    UWord kept_items;
}DbHashStats;
void db_calc_stats_hash(DbTableHash* tb, DbHashStats*);
Eterm erts_ets_hash_sizeof_ext_segtab(void);
void
erts_db_foreach_thr_prgr_offheap_hash(void (*func)(ErlOffHeap *, void *),
                                      void *arg);
#ifdef ERTS_ENABLE_LOCK_COUNT
void erts_lcnt_enable_db_hash_lock_count(DbTableHash *tb, int enable);
#endif
#endif