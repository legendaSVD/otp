#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "erl_dyn_lock_check.h"
#ifdef ERTS_DYN_LOCK_CHECK
#include "sys.h"
#include "erl_threads.h"
#define DLC_ASSERT(X) ERTS_ASSERT(X)
#ifdef ERTS_DYN_LOCK_CHECK_INTERNAL
# define MAX_LOCK_TYPES (64*2)
#else
# define MAX_LOCK_TYPES (32)
#endif
#define BITS_PER_WORD (sizeof(UWord)*8)
#define LOCK_TYPE_WORDS ((MAX_LOCK_TYPES-1)/BITS_PER_WORD + 1)
#define MAX_LOCK_NAME_SZ 64
static erts_atomic_t n_lock_types;
static erts_mtx_t lock_types_mtx;
struct lock_type
{
    char name[MAX_LOCK_NAME_SZ];
};
static struct lock_type lock_types[MAX_LOCK_TYPES];
static erts_tsd_key_t dlc_thread_key;
typedef struct
{
    UWord locked_now[LOCK_TYPE_WORDS];
    UWord locked_before[LOCK_TYPE_WORDS];
    struct {
        unsigned ix;
        unsigned cnt;
        unsigned trylock;
    } lock_order[MAX_LOCK_TYPES];
    unsigned n_locked;
}  dlc_thread_t;
static erts_atomic_t locked_before[MAX_LOCK_TYPES][LOCK_TYPE_WORDS];
static int check_lock_order(dlc_thread_t*, erts_dlc_t*);
#ifdef DLC_UNIT_TEST
static int is_dlc_unit_test = 0;
static void dlc_unit_test(void);
# define DLC_IS_UNIT_TEST() (is_dlc_unit_test)
#else
# define DLC_IS_UNIT_TEST() (0)
#endif
static int dlc_initialized = 0;
void erts_dlc_init(void)
{
    int i, j;
    erts_atomic_init_nob(&n_lock_types, 0);
    erts_tsd_key_create(&dlc_thread_key, "dyn_lock_check");
    for (i = 0; i < MAX_LOCK_TYPES; i++) {
        for (j = 0; j < LOCK_TYPE_WORDS; j++)
            erts_atomic_init_nob(&locked_before[i][j], 0);
    }
    erts_mtx_init(&lock_types_mtx, "dyn_lock_check", NIL,
                  (ERTS_LOCK_FLAGS_PROPERTY_STATIC |
                   ERTS_LOCK_FLAGS_CATEGORY_GENERIC));
    dlc_initialized = 1;
#ifdef DLC_UNIT_TEST
    dlc_unit_test();
#endif
}
void erts_dlc_create_lock(erts_dlc_t* dlc, const char* name)
{
    erts_aint_t i, n = erts_atomic_read_nob(&n_lock_types);
    int name_len;
    for (i = 0; name[i]; i++) {
        if (name[i] == '[')
            break;
    }
    name_len = i;
    for (i=0; i < n; i++) {
        if (sys_strncmp(name, lock_types[i].name, name_len) == 0) {
            dlc->ix = i;
            return;
        }
    }
    if (dlc_initialized)
        erts_mtx_lock(&lock_types_mtx);
    else
        DLC_ASSERT(n == 0);
    n = erts_atomic_read_nob(&n_lock_types);
    for ( ; i < n; i++) {
        if (sys_strncmp(name, lock_types[i].name, name_len) == 0) {
            dlc->ix = i;
            goto done;
        }
    }
    ERTS_ASSERT(n < MAX_LOCK_TYPES);
    ERTS_ASSERT(name_len < MAX_LOCK_NAME_SZ);
    sys_strncpy(lock_types[n].name, name, name_len);
    lock_types[n].name[name_len] = 0;
    erts_atomic_set_nob(&n_lock_types, n+1);
    dlc->ix = n;
done:
    if (dlc_initialized)
        erts_mtx_unlock(&lock_types_mtx);
}
#define IX_TO_BIT(IX) ((UWord)1 << ((IX) % BITS_PER_WORD))
#define IX_TO_WORD(IX) ((IX) / BITS_PER_WORD)
static dlc_thread_t *get_thr(void)
{
    dlc_thread_t *thr = (dlc_thread_t*) erts_tsd_get(dlc_thread_key);
    int i;
    if (!thr) {
        thr = malloc(sizeof(dlc_thread_t));
        for (i = 0; i < LOCK_TYPE_WORDS; i++) {
            thr->locked_now[i] = 0;
            thr->locked_before[i] = 0;
        }
        thr->n_locked = 0;
        erts_tsd_set(dlc_thread_key, thr);
    }
    return thr;
}
static ERTS_INLINE int is_bit_set(unsigned ix, const UWord* words)
{
    DLC_ASSERT(ix < MAX_LOCK_TYPES);
    return (words[IX_TO_WORD(ix)] & IX_TO_BIT(ix)) != (UWord)0;
}
static ERTS_INLINE int is_any_bit_set(const UWord* words)
{
    UWord bor = 0;
    int i=0;
    for (i = 0; i < LOCK_TYPE_WORDS; i++)
        bor |= words[i];
    return bor != (UWord)0;
}
static ERTS_INLINE void set_bit(unsigned ix, UWord* words)
{
    DLC_ASSERT(ix < MAX_LOCK_TYPES);
    words[IX_TO_WORD(ix)] |= IX_TO_BIT(ix);
}
static ERTS_INLINE void clr_bit(unsigned ix, UWord* words)
{
    DLC_ASSERT(ix < MAX_LOCK_TYPES);
    words[IX_TO_WORD(ix)] &= ~IX_TO_BIT(ix);
}
int erts_dlc_lock(erts_dlc_t* dlc)
{
    dlc_thread_t *thr = get_thr();
    if (thr->n_locked) {
        int i;
        DLC_ASSERT(is_any_bit_set(thr->locked_now));
        for (i=0; i < LOCK_TYPE_WORDS; i++) {
            UWord before = erts_atomic_read_nob(&locked_before[dlc->ix][i]);
            UWord new_before = (thr->locked_before[i] & ~before);
            if (new_before) {
                if (!check_lock_order(thr, dlc)) {
                    DLC_ASSERT(DLC_IS_UNIT_TEST());
                    return 0;
                }
                erts_atomic_read_bor_mb(&locked_before[dlc->ix][i],
                                        new_before);
                if (!check_lock_order(thr, dlc)) {
                    DLC_ASSERT(DLC_IS_UNIT_TEST());
                    abort();
                }
            }
        }
        if (is_bit_set(dlc->ix, thr->locked_now)) {
            DLC_ASSERT(is_bit_set(dlc->ix, thr->locked_before));
            i = thr->n_locked-1;
            while (dlc->ix != thr->lock_order[i].ix) {
                DLC_ASSERT(thr->lock_order[i].trylock);
                i--;
                DLC_ASSERT(i >= 0);
            }
            thr->lock_order[i].cnt++;
            thr->lock_order[i].trylock = 0;
            return 1;
        }
    }
    else {
        DLC_ASSERT(!is_any_bit_set(thr->locked_now));
        DLC_ASSERT(!is_any_bit_set(thr->locked_before));
    }
    set_bit(dlc->ix, thr->locked_now);
    set_bit(dlc->ix, thr->locked_before);
    thr->lock_order[thr->n_locked].ix = dlc->ix;
    thr->lock_order[thr->n_locked].cnt = 1;
    thr->lock_order[thr->n_locked].trylock = 0;
    thr->n_locked++;
    return 1;
}
static ERTS_INLINE int get_lock_order(dlc_thread_t* thr,
                                      erts_dlc_t* dlc)
{
    int i;
    DLC_ASSERT(is_bit_set(dlc->ix, thr->locked_before));
    for (i = 0; ; i++) {
        DLC_ASSERT(i < thr->n_locked);
        if (dlc->ix == thr->lock_order[i].ix)
            return i;
    }
}
void erts_dlc_trylock(erts_dlc_t* dlc, int locked)
{
    dlc_thread_t *thr = get_thr();
    if (!locked) {
        return;
    }
    if (is_bit_set(dlc->ix, thr->locked_now)) {
        int i = get_lock_order(thr, dlc);
        DLC_ASSERT(thr->lock_order[i].cnt > 0);
        thr->lock_order[i].cnt++;
    }
    else {
        set_bit(dlc->ix, thr->locked_now);
        if (!is_bit_set(dlc->ix, thr->locked_before)) {
            set_bit(dlc->ix, thr->locked_before);
            thr->lock_order[thr->n_locked].ix = dlc->ix;
            thr->lock_order[thr->n_locked].cnt = 1;
            thr->lock_order[thr->n_locked].trylock = 1;
            thr->n_locked++;
        }
        else {
            int i = get_lock_order(thr, dlc);
            DLC_ASSERT(thr->lock_order[i].cnt == 0);
            thr->lock_order[i].cnt = 1;
            thr->lock_order[i].trylock = 1;
        }
    }
}
void erts_dlc_unlock(erts_dlc_t* dlc)
{
    dlc_thread_t *thr = (dlc_thread_t*) erts_tsd_get(dlc_thread_key);
    int i;
    ERTS_ASSERT(thr);
    ERTS_ASSERT(is_bit_set(dlc->ix, thr->locked_now));
    DLC_ASSERT(is_bit_set(dlc->ix, thr->locked_before));
    DLC_ASSERT(thr->n_locked > 0);
    i = get_lock_order(thr, dlc);
    DLC_ASSERT(thr->lock_order[i].cnt > 0);
    thr->lock_order[i].cnt--;
    if (thr->lock_order[i].cnt > 0)
        return;
    clr_bit(dlc->ix, thr->locked_now);
    for (i = thr->n_locked-1; i >= 0; i--) {
        if (thr->lock_order[i].cnt) {
            DLC_ASSERT(is_bit_set(thr->lock_order[i].ix, thr->locked_now));
            if (!thr->lock_order[i].trylock) {
                break;
            }
        }
        else {
            int j;
            DLC_ASSERT(!is_bit_set(thr->lock_order[i].ix, thr->locked_now));
            DLC_ASSERT(is_bit_set(thr->lock_order[i].ix, thr->locked_before));
            clr_bit(thr->lock_order[i].ix, thr->locked_before);
            thr->n_locked--;
            for (j = i; j < thr->n_locked; j++) {
                DLC_ASSERT(thr->lock_order[j+1].trylock);
                thr->lock_order[j] = thr->lock_order[j+1];
            }
        }
    }
}
static int check_lock_order(dlc_thread_t *thr, erts_dlc_t* dlc)
{
    const UWord lock_bit = IX_TO_BIT(dlc->ix % 64);
    const unsigned lock_word = IX_TO_WORD(dlc->ix);
    int i, error = 0;
    for (i = 0; i < thr->n_locked; i++) {
        const unsigned ix = thr->lock_order[i].ix;
        if (ix != dlc->ix &&
            lock_bit & erts_atomic_read_nob(&locked_before[ix][lock_word])) {
            if (!error) {
                error = 1;
                erts_fprintf(stderr, "###### DYNAMIC LOCK ORDER VIOLATION ######\n");
                erts_fprintf(stderr, "# Trying to lock '%s'\n", lock_types[dlc->ix].name);
            }
            erts_fprintf(stderr, "# while '%s' is held\n",
                         lock_types[thr->lock_order[i].ix].name);
        }
    }
    if (error) {
        if (DLC_IS_UNIT_TEST())
            return 0;
        abort();
    }
    return 1;
}
#ifdef DLC_UNIT_TEST
static void dlc_clear_order(void)
{
    int i, j, n = erts_atomic_read_nob(&n_lock_types);
    for (i = 0; i < n; i++) {
        for (j = 0; j < LOCK_TYPE_WORDS; j++)
            erts_atomic_set_nob(&locked_before[i][j], 0);
    }
}
static void dlc_unit_test(void)
{
    erts_aint_t save_n_lock_types = erts_atomic_read_nob(&n_lock_types);
    dlc_thread_t* thr = get_thr();
    dlc_thread_t save_thr = *thr;
    erts_dlc_t A,B,C,D,E,F;
    ERTS_ASSERT(save_n_lock_types <= 1);
    is_dlc_unit_test = 1;
    erts_dlc_create_lock(&A, "A");
    erts_dlc_create_lock(&B, "B");
    erts_dlc_create_lock(&C, "C");
    erts_dlc_create_lock(&D, "D");
    erts_dlc_create_lock(&E, "E");
    erts_dlc_create_lock(&F, "F");
    ERTS_ASSERT(erts_dlc_lock(&A));
    ERTS_ASSERT(erts_dlc_lock(&C));
    ERTS_ASSERT(!erts_dlc_lock(&A));
    erts_dlc_unlock(&A);
    ERTS_ASSERT(!erts_dlc_lock(&A));
    erts_dlc_unlock(&C);
    ERTS_ASSERT(erts_dlc_lock(&A));
    ERTS_ASSERT(erts_dlc_lock(&B));
    ERTS_ASSERT(erts_dlc_lock(&C));
    erts_dlc_unlock(&A);
    erts_dlc_unlock(&B);
    erts_dlc_unlock(&C);
    ERTS_ASSERT(erts_dlc_lock(&A));
    ERTS_ASSERT(erts_dlc_lock(&C));
    ERTS_ASSERT(!erts_dlc_lock(&B));
    erts_dlc_unlock(&A);
    erts_dlc_unlock(&C);
    dlc_clear_order();
    ERTS_ASSERT(erts_dlc_lock(&A));
    ERTS_ASSERT(erts_dlc_lock(&B));
    erts_dlc_unlock(&A);
    ERTS_ASSERT(erts_dlc_lock(&C));
    erts_dlc_unlock(&B);
    ERTS_ASSERT(erts_dlc_lock(&D));
    erts_dlc_unlock(&C);
    ERTS_ASSERT(erts_dlc_lock(&E));
    erts_dlc_unlock(&D);
    ERTS_ASSERT(erts_dlc_lock(&F));
    erts_dlc_unlock(&E);
    erts_dlc_unlock(&F);
    ERTS_ASSERT(erts_dlc_lock(&F));
    ERTS_ASSERT(!erts_dlc_lock(&A));
    erts_dlc_unlock(&F);
    dlc_clear_order();
    ERTS_ASSERT(erts_dlc_lock(&A));
    erts_dlc_trylock(&B, 1);
    erts_dlc_unlock(&A);
    ERTS_ASSERT(erts_dlc_lock(&A));
    erts_dlc_unlock(&A);
    erts_dlc_unlock(&B);
    dlc_clear_order();
    ERTS_ASSERT(erts_dlc_lock(&A));
    ERTS_ASSERT(erts_dlc_lock(&B));
    ERTS_ASSERT(erts_dlc_lock(&C));
    erts_dlc_trylock(&D, 1);
    erts_dlc_trylock(&E, 1);
    ERTS_ASSERT(erts_dlc_lock(&F));
    erts_dlc_unlock(&C);
    erts_dlc_unlock(&F);
    ERTS_ASSERT(erts_dlc_lock(&B));
    erts_dlc_unlock(&B);
    ERTS_ASSERT(!erts_dlc_lock(&A));
    erts_dlc_unlock(&B);
    ERTS_ASSERT(erts_dlc_lock(&A));
    erts_dlc_unlock(&A);
    erts_dlc_unlock(&A);
    erts_dlc_unlock(&D);
    erts_dlc_unlock(&E);
    dlc_clear_order();
    ERTS_ASSERT(erts_dlc_lock(&A));
    erts_dlc_trylock(&B, 1);
    erts_dlc_trylock(&C, 1);
    ERTS_ASSERT(erts_dlc_lock(&B));
    erts_dlc_unlock(&B);
    ERTS_ASSERT(!erts_dlc_lock(&C));
    is_dlc_unit_test = 0;
    dlc_clear_order();
    erts_atomic_set_nob(&n_lock_types, save_n_lock_types);
    *thr = save_thr;
}
#endif
#endif