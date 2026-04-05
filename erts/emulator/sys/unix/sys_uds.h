#ifndef _ERL_UNIX_UDS_H
#define _ERL_UNIX_UDS_H
#include <sys/uio.h>
#if defined IOV_MAX
#define MAXIOV IOV_MAX
#elif defined UIO_MAXIOV
#define MAXIOV UIO_MAXIOV
#else
#define MAXIOV 16
#endif
int sys_uds_readv(int fd, struct iovec *iov, size_t iov_len,
                  int *fds, int fd_count, int flags);
int sys_uds_read(int fd, char *buff, size_t len,
                 int *fds, int fd_count, int flags);
int sys_uds_writev(int fd, struct iovec *iov, size_t iov_len,
                   int *fds, int fd_count, int flags);
int sys_uds_write(int fd, char *buff, size_t len,
                  int *fds, int fd_count, int flags);
#endif