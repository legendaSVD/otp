#ifndef ETHR_X86_MEMBAR_H__
#define ETHR_X86_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
#ifdef ETHR_X86_RUNTIME_CONF__
#define ETHR_INSTRUCTION_BARRIER ethr_instruction_fence__()
static __inline__ __attribute__((__always_inline__)) void
ethr_instruction_fence__(void)
{
    ETHR_ASSERT(ETHR_X86_RUNTIME_CONF_HAVE_CPUID__);
#if ETHR_SIZEOF_PTR == 4
__asm__ __volatile__("cpuid\n\t" :: "a"(0) : "ebx", "ecx", "edx", "memory");
#else
__asm__ __volatile__("cpuid\n\t" :: "a"(0) : "rbx", "rcx", "rdx", "memory");
#endif
}
#else
#endif
#define ETHR_NO_SSE2_MEMORY_BARRIER__			\
do {							\
    volatile ethr_sint32_t x__ = 0;			\
    __asm__ __volatile__ ("lock; orl $0x0, %0\n\t"	\
			  : "=m"(x__)			\
			  : "m"(x__)			\
			  : "memory");			\
} while (0)
static __inline__ void
ethr_cfence__(void)
{
    __asm__ __volatile__ ("" : : : "memory");
}
static __inline__ void
ethr_mfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MEMORY_BARRIER__;
    else
#endif
	__asm__ __volatile__ ("mfence\n\t" : : : "memory");
}
static __inline__ void
ethr_sfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MEMORY_BARRIER__;
    else
#endif
	__asm__ __volatile__ ("sfence\n\t" : : : "memory");
}
static __inline__ void
ethr_lfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MEMORY_BARRIER__;
    else
#endif
	__asm__ __volatile__ ("lfence\n\t" : : : "memory");
}
#define ETHR_X86_OUT_OF_ORDER_MEMBAR(B)				\
  ETHR_CHOOSE_EXPR((B) == ETHR_StoreStore,			\
		   ethr_sfence__(),				\
		   ETHR_CHOOSE_EXPR((B) == ETHR_LoadLoad,	\
				    ethr_lfence__(),		\
				    ethr_mfence__()))
#ifdef ETHR_X86_OUT_OF_ORDER
#define ETHR_MEMBAR(B) \
  ETHR_X86_OUT_OF_ORDER_MEMBAR((B))
#else
#define ETHR_MEMBAR(B) \
  ETHR_CHOOSE_EXPR((B) & ETHR_StoreLoad, ethr_mfence__(), ethr_cfence__())
#endif
#endif