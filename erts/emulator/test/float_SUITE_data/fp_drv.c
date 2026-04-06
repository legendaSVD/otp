#if defined(DEBUG) || 0
#  include <stdio.h>
#  define PRINTF(X) printf X
#else
#  define PRINTF(X)
#endif
#include <string.h>
#include <math.h>
#ifdef __WIN32__
#include <float.h>
#if defined (__GNUC__)
int _finite(double x);
#endif
#ifndef isfinite
#define isfinite _finite
#endif
#elif !defined(HAVE_ISFINITE) && defined(HAVE_FINITE)
#define isfinite finite
#elif !defined(HAVE_ISFINITE)
# error "No finite function found!"
#endif
#include "erl_driver.h"
#define ERTS_FP_CONTROL_TEST 0
#define ERTS_FP_THREAD_TEST 1
static ErlDrvSSizeT control(ErlDrvData, unsigned int, char *,
			    ErlDrvSizeT, char **, ErlDrvSizeT);
static ErlDrvEntry fp_drv_entry = {
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    "fp_drv",
    NULL ,
    NULL ,
    control,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL ,
    NULL
};
DRIVER_INIT(fp_drv)
{
    return &fp_drv_entry;
}
void *
do_test(void *unused)
{
    double x, y, z;
    x = 3.23e133;
    y = 3.57e257;
    z = x*y;
    if (isfinite(z))
	return "is finite (1)";
    x = 5.0;
    y = 0.0;
    z = x/y;
    if (isfinite(z))
	return "is finite (2)";
    z = log(-1.0);
    if (isfinite(z))
	return "is finite (3)";
    z = log(0.0);
    if (isfinite(z))
	return "is finite (4)";
    return "ok";
}
static ErlDrvSSizeT control(ErlDrvData drv_data,
			    unsigned int command,
			    char *buf, ErlDrvSizeT len,
			    char **rbuf, ErlDrvSizeT rlen)
{
    char *res_str;
    PRINTF(("control(%p, %d, ...) called\r\n", drv_data, command));
    switch (command) {
    case ERTS_FP_THREAD_TEST: {
	ErlDrvTid tid;
	ErlDrvSysInfo info;
	driver_system_info(&info, sizeof(ErlDrvSysInfo));
	if (!info.thread_support)
	    res_str = "skip: no thread support";
	else if (0 != erl_drv_thread_create("test", &tid, do_test, NULL, NULL))
	    res_str = "failed to create thread";
	else if (0 != erl_drv_thread_join(tid, (void**)&res_str))
	    res_str = "failed to join thread";
	break;
    }
    case ERTS_FP_CONTROL_TEST:
	res_str = do_test(NULL);
	break;
    default:
	res_str = "unknown command";
	break;
    }
 done: {
	int res_len = strlen(res_str);
	if (res_len > rlen) {
	    char *abuf = driver_alloc(sizeof(char)*res_len);
	    if (!abuf)
		return 0;
	    *rbuf = abuf;
	}
	memcpy((void *) *rbuf, (void *) res_str, res_len);
	return res_len;
    }
}