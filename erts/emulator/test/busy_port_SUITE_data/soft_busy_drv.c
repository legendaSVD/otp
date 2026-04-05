#define ERTS_TEST_BUSY_DRV_NAME "soft_busy_drv"
#define ERTS_TEST_BUSY_DRV_FLAGS  \
  (ERL_DRV_FLAG_USE_PORT_LOCKING|ERL_DRV_FLAG_SOFT_BUSY)
#include "hs_busy_drv.c"