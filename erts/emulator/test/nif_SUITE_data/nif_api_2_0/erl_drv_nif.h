#ifndef __ERL_DRV_NIF_H__
#define __ERL_DRV_NIF_H__
typedef struct {
    int driver_major_version;
    int driver_minor_version;
    char *erts_version;
    char *otp_release;
    int thread_support;
    int smp_support;
    int async_threads;
    int scheduler_threads;
    int nif_major_version;
    int nif_minor_version;
}  ErlDrvSysInfo;
typedef struct {
    int suggested_stack_size;
} ErlDrvThreadOpts;
#endif