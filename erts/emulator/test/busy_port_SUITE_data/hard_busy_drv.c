#define ERTS_TEST_BUSY_DRV_NAME "hard_busy_drv"
#define ERTS_TEST_BUSY_DRV_FLAGS  \
  ERL_DRV_FLAG_USE_PORT_LOCKING
#include "hs_busy_drv.c"