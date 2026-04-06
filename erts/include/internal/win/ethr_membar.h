#if (!defined(ETHR_WIN_MEMBAR_H__) \
     && (defined(_MSC_VER) && _MSC_VER >= 1400) \
     && (defined(_M_AMD64) \
	 || defined(_M_IA64) \
	 || defined(ETHR_HAVE__INTERLOCKEDCOMPAREEXCHANGE)))
#define ETHR_WIN_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
#include <intrin.h>
#undef ETHR_COMPILER_BARRIER
#define ETHR_COMPILER_BARRIER _ReadWriteBarrier()
#pragma intrinsic(_ReadWriteBarrier)
#pragma intrinsic(_InterlockedCompareExchange)
#define ETHR_MB_USING_INTERLOCKED__			\
do {							\
    volatile long x___ = 0;				\
    (void) _InterlockedCompareExchange(&x___, 2, 1);	\
} while (0)
#if defined(_M_IA64)
#define ETHR_MEMBAR(B) __mf()
#elif defined(_M_AMD64) || defined(_M_IX86)
#include <immintrin.h>
#include <emmintrin.h>
#include <mmintrin.h>
#ifdef ETHR_X86_RUNTIME_CONF__
#define ETHR_INSTRUCTION_BARRIER ethr_instruction_fence__()
#pragma intrinsic(__cpuid)
static ETHR_FORCE_INLINE void
ethr_instruction_fence__(void)
{
    int ignored[4];
    ETHR_ASSERT(ETHR_X86_RUNTIME_CONF_HAVE_CPUID__);
    __cpuid(ignored, 0);
}
#else
#endif
#if ETHR_SIZEOF_PTR == 4
#  define ETHR_NO_SSE2_MB__ ETHR_MB_USING_INTERLOCKED__
#endif
#pragma intrinsic(_mm_mfence)
#pragma intrinsic(_mm_sfence)
#pragma intrinsic(_mm_lfence)
static ETHR_FORCE_INLINE void
ethr_cfence__(void)
{
    _ReadWriteBarrier();
}
static ETHR_FORCE_INLINE void
ethr_mfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MB__;
    else
#endif
	_mm_mfence();
}
static ETHR_FORCE_INLINE void
ethr_sfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MB__;
    else
#endif
	_mm_sfence();
}
static ETHR_FORCE_INLINE void
ethr_lfence__(void)
{
#if ETHR_SIZEOF_PTR == 4
    if (ETHR_X86_RUNTIME_CONF_HAVE_NO_SSE2__)
	ETHR_NO_SSE2_MB__;
    else
#endif
	_mm_lfence();
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
#else
#define ETHR_MEMBAR(B) ETHR_MB_USING_INTERLOCKED__
#define ETHR_READ_DEPEND_MEMORY_BARRIER ETHR_MB_USING_INTERLOCKED__
#endif
#endif