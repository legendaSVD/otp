#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "erl_driver.h"
#define THR_MSG_BLAST_NO_PROCS 10
#define THR_MSG_BLAST_NO_SENDS_PER_PROC 10000
#define THR_MSG_BLAST_THREADS 32
static void stop(ErlDrvData drv_data);
static ErlDrvData start(ErlDrvPort port,
			char *command);
static ErlDrvSSizeT control(ErlDrvData drv_data,
			    unsigned int command,
			    char *buf, ErlDrvSizeT len,
			    char **rbuf, ErlDrvSizeT rlen);
static ErlDrvEntry thr_msg_blast_drv_entry = {
    NULL ,
    start,
    stop,
    NULL ,
    NULL ,
    NULL ,
    "thr_msg_blast_drv",
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
typedef struct {
    ErlDrvPort port;
    ErlDrvTermData td_port;
    ErlDrvTermData hi;
    ErlDrvTid tid[THR_MSG_BLAST_THREADS];
    int no_thrs;
    ErlDrvTermData proc[THR_MSG_BLAST_NO_PROCS];
    int no_procs;
} thr_msg_blast_data_t;
DRIVER_INIT(thr_msg_blast_drv)
{
    return &thr_msg_blast_drv_entry;
}
static void stop(ErlDrvData drv_data)
{
    int i;
    thr_msg_blast_data_t *tmbd = (thr_msg_blast_data_t *) drv_data;
    for (i = 0; i < tmbd->no_thrs; i++)
	erl_drv_thread_join(tmbd->tid[i], NULL);
    driver_free((void *) tmbd);
}
static ErlDrvData start(ErlDrvPort port,
			char *command)
{
    thr_msg_blast_data_t *tmbd;
    tmbd = driver_alloc(sizeof(thr_msg_blast_data_t));
    if (!tmbd)
	return ERL_DRV_ERROR_GENERAL;
    tmbd->port = port;
    tmbd->td_port = driver_mk_port(port);
    tmbd->hi = driver_mk_atom("hi");
    tmbd->no_thrs = 0;
    tmbd->no_procs = 1;
    tmbd->proc[0] = driver_caller(port);
    return (ErlDrvData) tmbd;
}
static void *thread(void *);
static ErlDrvSSizeT control(ErlDrvData drv_data,
			    unsigned int command,
			    char *buf, ErlDrvSizeT len,
			    char **rbuf, ErlDrvSizeT rlen)
{
    thr_msg_blast_data_t *tmbd = (thr_msg_blast_data_t *) drv_data;
    char *res_str = "error";
    if (tmbd->no_procs >= THR_MSG_BLAST_NO_PROCS) {
	int i;
	for (i = 0; i < tmbd->no_thrs; i++)
	    erl_drv_thread_join(tmbd->tid[i], NULL);
	tmbd->no_thrs = 0;
	res_str = "done";
    }
    else {
	tmbd->proc[tmbd->no_procs++] = driver_caller(tmbd->port);
	if (tmbd->no_procs == THR_MSG_BLAST_NO_PROCS) {
	    for (tmbd->no_thrs = 0;
		 tmbd->no_thrs < THR_MSG_BLAST_THREADS;
		 tmbd->no_thrs++) {
		int res = erl_drv_thread_create("test",
						&tmbd->tid[tmbd->no_thrs],
						thread,
						tmbd,
						NULL);
		if (res != 0) {
		    driver_failure_posix(tmbd->port, res);
		    goto done;
		}
	    }
	}
	res_str = "receiver";
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
static void *thread(void *varg)
{
    int s, p;
    thr_msg_blast_data_t *tmbd = (thr_msg_blast_data_t *) varg;
    ErlDrvTermData spec[] = {
	ERL_DRV_PORT, tmbd->td_port,
	ERL_DRV_ATOM, tmbd->hi,
	ERL_DRV_TUPLE, 2
    };
    for (s = 0; s < THR_MSG_BLAST_NO_SENDS_PER_PROC; s++) {
	for (p = 0; p < THR_MSG_BLAST_NO_PROCS; p++) {
	    int res = erl_drv_send_term(tmbd->td_port, tmbd->proc[p],
					spec, sizeof(spec)/sizeof(spec[0]));
	    if (p == 0 && res <= 0)
		abort();
	}
    }
    return NULL;
}