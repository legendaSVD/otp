#ifndef ERL_MSACC_H__
#define ERL_MSACC_H__
#if defined(ERTS_ENABLE_MSACC) && ERTS_ENABLE_MSACC == 2
#define ERTS_MSACC_EXTENDED_STATES 1
#endif
#define ERTS_MSACC_DISABLE 0
#define ERTS_MSACC_ENABLE  1
#define ERTS_MSACC_RESET   2
#define ERTS_MSACC_GATHER  3
#ifndef ERTS_MSACC_EXTENDED_STATES
#define ERTS_MSACC_STATE_AUX       0
#define ERTS_MSACC_STATE_CHECK_IO  1
#define ERTS_MSACC_STATE_EMULATOR  2
#define ERTS_MSACC_STATE_GC        3
#define ERTS_MSACC_STATE_OTHER     4
#define ERTS_MSACC_STATE_PORT      5
#define ERTS_MSACC_STATE_SLEEP     6
#define ERTS_MSACC_STATE_COUNT 7
#if defined(ERTS_MSACC_STATE_STRINGS) && defined(ERTS_ENABLE_MSACC)
static char *erts_msacc_states[] = {
    "aux",
    "check_io",
    "emulator",
    "gc",
    "other",
    "port",
    "sleep"
};
#endif
#else
#define ERTS_MSACC_STATE_ALLOC     0
#define ERTS_MSACC_STATE_AUX       1
#define ERTS_MSACC_STATE_BIF       2
#define ERTS_MSACC_STATE_BUSY_WAIT 3
#define ERTS_MSACC_STATE_CHECK_IO  4
#define ERTS_MSACC_STATE_EMULATOR  5
#define ERTS_MSACC_STATE_ETS       6
#define ERTS_MSACC_STATE_GC        7
#define ERTS_MSACC_STATE_GC_FULL   8
#define ERTS_MSACC_STATE_NIF       9
#define ERTS_MSACC_STATE_OTHER     10
#define ERTS_MSACC_STATE_PORT      11
#define ERTS_MSACC_STATE_SEND      12
#define ERTS_MSACC_STATE_SLEEP     13
#define ERTS_MSACC_STATE_TIMERS    14
#define ERTS_MSACC_STATIC_STATE_COUNT 15
#ifdef ERTS_MSACC_EXTENDED_BIFS
#define ERTS_MSACC_STATE_COUNT (ERTS_MSACC_STATIC_STATE_COUNT + BIF_SIZE)
#else
#define ERTS_MSACC_STATE_COUNT ERTS_MSACC_STATIC_STATE_COUNT
#endif
#ifdef ERTS_MSACC_STATE_STRINGS
static char *erts_msacc_states[] = {
    "alloc",
    "aux",
    "bif",
    "busy_wait",
    "check_io",
    "emulator",
    "ets",
    "gc",
    "gc_full",
    "nif",
    "other",
    "port",
    "send",
    "sleep",
    "timers"
#ifdef ERTS_MSACC_EXTENDED_BIFS
#define BIF_LIST(Mod,Func,Arity,BifFuncAddr,FuncAddr,Num)	\
        ,"bif_" #Mod "_" #Func "_" #Arity
#include "erl_bif_list.h"
#undef BIF_LIST
#endif
};
#endif
#endif
typedef struct erl_msacc_t_ ErtsMsAcc;
typedef struct erl_msacc_p_cnt_t_ {
    ErtsSysPerfCounter pc;
#ifdef ERTS_MSACC_STATE_COUNTERS
    Uint64 sc;
#endif
} ErtsMsAccPerfCntr;
struct erl_msacc_t_ {
    int unmanaged;
    erts_mtx_t mtx;
    ErtsMsAcc *next;
    erts_tid_t tid;
    Eterm id;
    char *type;
    ErtsSysPerfCounter perf_counter;
    Uint state;
    ErtsMsAccPerfCntr counters[];
};
#ifdef ERTS_ENABLE_MSACC
extern erts_tsd_key_t ERTS_WRITE_UNLIKELY(erts_msacc_key);
#ifdef ERTS_MSACC_ALWAYS_ON
#define erts_msacc_enabled 1
#else
extern int ERTS_WRITE_UNLIKELY(erts_msacc_enabled);
#endif
#define ERTS_MSACC_TSD_GET() ((ErtsMsAcc *)erts_tsd_get(erts_msacc_key))
#define ERTS_MSACC_TSD_SET(tsd) erts_tsd_set(erts_msacc_key,tsd)
void erts_msacc_early_init(void);
void erts_msacc_init(void);
void erts_msacc_init_thread(char *type, int id, int liberty);
void erts_msacc_update_cache(ErtsMsAcc **cache);
#define ERTS_MSACC_IS_ENABLED() ERTS_UNLIKELY(erts_msacc_enabled)
#define ERTS_MSACC_DECLARE_CACHE()                                      \
    ErtsMsAcc *ERTS_MSACC_UPDATE_CACHE();                                 \
    ERTS_DECLARE_DUMMY(Uint __erts_msacc_state) = ERTS_MSACC_STATE_OTHER;
#define ERTS_MSACC_IS_ENABLED_CACHED() ERTS_UNLIKELY(__erts_msacc_cache != NULL)
#define ERTS_MSACC_UPDATE_CACHE()                                       \
    __erts_msacc_cache = erts_msacc_enabled ? ERTS_MSACC_TSD_GET() : NULL
#define ERTS_MSACC_PUSH_STATE()                           \
    ERTS_MSACC_DECLARE_CACHE();                           \
    ERTS_MSACC_PUSH_STATE_CACHED()
#define ERTS_MSACC_SET_STATE(state)                                     \
    ERTS_MSACC_DECLARE_CACHE();                                         \
    ERTS_MSACC_SET_STATE_CACHED(state)
#define ERTS_MSACC_PUSH_AND_SET_STATE(state)                    \
    ERTS_MSACC_PUSH_STATE(); ERTS_MSACC_SET_STATE_CACHED(state)
#define ERTS_MSACC_PUSH_STATE_CACHED()                                  \
    __erts_msacc_state = ERTS_MSACC_IS_ENABLED_CACHED() ?               \
        erts_msacc_get_state_um__(__erts_msacc_cache) : ERTS_MSACC_STATE_OTHER
#define ERTS_MSACC_SET_STATE_CACHED(state) \
    if (ERTS_MSACC_IS_ENABLED_CACHED())                         \
        erts_msacc_set_state_um__(__erts_msacc_cache, state, 1)
#define ERTS_MSACC_PUSH_AND_SET_STATE_CACHED(state) \
    ERTS_MSACC_PUSH_STATE_CACHED(); ERTS_MSACC_SET_STATE_CACHED(state)
#define ERTS_MSACC_POP_STATE()                                          \
    if (ERTS_MSACC_IS_ENABLED_CACHED())                                 \
        erts_msacc_set_state_um__(__erts_msacc_cache, __erts_msacc_state, 0)
#define ERTS_MSACC_PUSH_STATE_M()                         \
    ERTS_MSACC_DECLARE_CACHE();                           \
    ERTS_MSACC_PUSH_STATE_CACHED_M()
#define ERTS_MSACC_PUSH_STATE_CACHED_M() \
    do { \
        if (ERTS_MSACC_IS_ENABLED_CACHED()) { \
            ASSERT(!__erts_msacc_cache->unmanaged); \
            __erts_msacc_state = erts_msacc_get_state_m__(__erts_msacc_cache); \
        } else { \
            __erts_msacc_state = ERTS_MSACC_STATE_OTHER; \
        } \
    } while(0)
#define ERTS_MSACC_SET_STATE_M(state)                   \
    ERTS_MSACC_DECLARE_CACHE();                         \
    ERTS_MSACC_SET_STATE_CACHED_M(state)
#define ERTS_MSACC_SET_STATE_CACHED_M(state) \
    do { \
        if (ERTS_MSACC_IS_ENABLED_CACHED()) { \
            ASSERT(!__erts_msacc_cache->unmanaged); \
            erts_msacc_set_state_m__(__erts_msacc_cache, state, 1); \
        } \
    } while(0)
#define ERTS_MSACC_POP_STATE_M() \
    do { \
        if (ERTS_MSACC_IS_ENABLED_CACHED()) { \
            ASSERT(!__erts_msacc_cache->unmanaged); \
            erts_msacc_set_state_m__(__erts_msacc_cache, __erts_msacc_state, 0); \
        } \
    } while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_M(state)                    \
    ERTS_MSACC_PUSH_STATE_M(); ERTS_MSACC_SET_STATE_CACHED_M(state)
ERTS_GLB_INLINE
void erts_msacc_set_state_um__(ErtsMsAcc *msacc,Uint state,int increment);
ERTS_GLB_INLINE
void erts_msacc_set_state_m__(ErtsMsAcc *msacc,Uint state,int increment);
ERTS_GLB_INLINE
Uint erts_msacc_get_state_um__(ErtsMsAcc *msacc);
ERTS_GLB_INLINE
Uint erts_msacc_get_state_m__(ErtsMsAcc *msacc);
#if ERTS_GLB_INLINE_INCL_FUNC_DEF
ERTS_GLB_INLINE
Uint erts_msacc_get_state_um__(ErtsMsAcc *msacc) {
    Uint state;
    if (msacc->unmanaged)
        erts_mtx_lock(&msacc->mtx);
    state = msacc->state;
    if (msacc->unmanaged)
        erts_mtx_unlock(&msacc->mtx);
    return state;
}
ERTS_GLB_INLINE
Uint erts_msacc_get_state_m__(ErtsMsAcc *msacc) {
    return msacc->state;
}
ERTS_GLB_INLINE
void erts_msacc_set_state_um__(ErtsMsAcc *msacc, Uint new_state, int increment) {
    if (ERTS_UNLIKELY(msacc->unmanaged)) {
        erts_mtx_lock(&msacc->mtx);
        if (ERTS_LIKELY(!msacc->perf_counter)) {
            msacc->state = new_state;
            erts_mtx_unlock(&msacc->mtx);
            return;
        }
    }
    erts_msacc_set_state_m__(msacc,new_state,increment);
    if (ERTS_UNLIKELY(msacc->unmanaged))
        erts_mtx_unlock(&msacc->mtx);
}
ERTS_GLB_INLINE
void erts_msacc_set_state_m__(ErtsMsAcc *msacc, Uint new_state, int increment) {
    ErtsSysPerfCounter prev_perf_counter;
    Sint64 diff;
    if (new_state == msacc->state)
        return;
    prev_perf_counter = msacc->perf_counter;
    msacc->perf_counter = erts_sys_perf_counter();
    diff = msacc->perf_counter - prev_perf_counter;
    ASSERT(diff >= 0);
    msacc->counters[msacc->state].pc += diff;
#ifdef ERTS_MSACC_STATE_COUNTERS
    msacc->counters[new_state].sc += increment;
#endif
    msacc->state = new_state;
}
#endif
#else
#define ERTS_MSACC_IS_ENABLED() 0
#define erts_msacc_early_init() do {} while(0)
#define erts_msacc_init() do {} while(0)
#define erts_msacc_init_thread(type, id, liberty) do {} while(0)
#define ERTS_MSACC_PUSH_STATE() do {} while(0)
#define ERTS_MSACC_PUSH_STATE_CACHED() do {} while(0)
#define ERTS_MSACC_POP_STATE() do {} while(0)
#define ERTS_MSACC_SET_STATE(state) do {} while(0)
#define ERTS_MSACC_SET_STATE_CACHED(state) do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE(state) do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_CACHED(state) do {} while(0)
#define ERTS_MSACC_UPDATE_CACHE() do {} while(0)
#define ERTS_MSACC_IS_ENABLED_CACHED() do {} while(0)
#define ERTS_MSACC_DECLARE_CACHE()
#define ERTS_MSACC_PUSH_STATE_M() do {} while(0)
#define ERTS_MSACC_PUSH_STATE_CACHED_M() do {} while(0)
#define ERTS_MSACC_SET_STATE_CACHED_M(state) do {} while(0)
#define ERTS_MSACC_POP_STATE_M() do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_M(state) do {} while(0)
#define ERTS_MSACC_SET_BIF_STATE_CACHED_X(Mod,Addr) do {} while(0)
#endif
#ifndef ERTS_MSACC_EXTENDED_STATES
#define ERTS_MSACC_PUSH_STATE_X() do {} while(0)
#define ERTS_MSACC_POP_STATE_X() do {} while(0)
#define ERTS_MSACC_SET_STATE_X(state) do {} while(0)
#define ERTS_MSACC_SET_STATE_M_X(state) do {} while(0)
#define ERTS_MSACC_SET_STATE_CACHED_X(state) do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_X(state) do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_CACHED_X(state) do {} while(0)
#define ERTS_MSACC_UPDATE_CACHE_X() do {} while(0)
#define ERTS_MSACC_IS_ENABLED_CACHED_X() 0
#define ERTS_MSACC_DECLARE_CACHE_X()
#define ERTS_MSACC_PUSH_STATE_M_X() do {} while(0)
#define ERTS_MSACC_PUSH_STATE_CACHED_M_X() do {} while(0)
#define ERTS_MSACC_SET_STATE_CACHED_M_X(state)
#define ERTS_MSACC_POP_STATE_M_X() do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_M_X(state) do {} while(0)
#define ERTS_MSACC_PUSH_AND_SET_STATE_CACHED_M_X(state) do {} while(0)
#define ERTS_MSACC_SET_BIF_STATE_CACHED_X(Mod,Addr) do {} while(0)
#else
const void *erts_msacc_set_bif_state(ErtsMsAcc *msacc,
                                     Eterm mod,
                                     const void *bif);
#define ERTS_MSACC_PUSH_STATE_X() ERTS_MSACC_PUSH_STATE()
#define ERTS_MSACC_POP_STATE_X() ERTS_MSACC_POP_STATE()
#define ERTS_MSACC_SET_STATE_X(state) ERTS_MSACC_SET_STATE(state)
#define ERTS_MSACC_SET_STATE_M_X(state) ERTS_MSACC_SET_STATE_M(state)
#define ERTS_MSACC_SET_STATE_CACHED_X(state) ERTS_MSACC_SET_STATE_CACHED(state)
#define ERTS_MSACC_PUSH_AND_SET_STATE_X(state) ERTS_MSACC_PUSH_AND_SET_STATE(state)
#define ERTS_MSACC_PUSH_AND_SET_STATE_CACHED_X(state) ERTS_MSACC_PUSH_AND_SET_STATE_CACHED(state)
#define ERTS_MSACC_UPDATE_CACHE_X() ERTS_MSACC_UPDATE_CACHE()
#define ERTS_MSACC_IS_ENABLED_CACHED_X() ERTS_MSACC_IS_ENABLED_CACHED()
#define ERTS_MSACC_DECLARE_CACHE_X() ERTS_MSACC_DECLARE_CACHE()
#define ERTS_MSACC_PUSH_STATE_M_X() ERTS_MSACC_PUSH_STATE_M()
#define ERTS_MSACC_PUSH_STATE_CACHED_M_X() ERTS_MSACC_PUSH_STATE_CACHED_M()
#define ERTS_MSACC_SET_STATE_CACHED_M_X(state) ERTS_MSACC_SET_STATE_CACHED_M(state)
#define ERTS_MSACC_POP_STATE_M_X() ERTS_MSACC_POP_STATE_M()
#define ERTS_MSACC_PUSH_AND_SET_STATE_M_X(state) ERTS_MSACC_PUSH_AND_SET_STATE_M(state)
#define ERTS_MSACC_SET_BIF_STATE_CACHED_X(Mod,Addr)                       \
    if (ERTS_MSACC_IS_ENABLED_CACHED_X()) {                               \
        (void)erts_msacc_set_bif_state(__erts_msacc_cache, Mod, Addr);    \
    }
#endif
#endif