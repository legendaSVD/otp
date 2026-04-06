#ifndef ERL_ERRNO_H__
#define ERL_ERRNO_H__
#include <errno.h>
#ifndef ENOTSUP
#  ifdef EOPNOTSUPP
#    define ENOTSUP EOPNOTSUPP
#else
#    define ENOTSUP INT_MAX
#  endif
#endif
#ifdef __WIN32__
#  ifndef EWOULDBLOCK
#    define EWOULDBLOCK (10035)
#  endif
#  ifndef ETIMEDOUT
#    define ETIMEDOUT (10060)
#  endif
#else
#  ifndef EWOULDBLOCK
#    define EWOULDBLOCK EAGAIN
#  endif
#  ifndef ETIMEDOUT
#    define ETIMEDOUT EAGAIN
#  endif
#  ifndef EINTR
#    define EINTR (INT_MAX-1)
#  endif
#endif
#endif