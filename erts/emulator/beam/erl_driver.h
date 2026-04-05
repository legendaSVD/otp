#ifndef __ERL_DRIVER_H__
#define __ERL_DRIVER_H__
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#define ERL_DRV_DEPRECATED_FUNC
#ifdef __GNUC__
#  if __GNUC__ >= 3
#    undef ERL_DRV_DEPRECATED_FUNC
#    define ERL_DRV_DEPRECATED_FUNC __attribute__((deprecated))
#  endif
#endif
#include "erl_drv_nif.h"
#include <stdlib.h>
#if defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_)
#  ifndef STATIC_ERLANG_DRIVER
#    define ERL_DRIVER_TYPES_ONLY
#    define WIN32_DYNAMIC_ERL_DRIVER
#  endif
#  define EXTERN extern
#else
#  define EXTERN ERL_NAPI_EXPORT extern
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define ERL_DRV_READ  ((int)ERL_NIF_SELECT_READ)
#define ERL_DRV_WRITE ((int)ERL_NIF_SELECT_WRITE)
#define ERL_DRV_USE   ((int)ERL_NIF_SELECT_STOP)
#define ERL_DRV_USE_NO_CALLBACK (ERL_DRV_USE | (ERL_DRV_USE  << 1))
#define DO_READ  ERL_DRV_READ
#define DO_WRITE ERL_DRV_WRITE
#define ERL_DRV_EXTENDED_MARKER		(0xfeeeeeed)
#define ERL_DRV_EXTENDED_MAJOR_VERSION	3
#define ERL_DRV_EXTENDED_MINOR_VERSION	3
#define ERL_DRV_MIN_REQUIRED_MAJOR_VERSION_ON_LOAD 2
#define PORT_CONTROL_FLAG_BINARY	(1 << 0)
#define PORT_CONTROL_FLAG_HEAVY		(1 << 1)
#define PORT_FLAG_BINARY                (1 << 0)
#define PORT_FLAG_LINE                  (1 << 1)
#define ERL_DRV_FLAG_USE_PORT_LOCKING	(1 << 0)
#define ERL_DRV_FLAG_SOFT_BUSY		(1 << 1)
#define ERL_DRV_FLAG_NO_BUSY_MSGQ	(1 << 2)
#define ERL_DRV_FLAG_USE_INIT_ACK	(1 << 3)
typedef ErlNapiUInt64 ErlDrvUInt64;
typedef ErlNapiSInt64 ErlDrvSInt64;
typedef ErlNapiUInt ErlDrvUInt;
typedef ErlNapiSInt ErlDrvSInt;
typedef ErlNapiUInt ErlDrvTermData;
#if defined(__WIN32__) || defined(_WIN32)
typedef ErlDrvUInt ErlDrvSizeT;
typedef ErlDrvSInt ErlDrvSSizeT;
#else
typedef size_t ErlDrvSizeT;
typedef ssize_t ErlDrvSSizeT;
#endif
typedef struct erl_drv_binary {
    ErlDrvSInt orig_size;
    char orig_bytes[1];
} ErlDrvBinary;
typedef struct _erl_drv_data* ErlDrvData;
#ifndef ERL_SYS_DRV
typedef struct _erl_drv_event* ErlDrvEvent;
#endif
typedef struct _erl_drv_port* ErlDrvPort;
typedef struct _erl_drv_port* ErlDrvThreadData;
typedef struct {
    unsigned long megasecs;
    unsigned long secs;
    unsigned long microsecs;
} ErlDrvNowData;
typedef ErlDrvSInt64 ErlDrvTime;
#define ERL_DRV_TIME_ERROR ((ErlDrvSInt64) ERTS_NAPI_TIME_ERROR__)
typedef enum {
    ERL_DRV_SEC = ERTS_NAPI_SEC__,
    ERL_DRV_MSEC = ERTS_NAPI_MSEC__,
    ERL_DRV_USEC = ERTS_NAPI_USEC__,
    ERL_DRV_NSEC = ERTS_NAPI_NSEC__
} ErlDrvTimeUnit;
#define ERL_DRV_ERROR_GENERAL ((ErlDrvData) -1)
#define ERL_DRV_ERROR_ERRNO ((ErlDrvData) -2)
#define ERL_DRV_ERROR_BADARG ((ErlDrvData) -3)
typedef struct erl_io_vec {
    int vsize;
    ErlDrvSizeT size;
    SysIOVec* iov;
    ErlDrvBinary** binv;
} ErlIOVec;
typedef struct ErlDrvTid_ *ErlDrvTid;
typedef struct ErlDrvMutex_ ErlDrvMutex;
typedef struct ErlDrvCond_ ErlDrvCond;
typedef struct ErlDrvRWLock_ ErlDrvRWLock;
typedef int ErlDrvTSDKey;
typedef struct erl_drv_port_data_lock * ErlDrvPDL;
typedef struct erl_drv_entry {
    int (*init)(void);
#ifndef ERL_SYS_DRV
    ErlDrvData (*start)(ErlDrvPort port, char *command);
#else
    ErlDrvData (*start)(ErlDrvPort port, char *command, SysDriverOpts* opts);
#endif
    void (*stop)(ErlDrvData drv_data);
    void (*output)(ErlDrvData drv_data, char *buf, ErlDrvSizeT len);
    void (*ready_input)(ErlDrvData drv_data, ErlDrvEvent event);
    void (*ready_output)(ErlDrvData drv_data, ErlDrvEvent event);
    char *driver_name;
    void (*finish)(void);
    void *handle;
    ErlDrvSSizeT (*control)(ErlDrvData drv_data, unsigned int command,
			    char *buf, ErlDrvSizeT len, char **rbuf,
			    ErlDrvSizeT rlen);
    void (*timeout)(ErlDrvData drv_data);
    void (*outputv)(ErlDrvData drv_data, ErlIOVec *ev);
    void (*ready_async)(ErlDrvData drv_data, ErlDrvThreadData thread_data);
    void (*flush)(ErlDrvData drv_data);
    ErlDrvSSizeT (*call)(ErlDrvData drv_data,
			 unsigned int command, char *buf, ErlDrvSizeT len,
			 char **rbuf, ErlDrvSizeT rlen,
			 unsigned int *flags);
    void (*unused_event_callback)(void);
    int extended_marker;
    int major_version;
    int minor_version;
    int driver_flags;
    void *handle2;
    void (*process_exit)(ErlDrvData drv_data, ErlDrvMonitor *monitor);
    void (*stop_select)(ErlDrvEvent event, void* reserved);
    void (*emergency_close)(ErlDrvData drv_data);
} ErlDrvEntry;
#ifdef STATIC_ERLANG_DRIVER
#  define ERLANG_DRIVER_NAME(NAME) NAME ## _driver_init
#  define ERL_DRIVER_EXPORT
#else
#  define ERLANG_DRIVER_NAME(NAME) driver_init
#  if defined(__GNUC__) && __GNUC__ >= 4
#    define ERL_DRIVER_EXPORT __attribute__ ((visibility("default")))
#  elif defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#    define ERL_DRIVER_EXPORT __global
#  else
#    define ERL_DRIVER_EXPORT
#  endif
#endif
#ifndef ERL_DRIVER_TYPES_ONLY
#define DRIVER_INIT(DRIVER_NAME) \
    ERL_DRIVER_EXPORT ErlDrvEntry* ERLANG_DRIVER_NAME(DRIVER_NAME)(void); \
    ERL_DRIVER_EXPORT ErlDrvEntry* ERLANG_DRIVER_NAME(DRIVER_NAME)(void)
#define ERL_DRV_BUSY_MSGQ_DISABLED	(~((ErlDrvSizeT) 0))
#define ERL_DRV_BUSY_MSGQ_READ_ONLY	((ErlDrvSizeT) 0)
#define ERL_DRV_BUSY_MSGQ_LIM_MAX	(ERL_DRV_BUSY_MSGQ_DISABLED - 1)
#define ERL_DRV_BUSY_MSGQ_LIM_MIN	((ErlDrvSizeT) 1)
EXTERN void erl_drv_busy_msgq_limits(ErlDrvPort port,
				     ErlDrvSizeT *low,
				     ErlDrvSizeT *high);
EXTERN int driver_select(ErlDrvPort port, ErlDrvEvent event, int mode, int on);
EXTERN int driver_output(ErlDrvPort port, char *buf, ErlDrvSizeT len);
EXTERN int driver_output2(ErlDrvPort port, char *hbuf, ErlDrvSizeT hlen,
			  char *buf, ErlDrvSizeT len);
EXTERN int driver_output_binary(ErlDrvPort port, char *hbuf, ErlDrvSizeT hlen,
				ErlDrvBinary* bin,
				ErlDrvSizeT offset, ErlDrvSizeT len);
EXTERN int driver_outputv(ErlDrvPort port, char* hbuf, ErlDrvSizeT hlen,
			  ErlIOVec *ev, ErlDrvSizeT skip);
EXTERN ErlDrvSizeT driver_vec_to_buf(ErlIOVec *ev, char *buf, ErlDrvSizeT len);
EXTERN int driver_set_timer(ErlDrvPort port, unsigned long time);
EXTERN int driver_cancel_timer(ErlDrvPort port);
EXTERN int driver_read_timer(ErlDrvPort port, unsigned long *time_left);
EXTERN int erl_drv_consume_timeslice(ErlDrvPort port, int percent);
EXTERN char* erl_errno_id(int error);
EXTERN int driver_failure_eof(ErlDrvPort port);
EXTERN int driver_failure_atom(ErlDrvPort port, char *string);
EXTERN int driver_failure_posix(ErlDrvPort port, int error);
EXTERN int driver_failure(ErlDrvPort port, int error);
EXTERN int driver_exit (ErlDrvPort port, int err);
EXTERN ErlDrvPDL driver_pdl_create(ErlDrvPort);
EXTERN void driver_pdl_lock(ErlDrvPDL);
EXTERN void driver_pdl_unlock(ErlDrvPDL);
EXTERN ErlDrvSInt driver_pdl_get_refc(ErlDrvPDL);
EXTERN ErlDrvSInt driver_pdl_inc_refc(ErlDrvPDL);
EXTERN ErlDrvSInt driver_pdl_dec_refc(ErlDrvPDL);
EXTERN int
driver_monitor_process(ErlDrvPort port, ErlDrvTermData process,
		       ErlDrvMonitor *monitor);
EXTERN int
driver_demonitor_process(ErlDrvPort port, const ErlDrvMonitor *monitor);
EXTERN ErlDrvTermData
driver_get_monitored_process(ErlDrvPort port, const ErlDrvMonitor *monitor);
EXTERN int driver_compare_monitors(const ErlDrvMonitor *monitor1,
				   const ErlDrvMonitor *monitor2);
EXTERN void set_busy_port(ErlDrvPort port, int on);
EXTERN void set_port_control_flags(ErlDrvPort port, int flags);
EXTERN int  get_port_flags(ErlDrvPort port);
EXTERN void driver_free_binary(ErlDrvBinary *bin);
EXTERN ErlDrvBinary* driver_alloc_binary(ErlDrvSizeT size)
    ERL_NAPI_ATTR_MALLOC_UD(driver_free_binary,1);
EXTERN ErlDrvBinary* driver_realloc_binary(ErlDrvBinary *bin, ErlDrvSizeT size)
    ERL_NAPI_ATTR_WUR;
EXTERN ErlDrvSInt driver_binary_get_refc(ErlDrvBinary *dbp);
EXTERN ErlDrvSInt driver_binary_inc_refc(ErlDrvBinary *dbp);
EXTERN ErlDrvSInt driver_binary_dec_refc(ErlDrvBinary *dbp);
EXTERN void driver_free(void *ptr);
EXTERN void *driver_alloc(ErlDrvSizeT size)
    ERL_NAPI_ATTR_MALLOC_USD(1, driver_free, 1);
EXTERN void *driver_realloc(void *ptr, ErlDrvSizeT size)
    ERL_NAPI_ATTR_ALLOC_SIZE(2);
EXTERN int driver_enq(ErlDrvPort port, char* buf, ErlDrvSizeT len);
EXTERN int driver_pushq(ErlDrvPort port, char* buf, ErlDrvSizeT len);
EXTERN ErlDrvSizeT driver_deq(ErlDrvPort port, ErlDrvSizeT size);
EXTERN ErlDrvSizeT driver_sizeq(ErlDrvPort port);
EXTERN int driver_enq_bin(ErlDrvPort port, ErlDrvBinary *bin, ErlDrvSizeT offset,
			  ErlDrvSizeT len);
EXTERN int driver_pushq_bin(ErlDrvPort port, ErlDrvBinary *bin, ErlDrvSizeT offset,
			    ErlDrvSizeT len);
EXTERN ErlDrvSizeT driver_peekqv(ErlDrvPort port, ErlIOVec *ev);
EXTERN SysIOVec* driver_peekq(ErlDrvPort port, int *vlen);
EXTERN int driver_enqv(ErlDrvPort port, ErlIOVec *ev, ErlDrvSizeT skip);
EXTERN int driver_pushqv(ErlDrvPort port, ErlIOVec *ev, ErlDrvSizeT skip);
EXTERN void add_driver_entry(ErlDrvEntry *de);
EXTERN int remove_driver_entry(ErlDrvEntry *de);
EXTERN void driver_system_info(ErlDrvSysInfo *sip, size_t si_size);
EXTERN void erl_drv_mutex_destroy(ErlDrvMutex *mtx);
EXTERN ErlDrvMutex *erl_drv_mutex_create(char *name) ERL_NAPI_ATTR_MALLOC_D(erl_drv_mutex_destroy,1);
EXTERN int erl_drv_mutex_trylock(ErlDrvMutex *mtx);
EXTERN void erl_drv_mutex_lock(ErlDrvMutex *mtx);
EXTERN void erl_drv_mutex_unlock(ErlDrvMutex *mtx);
EXTERN void erl_drv_cond_destroy(ErlDrvCond *cnd);
EXTERN ErlDrvCond *erl_drv_cond_create(char *name) ERL_NAPI_ATTR_MALLOC_D(erl_drv_cond_destroy,1);
EXTERN void erl_drv_cond_signal(ErlDrvCond *cnd);
EXTERN void erl_drv_cond_broadcast(ErlDrvCond *cnd);
EXTERN void erl_drv_cond_wait(ErlDrvCond *cnd, ErlDrvMutex *mtx);
EXTERN void erl_drv_rwlock_destroy(ErlDrvRWLock *rwlck);
EXTERN ErlDrvRWLock *erl_drv_rwlock_create(char *name) ERL_NAPI_ATTR_MALLOC_D(erl_drv_rwlock_destroy,1);
EXTERN int erl_drv_rwlock_tryrlock(ErlDrvRWLock *rwlck);
EXTERN void erl_drv_rwlock_rlock(ErlDrvRWLock *rwlck);
EXTERN void erl_drv_rwlock_runlock(ErlDrvRWLock *rwlck);
EXTERN int erl_drv_rwlock_tryrwlock(ErlDrvRWLock *rwlck);
EXTERN void erl_drv_rwlock_rwlock(ErlDrvRWLock *rwlck);
EXTERN void erl_drv_rwlock_rwunlock(ErlDrvRWLock *rwlck);
EXTERN int erl_drv_tsd_key_create(char *name, ErlDrvTSDKey *key);
EXTERN void erl_drv_tsd_key_destroy(ErlDrvTSDKey key);
EXTERN void erl_drv_tsd_set(ErlDrvTSDKey key, void *data);
EXTERN void *erl_drv_tsd_get(ErlDrvTSDKey key);
EXTERN void erl_drv_thread_opts_destroy(ErlDrvThreadOpts *opts);
EXTERN ErlDrvThreadOpts *erl_drv_thread_opts_create(char *name) ERL_NAPI_ATTR_MALLOC_D(erl_drv_thread_opts_destroy,1);
EXTERN int erl_drv_thread_create(char *name,
				 ErlDrvTid *tid,
				 void * (*func)(void *),
				 void *args,
				 ErlDrvThreadOpts *opts);
EXTERN ErlDrvTid erl_drv_thread_self(void);
EXTERN int erl_drv_equal_tids(ErlDrvTid tid1, ErlDrvTid tid2);
EXTERN void erl_drv_thread_exit(void *resp);
EXTERN int erl_drv_thread_join(ErlDrvTid, void **respp);
EXTERN char* erl_drv_mutex_name(ErlDrvMutex *mtx);
EXTERN char* erl_drv_cond_name(ErlDrvCond *cnd);
EXTERN char* erl_drv_rwlock_name(ErlDrvRWLock *rwlck);
EXTERN char* erl_drv_thread_name(ErlDrvTid tid);
#if defined(__DARWIN__)
EXTERN int erl_drv_stolen_main_thread_join(ErlDrvTid tid, void **respp);
EXTERN int erl_drv_steal_main_thread(char *name,
                                     ErlDrvTid *dtid,
                                     void* (*func)(void*),
                                     void* arg,
                                     ErlDrvThreadOpts *opts);
#endif
EXTERN int null_func(void);
#endif
#define DRIVER_CALL_KEEP_BUFFER 0x1
#define TERM_DATA(x) ((ErlDrvTermData) (x))
#define ERL_DRV_NIL         ((ErlDrvTermData) 1)
#define ERL_DRV_ATOM        ((ErlDrvTermData) 2)
#define ERL_DRV_INT         ((ErlDrvTermData) 3)
#define ERL_DRV_PORT        ((ErlDrvTermData) 4)
#define ERL_DRV_BINARY      ((ErlDrvTermData) 5)
#define ERL_DRV_STRING      ((ErlDrvTermData) 6)
#define ERL_DRV_TUPLE       ((ErlDrvTermData) 7)
#define ERL_DRV_LIST        ((ErlDrvTermData) 8)
#define ERL_DRV_STRING_CONS ((ErlDrvTermData) 9)
#define ERL_DRV_PID         ((ErlDrvTermData) 10)
#define ERL_DRV_FLOAT       ((ErlDrvTermData) 11)
#define ERL_DRV_EXT2TERM    ((ErlDrvTermData) 12)
#define ERL_DRV_UINT        ((ErlDrvTermData) 13)
#define ERL_DRV_BUF2BINARY  ((ErlDrvTermData) 14)
#define ERL_DRV_INT64       ((ErlDrvTermData) 15)
#define ERL_DRV_UINT64      ((ErlDrvTermData) 16)
#define ERL_DRV_MAP         ((ErlDrvTermData) 17)
#ifndef ERL_DRIVER_TYPES_ONLY
EXTERN ErlDrvTermData driver_mk_atom(char*);
EXTERN ErlDrvTermData driver_mk_port(ErlDrvPort);
EXTERN ErlDrvTermData driver_connected(ErlDrvPort);
EXTERN ErlDrvTermData driver_caller(ErlDrvPort);
EXTERN const ErlDrvTermData driver_term_nil;
EXTERN ErlDrvTermData driver_mk_term_nil(void);
EXTERN ErlDrvPort driver_create_port(ErlDrvPort creator_port,
				     ErlDrvTermData connected,
				     char* name,
				     ErlDrvData drv_data);
EXTERN int driver_output_term(ErlDrvPort ix,
			      ErlDrvTermData* data,
			      int len) ERL_DRV_DEPRECATED_FUNC;
EXTERN int driver_send_term(ErlDrvPort ix,
			    ErlDrvTermData to,
			    ErlDrvTermData* data,
			    int len) ERL_DRV_DEPRECATED_FUNC;
EXTERN int erl_drv_output_term(ErlDrvTermData port,
			       ErlDrvTermData* data,
			       int len);
EXTERN int erl_drv_send_term(ErlDrvTermData port,
			     ErlDrvTermData to,
			     ErlDrvTermData* data,
			     int len);
EXTERN unsigned int driver_async_port_key(ErlDrvPort port);
EXTERN long driver_async(ErlDrvPort ix,
			 unsigned int* key,
			 void (*async_invoke)(void*),
			 void* async_data,
			 void (*async_free)(void*));
EXTERN int driver_lock_driver(ErlDrvPort ix);
EXTERN int driver_get_now(ErlDrvNowData *now) ERL_DRV_DEPRECATED_FUNC;
EXTERN ErlDrvTime erl_drv_monotonic_time(ErlDrvTimeUnit time_unit);
EXTERN ErlDrvTime erl_drv_time_offset(ErlDrvTimeUnit time_unit);
EXTERN ErlDrvTime erl_drv_convert_time_unit(ErlDrvTime val,
					    ErlDrvTimeUnit from,
					    ErlDrvTimeUnit to);
EXTERN void *driver_dl_open(char *);
EXTERN void *driver_dl_sym(void *, char *);
EXTERN int driver_dl_close(void *);
EXTERN char *driver_dl_error(void);
EXTERN int erl_drv_putenv(const char *key, char *value);
EXTERN int erl_drv_getenv(const char *key, char *value, size_t *value_size);
EXTERN void erl_drv_init_ack(ErlDrvPort ix, ErlDrvData res);
EXTERN void erl_drv_set_os_pid(ErlDrvPort ix, ErlDrvSInt pid);
#endif
#ifdef WIN32_DYNAMIC_ERL_DRIVER
#  include "erl_win_dyn_driver.h"
#endif
#ifdef __cplusplus
}
#endif
#endif
void dtrace_drvport_str(ErlDrvPort port, char *port_buf);