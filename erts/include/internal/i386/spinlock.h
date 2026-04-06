#ifndef ETHREAD_I386_SPINLOCK_H
#define ETHREAD_I386_SPINLOCK_H
#define ETHR_HAVE_NATIVE_SPINLOCKS 1
#define ETHR_NATIVE_SPINLOCK_IMPL "ethread"
typedef struct {
    volatile unsigned int lock;
} ethr_native_spinlock_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
static ETHR_INLINE void
ethr_native_spinlock_init(ethr_native_spinlock_t *lock)
{
    lock->lock = 0;
}
static ETHR_INLINE void
ethr_native_spin_unlock(ethr_native_spinlock_t *lock)
{
#if !defined(__x86_64__)
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__) {
	char tmp = 0;
	__asm__ __volatile__(
	    "xchgb %b0, %1"
	    : "=q"(tmp), "=m"(lock->lock)
	    : "0"(tmp) : "memory");
    }
    else
#endif
    {
	ETHR_MEMBAR(ETHR_LoadStore|ETHR_StoreStore);
	*(unsigned char*)&lock->lock = 0;
    }
}
static ETHR_INLINE int
ethr_native_spin_trylock(ethr_native_spinlock_t *lock)
{
    char tmp = 1;
    __asm__ __volatile__(
	"xchgb %b0, %1"
	: "=q"(tmp), "=m"(lock->lock)
	: "0"(tmp) : "memory");
    return tmp == 0;
}
static ETHR_INLINE int
ethr_native_spin_is_locked(ethr_native_spinlock_t *lock)
{
    return *(volatile unsigned char*)&lock->lock != 0;
}
static ETHR_INLINE void
ethr_native_spin_lock(ethr_native_spinlock_t *lock)
{
    for(;;) {
	if (__builtin_expect(ethr_native_spin_trylock(lock) != 0, 1))
	    break;
	do {
	    __asm__ __volatile__("rep;nop" : "=m"(lock->lock) : : "memory");
	} while (ethr_native_spin_is_locked(lock));
    }
}
#endif
#endif