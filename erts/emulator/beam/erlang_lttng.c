#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef USE_LTTNG
#define TRACEPOINT_CREATE_PROBES
#define TRACEPOINT_DEFINE
#include "erlang_lttng.h"
#endif