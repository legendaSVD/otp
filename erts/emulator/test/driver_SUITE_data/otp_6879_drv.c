#include <stdio.h>
#include <string.h>
#include "erl_driver.h"
static ErlDrvSSizeT call(ErlDrvData drv_data,
			 unsigned int command,
			 char *buf, ErlDrvSizeT len,
			 char **rbuf, ErlDrvSizeT rlen,
			 unsigned int *flags);
static ErlDrvEntry otp_6879_drv_entry = {
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    "otp_6879_drv",
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    call,
    NULL ,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL ,
    NULL
};
DRIVER_INIT(otp_6879_drv)
{
    return &otp_6879_drv_entry;
}
static ErlDrvSSizeT call(ErlDrvData drv_data,
			 unsigned int command,
			 char *buf, ErlDrvSizeT len,
			 char **rbuf, ErlDrvSizeT rlen,
			 unsigned int *flags)
{
    if (len > rlen)
	*rbuf = driver_alloc(len);
    memcpy((void *) *rbuf, (void *) buf, len);
    return len;
}