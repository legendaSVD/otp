#include "erl_driver.h"
static void stop(ErlDrvData drv_data);
static ErlDrvData start(ErlDrvPort port,
			char *command);
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len);
static void flush(ErlDrvData drv_data);
static void timeout(ErlDrvData drv_data);
static void process_exit(ErlDrvData drv_data, ErlDrvMonitor *monitor);
static ErlDrvEntry unlink_signal_entry = {
    NULL ,
    start,
    stop,
    output,
    NULL ,
    NULL ,
    "unlink_signal_drv",
    NULL ,
    NULL ,
    NULL ,
    timeout,
    NULL ,
    NULL ,
    flush,
    NULL ,
    NULL ,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL ,
    process_exit,
    NULL
};
DRIVER_INIT(unlink_signal_entry)
{
    return &unlink_signal_entry;
}
typedef struct {
    ErlDrvPort port;
    int timeout_count;
} us_drv_state;
static void stop(ErlDrvData drv_data)
{
    driver_free((void *) drv_data);
}
static ErlDrvData start(ErlDrvPort port,
			char *command)
{
    us_drv_state *state = (us_drv_state *) driver_alloc(sizeof(us_drv_state));
    state->port = port;
    state->timeout_count = 0;
    return (ErlDrvData) state;
}
static void output(ErlDrvData drv_data,
		   char *buf, ErlDrvSizeT len)
{
    us_drv_state *state = (us_drv_state *) drv_data;
    driver_set_timer(state->port, 2);
}
static void flush(ErlDrvData drv_data)
{
    us_drv_state *state = (us_drv_state *) drv_data;
    driver_set_timer(state->port, 5);
}
static void timeout(ErlDrvData drv_data)
{
    us_drv_state *state = (us_drv_state *) drv_data;
    state->timeout_count++;
    if (state->timeout_count == 1) {
        int i, limit;
        ErlDrvTermData connected = driver_connected(state->port);
        driver_enq(state->port, "x", 1);
        limit = (int) (((unsigned)state->port) % 1000);
        for (i = 0; i < limit; i++) {
            ErlDrvMonitor *monitor = driver_alloc(sizeof(ErlDrvMonitor));
            driver_monitor_process(state->port, connected, monitor);
            driver_demonitor_process(state->port, monitor);
            driver_free(monitor);
        }
        driver_exit(state->port, 0);
    }
    else {
        driver_deq(state->port, 1);
    }
}
static void
process_exit(ErlDrvData drv_data, ErlDrvMonitor *monitor)
{
    driver_free(monitor);
}