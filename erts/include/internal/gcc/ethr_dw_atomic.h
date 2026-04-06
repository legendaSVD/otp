#undef ETHR_INCLUDE_DW_ATOMIC_IMPL__
#if !defined(ETHR_GCC_ATOMIC_DW_ATOMIC_H__)				\
    && ((ETHR_HAVE___sync_val_compare_and_swap & (2*ETHR_SIZEOF_PTR))	\
	|| (ETHR_HAVE___atomic_compare_exchange_n & (2*ETHR_SIZEOF_PTR)))
#  define ETHR_GCC_ATOMIC_DW_ATOMIC_H__
#  define ETHR_INCLUDE_DW_ATOMIC_IMPL__
#endif
#ifdef ETHR_INCLUDE_DW_ATOMIC_IMPL__
#  define ETHR_HAVE_NATIVE_SU_DW_ATOMIC
#if ((ETHR_HAVE___sync_val_compare_and_swap & (2*ETHR_SIZEOF_PTR))	\
     && (ETHR_HAVE___atomic_compare_exchange_n & (2*ETHR_SIZEOF_PTR)))
#  define ETHR_NATIVE_DW_ATOMIC_IMPL "gcc_atomic_and_sync_builtins"
#elif (ETHR_HAVE___atomic_compare_exchange_n & (2*ETHR_SIZEOF_PTR))
#  define ETHR_NATIVE_DW_ATOMIC_IMPL "gcc_atomic_builtins"
#elif (ETHR_HAVE___sync_val_compare_and_swap & (2*ETHR_SIZEOF_PTR))
#  define ETHR_NATIVE_DW_ATOMIC_IMPL "gcc_sync_builtins"
#else
#  error "!?"
#endif
#  if ETHR_SIZEOF_PTR == 4
#    define ETHR_DW_NATMC_ALIGN_MASK__ 0x7
#    define ETHR_NATIVE_SU_DW_SINT_T ethr_sint64_t
#  elif ETHR_SIZEOF_PTR == 8
#    define ETHR_DW_NATMC_ALIGN_MASK__ 0xf
#    define ETHR_NATIVE_SU_DW_SINT_T ethr_sint128_t
#  endif
typedef volatile ETHR_NATIVE_SU_DW_SINT_T * ethr_native_dw_ptr_t;
#define ETHR_DW_NATMC_MEM__(VAR) \
   (&(VAR)->c[(int) ((ethr_uint_t) &(VAR)->c[0]) & ETHR_DW_NATMC_ALIGN_MASK__])
typedef union {
    volatile ETHR_NATIVE_SU_DW_SINT_T dw_sint;
    volatile ethr_sint_t sint[3];
    volatile char c[ETHR_SIZEOF_PTR*3];
} ethr_native_dw_atomic_t;
#if defined(ETHR_TRY_INLINE_FUNCS) || defined(ETHR_ATOMIC_IMPL__)
#  ifdef ETHR_DEBUG
#    define ETHR_DW_DBG_ALIGNED__(PTR) \
       ETHR_ASSERT((((ethr_uint_t) (PTR)) & ETHR_DW_NATMC_ALIGN_MASK__) == 0);
#  else
#    define ETHR_DW_DBG_ALIGNED__(PTR)
#  endif
#define ETHR_HAVE_ETHR_NATIVE_DW_ATOMIC_ADDR 1
static ETHR_INLINE ethr_sint_t *
ethr_native_dw_atomic_addr(ethr_native_dw_atomic_t *var)
{
    return (ethr_sint_t *) ETHR_DW_NATMC_MEM__(var);
}
#if (ETHR_HAVE___atomic_store_n & (2*ETHR_SIZEOF_PTR))
#if (ETHR_GCC_RELAXED_VERSIONS__ & (2*ETHR_SIZEOF_PTR))
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_SET 1
static ETHR_INLINE void
ethr_native_su_dw_atomic_set(ethr_native_dw_atomic_t *var,
			     ETHR_NATIVE_SU_DW_SINT_T value)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    __atomic_store_n(p, value, __ATOMIC_RELAXED);
}
#endif
#if (ETHR_GCC_RELB_VERSIONS__ & (2*ETHR_SIZEOF_PTR))
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_SET_RELB 1
static ETHR_INLINE void
ethr_native_su_dw_atomic_set_relb(ethr_native_dw_atomic_t *var,
				  ETHR_NATIVE_SU_DW_SINT_T value)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    __atomic_store_n(p, value, __ATOMIC_RELEASE);
}
#endif
#endif
#if (ETHR_HAVE___atomic_load_n & (2*ETHR_SIZEOF_PTR))
#if (ETHR_GCC_RELAXED_VERSIONS__ & (2*ETHR_SIZEOF_PTR))
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_READ 1
static ETHR_INLINE ETHR_NATIVE_SU_DW_SINT_T
ethr_native_su_dw_atomic_read(ethr_native_dw_atomic_t *var)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return __atomic_load_n(p, __ATOMIC_RELAXED);
}
#endif
#if ((ETHR_GCC_ACQB_VERSIONS__ & (2*ETHR_SIZEOF_PTR))	\
     & ~ETHR___atomic_load_ACQUIRE_barrier_bug)
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_READ_ACQB 1
static ETHR_INLINE ETHR_NATIVE_SU_DW_SINT_T
ethr_native_su_dw_atomic_read_acqb(ethr_native_dw_atomic_t *var)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return __atomic_load_n(p, __ATOMIC_ACQUIRE);
}
#endif
#endif
#if (ETHR_HAVE___atomic_compare_exchange_n & (2*ETHR_SIZEOF_PTR))
#if (ETHR_GCC_RELAXED_MOD_VERSIONS__ & (2*ETHR_SIZEOF_PTR))
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_CMPXCHG 1
static ETHR_INLINE ETHR_NATIVE_SU_DW_SINT_T
ethr_native_su_dw_atomic_cmpxchg(ethr_native_dw_atomic_t *var,
				 ETHR_NATIVE_SU_DW_SINT_T new_val,
				 ETHR_NATIVE_SU_DW_SINT_T exp)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_NATIVE_SU_DW_SINT_T xchg = exp;
    ETHR_DW_DBG_ALIGNED__(p);
    if (__atomic_compare_exchange_n(p,
				    &xchg,
				    new_val,
				    0,
				    __ATOMIC_RELAXED,
				    __ATOMIC_RELAXED))
	return exp;
    return xchg;
}
#endif
#if (ETHR_GCC_ACQB_MOD_VERSIONS__ & (2*ETHR_SIZEOF_PTR))
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_CMPXCHG_ACQB 1
static ETHR_INLINE ETHR_NATIVE_SU_DW_SINT_T
ethr_native_su_dw_atomic_cmpxchg_acqb(ethr_native_dw_atomic_t *var,
				      ETHR_NATIVE_SU_DW_SINT_T new_val,
				      ETHR_NATIVE_SU_DW_SINT_T exp)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_NATIVE_SU_DW_SINT_T xchg = exp;
    ETHR_DW_DBG_ALIGNED__(p);
    if (__atomic_compare_exchange_n(p,
				    &xchg,
				    new_val,
				    0,
				    __ATOMIC_ACQUIRE,
				    __ATOMIC_ACQUIRE))
	return exp;
    return xchg;
}
#endif
#endif
#if ((ETHR_HAVE___sync_val_compare_and_swap & (2*ETHR_SIZEOF_PTR)) \
     & ETHR_GCC_MB_MOD_VERSIONS__)
#define ETHR_HAVE_ETHR_NATIVE_SU_DW_ATOMIC_CMPXCHG_MB 1
static ETHR_INLINE ETHR_NATIVE_SU_DW_SINT_T
ethr_native_su_dw_atomic_cmpxchg_mb(ethr_native_dw_atomic_t *var,
				    ETHR_NATIVE_SU_DW_SINT_T new_val,
				    ETHR_NATIVE_SU_DW_SINT_T old)
{
    ethr_native_dw_ptr_t p = (ethr_native_dw_ptr_t) ETHR_DW_NATMC_MEM__(var);
    ETHR_DW_DBG_ALIGNED__(p);
    return __sync_val_compare_and_swap(p, old, new_val);
}
#endif
#endif
#endif