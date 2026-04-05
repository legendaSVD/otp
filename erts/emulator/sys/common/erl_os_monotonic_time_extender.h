#ifndef ERL_OS_MONOTONIC_TIME_EXTENDER_H__
#define ERL_OS_MONOTONIC_TIME_EXTENDER_H__
#include "sys.h"
#include "erl_threads.h"
typedef struct {
    Uint32 (*raw_os_monotonic_time)(void);
    erts_atomic32_t extend[2];
    int check_interval;
} ErtsOsMonotonicTimeExtendState;
#  define ERTS_EXTEND_OS_MONOTONIC_TIME(S, RT)				\
    ((((ErtsMonotonicTime)						\
       erts_atomic32_read_nob(&((S)->extend[((int) ((RT) >> 31)) & 1]))) \
      << 32)								\
     + (RT))
void
erts_init_os_monotonic_time_extender(ErtsOsMonotonicTimeExtendState *statep,
				     Uint32 (*raw_os_monotonic_time)(void),
				     int check_seconds);
void
erts_late_init_os_monotonic_time_extender(ErtsOsMonotonicTimeExtendState *statep);
#endif