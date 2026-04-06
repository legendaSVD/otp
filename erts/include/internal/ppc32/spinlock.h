#ifndef ETHREAD_PPC_SPINLOCK_H
#define ETHREAD_PPC_SPINLOCK_H
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
    ETHR_MEMBAR(ETHR_LoadStore|ETHR_StoreStore);
    lock->lock = 0;
}
static ETHR_INLINE int
ethr_native_spin_trylock(ethr_native_spinlock_t *lock)
{
    unsigned int prev;
    __asm__ __volatile__(
	"1:\t"
	"lwarx	%0,0,%1\n\t"
	"cmpwi	0,%0,0\n\t"
	"bne-	2f\n\t"
	"stwcx.	%2,0,%1\n\t"
	"bne-	1b\n\t"
	"isync\n\t"
	"2:"
	: "=&r"(prev)
	: "r"(&lock->lock), "r"(1)
	: "cr0", "memory");
    return prev == 0;
}
static ETHR_INLINE int
ethr_native_spin_is_locked(ethr_native_spinlock_t *lock)
{
    return lock->lock != 0;
}
static ETHR_INLINE void
ethr_native_spin_lock(ethr_native_spinlock_t *lock)
{
    for(;;) {
	if (__builtin_expect(ethr_native_spin_trylock(lock) != 0, 1))
	    break;
	do {
	    __asm__ __volatile__("":::"memory");
	} while (ethr_native_spin_is_locked(lock));
    }
}
#endif
#endif