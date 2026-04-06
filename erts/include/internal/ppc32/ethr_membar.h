#ifndef ETHR_PPC_MEMBAR_H__
#define ETHR_PPC_MEMBAR_H__
#define ETHR_LoadLoad	(1 << 0)
#define ETHR_LoadStore	(1 << 1)
#define ETHR_StoreLoad	(1 << 2)
#define ETHR_StoreStore	(1 << 3)
static __inline__ void
ethr_lwsync__(void)
{
#ifdef ETHR_PPC_HAVE_NO_LWSYNC
    __asm__ __volatile__ ("sync\n\t" : : : "memory");
#else
#ifndef ETHR_PPC_HAVE_LWSYNC
    if (ETHR_PPC_RUNTIME_CONF_HAVE_NO_LWSYNC__)
	__asm__ __volatile__ ("sync\n\t" : : : "memory");
    else
#endif
	__asm__ __volatile__ ("lwsync\n\t" : : : "memory");
#endif
}
static __inline__ void
ethr_sync__(void)
{
    __asm__ __volatile__ ("sync\n\t" : : : "memory");
}
#define ETHR_MEMBAR(B) \
  ETHR_CHOOSE_EXPR((B) & ETHR_StoreLoad, ethr_sync__(), ethr_lwsync__())
#endif