#include "erl_internal_test.h"
#include "erl_misc_utils.h"
const ErtsInternalTest erts_internal_test_instance = {
    .erts_milli_sleep = &erts_milli_sleep,
    .ethr_atomic_dec = &ethr_atomic_dec,
    .ethr_atomic_inc = &ethr_atomic_inc,
    .ethr_atomic_init = &ethr_atomic_init,
    .ethr_atomic_read = &ethr_atomic_read,
    .ethr_atomic_set = &ethr_atomic_set,
    .ethr_rwmutex_destroy = &ethr_rwmutex_destroy,
    .ethr_rwmutex_init_opt = &ethr_rwmutex_init_opt,
    .ethr_rwmutex_rlock = &ethr_rwmutex_rlock,
    .ethr_rwmutex_runlock = &ethr_rwmutex_runlock,
    .ethr_rwmutex_rwlock = &ethr_rwmutex_rwlock,
    .ethr_rwmutex_rwunlock = &ethr_rwmutex_rwunlock,
    .ethr_rwmutex_tryrlock = &ethr_rwmutex_tryrlock,
    .ethr_rwmutex_tryrwlock = &ethr_rwmutex_tryrwlock,
    .ethr_thr_create = &ethr_thr_create,
    .ethr_thr_join = &ethr_thr_join
};