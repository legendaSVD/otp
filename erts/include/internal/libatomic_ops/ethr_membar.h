#ifndef ETHR_LIBATOMIC_OPS_MEMBAR_H__
#define ETHR_LIBATOMIC_OPS_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
#ifndef AO_HAVE_nop_full
#  error "No AO_nop_full()"
#endif
static __inline__ void
ethr_mb__(void)
{
    AO_nop_full();
}
static __inline__ void
ethr_rb__(void)
{
#ifdef AO_HAVE_nop_read
    AO_nop_read();
#else
    AO_nop_full();
#endif
}
static __inline__ void
ethr_wb__(void)
{
#ifdef AO_HAVE_nop_write
    AO_nop_write();
#else
    AO_nop_full();
#endif
}
#define ETHR_MEMBAR(B)						\
  ETHR_CHOOSE_EXPR((B) == ETHR_StoreStore,			\
		   ethr_wb__(),					\
		   ETHR_CHOOSE_EXPR((B) == ETHR_LoadLoad,	\
				    ethr_rb__(),		\
				    ethr_mb__()))
#define ETHR_COMPILER_BARRIER AO_compiler_barrier()
#ifdef AO_NO_DD_ORDERING
#  define ETHR_READ_DEPEND_MEMORY_BARRIER ETHR_READ_MEMORY_BARRIER
#endif
#endif