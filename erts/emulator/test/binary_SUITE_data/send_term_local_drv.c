#include "erl_driver.h"
#include <string.h>
static void stop(ErlDrvData drv_data);
static ErlDrvData start(ErlDrvPort port,
			char *command);
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len);
static ErlDrvEntry send_term_local_drv_entry = {
    NULL ,
    start,
    stop,
    output,
    NULL ,
    NULL ,
    "send_term_local_drv",
    NULL ,
    NULL ,
    NULL ,
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
DRIVER_INIT(send_term_local_drv)
{
    return &send_term_local_drv_entry;
}
static void stop(ErlDrvData drv_data)
{
}
static ErlDrvData start(ErlDrvPort port,
			char *command)
{
    return (ErlDrvData) port;
}
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len)
{
    ErlDrvPort port = (ErlDrvPort) drv_data;
    ErlDrvTermData term_port = driver_mk_port(port);
    ErlDrvTermData caller = driver_caller(port);
    int res;
    ErlDrvTermData spec[] = {
	ERL_DRV_PORT, term_port,
	ERL_DRV_EXT2TERM, (ErlDrvTermData) buf, len,
	ERL_DRV_TUPLE, 2
    };
    if (0 >= erl_drv_send_term(term_port, caller,
                               spec, sizeof(spec)/sizeof(spec[0]))) {
        char *bad_term = "bad_term_error";
        ErlDrvTermData spec[] = {
            ERL_DRV_PORT, term_port,
            ERL_DRV_STRING, (ErlDrvTermData) bad_term, strlen(bad_term),
            ERL_DRV_TUPLE, 2
        };
        if (0 >= erl_drv_send_term(term_port, caller, spec,
                                   sizeof(spec)/sizeof(spec[0]))) {
            driver_failure_atom(port, "failed_to_bad_term_error");
        }
    }
}