#ifndef ETHR_OPTIMIZED_FALLBACKS_H__
#define ETHR_OPTIMIZED_FALLBACKS_H__
#if defined(ETHR_HAVE_NATIVE_SPINLOCKS)
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
static ETHR_INLINE int
ethr_native_spinlock_destroy(ethr_native_spinlock_t *lock)
{
    return 0;
}
#endif
#elif defined(ETHR_HAVE_PTHREAD_SPIN_LOCK)
#define ETHR_HAVE_NATIVE_SPINLOCKS 1
#define ETHR_NATIVE_SPINLOCK_IMPL "pthread"
typedef pthread_spinlock_t ethr_native_spinlock_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
static ETHR_INLINE void
ethr_native_spinlock_init(ethr_native_spinlock_t *lock)
{
    int err = pthread_spin_init((pthread_spinlock_t *) lock, 0);
    if (err)
	ETHR_FATAL_ERROR__(err);
}
static ETHR_INLINE int
ethr_native_spinlock_destroy(ethr_native_spinlock_t *lock)
{
    return pthread_spin_destroy((pthread_spinlock_t *) lock);
}
static ETHR_INLINE void
ethr_native_spin_unlock(ethr_native_spinlock_t *lock)
{
    int err = pthread_spin_unlock((pthread_spinlock_t *) lock);
    if (err)
	ETHR_FATAL_ERROR__(err);
}
static ETHR_INLINE void
ethr_native_spin_lock(ethr_native_spinlock_t *lock)
{
    int err = pthread_spin_lock((pthread_spinlock_t *) lock);
    if (err)
	ETHR_FATAL_ERROR__(err);
}
#endif
#elif defined(ETHR_HAVE_32BIT_NATIVE_ATOMIC_OPS)
#define ETHR_HAVE_NATIVE_SPINLOCKS 1
#define ETHR_NATIVE_SPINLOCK_IMPL "native-atomics"
typedef ethr_atomic32_t ethr_native_spinlock_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
#undef ETHR_NSPN_AOP__
#define ETHR_NSPN_AOP__(X) ETHR_INLINE_ATMC32_FUNC_NAME_(ethr_atomic32_ ## X)
static ETHR_INLINE void
ethr_native_spinlock_init(ethr_native_spinlock_t *lock)
{
    ETHR_NSPN_AOP__(init)(lock, 0);
}
static ETHR_INLINE int
ethr_native_spinlock_destroy(ethr_native_spinlock_t *lock)
{
    return ETHR_NSPN_AOP__(read)(lock) == 0 ? 0 : EBUSY;
}
static ETHR_INLINE void
ethr_native_spin_unlock(ethr_native_spinlock_t *lock)
{
    ETHR_ASSERT(ETHR_NSPN_AOP__(read)(lock) == 1);
    ETHR_NSPN_AOP__(set_relb)(lock, 0);
}
static ETHR_INLINE void
ethr_native_spin_lock(ethr_native_spinlock_t *lock)
{
    while (ETHR_NSPN_AOP__(cmpxchg_acqb)(lock, 1, 0) != 0) {
	while (ETHR_NSPN_AOP__(read)(lock) != 0)
	    ETHR_SPIN_BODY;
    }
    ETHR_COMPILER_BARRIER;
}
#undef ETHR_NSPN_AOP__
#endif
#endif
#if defined(ETHR_HAVE_NATIVE_RWSPINLOCKS)
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
static ETHR_INLINE int
ethr_native_rwlock_destroy(ethr_native_rwlock_t *lock)
{
    return 0;
}
#endif
#elif defined(ETHR_HAVE_32BIT_NATIVE_ATOMIC_OPS)
#define ETHR_HAVE_NATIVE_RWSPINLOCKS 1
#define ETHR_NATIVE_RWSPINLOCK_IMPL "native-atomics"
typedef ethr_atomic32_t ethr_native_rwlock_t;
#  define ETHR_WLOCK_FLAG__ (((ethr_sint32_t) 1) << 30)
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
#undef ETHR_NRWSPN_AOP__
#define ETHR_NRWSPN_AOP__(X) ETHR_INLINE_ATMC32_FUNC_NAME_(ethr_atomic32_ ## X)
static ETHR_INLINE void
ethr_native_rwlock_init(ethr_native_rwlock_t *lock)
{
    ETHR_NRWSPN_AOP__(init)(lock, 0);
}
static ETHR_INLINE int
ethr_native_rwlock_destroy(ethr_native_rwlock_t *lock)
{
    return ETHR_NRWSPN_AOP__(read)(lock) == 0 ? 0 : EBUSY;
}
static ETHR_INLINE void
ethr_native_read_unlock(ethr_native_rwlock_t *lock)
{
    ETHR_ASSERT(ETHR_NRWSPN_AOP__(read)(lock) >= 0);
    ETHR_NRWSPN_AOP__(dec_relb)(lock);
}
static ETHR_INLINE void
ethr_native_read_lock(ethr_native_rwlock_t *lock)
{
    ethr_sint32_t act, exp = 0;
    while (1) {
	act = ETHR_NRWSPN_AOP__(cmpxchg_acqb)(lock, exp+1, exp);
	if (act == exp)
	    break;
	while (act & ETHR_WLOCK_FLAG__) {
	    ETHR_SPIN_BODY;
	    act = ETHR_NRWSPN_AOP__(read)(lock);
	}
	exp = act;
    }
    ETHR_COMPILER_BARRIER;
}
static ETHR_INLINE void
ethr_native_write_unlock(ethr_native_rwlock_t *lock)
{
    ETHR_ASSERT(ETHR_NRWSPN_AOP__(read)(lock) == ETHR_WLOCK_FLAG__);
    ETHR_NRWSPN_AOP__(set_relb)(lock, 0);
}
static ETHR_INLINE void
ethr_native_write_lock(ethr_native_rwlock_t *lock)
{
    ethr_sint32_t act, exp = 0;
    while (1) {
	act = ETHR_NRWSPN_AOP__(cmpxchg_acqb)(lock, exp|ETHR_WLOCK_FLAG__, exp);
	if (act == exp)
	    break;
	while (act & ETHR_WLOCK_FLAG__) {
	    ETHR_SPIN_BODY;
	    act = ETHR_NRWSPN_AOP__(read)(lock);
	}
	exp = act;
    }
    act |= ETHR_WLOCK_FLAG__;
    while (act != ETHR_WLOCK_FLAG__) {
	ETHR_SPIN_BODY;
	act = ETHR_NRWSPN_AOP__(read_acqb)(lock);
    }
    ETHR_COMPILER_BARRIER;
}
#undef ETHR_NRWSPN_AOP__
#endif
#endif
#endif