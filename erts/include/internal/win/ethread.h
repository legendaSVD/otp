#ifndef ETHREAD_WIN_H__
#define ETHREAD_WIN_H__
#include "ethr_membar.h"
#define ETHR_ATOMIC_WANT_32BIT_IMPL__
#include "ethr_atomic.h"
#if ETHR_SIZEOF_PTR == 8
#  define ETHR_ATOMIC_WANT_64BIT_IMPL__
#  include "ethr_atomic.h"
#endif
#include "ethr_dw_atomic.h"
#endif