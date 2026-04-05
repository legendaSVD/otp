#define VSN_MISMATCH_DRV_NAME_STR		"smaller_major_vsn_drv"
#define VSN_MISMATCH_DRV_NAME			smaller_major_vsn_drv
#define VSN_MISMATCH_DRV_MAJOR_VSN_DIFF		(ERL_DRV_MIN_REQUIRED_MAJOR_VERSION_ON_LOAD - ERL_DRV_EXTENDED_MAJOR_VERSION - 1)
#define VSN_MISMATCH_DRV_MINOR_VSN_DIFF		0
#include "vsn_mismatch_drv_impl.c"