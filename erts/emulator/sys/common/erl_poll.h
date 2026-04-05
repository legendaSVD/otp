#ifndef ERL_POLL_H__
#define ERL_POLL_H__
#include "sys.h"
#define ERTS_POLL_NO_TIMEOUT ERTS_MONOTONIC_TIME_MIN
#define ERTS_POLL_INF_TIMEOUT ERTS_MONOTONIC_TIME_MAX
#ifdef ERTS_ENABLE_KERNEL_POLL
#  undef ERTS_ENABLE_KERNEL_POLL
#  define ERTS_ENABLE_KERNEL_POLL 1
#  if defined(ERTS_NO_KERNEL_POLL_VERSION)
#    define ERTS_POLL_EXPORT(FUNC) FUNC ## _flbk
#    undef ERTS_NO_KERNEL_POLL_VERSION
#    define ERTS_NO_KERNEL_POLL_VERSION 1
#    define ERTS_KERNEL_POLL_VERSION 0
#  else
#    undef ERTS_KERNEL_POLL_VERSION
#    define ERTS_KERNEL_POLL_VERSION 1
#    define ERTS_NO_KERNEL_POLL_VERSION 0
#    define ERTS_POLL_EXPORT(FUNC) FUNC
#  endif
#else
#    define ERTS_POLL_EXPORT(FUNC) FUNC
#    define ERTS_ENABLE_KERNEL_POLL 0
#    define ERTS_NO_KERNEL_POLL_VERSION 1
#    define ERTS_KERNEL_POLL_VERSION 0
#endif
#undef ERTS_POLL_USE_KQUEUE
#define ERTS_POLL_USE_KQUEUE 0
#undef ERTS_POLL_USE_EPOLL
#define ERTS_POLL_USE_EPOLL 0
#undef ERTS_POLL_USE_DEVPOLL
#define ERTS_POLL_USE_DEVPOLL 0
#undef ERTS_POLL_USE_POLL
#define ERTS_POLL_USE_POLL 0
#undef ERTS_POLL_USE_SELECT
#define ERTS_POLL_USE_SELECT 0
#define ERTS_POLL_USE_EPOLL_EVS 0
#define ERTS_POLL_USE_KQUEUE_EVS 0
#define ERTS_POLL_USE_DEVPOLL_EVS 0
#define ERTS_POLL_USE_POLL_EVS 0
#define ERTS_POLL_USE_SELECT_EVS 0
#define ERTS_POLL_USE_KERNEL_POLL ERTS_KERNEL_POLL_VERSION
#if ERTS_ENABLE_KERNEL_POLL
#  if defined(HAVE_SYS_EVENT_H)
#    undef ERTS_POLL_USE_KQUEUE_EVS
#    define ERTS_POLL_USE_KQUEUE_EVS 1
#    undef ERTS_POLL_USE_KQUEUE
#    define ERTS_POLL_USE_KQUEUE ERTS_KERNEL_POLL_VERSION
#  elif defined(HAVE_SYS_EPOLL_H)
#    undef ERTS_POLL_USE_EPOLL_EVS
#    define ERTS_POLL_USE_EPOLL_EVS 1
#    undef ERTS_POLL_USE_EPOLL
#    define ERTS_POLL_USE_EPOLL ERTS_KERNEL_POLL_VERSION
#  elif defined(HAVE_SYS_DEVPOLL_H)
#    undef ERTS_POLL_USE_DEVPOLL_EVS
#    define ERTS_POLL_USE_DEVPOLL_EVS 1
#    undef ERTS_POLL_USE_DEVPOLL
#    define ERTS_POLL_USE_DEVPOLL ERTS_KERNEL_POLL_VERSION
#  else
#    error "Missing kernel poll implementation of erts_poll()"
#  endif
#endif
#if ERTS_NO_KERNEL_POLL_VERSION
#  if defined(ERTS_USE_POLL)
#    undef ERTS_POLL_USE_POLL_EVS
#    define ERTS_POLL_USE_POLL_EVS 1
#    undef ERTS_POLL_USE_POLL
#    define ERTS_POLL_USE_POLL 1
#  elif !defined(__WIN32__)
#    undef ERTS_POLL_USE_SELECT_EVS
#    define ERTS_POLL_USE_SELECT_EVS 1
#    undef ERTS_POLL_USE_SELECT
#    define ERTS_POLL_USE_SELECT 1
#  endif
#endif
#define ERTS_POLL_USE_FALLBACK (ERTS_POLL_USE_KQUEUE || ERTS_POLL_USE_EPOLL)
#define ERTS_POLL_USE_SCHEDULER_POLLING (ERTS_POLL_USE_KQUEUE || ERTS_POLL_USE_EPOLL)
#define ERTS_POLL_SCHEDULER_POLLING_TIMEOUT 10
#define ERTS_POLL_USE_TIMERFD 0
typedef Uint32 ErtsPollEvents;
typedef enum {
    ERTS_POLL_OP_ADD = 0,
    ERTS_POLL_OP_MOD = 1,
    ERTS_POLL_OP_DEL = 2
} ErtsPollOp;
#define op2str(op) (op == ERTS_POLL_OP_ADD ? "add" :            \
                    (op == ERTS_POLL_OP_MOD ? "mod" : "del"))
#if defined(__WIN32__)
#define ERTS_POLL_EV_IN    1
#define ERTS_POLL_EV_OUT   2
#define ERTS_POLL_EV_ERR   4
#define ERTS_POLL_EV_NVAL  8
#define ERTS_POLL_EV_E2N(EV) 		(EV)
#define ERTS_POLL_EV_N2E(EV) 		(EV)
#elif ERTS_POLL_USE_EPOLL_EVS
#include <sys/epoll.h>
#if ERTS_POLL_USE_EPOLL
#if defined(HAVE_SYS_TIMERFD_H) && !(defined(__ANDROID__) && (__ANDROID_API__ < 19))
#include <sys/timerfd.h>
#undef ERTS_POLL_USE_TIMERFD
#define ERTS_POLL_USE_TIMERFD 1
#endif
#endif
#define ERTS_POLL_EV_E2N(EV) \
  ((uint32_t) (EV))
#define ERTS_POLL_EV_N2E(EV) \
  ((ErtsPollEvents) (EV) & ~EPOLLONESHOT)
#define ERTS_POLL_EV_IN			ERTS_POLL_EV_N2E(EPOLLIN)
#define ERTS_POLL_EV_OUT		ERTS_POLL_EV_N2E(EPOLLOUT)
#define ERTS_POLL_EV_NVAL		ERTS_POLL_EV_N2E(EPOLLET)
#define ERTS_POLL_EV_ERR		ERTS_POLL_EV_N2E(EPOLLERR|EPOLLHUP)
typedef struct epoll_event ErtsPollResFd;
#define ERTS_POLL_RES_GET_FD(evt) ((ErtsSysFdType)((evt)->data.fd))
#define ERTS_POLL_RES_SET_FD(evt, ident) (evt)->data.fd = ident
#define ERTS_POLL_RES_GET_EVTS(evt) ERTS_POLL_EV_N2E((evt)->events)
#define ERTS_POLL_RES_SET_EVTS(evt, evts) (evt)->events = ERTS_POLL_EV_E2N(evts)
#elif ERTS_POLL_USE_DEVPOLL_EVS
#include <sys/devpoll.h>
#define ERTS_POLL_EV_E2N(EV) \
  ((short) ((EV) & ~((~((ErtsPollEvents) 0)) << 8*SIZEOF_SHORT)))
#define ERTS_POLL_EV_N2E(EV) \
  ((ErtsPollEvents) ((unsigned short) (EV)))
#define ERTS_POLL_EV_IN			ERTS_POLL_EV_N2E(POLLIN)
#define ERTS_POLL_EV_OUT		ERTS_POLL_EV_N2E(POLLOUT)
#define ERTS_POLL_EV_NVAL		ERTS_POLL_EV_N2E(POLLNVAL)
#define ERTS_POLL_EV_ERR		ERTS_POLL_EV_N2E(POLLERR|POLLHUP)
typedef struct pollfd ErtsPollResFd;
#define ERTS_POLL_RES_GET_FD(evt) ((ErtsSysFdType)((evt)->fd))
#define ERTS_POLL_RES_SET_FD(evt, ident) (evt)->fd = ident
#define ERTS_POLL_RES_GET_EVTS(evt) ERTS_POLL_EV_N2E((evt)->revents)
#define ERTS_POLL_RES_SET_EVTS(evt, evts) (evt)->revents = ERTS_POLL_EV_E2N(evts)
#elif ERTS_POLL_USE_KQUEUE_EVS
#include <sys/event.h>
#ifdef ERTS_USE_POLL
#  undef ERTS_POLL_USE_POLL_EVS
#  define ERTS_POLL_USE_POLL_EVS   1
#elif !defined(__WIN32__)
#  undef ERTS_POLL_USE_SELECT_EVS
#  define ERTS_POLL_USE_SELECT_EVS 1
#endif
typedef struct kevent ErtsPollResFd;
#define ERTS_POLL_RES_GET_FD(evt) ((ErtsSysFdType)((evt)->ident))
#define ERTS_POLL_RES_SET_FD(evt, fd) (evt)->ident = fd
#define ERTS_POLL_RES_GET_EVTS(evt) ERTS_POLL_EV_N2E((ErtsPollEvents)(evt)->udata)
#define ERTS_POLL_RES_SET_EVTS(evt, evts) (evt)->udata = (void*)(UWord)(ERTS_POLL_EV_E2N(evts))
#endif
#if ERTS_POLL_USE_POLL_EVS
#include <poll.h>
#define ERTS_POLL_EV_NKP_E2N(EV) \
  ((short) ((EV) & ~((~((ErtsPollEvents) 0)) << 8*SIZEOF_SHORT)))
#define ERTS_POLL_EV_NKP_N2E(EV) \
  ((ErtsPollEvents) ((unsigned short) (EV)))
#ifdef POLLRDNORM
#define ERTS_POLL_EV_NKP_IN		ERTS_POLL_EV_N2E(POLLIN|POLLRDNORM)
#else
#define ERTS_POLL_EV_NKP_IN		ERTS_POLL_EV_N2E(POLLIN)
#endif
#define ERTS_POLL_EV_NKP_OUT		ERTS_POLL_EV_N2E(POLLOUT)
#define ERTS_POLL_EV_NKP_NVAL		ERTS_POLL_EV_N2E(POLLNVAL)
#define ERTS_POLL_EV_NKP_ERR		ERTS_POLL_EV_N2E(POLLERR|POLLHUP)
#elif ERTS_POLL_USE_SELECT_EVS
#define ERTS_POLL_EV_NKP_E2N(EV) (EV)
#define ERTS_POLL_EV_NKP_N2E(EV) (EV)
#define ERTS_POLL_EV_NKP_IN		(((ErtsPollEvents) 1) << 0)
#define ERTS_POLL_EV_NKP_OUT		(((ErtsPollEvents) 1) << 1)
#define ERTS_POLL_EV_NKP_NVAL		(((ErtsPollEvents) 1) << 2)
#define ERTS_POLL_EV_NKP_ERR		(((ErtsPollEvents) 1) << 3)
#endif
#if !defined(ERTS_POLL_EV_E2N) && defined(ERTS_POLL_EV_NKP_E2N)
#define ERTS_POLL_EV_E2N(EV) 		ERTS_POLL_EV_NKP_E2N((EV))
#define ERTS_POLL_EV_N2E(EV) 		ERTS_POLL_EV_NKP_N2E((EV))
#define ERTS_POLL_EV_IN			ERTS_POLL_EV_NKP_IN
#define ERTS_POLL_EV_OUT		ERTS_POLL_EV_NKP_OUT
#define ERTS_POLL_EV_NVAL		ERTS_POLL_EV_NKP_NVAL
#define ERTS_POLL_EV_ERR		ERTS_POLL_EV_NKP_ERR
#endif
#if !ERTS_ENABLE_KERNEL_POLL
typedef struct _ErtsPollResFd {
    ErtsSysFdType fd;
    ErtsPollEvents events;
} ErtsPollResFd;
#define ERTS_POLL_RES_GET_FD(evt) (evt)->fd
#define ERTS_POLL_RES_SET_FD(evt, ident) (evt)->fd = (ident)
#define ERTS_POLL_RES_GET_EVTS(evt) ERTS_POLL_EV_N2E((evt)->events)
#define ERTS_POLL_RES_SET_EVTS(evt, evts) (evt)->events = ERTS_POLL_EV_E2N(evts)
#endif
#define ERTS_POLL_EV_NONE ERTS_POLL_EV_N2E((UINT_MAX & ~(ERTS_POLL_EV_IN|ERTS_POLL_EV_OUT|ERTS_POLL_EV_NVAL|ERTS_POLL_EV_ERR)))
#define ev2str(ev)                                                     \
    (((ev) == 0 || (ev) == ERTS_POLL_EV_NONE) ? "NONE" :               \
     ((ev) == ERTS_POLL_EV_IN ? "IN" :                                 \
      ((ev) == ERTS_POLL_EV_OUT ? "OUT" :                              \
       ((ev) == (ERTS_POLL_EV_IN|ERTS_POLL_EV_OUT) ? "IN|OUT" :        \
        ((ev) & ERTS_POLL_EV_ERR ? "ERR" :                             \
         ((ev) & ERTS_POLL_EV_NVAL ? "NVAL" : "OTHER"))))))
typedef struct ERTS_POLL_EXPORT(erts_pollset) ErtsPollSet;
typedef struct {
    char *primary;
    char *kernel_poll;
    Uint memory_size;
    Uint poll_set_size;
    int lazy_updates;
    Uint pending_updates;
    int batch_updates;
    int concurrent_updates;
    int is_fallback;
    Uint max_fds;
    Uint active_fds;
    Uint poll_threads;
} ErtsPollInfo;
#if defined(ERTS_POLL_USE_FALLBACK) && ERTS_KERNEL_POLL_VERSION
#  undef ERTS_POLL_EXPORT
#  define ERTS_POLL_EXPORT(FUNC) FUNC ## _flbk
#  include "erl_poll_api.h"
#  undef ERTS_POLL_EXPORT
#  define ERTS_POLL_EXPORT(FUNC) FUNC
#elif !defined(ERTS_POLL_USE_FALLBACK)
#  define ERTS_POLL_USE_FALLBACK 0
#endif
#include "erl_poll_api.h"
int erts_poll_new_table_len(int old_len, int need_len);
#endif