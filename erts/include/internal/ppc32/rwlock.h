#ifndef ETHREAD_PPC_RWLOCK_H
#define ETHREAD_PPC_RWLOCK_H
#define ETHR_HAVE_NATIVE_RWSPINLOCKS 1
#define ETHR_NATIVE_RWSPINLOCK_IMPL "ethread"
typedef struct {
    volatile int lock;
} ethr_native_rwlock_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_AUX_IMPL__)
static ETHR_INLINE void
ethr_native_rwlock_init(ethr_native_rwlock_t *lock)
{
    lock->lock = 0;
}
static ETHR_INLINE void
ethr_native_read_unlock(ethr_native_rwlock_t *lock)
{
    int tmp;
    ETHR_MEMBAR(ETHR_LoadStore|ETHR_StoreStore);
    __asm__ __volatile__(
	"1:\t"
	"lwarx	%0,0,%1\n\t"
	"addic	%0,%0,1\n\t"
	"stwcx.	%0,0,%1\n\t"
	"bne-	1b"
	: "=&r"(tmp)
	: "r"(&lock->lock)
	: "cr0", "memory");
}
static ETHR_INLINE int
ethr_native_read_trylock(ethr_native_rwlock_t *lock)
{
    int counter;
    __asm__ __volatile__(
	"1:\t"
	"lwarx	%0,0,%1\n\t"
	"addic.	%0,%0,-1\n\t"
	"bge-	2f\n\t"
	"stwcx.	%0,0,%1\n\t"
	"bne-	1b\n\t"
	"isync\n\t"
	"2:"
	: "=&r"(counter)
	: "r"(&lock->lock)
	: "cr0", "memory"
#if __GNUC__ > 2
	,"xer"
#endif
	);
    return counter < 0;
}
static ETHR_INLINE int
ethr_native_read_is_locked(ethr_native_rwlock_t *lock)
{
    return lock->lock > 0;
}
static ETHR_INLINE void
ethr_native_read_lock(ethr_native_rwlock_t *lock)
{
    for(;;) {
	if (__builtin_expect(ethr_native_read_trylock(lock) != 0, 1))
	    break;
	do {
	    __asm__ __volatile__("":::"memory");
	} while (ethr_native_read_is_locked(lock));
    }
}
static ETHR_INLINE void
ethr_native_write_unlock(ethr_native_rwlock_t *lock)
{
    ETHR_MEMBAR(ETHR_LoadStore|ETHR_StoreStore);
    lock->lock = 0;
}
static ETHR_INLINE int
ethr_native_write_trylock(ethr_native_rwlock_t *lock)
{
    int prev;
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
ethr_native_write_is_locked(ethr_native_rwlock_t *lock)
{
    return lock->lock != 0;
}
static ETHR_INLINE void
ethr_native_write_lock(ethr_native_rwlock_t *lock)
{
    for(;;) {
	if (__builtin_expect(ethr_native_write_trylock(lock) != 0, 1))
	    break;
	do {
	    __asm__ __volatile__("":::"memory");
	} while (ethr_native_write_is_locked(lock));
    }
}
#endif
#endif