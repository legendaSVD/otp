#ifndef UNIX
#if !defined(__WIN32__)
#define UNIX 1
#endif
#endif
#if defined(DEBUG) || 0
#  define PRINTF(X) printf X
#else
#  define PRINTF(X)
#endif
#if defined(UNIX)
#include <stdio.h>
#include <string.h>
#elif defined(__WIN32__)
#include <windows.h>
#endif
#ifdef HAVE_UNISTD_H
#   include <unistd.h>
#endif
#include <errno.h>
#include "erl_driver.h"
#define PEEK_NONXQ_TEST 0
#define PEEK_NONXQ_WAIT 1
typedef struct {
    ErlDrvTermData caller;
    ErlDrvPort port;
    int cmd;
} PeekNonXQDrvData;
typedef struct {
    ErlDrvPort port;
    ErlDrvPDL pdl;
} AsyncData;
static ErlDrvData start(ErlDrvPort, char *);
static void stop(ErlDrvData);
static ErlDrvSSizeT control(ErlDrvData, unsigned int,
			    char *, ErlDrvSizeT, char **, ErlDrvSizeT);
static void ready_async(ErlDrvData, ErlDrvThreadData);
static void async_test(void *);
static void async_wait(void *);
static void async_free(void *);
static void do_sleep(unsigned);
static ErlDrvEntry peek_non_existing_queue_drv_entry = {
    NULL ,
    start,
    stop,
    NULL ,
    NULL ,
    NULL ,
    "peek_non_existing_queue_drv",
    NULL ,
    NULL ,
    control,
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
DRIVER_INIT(peek_non_existing_queue_drv)
{
    return &peek_non_existing_queue_drv_entry;
}
static ErlDrvData
start(ErlDrvPort port, char *command)
{
    PeekNonXQDrvData *dp = driver_alloc(sizeof(PeekNonXQDrvData));
    if (!dp) {
	errno = ENOMEM;
	return ERL_DRV_ERROR_ERRNO;
    }
    dp->port = port;
    return (ErlDrvData) dp;
}
static void stop(ErlDrvData drv_data)
{
    driver_free(drv_data);
}
static ErlDrvSSizeT control(ErlDrvData drv_data,
			    unsigned int command,
			    char *buf, ErlDrvSizeT len,
			    char **rbuf, ErlDrvSizeT rlen)
{
    PeekNonXQDrvData *dp = (PeekNonXQDrvData *) drv_data;
    unsigned int key = 0;
    char *res_str = "ok";
    ErlDrvSysInfo si;
    driver_system_info(&si, sizeof(ErlDrvSysInfo));
    if (si.async_threads == 0) {
	res_str = "skipped: No async-threads available";
	goto done;
    }
    dp->cmd = command;
    dp->caller = driver_caller(dp->port);
    switch (command) {
    case PEEK_NONXQ_TEST: {
	AsyncData *adp = driver_alloc(sizeof(AsyncData));
	if (!adp) {
	    res_str = "enomem";
	    goto done;
	}
	driver_enq(dp->port, "!", 1);
	adp->port = dp->port;
	adp->pdl = driver_pdl_create(dp->port);
	(void) driver_async(dp->port, &key, async_test, adp, async_free);
	break;
    }
    case PEEK_NONXQ_WAIT:
	(void) driver_async(dp->port, &key, async_wait, NULL, NULL);
	break;
    }
 done: {
	ErlDrvSSizeT res_len = strlen(res_str);
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
static void ready_async(ErlDrvData drv_data, ErlDrvThreadData thread_data)
{
    PeekNonXQDrvData *dp = (PeekNonXQDrvData *) drv_data;
    if (dp->cmd == PEEK_NONXQ_WAIT) {
	ErlDrvTermData port_id = driver_mk_port(dp->port);
	ErlDrvTermData spec[] = {
	    ERL_DRV_PORT, port_id,
	    ERL_DRV_ATOM, driver_mk_atom("test_successful"),
	    ERL_DRV_TUPLE, 2
	};
	erl_drv_send_term(port_id,
			  dp->caller,
			  spec,
			  sizeof(spec) / sizeof(spec[0]));
    }
    if (thread_data)
	driver_free(thread_data);
}
static void async_test(void *vadp)
{
    SysIOVec *vec;
    int vlen = 4711;
    AsyncData *adp = (AsyncData *)vadp;
    do_sleep(1);
    driver_pdl_lock(adp->pdl);
    vec = driver_peekq(adp->port, &vlen);
    if (vlen >= 0 || vec)
	abort();
    vlen = driver_sizeq(adp->port);
    if (vlen >= 0)
	abort();
    driver_pdl_unlock(adp->pdl);
}
static void async_wait(void *vadp)
{
}
static void async_free(void *vadp)
{
    driver_free(vadp);
}
static void
do_sleep(unsigned secs)
{
#ifdef __WIN32__
	Sleep((DWORD) secs*1000);
#else
	sleep(secs);
#endif
}