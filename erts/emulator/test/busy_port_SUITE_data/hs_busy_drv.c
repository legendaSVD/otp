#include <errno.h>
#include "erl_driver.h"
ErlDrvData start(ErlDrvPort port, char *command);
void output(ErlDrvData drv_data, char *buf, ErlDrvSizeT len);
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen);
static ErlDrvEntry busy_drv_entry = {
    NULL ,
    start,
    NULL ,
    output,
    NULL ,
    NULL ,
    ERTS_TEST_BUSY_DRV_NAME,
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
    ERTS_TEST_BUSY_DRV_FLAGS,
    NULL ,
    NULL ,
    NULL
};
DRIVER_INIT(busy_drv)
{
    return &busy_drv_entry;
}
ErlDrvData start(ErlDrvPort port, char *command)
{
    return (ErlDrvData) port;
}
void output(ErlDrvData drv_data, char *buf, ErlDrvSizeT len)
{
    int res;
    ErlDrvPort port = (ErlDrvPort) drv_data;
    ErlDrvTermData msg[] = {
	ERL_DRV_PORT,	driver_mk_port(port),
	ERL_DRV_ATOM,	driver_mk_atom("caller"),
	ERL_DRV_PID,	driver_caller(port),
	ERL_DRV_TUPLE,	(ErlDrvTermData) 3
    };
    res = erl_drv_output_term(driver_mk_port(port), msg, sizeof(msg)/sizeof(ErlDrvTermData));
    if (res <= 0)
	driver_failure_atom(port, "erl_drv_output_term failed");
}
ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
		     ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen)
{
    switch (command) {
    case 'B':
	set_busy_port((ErlDrvPort) drv_data, 1);
	break;
    case 'N':
	set_busy_port((ErlDrvPort) drv_data, 0);
	break;
    default:
	driver_failure_posix((ErlDrvPort) drv_data, EINVAL);
	break;
    }
    return 0;
}