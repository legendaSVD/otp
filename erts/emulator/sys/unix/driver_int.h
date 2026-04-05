#ifndef __DRIVER_INT_H__
#define __DRIVER_INT_H__
#ifdef HAVE_SYS_UIO_H
#include <sys/types.h>
#include <sys/uio.h>
typedef struct iovec SysIOVec;
#else
typedef struct {
    char* iov_base;
    int   iov_len;
} SysIOVec;
#endif
#endif