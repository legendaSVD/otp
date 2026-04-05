#include "erl_driver.h"
#define NO_ASYNC_JOBS 10000
static void stop(ErlDrvData drv_data);
static ErlDrvData start(ErlDrvPort port,
			char *command);
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len);
static void ready_async(ErlDrvData drv_data,
			ErlDrvThreadData thread_data);
static ErlDrvEntry async_blast_drv_entry = {
    NULL ,
    start,
    stop,
    output,
    NULL ,
    NULL ,
    "async_blast_drv",
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    NULL ,
    ready_async,
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
typedef struct {
    ErlDrvPort port;
    ErlDrvTermData port_id;
    ErlDrvTermData caller;
    int counter;
} async_blast_data_t;
DRIVER_INIT(async_blast_drv)
{
    return &async_blast_drv_entry;
}
static void stop(ErlDrvData drv_data)
{
    driver_free((void *) drv_data);
}
static ErlDrvData start(ErlDrvPort port,
			char *command)
{
    async_blast_data_t *abd;
    abd = driver_alloc(sizeof(async_blast_data_t));
    if (!abd)
	return ERL_DRV_ERROR_GENERAL;
    abd->port = port;
    abd->port_id = driver_mk_port(port);
    abd->counter = 0;
    return (ErlDrvData) abd;
}
static void async_invoke(void *data)
{
}
#include <stdio.h>
static void ready_async(ErlDrvData drv_data,
			ErlDrvThreadData thread_data)
{
    async_blast_data_t *abd = (async_blast_data_t *) drv_data;
    if (--abd->counter == 0) {
	ErlDrvTermData spec[] = {
	    ERL_DRV_PORT, abd->port_id,
	    ERL_DRV_ATOM, driver_mk_atom("done"),
	    ERL_DRV_TUPLE, 2
	};
	erl_drv_send_term(abd->port_id, abd->caller,
			  spec, sizeof(spec)/sizeof(spec[0]));
    }
}
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len)
{
    async_blast_data_t *abd = (async_blast_data_t *) drv_data;
    if (abd->counter == 0) {
	int i;
	abd->caller = driver_caller(abd->port);
	abd->counter = NO_ASYNC_JOBS;
	for (i = 0; i < NO_ASYNC_JOBS; i++) {
	    if (0 > driver_async(abd->port, NULL, async_invoke, NULL, NULL)) {
		driver_failure_atom(abd->port, "driver_async_failed");
		break;
	    }
	}
    }
}