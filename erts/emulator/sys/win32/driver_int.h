#ifndef __DRIVER_INT_H__
#define __DRIVER_INT_H__
#if !defined __WIN32__
#  define __WIN32__
#endif
typedef struct _SysIOVec {
    unsigned long iov_len;
    char* iov_base;
} SysIOVec;
#endif