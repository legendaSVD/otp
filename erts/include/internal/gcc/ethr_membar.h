#if defined(__i386__) || defined(__x86_64__)
#  include "../i386/ethr_membar.h"
#elif defined(__sparc__)
#  include "../sparc32/ethr_membar.h"
#elif defined(__powerpc__) || defined(__ppc__) || defined(__powerpc64__)
#  include "../ppc32/ethr_membar.h"
#elif !defined(ETHR_GCC_ATOMIC_MEMBAR_H__)			\
    && (ETHR_HAVE_GCC_ASM_ARM_DMB_INSTRUCTION			\
	|| ETHR_HAVE___sync_synchronize				\
	|| (ETHR_HAVE___sync_val_compare_and_swap & 12))
#define ETHR_GCC_ATOMIC_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
#define ETHR_COMPILER_BARRIER __asm__ __volatile__("" : : : "memory")
#if ETHR_HAVE_GCC_ASM_ARM_ISB_SY_INSTRUCTION
#define ETHR_INSTRUCTION_BARRIER ethr_instruction_fence__()
static __inline__ __attribute__((__always_inline__)) void
ethr_instruction_fence__(void)
{
    __asm__ __volatile__("isb sy" : : : "memory");
}
#else
#endif
#if ETHR_HAVE_GCC_ASM_ARM_DMB_INSTRUCTION
static __inline__ __attribute__((__always_inline__)) void
ethr_full_fence__(void)
{
    __asm__ __volatile__("dmb sy" : : : "memory");
}
#if ETHR_HAVE_GCC_ASM_ARM_DMB_ST_INSTRUCTION
static __inline__ __attribute__((__always_inline__)) void
ethr_store_fence__(void)
{
    __asm__ __volatile__("dmb st" : : : "memory");
}
#endif
#if ETHR_HAVE_GCC_ASM_ARM_DMB_LD_INSTRUCTION
static __inline__ __attribute__((__always_inline__)) void
ethr_load_fence__(void)
{
    __asm__ __volatile__("dmb ld" : : : "memory");
}
#endif
#if ETHR_HAVE_GCC_ASM_ARM_DMB_ST_INSTRUCTION && ETHR_HAVE_GCC_ASM_ARM_DMB_LD_INSTRUCTION
#define ETHR_MEMBAR(B)                                                  \
    ETHR_CHOOSE_EXPR((B) == ETHR_StoreStore,                            \
                     ethr_store_fence__(),                              \
                     ETHR_CHOOSE_EXPR((B) & (ETHR_StoreStore            \
                                             | ETHR_StoreLoad),         \
                                      ethr_full_fence__(),              \
                                      ethr_load_fence__()))
#elif ETHR_HAVE_GCC_ASM_ARM_DMB_ST_INSTRUCTION
#define ETHR_MEMBAR(B)                                                  \
    ETHR_CHOOSE_EXPR((B) == ETHR_StoreStore,                            \
                     ethr_store_fence__(), \
                     ethr_full_fence__())
#elif ETHR_HAVE_GCC_ASM_ARM_DMB_LD_INSTRUCTION
#define ETHR_MEMBAR(B)                                                  \
    ETHR_CHOOSE_EXPR((B) & (ETHR_StoreStore                             \
                            | ETHR_StoreLoad),                          \
                     ethr_full_fence__(),                               \
                     ethr_load_fence__())
#else
#define ETHR_MEMBAR(B)                                                  \
 ethr_full_fence__()
#endif
#elif ETHR_HAVE___sync_synchronize
static __inline__ __attribute__((__always_inline__)) void
ethr_full_fence__(void)
{
    ETHR_COMPILER_BARRIER;
    __sync_synchronize();
    ETHR_COMPILER_BARRIER;
}
#else
#if (ETHR_HAVE___sync_val_compare_and_swap & 4)
#  define ETHR_MB_T__ ethr_sint32_t
#elif (ETHR_HAVE___sync_val_compare_and_swap & 8)
#  define ETHR_MB_T__ ethr_sint64_t
#endif
static __inline__ __attribute__((__always_inline__)) void
ethr_full_fence__(void)
{
    volatile ETHR_MB_T__ x = 0;
    (void) __sync_val_compare_and_swap(&x, (ETHR_MB_T__) 0, (ETHR_MB_T__) 1);
}
#endif
#ifndef ETHR_MEMBAR
#  define ETHR_MEMBAR(B) ethr_full_fence__()
#endif
#if !defined(__ia64__) && !defined(__arm__) && !defined(__arm64__) \
    && !defined(__aarch32__) && !defined(__aarch64__)
#  define ETHR_READ_DEPEND_MEMORY_BARRIER ETHR_MEMBAR(ETHR_LoadLoad)
#endif
#endif