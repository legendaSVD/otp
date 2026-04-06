#ifndef ETHREAD_SPARC32_ETHREAD_H
#define ETHREAD_SPARC32_ETHREAD_H
#include "ethr_membar.h"
#define ETHR_ATOMIC_WANT_32BIT_IMPL__
#include "atomic.h"
#if ETHR_SIZEOF_PTR == 8
#  define ETHR_ATOMIC_WANT_64BIT_IMPL__
#  include "atomic.h"
#endif
#include "spinlock.h"
#include "rwlock.h"
#endif