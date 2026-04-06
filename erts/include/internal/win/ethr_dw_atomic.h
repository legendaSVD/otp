#undef ETHR_INCLUDE_DW_ATOMIC_IMPL__
#ifndef ETHR_X86_DW_ATOMIC_H__
#  define ETHR_X86_DW_ATOMIC_H__
#  if ((ETHR_SIZEOF_PTR == 4 \
        && defined(ETHR_HAVE__INTERLOCKEDCOMPAREEXCHANGE64)) \
       || (ETHR_SIZEOF_PTR == 8 \
           && defined(ETHR_HAVE__INTERLOCKEDCOMPAREEXCHANGE128)))
#    define ETHR_INCLUDE_DW_ATOMIC_IMPL__
#  endif
#endif
#ifdef ETHR_INCLUDE_DW_ATOMIC_IMPL__
#  if ETHR_SIZEOF_PTR == 4
#    define ETHR_HAVE_NATIVE_SU_DW_ATOMIC
#  else
#    define ETHR_HAVE_NATIVE_DW_ATOMIC
#  endif
#  define ETHR_NATIVE_DW_ATOMIC_IMPL "windows-interlocked"
#  if defined(_M_IX86) || defined(_M_AMD64)
#    define ETHR_RTCHK_USE_NATIVE_DW_ATOMIC_IMPL__ \
       ETHR_X86_RUNTIME_CONF_HAVE_DW_CMPXCHG__
#  endif
#  include <intrin.h>
#  if ETHR_SIZEOF_PTR == 4
#    pragma intrinsic(_InterlockedCompareExchange64)
#    define ETHR_DW_NATMC_ALIGN_MASK__ 0x7
#    define ETHR_NATIVE_SU_DW_SINT_T ethr_sint64_t
#  else
#    pragma intrinsic(_InterlockedCompareExchange128)
#    define ETHR_DW_NATMC_ALIGN_MASK__ 0xf
#  endif
typedef volatile __int64 * ethr_native_dw_ptr_t;
#define ETHR_DW_NATMC_MEM__(VAR) \
   (&(VAR)->c[(int) ((ethr_uint_t) &(VAR)->c[0]) & ETHR_DW_NATMC_ALIGN_MASK__])
typedef union {
#ifdef ETHR_NATIVE_SU_DW_SINT_T
    volatile ETHR_NATIVE_SU_DW_SINT_T dw_sint;
#endif
    volatile ethr_sint_t sint[3];
    volatile char c[ETHR_SIZEOF_PTR*3];
} ethr_native_dw_atomic_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_ATOMIC_IMPL__)
#ifdef ETHR_DEBUG
#  define ETHR_DW_DBG_ALIGNED__(PTR) \
     ETHR_ASSERT((((ethr_uint_t) (PTR)) & ETHR_DW_NATMC_ALIGN_MASK__) == 0);
#else
#  define ETHR_DW_DBG_ALIGNED__(PTR)
#endif
#define ETHR_HAVE_ETHR_NATIVE_DW_ATOMIC_ADDR
static ETHR_INLINE ethr_sint_t *
ethr_native_dw_atomic_addr(ethr_native_dw_atomic_t *var)
{
    ethr_sint_t *p = (ethr_sint_t *) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return p;
}
#if ETHR_SIZEOF_PTR == 4
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_CMPXCHG_MB
static ETHR_INLINE ethr_sint64_t
ethr_native_su_dw_atomic_cmpxchg_mb(ethr_native_dw_atomic_t *var,
				    ethr_sint64_t new_value,
				    ethr_sint64_t exp)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return (ethr_sint64_t) _InterlockedCompareExchange64(p, new_value, exp);
}
#elif ETHR_SIZEOF_PTR == 8
#define ETHR_HAVE_ETHR_NATIVE_DW_ATOMIC_CMPXCHG_MB
#ifdef ETHR_BIGENDIAN
#  define ETHR_WIN_LOW_WORD__ 1
#  define ETHR_WIN_HIGH_WORD__ 0
#else
#  define ETHR_WIN_LOW_WORD__ 0
#  define ETHR_WIN_HIGH_WORD__ 1
#endif
static ETHR_INLINE int
ethr_native_dw_atomic_cmpxchg_mb(ethr_native_dw_atomic_t *var,
				 ethr_sint_t *new_value,
				 ethr_sint_t *xchg)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return (int) _InterlockedCompareExchange128(p,
						new_value[ETHR_WIN_HIGH_WORD__],
						new_value[ETHR_WIN_LOW_WORD__],
						xchg);
}
#endif
#endif
#endif