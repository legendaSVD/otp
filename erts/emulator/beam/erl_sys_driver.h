#ifndef __ERL_SYS_DRIVER_H__
#define __ERL_SYS_DRIVER_H__
#ifdef __ERL_DRIVER_H__
#error erl_sys_driver.h cannot be included after erl_driver.h
#endif
#define ERL_SYS_DRV
typedef SWord ErlDrvEvent;
typedef struct _SysDriverOpts SysDriverOpts;
#include "erl_driver.h"
struct _SysDriverOpts {
    Uint ifd;
    Uint ofd;
    int packet_bytes;
    int read_write;
    int use_stdio;
    int redir_stderr;
    int hide_window;
    int exit_status;
    int overlapped_io;
    erts_osenv_t envir;
    char **argv;
    char *wd;
    unsigned spawn_type;
    int parallelism;
    ErlDrvSizeT high_watermark;
    ErlDrvSizeT low_watermark;
    ErlDrvSizeT high_msgq_watermark;
    ErlDrvSizeT low_msgq_watermark;
    char port_watermarks_set;
    char msgq_watermarks_set;
};
#endif