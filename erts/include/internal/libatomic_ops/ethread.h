#ifndef ETHREAD_LIBATOMIC_OPS_H__
#define ETHREAD_LIBATOMIC_OPS_H__
#if (defined(ETHR_HAVE_LIBATOMIC_OPS) \
     && ((ETHR_SIZEOF_AO_T == 4 && !defined(ETHR_HAVE_NATIVE_ATOMIC32)) \
	 || (ETHR_SIZEOF_AO_T == 8 && !defined(ETHR_HAVE_NATIVE_ATOMIC64))))
#if defined(__x86_64__)
#define AO_USE_PENTIUM4_INSTRS
#endif
#define ETHR_NATIVE_IMPL__ "libatomic_ops"
#include "atomic_ops.h"
#include "ethr_membar.h"
#include "ethr_atomic.h"
#include "ethr_dw_atomic.h"
#endif
#endif