#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "erl_driver.h"
ErlDrvData start(ErlDrvPort port, char *command);
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen);
static ErlDrvEntry thr_alloc_drv_entry = {
    NULL ,
    start,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    "thr_alloc_drv",
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
DRIVER_INIT(thr_alloc_drv)
{
    return &thr_alloc_drv_entry;
}
void *
test_thread(void *vsize)
{
    int i;
    int size = (int) (ErlDrvSInt) vsize;
    void *mem;
    mem = driver_alloc(size);
    if (mem)
	driver_free(mem);
}
ErlDrvData start(ErlDrvPort port, char *command)
{
    return (ErlDrvData) port;
}
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen)
{
    ErlDrvPort port = (ErlDrvPort) drv_data;
    char *result = "failure";
    ErlDrvSSizeT result_len;
    if (len <= 20) {
	int res;
	ErlDrvTid tid;
	char ibuf[21];
	int size;
	memcpy((void *) ibuf, buf, len);
	ibuf[len] = '\0';
	size = atoi(ibuf);
	if (size > 0) {
	    res = erl_drv_thread_create("test_thread",
					&tid,
					test_thread,
					(void *) (ErlDrvSInt) size,
					NULL);
	    if (res == 0) {
		res = erl_drv_thread_join(tid, NULL);
		if (res == 0)
		    result = "ok";
	    }
	    if (res != 0)
		driver_failure_posix(port, res);
	}
    }
    result_len = strlen(result);
    if (result_len <= rlen) {
	memcpy(*rbuf, result, result_len);
	return result_len;
    }
    else {
	*rbuf = driver_alloc(result_len);
	if (!*rbuf) {
	    driver_failure_posix(port, ENOMEM);
	    return 0;
	}
	else {
	    memcpy(*rbuf, result, result_len);
	    return result_len;
	}
    }
}