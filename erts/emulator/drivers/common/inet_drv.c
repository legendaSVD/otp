#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if (defined(HAVE_SCTP_H) && defined(__sun) && defined(__SVR4))
#define  SOLARIS10    1
#define  _XPG4_2
#define  __EXTENSIONS__
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <stdint.h>
#define IDENTITY(c) c
#define STRINGIFY_1(b) IDENTITY(#b)
#define STRINGIFY(a) STRINGIFY_1(a)
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#ifdef HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif
#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif
#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#ifdef HAVE_SENDFILE
#if defined(__linux__) || (defined(__sun) && defined(__SVR4))
    #include <sys/sendfile.h>
#elif defined(__FreeBSD__) || defined(__DragonFly__)
    #define __BSD_VISIBLE 1
    #include <sys/socket.h>
#endif
#endif
#if defined(__APPLE__) && defined(__MACH__) && !defined(__DARWIN__)
    #define __DARWIN__ 1
#endif
#define FATAL_MALLOC
#ifdef HAVE_GCC_DIAG_IGNORE_WADDRESS_OF_PACKED_MEMBER
#define PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER()                        \
    _Pragma("GCC diagnostic push")                                      \
    _Pragma("GCC diagnostic ignored \"-Waddress-of-packed-member\"")    \
    do { } while(0)
#define POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER() \
    _Pragma("GCC diagnostic pop")               \
    do { } while(0)
#else
#define PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER() \
    do { } while(0)
#define POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER() \
    do { } while(0)
#endif
#include "erl_driver.h"
#if !defined(TRUE)
#define TRUE  1
#endif
#if !defined(FALSE)
#define FALSE 0
#endif
#ifdef INET_DRV_DEBUG
#define DDBG_DEFAULT TRUE
#else
#define DDBG_DEFAULT FALSE
#endif
#define DDBG(__D__, __ARG__)                                    \
    do if ( (__D__)->debug ) { erts_printf __ARG__ ; } while (0)
#define B2S(__B__)   ((__B__) ? "true" : "false")
#define SH2S(__H__)  (((__H__) == TCP_SHUT_WR) ? "write" :          \
                      (((__H__) == TCP_SHUT_RD) ? "read" :          \
                       (((__H__) == TCP_SHUT_RDWR) ? "read-write" : \
                        "undefined")))
#define A2S(__A__)   (((__A__) == INET_PASSIVE) ? "passive" : \
                      (((__A__) == INET_ACTIVE) ? "active" :  \
                       (((__A__) == INET_ONCE) ? "once" :     \
                        (((__A__) == INET_MULTI) ? "multi" :  \
                         "undefined"))))
#define M2S(__M__)   (((__M__) == INET_MODE_LIST) ? "list" :      \
                      (((__M__) == INET_MODE_BINARY) ? "binary" : \
                       "undefined"))
#define D2S(__D__)   (((__D__) == INET_DELIVER_PORT) ? "port" :         \
                      (((__D__) == INET_DELIVER_TERM) ? "term" :        \
                       "undefined"))
#define DOM2S(__D__)   (((__D__) == INET_AF_INET) ? "inet" :            \
                        (((__D__) == INET_AF_INET6) ? "inet6" :         \
                         (((__D__) == INET_AF_LOCAL) ? "local" :        \
                          "undefined")))
#if defined(AF_LINK)
#define FAM2S(__F__)   (((__F__) == AF_INET) ? "inet" :            \
                        (((__F__) == AF_INET6) ? "inet6" :         \
                         (((__F__) == AF_LINK) ? "link" :          \
                          "undefined")))
#elif defined(AF_PACKET)
#define FAM2S(__F__)   (((__F__) == AF_INET) ? "inet" :            \
                        (((__F__) == AF_INET6) ? "inet6" :         \
                         (((__F__) == AF_PACKET) ? "packet" :      \
                          "undefined")))
#else
#define FAM2S(__F__)   (((__F__) == AF_INET) ? "inet" :            \
                        (((__F__) == AF_INET6) ? "inet6" :         \
                         "undefined"))
#endif
#if defined(__WIN32__) && defined(ARCH_64)
#define SOCKET_FSTR "%lld"
#else
#define SOCKET_FSTR "%d"
#endif
#ifdef __WIN32__
#define IS_SOCKET_ERROR(val) ((val) == SOCKET_ERROR)
#else
#define IS_SOCKET_ERROR(val) ((val) < 0)
#endif
#ifdef __WIN32__
#define LLU "%I64u"
#else
#define LLU "%llu"
#endif
typedef unsigned long long llu_t;
#ifndef INT16_MIN
#define INT16_MIN (-32768)
#endif
#ifndef INT16_MAX
#define INT16_MAX (32767)
#endif
#ifdef __WIN32__
#define  STRNCASECMP strncasecmp
#define INCL_WINSOCK_API_TYPEDEFS 1
#ifndef WINDOWS_H_INCLUDES_WINSOCK2_H
#include <winsock2.h>
#endif
#include <windows.h>
#include <Ws2tcpip.h>
#ifdef HAVE_SDKDDKVER_H
#  include <sdkddkver.h>
#  ifdef NTDDI_VERSION
#    undef NTDDI_VERSION
#  endif
#  define NTDDI_VERSION NTDDI_WINXP
#endif
#include <iphlpapi.h>
#undef WANT_NONBLOCKING
#include "sys.h"
#undef EWOULDBLOCK
#undef ETIMEDOUT
#ifdef EINPROGRESS
#undef EINPROGRESS
#endif
#ifdef EALREADY
#undef EALREADY
#endif
#ifdef ENOTSOCK
#undef ENOTSOCK
#endif
#ifdef EDESTADDRREQ
#undef EDESTADDRREQ
#endif
#ifdef EMSGSIZE
#undef EMSGSIZE
#endif
#ifdef EPROTOTYPE
#undef EPROTOTYPE
#endif
#ifdef ENOPROTOOPT
#undef ENOPROTOOPT
#endif
#ifdef EPROTONOSUPPORT
#undef EPROTONOSUPPORT
#endif
#ifdef EOPNOTSUPP
#undef EOPNOTSUPP
#endif
#ifdef EAFNOSUPPORT
#undef EAFNOSUPPORT
#endif
#ifdef EADDRINUSE
#undef EADDRINUSE
#endif
#ifdef EADDRNOTAVAIL
#undef EADDRNOTAVAIL
#endif
#ifdef ENETDOWN
#undef ENETDOWN
#endif
#ifdef ENETUNREACH
#undef ENETUNREACH
#endif
#ifdef ENETRESET
#undef ENETRESET
#endif
#ifdef ECONNABORTED
#undef ECONNABORTED
#endif
#ifdef ECONNRESET
#undef ECONNRESET
#endif
#ifdef ENOBUFS
#undef ENOBUFS
#endif
#ifdef EISCONN
#undef EISCONN
#endif
#ifdef ENOTCONN
#undef ENOTCONN
#endif
#ifdef ECONNREFUSED
#undef ECONNREFUSED
#endif
#ifdef ELOOP
#undef ELOOP
#endif
#ifdef EHOSTUNREACH
#undef EHOSTUNREACH
#endif
#define HAVE_MULTICAST_SUPPORT
#define HAVE_UDP
#define ERRNO_BLOCK             WSAEWOULDBLOCK
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#undef ENAMETOOLONG
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#undef ENOTEMPTY
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#define INVALID_EVENT           WSA_INVALID_EVENT
static BOOL (WINAPI *fpSetHandleInformation)(HANDLE,DWORD,DWORD);
#define sock_open(af, type, proto) \
    make_noninheritable_handle(socket((af), (type), (proto)))
#define sock_close(s)              closesocket((s))
#define sock_shutdown(s, how)      shutdown((s), (how))
#define sock_accept(s, addr, len) \
    make_noninheritable_handle(accept((s), (addr), (len)))
#define sock_connect(s, addr, len) connect((s), (addr), (len))
#define sock_listen(s, b)          listen((s), (b))
#define sock_bind(s, addr, len)    bind((s), (addr), (len))
#define sock_getopt(s,t,n,v,l)     getsockopt((s),(t),(n),(v),(l))
#define sock_setopt(s,t,n,v,l)     setsockopt((s),(t),(n),(v),(l))
#define sock_name(s, addr, len)    getsockname((s), (addr), (len))
#define sock_peer(s, addr, len)    getpeername((s), (addr), (len))
#define sock_ntohs(x)              ntohs((x))
#define sock_ntohl(x)              ntohl((x))
#define sock_htons(x)              htons((x))
#define sock_htonl(x)              htonl((x))
#define sock_send(s,buf,len,flag)  send((s),(buf),(len),(flag))
#define sock_sendv(s, vec, size, np, flag) \
            WSASend((s),(WSABUF*)(vec),(size),(np),(flag),NULL,NULL)
#define sock_recv(s,buf,len,flag)  recv((s),(buf),(len),(flag))
#define sock_recvfrom(s,buf,blen,flag,addr,alen) \
	    recvfrom((s),(buf),(blen),(flag),(addr),(alen))
#define sock_sendto(s,buf,blen,flag,addr,alen) \
	    sendto((s),(buf),(blen),(flag),(addr),(alen))
#define sock_hostname(buf, len)    gethostname((buf), (len))
#define sock_getservbyname(name,proto) getservbyname((name),(proto))
#define sock_getservbyport(port,proto) getservbyport((port),(proto))
#define sock_errno() WSAGetLastError()
#define sock_create_event(d)       WSACreateEvent()
#define sock_close_event(e)        WSACloseEvent(e)
#define sock_select(D, Flags, OnOff) winsock_event_select(D, Flags, OnOff)
#define SET_BLOCKING(s)           ioctlsocket(s, FIONBIO, &zero_value)
#define SET_NONBLOCKING(s)        ioctlsocket(s, FIONBIO, &one_value)
static unsigned long zero_value = 0;
static unsigned long one_value = 1;
#define TCP_SHUT_WR    SD_SEND
#define TCP_SHUT_RD    SD_RECEIVE
#define TCP_SHUT_RDWR  SD_BOTH
#else
#include <sys/time.h>
#ifdef NETDB_H_NEEDS_IN_H
#include <netinet/in.h>
#endif
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef DEF_INADDR_LOOPBACK_IN_RPC_TYPES_H
#include <rpc/types.h>
#endif
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/param.h>
#ifdef HAVE_ARPA_NAMESER_H
#include <arpa/nameser.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#include <net/if.h>
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif
#ifdef HAVE_SETNS_H
#include <setns.h>
#endif
#define HAVE_UDP
#undef HAVE_SCTP
#if defined(HAVE_SCTP_H)
#include <netinet/sctp.h>
#ifndef     HAVE_SCTP
#    define HAVE_SCTP
#endif
#if ! HAVE_DECL_SCTP_UNORDERED
#     define    SCTP_UNORDERED  MSG_UNORDERED
#endif
#if ! HAVE_DECL_SCTP_ADDR_OVER
#     define    SCTP_ADDR_OVER  MSG_ADDR_OVER
#endif
#if ! HAVE_DECL_SCTP_ABORT
#     define    SCTP_ABORT      MSG_ABORT
#endif
#if ! HAVE_DECL_SCTP_EOF
#     define    SCTP_EOF        MSG_EOF
#endif
#if ! HAVE_DECL_SCTP_CLOSED && HAVE_DECL_SCTPS_IDLE
#    define SCTP_CLOSED SCTPS_IDLE
#    undef HAVE_DECL_SCTP_CLOSED
#    define HAVE_DECL_SCTP_CLOSED 1
#endif
#if ! HAVE_DECL_SCTP_BOUND && HAVE_DECL_SCTPS_BOUND
#    define SCTP_BOUND SCTPS_BOUND
#    undef HAVE_DECL_SCTP_BOUND
#    define HAVE_DECL_SCTP_BOUND 1
#endif
#if ! HAVE_DECL_SCTP_LISTEN && HAVE_DECL_SCTPS_LISTEN
#    define SCTP_LISTEN SCTPS_LISTEN
#    undef HAVE_DECL_SCTP_LISTEN
#    define HAVE_DECL_SCTP_LISTEN 1
#endif
#if ! HAVE_DECL_SCTP_COOKIE_WAIT && HAVE_DECL_SCTPS_COOKIE_WAIT
#    define SCTP_COOKIE_WAIT SCTPS_COOKIE_WAIT
#    undef HAVE_DECL_SCTP_COOKIE_WAIT
#    define HAVE_DECL_SCTP_COOKIE_WAIT 1
#endif
#if ! HAVE_DECL_SCTP_COOKIE_ECHOED && HAVE_DECL_SCTPS_COOKIE_ECHOED
#    define SCTP_COOKIE_ECHOED SCTPS_COOKIE_ECHOED
#    undef HAVE_DECL_SCTP_COOKIE_ECHOED
#    define HAVE_DECL_SCTP_COOKIE_ECHOED 1
#endif
#if ! HAVE_DECL_SCTP_ESTABLISHED && HAVE_DECL_SCTPS_ESTABLISHED
#    define SCTP_ESTABLISHED SCTPS_ESTABLISHED
#    undef HAVE_DECL_SCTP_ESTABLISHED
#    define HAVE_DECL_SCTP_ESTABLISHED 1
#endif
#if ! HAVE_DECL_SCTP_SHUTDOWN_PENDING && HAVE_DECL_SCTPS_SHUTDOWN_PENDING
#    define SCTP_SHUTDOWN_PENDING SCTPS_SHUTDOWN_PENDING
#    undef HAVE_DECL_SCTP_SHUTDOWN_PENDING
#    define HAVE_DECL_SCTP_SHUTDOWN_PENDING 1
#endif
#if ! HAVE_DECL_SCTP_SHUTDOWN_SENT && HAVE_DECL_SCTPS_SHUTDOWN_SENT
#    define SCTP_SHUTDOWN_SENT SCTPS_SHUTDOWN_SENT
#    undef HAVE_DECL_SCTP_SHUTDOWN_SENT
#    define HAVE_DECL_SCTP_SHUTDOWN_SENT 1
#endif
#if ! HAVE_DECL_SCTP_SHUTDOWN_RECEIVED && HAVE_DECL_SCTPS_SHUTDOWN_RECEIVED
#    define SCTP_SHUTDOWN_RECEIVED SCTPS_SHUTDOWN_RECEIVED
#    undef HAVE_DECL_SCTP_SHUTDOWN_RECEIVED
#    define HAVE_DECL_SCTP_SHUTDOWN_RECEIVED 1
#endif
#if ! HAVE_DECL_SCTP_SHUTDOWN_ACK_SENT && HAVE_DECL_SCTPS_SHUTDOWN_ACK_SENT
#    define SCTP_SHUTDOWN_ACK_SENT SCTPS_SHUTDOWN_ACK_SENT
#    undef HAVE_DECL_SCTP_SHUTDOWN_ACK_SENT
#    define HAVE_DECL_SCTP_SHUTDOWN_ACK_SENT 1
#endif
#if !defined(SCTP_ADAPTATION_LAYER) && defined (SCTP_ADAPTION_LAYER)
#     define SCTP_ADAPTATION_LAYER       SCTP_ADAPTION_LAYER
#     define SCTP_ADAPTATION_INDICATION  SCTP_ADAPTION_INDICATION
#     define sctp_adaptation_event       sctp_adaption_event
#     define sctp_setadaptation          sctp_setadaption
#     define sn_adaptation_event         sn_adaption_event
#     define sai_adaptation_ind          sai_adaption_ind
#     define ssb_adaptation_ind          ssb_adaption_ind
#     define sctp_adaptation_layer_event sctp_adaption_layer_event
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_BINDX)
static typeof(sctp_bindx) *p_sctp_bindx = NULL;
#else
static int (*p_sctp_bindx)
	(int sd, struct sockaddr *addrs, int addrcnt, int flags) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_PEELOFF)
static typeof(sctp_peeloff) *p_sctp_peeloff = NULL;
#else
static int (*p_sctp_peeloff)
        (int sd, sctp_assoc_t assoc_id) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_GETLADDRS)
static typeof(sctp_getladdrs) *p_sctp_getladdrs = NULL;
#else
static int (*p_sctp_getladdrs)
        (int sd, sctp_assoc_t assoc_id, struct sockaddr **ss) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_FREELADDRS)
static typeof(sctp_freeladdrs) *p_sctp_freeladdrs = NULL;
#else
static void (*p_sctp_freeladdrs)(struct sockaddr *addrs) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_GETPADDRS)
static typeof(sctp_getpaddrs) *p_sctp_getpaddrs = NULL;
#else
static int (*p_sctp_getpaddrs)
        (int sd, sctp_assoc_t assoc_id, struct sockaddr **ss) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_FREEPADDRS)
static typeof(sctp_freepaddrs) *p_sctp_freepaddrs = NULL;
#else
static void (*p_sctp_freepaddrs)(struct sockaddr *addrs) = NULL;
#endif
#if defined(__GNUC__) && defined(HAVE_SCTP_CONNECTX)
static typeof(sctp_connectx) *p_sctp_connectx = NULL;
#else
static int (*p_sctp_connectx)
        (int sd, struct sockaddr * addrs, int addrcnt, sctp_assoc_t * assoc_id) = NULL;
#endif
#endif
#ifndef WANT_NONBLOCKING
#define WANT_NONBLOCKING
#endif
#include "sys.h"
#ifdef INET_DRV_DEBUG
#define DEBUG 1
#undef  DEBUGF
#define DEBUGF(__X__) erts_printf __X__
#endif
#if !defined(HAVE_STRNCASECMP)
#define STRNCASECMP my_strncasecmp
static int my_strncasecmp(const char *s1, const char *s2, size_t n)
{
    int i;
    for (i=0;i<n-1 && s1[i] && s2[i] && toupper(s1[i]) == toupper(s2[i]);++i)
	;
    return (toupper(s1[i]) - toupper(s2[i]));
}
#else
#define  STRNCASECMP strncasecmp
#endif
#define INVALID_SOCKET -1
#define INVALID_EVENT  -1
#define SOCKET_ERROR   -1
#define SOCKET int
#define HANDLE long int
#define FD_READ    ERL_DRV_READ
#define FD_WRITE   ERL_DRV_WRITE
#define FD_CLOSE   0
#define FD_CONNECT ERL_DRV_WRITE
#define FD_ACCEPT  ERL_DRV_READ
#define sock_connect(s, addr, len)  connect((s), (addr), (len))
#define sock_listen(s, b)           listen((s), (b))
#define sock_bind(s, addr, len)     bind((s), (addr), (len))
#define sock_getopt(s,t,n,v,l)      getsockopt((s),(t),(n),(v),(l))
#define sock_setopt(s,t,n,v,l)      setsockopt((s),(t),(n),(v),(l))
#define sock_name(s, addr, len)     getsockname((s), (addr), (len))
#define sock_peer(s, addr, len)     getpeername((s), (addr), (len))
#define sock_ntohs(x)               ntohs((x))
#define sock_ntohl(x)               ntohl((x))
#define sock_htons(x)               htons((x))
#define sock_htonl(x)               htonl((x))
#define sock_accept(s, addr, len)   accept((s), (addr), (len))
#define sock_send(s,buf,len,flag)   send((s),(buf),(len),(flag))
#define sock_sendto(s,buf,blen,flag,addr,alen) \
                sendto((s),(buf),(blen),(flag),(addr),(alen))
#define sock_sendv(s, vec, size, np, flag) \
		(*(np) = writev((s), (struct iovec*)(vec), (size)))
#define sock_sendmsg(s,msghdr,flag) sendmsg((s),(msghdr),(flag))
#define sock_open(af, type, proto)  socket((af), (type), (proto))
#define sock_close(s)               close((s))
#define sock_shutdown(s, how)       shutdown((s), (how))
#define sock_hostname(buf, len)     gethostname((buf), (len))
#define sock_getservbyname(name,proto) getservbyname((name), (proto))
#define sock_getservbyport(port,proto) getservbyport((port), (proto))
#define sock_recv(s,buf,len,flag)   recv((s),(buf),(len),(flag))
#define sock_recvfrom(s,buf,blen,flag,addr,alen) \
                recvfrom((s),(buf),(blen),(flag),(addr),(alen))
#define sock_recvmsg(s,msghdr,flag) recvmsg((s),(msghdr),(flag))
#define sock_errno()                errno
#define sock_create_event(d)        ((d)->s)
#define sock_close_event(e)
#define inet_driver_select(port, e, mode, on) \
                                    driver_select(port, e, mode | (on?ERL_DRV_USE:0), on)
#define sock_select(d, flags, onoff) do { \
        ASSERT(!INET_IGNORED(d));         \
        (d)->event_mask = (onoff) ? \
                 ((d)->event_mask | (flags)) : \
                 ((d)->event_mask & ~(flags)); \
        DEBUGF(("(%s / %d) sock_select(%p): flags=%02X, onoff=%d, event_mask=%02lX\r\n", \
		__FILE__, __LINE__, (d)->port, (flags), (onoff), (unsigned long) (d)->event_mask)); \
        inet_driver_select((d)->port, (ErlDrvEvent)(long)(d)->event, (flags), (onoff)); \
   } while(0)
#define TCP_SHUT_WR    SHUT_WR
#define TCP_SHUT_RD    SHUT_RD
#define TCP_SHUT_RDWR  SHUT_RDWR
#endif
#ifdef HAVE_SOCKLEN_T
#  define SOCKLEN_T socklen_t
#elif defined(__WIN32__)
#  define SOCKLEN_T int
#else
#  warning "Non-Windows OS without type 'socklen_t'"
#  define SOCKLEN_T size_t
#endif
#include "packet_parser.h"
#if defined(HAVE_SYS_UN_H) || defined(SO_BINDTODEVICE)
static size_t my_strnlen(const char *s, size_t maxlen)
{
    size_t i = 0;
    while (i < maxlen && s[i] != '\0')
        i++;
    return i;
}
#endif
#ifdef VALGRIND
#  include <valgrind/memcheck.h>
#else
#  define VALGRIND_MAKE_MEM_DEFINED(ptr,size)
#endif
#ifndef __WIN32__
#define LEN_CMSG_DATA(cmsg)                                             \
    ((cmsg)->cmsg_len < sizeof (struct cmsghdr) ? 0 :                   \
     (cmsg)->cmsg_len - ((char*)CMSG_DATA(cmsg) - (char*)(cmsg)))
#define NXT_CMSG_HDR(cmsg)                                              \
    ((struct cmsghdr*)(((char*)(cmsg)) + CMSG_SPACE(LEN_CMSG_DATA(cmsg))))
#endif
#if !defined(IPV6_PKTOPTIONS) && defined(IPV6_2292PKTOPTIONS)
#define IPV6_PKTOPTIONS IPV6_2292PKTOPTIONS
#endif
#define INET_ERRNO_SYSTEM_LIMIT  (15 << 8)
#define INET_AF_UNSPEC      0
#define INET_AF_INET        1
#define INET_AF_INET6       2
#define INET_AF_ANY         3
#define INET_AF_LOOPBACK    4
#define INET_AF_LOCAL       5
#define INET_AF_UNDEFINED   6
#define INET_AF_LIST        7
#define INET_TYPE_STREAM    1
#define INET_TYPE_DGRAM     2
#define INET_TYPE_SEQPACKET 3
#define INET_PROTO_DEFAULT  0
#define INET_PROTO_TCP      1
#define INET_PROTO_UDP      2
#define INET_PROTO_SCTP     3
#define INET_PROTO_MPTCP    4
#define INET_MODE_LIST      0
#define INET_MODE_BINARY    1
#define INET_DELIVER_PORT   0
#define INET_DELIVER_TERM   1
#define INET_PASSIVE        0
#define INET_ACTIVE         1
#define INET_ONCE           2
#define INET_MULTI          3
#define INET_F_OPEN         0x0001
#define INET_F_ACTIVE       0x0004
#define INET_F_LISTEN       0x0008
#define INET_F_CON          0x0010
#define INET_F_ACC          0x0020
#define INET_F_LST          0x0040
#define INET_F_BUSY         0x0080
#define INET_F_MULTI_CLIENT 0x0100
#define INET_REQ_OPEN          1
#define INET_REQ_CLOSE         2
#define INET_REQ_CONNECT       3
#define INET_REQ_PEER          4
#define INET_REQ_NAME          5
#define INET_REQ_BIND          6
#define INET_REQ_SETOPTS       7
#define INET_REQ_GETOPTS       8
#define INET_REQ_GETSTAT       11
#define INET_REQ_GETHOSTNAME   12
#define INET_REQ_FDOPEN        13
#define INET_REQ_GETFD         14
#define INET_REQ_GETTYPE       15
#define INET_REQ_GETSTATUS     16
#define INET_REQ_GETSERVBYNAME 17
#define INET_REQ_GETSERVBYPORT 18
#define INET_REQ_SETNAME       19
#define INET_REQ_SETPEER       20
#define INET_REQ_GETIFLIST     21
#define INET_REQ_IFGET         22
#define INET_REQ_IFSET         23
#define INET_REQ_SUBSCRIBE     24
#define INET_REQ_GETIFADDRS    25
#define INET_REQ_ACCEPT        26
#define INET_REQ_LISTEN        27
#define INET_REQ_IGNOREFD      28
#define INET_REQ_GETLADDRS     29
#define INET_REQ_GETPADDRS     30
#define TCP_REQ_RECV           42
#define TCP_REQ_UNRECV         43
#define TCP_REQ_SHUTDOWN       44
#define TCP_REQ_SENDFILE       45
#define PACKET_REQ_RECV        60
#define SCTP_REQ_BINDX	       62
#define SCTP_REQ_PEELOFF       63
#define INET_SUBS_EMPTY_OUT_Q  1
#define TCP_ADDF_DELAY_SEND    1
#define TCP_ADDF_CLOSE_SENT    2
#define TCP_ADDF_DELAYED_CLOSE_RECV 4
#define TCP_ADDF_DELAYED_CLOSE_SEND 8
#define TCP_ADDF_PENDING_SHUT_WR   16
#define TCP_ADDF_PENDING_SHUT_RDWR 32
#define TCP_ADDF_PENDING_SHUTDOWN \
		(TCP_ADDF_PENDING_SHUT_WR | TCP_ADDF_PENDING_SHUT_RDWR)
#define TCP_ADDF_SHOW_ECONNRESET   64
#define TCP_ADDF_DELAYED_ECONNRESET 128
#define TCP_ADDF_SHUTDOWN_WR_DONE 256
#define TCP_ADDF_LINGER_ZERO 	  512
#define TCP_ADDF_SENDFILE         1024
#define TCP_ADDF_NO_READ_AHEAD    2048
#define INET_REP_ERROR       0
#define INET_REP_OK          1
#define INET_REP             2
#define INET_OPT_REUSEADDR  0
#define INET_OPT_KEEPALIVE  1
#define INET_OPT_DONTROUTE  2
#define INET_OPT_LINGER     3
#define INET_OPT_BROADCAST  4
#define INET_OPT_OOBINLINE  5
#define INET_OPT_SNDBUF     6
#define INET_OPT_RCVBUF     7
#define INET_OPT_PRIORITY   8
#define INET_OPT_TOS        9
#define TCP_OPT_NODELAY     10
#define UDP_OPT_MULTICAST_IF 11
#define UDP_OPT_MULTICAST_TTL 12
#define UDP_OPT_MULTICAST_LOOP 13
#define UDP_OPT_ADD_MEMBERSHIP 14
#define UDP_OPT_DROP_MEMBERSHIP 15
#define INET_OPT_IPV6_V6ONLY 16
#define INET_OPT_REUSEPORT   17
#define INET_OPT_REUSEPORT_LB 18
#define INET_OPT_EXCLUSIVEADDRUSE 19
#define INET_LOPT_BUFFER      20
#define INET_LOPT_HEADER      21
#define INET_LOPT_ACTIVE      22
#define INET_LOPT_PACKET      23
#define INET_LOPT_MODE        24
#define INET_LOPT_DELIVER     25
#define INET_LOPT_EXITONCLOSE 26
#define INET_LOPT_TCP_HIWTRMRK     27
#define INET_LOPT_TCP_LOWTRMRK     28
#define INET_LOPT_TCP_SEND_TIMEOUT 30
#define INET_LOPT_TCP_DELAY_SEND   31
#define INET_LOPT_PACKET_SIZE      32
#define INET_LOPT_UDP_READ_PACKETS 33
#define INET_OPT_RAW               34
#define INET_LOPT_TCP_SEND_TIMEOUT_CLOSE 35
#define INET_LOPT_MSGQ_HIWTRMRK     36
#define INET_LOPT_MSGQ_LOWTRMRK     37
#define INET_LOPT_NETNS             38
#define INET_LOPT_TCP_SHOW_ECONNRESET 39
#define INET_LOPT_LINE_DELIM        40
#define INET_OPT_TCLASS             41
#define INET_OPT_BIND_TO_DEVICE     42
#define INET_OPT_RECVTOS            43
#define INET_OPT_RECVTCLASS         44
#define INET_OPT_PKTOPTIONS         45
#define INET_OPT_TTL                46
#define INET_OPT_RECVTTL            47
#define TCP_OPT_NOPUSH              48
#define INET_LOPT_TCP_READ_AHEAD    49
#define INET_LOPT_NON_BLOCK_SEND    50
#define TCP_OPT_KEEPCNT             51
#define TCP_OPT_KEEPIDLE            52
#define TCP_OPT_KEEPINTVL           53
#define TCP_OPT_USER_TIMEOUT        54
#define INET_LOPT_DEBUG             99
#define SCTP_OPT_RTOINFO		100
#define SCTP_OPT_ASSOCINFO		101
#define SCTP_OPT_INITMSG		102
#define SCTP_OPT_AUTOCLOSE		103
#define SCTP_OPT_NODELAY		104
#define SCTP_OPT_DISABLE_FRAGMENTS	105
#define SCTP_OPT_I_WANT_MAPPED_V4_ADDR	106
#define SCTP_OPT_MAXSEG			107
#define SCTP_OPT_SET_PEER_PRIMARY_ADDR  108
#define SCTP_OPT_PRIMARY_ADDR		109
#define SCTP_OPT_ADAPTATION_LAYER 	110
#define SCTP_OPT_PEER_ADDR_PARAMS	111
#define SCTP_OPT_DEFAULT_SEND_PARAM	112
#define SCTP_OPT_EVENTS			113
#define SCTP_OPT_DELAYED_ACK_TIME	114
#define SCTP_OPT_STATUS			115
#define SCTP_OPT_GET_PEER_ADDR_INFO	116
#define INET_IFOPT_ADDR       1
#define INET_IFOPT_BROADADDR  2
#define INET_IFOPT_DSTADDR    3
#define INET_IFOPT_MTU        4
#define INET_IFOPT_NETMASK    5
#define INET_IFOPT_FLAGS      6
#define INET_IFOPT_HWADDR     7
#define INET_STAT_RECV_CNT   1
#define INET_STAT_RECV_MAX   2
#define INET_STAT_RECV_AVG   3
#define INET_STAT_RECV_DVI   4
#define INET_STAT_SEND_CNT   5
#define INET_STAT_SEND_MAX   6
#define INET_STAT_SEND_AVG   7
#define INET_STAT_SEND_PND   8
#define INET_STAT_RECV_OCT   9
#define INET_STAT_SEND_OCT   10
#define INET_IFF_UP            0x0001
#define INET_IFF_BROADCAST     0x0002
#define INET_IFF_LOOPBACK      0x0004
#define INET_IFF_POINTTOPOINT  0x0008
#define INET_IFF_RUNNING       0x0010
#define INET_IFF_MULTICAST     0x0020
#define INET_IFF_DOWN            0x0100
#define INET_IFF_NBROADCAST      0x0200
#define INET_IFF_NPOINTTOPOINT   0x0800
#define SCTP_FLAG_UNORDERED (1 )
#define SCTP_FLAG_ADDR_OVER (2 )
#define SCTP_FLAG_ABORT     (4 )
#define SCTP_FLAG_EOF       (8 )
#define SCTP_FLAG_SNDALL   (16 )
#define SCTP_FLAG_HB_ENABLE	    (1 )
#define SCTP_FLAG_HB_DISABLE	    (2 )
#define SCTP_FLAG_HB_DEMAND	    (4 )
#define	SCTP_FLAG_PMTUD_ENABLE	    (8 )
#define	SCTP_FLAG_PMTUD_DISABLE    (16 )
#define SCTP_FLAG_SACDELAY_ENABLE  (32 )
#define SCTP_FLAG_SACDELAY_DISABLE (64 )
#define INET_CMSG_RECVTOS        (1 << 0)
#define INET_CMSG_RECVTCLASS     (1 << 1)
#define INET_CMSG_RECVTTL        (1 << 2)
#define INET_FLG_BUFFER_SET      (1 << 0)
#define INET_FLG_IS_IGNORED      (1 << 1)
#define INET_FLG_IS_IGNORED_RD   (1 << 2)
#define INET_FLG_IS_IGNORED_WR   (1 << 3)
#define INET_FLG_IS_IGNORED_PASS (1 << 4)
#define INET_FLG_NON_BLOCK_SEND  (1 << 5)
#define INET_STATE_CLOSED          (0)
#define INET_STATE_OPEN            (INET_F_OPEN)
#define INET_STATE_CONNECTED       (INET_STATE_OPEN | INET_F_ACTIVE)
#define INET_STATE_LISTENING       (INET_STATE_OPEN | INET_F_LISTEN)
#define INET_STATE_CONNECTING      (INET_STATE_OPEN | INET_F_CON)
#define INET_STATE_ACCEPTING       (INET_STATE_LISTENING | INET_F_ACC)
#define INET_STATE_MULTI_ACCEPTING (INET_STATE_ACCEPTING | INET_F_MULTI_CLIENT)
#define IS_OPEN(d) \
 (((d)->state & INET_F_OPEN) == INET_F_OPEN)
#define IS_CONNECTED(d) \
  (((d)->state & INET_STATE_CONNECTED) == INET_STATE_CONNECTED)
#define IS_CONNECTING(d) \
  (((d)->state & INET_F_CON) == INET_F_CON)
#define IS_BUSY(d) \
  (((d)->state & INET_F_BUSY) == INET_F_BUSY)
#define INET_MAX_OPT_BUFFER (64*1024)
#if !defined(INET_BUFFER_TCP_DEFAULT)
#define INET_BUFFER_TCP_DEFAULT      9216
#endif
#if !defined(INET_BUFFER_UDP_DEFAULT)
#define INET_BUFFER_UDP_DEFAULT      9216
#endif
#if !defined(INET_BUFFER_SCTP_DEFAULT)
#define INET_BUFFER_SCTP_DEFAULT     9216
#endif
#define INET_BUFFER_MIN              1
#define INET_UDP_THEORETICAL_MAX_MTU (1 << 16)
#define INET_HIGH_WATERMARK (1024*8)
#define INET_LOW_WATERMARK  (1024*4)
#define INET_HIGH_MSGQ_WATERMARK (1024*8)
#define INET_LOW_MSGQ_WATERMARK  (1024*4)
#define INET_INFINITY  0xffffffff
#define INET_MAX_ASYNC 1
#define INET_PACKET_POLL     5
#define INET_IFNAMSIZ          16
#define INET_IGNORE_CLEAR(desc) ((desc)->flags & ~(INET_IGNORE_READ|INET_IGNORE_WRITE|INET_IGNORE_PASSIVE))
#define INET_IGNORED(desc)   ((desc)->flags & INET_FLG_IS_IGNORED)
#define INET_IGNORE_READ    (INET_FLG_IS_IGNORED|INET_FLG_IS_IGNORED_RD)
#define INET_IGNORE_WRITE   (INET_FLG_IS_IGNORED|INET_FLG_IS_IGNORED_WR)
#define INET_IGNORE_PASSIVE (INET_FLG_IS_IGNORED|INET_FLG_IS_IGNORED_PASS)
#define IS_NON_BLOCK_SEND(desc)    ((desc)->flags & INET_FLG_NON_BLOCK_SEND)
#define SET_NON_BLOCK_SEND(desc)   ((desc)->flags & INET_FLG_NON_BLOCK_SEND)
#define CLEAR_NON_BLOCK_SEND(desc) ((desc)->flags & ~(INET_FLG_NON_BLOCK_SEND))
#ifdef  HAVE_SCTP
#define PACKET_ERL_DRV_TERM_DATA_LEN  512
#else
#ifndef __WIN32__
#define PACKET_ERL_DRV_TERM_DATA_LEN  64
#else
#define PACKET_ERL_DRV_TERM_DATA_LEN  32
#endif
#endif
typedef struct _tcp_descriptor tcp_descriptor;
#if defined(TCP_CORK)
#define INET_TCP_NOPUSH TCP_CORK
#elif defined(TCP_NOPUSH) && !defined(__DARWIN__)
#define INET_TCP_NOPUSH TCP_NOPUSH
#endif
#define BIN_REALLOC_MARGIN(x)  ((x)/4)
typedef union {
    struct sockaddr sa;
    struct sockaddr_in sai;
#ifdef HAVE_IN6
    struct sockaddr_in6 sai6;
#endif
#ifdef HAVE_SYS_UN_H
    struct sockaddr_un sal;
#endif
} inet_address;
#if defined(HAVE_IN6)
#if defined(IPV6_ADD_MEMBERSHIP)
#define INET_ADD_MEMBERSHIP IPV6_ADD_MEMBERSHIP
#elif defined(IPV6_JOIN_GROUP)
#define INET_ADD_MEMBERSHIP IPV6_JOIN_GROUP
#endif
#if defined(IPV6_DROP_MEMBERSHIP)
#define INET_DROP_MEMBERSHIP IPV6_DROP_MEMBERSHIP
#elif defined(IPV6_LEAVE_GROUP)
#define INET_DROP_MEMBERSHIP IPV6_LEAVE_GROUP
#endif
#endif
#define inet_address_port(x)			\
  ((((x)->sai.sin_family == AF_INET) ||		\
    ((x)->sai.sin_family == AF_INET6)) ?	\
   ((x)->sai.sin_port) : -1)
#ifdef HAVE_SYS_UN_H
#define localaddrlen(data)				\
  ((((unsigned char*)(data))[0] == INET_AF_LOCAL) ?	\
   (1 + 1 + ((unsigned char*)(data))[1]) : 1)
#else
#define localaddrlen(data) (1)
#endif
#if defined(HAVE_IN6) && defined(AF_INET6)
#define addrlen(data)					\
    ((((unsigned char*)(data))[0] == INET_AF_INET) ?	\
     (1 + 2 + 4) :					\
     ((((unsigned char*)(data))[0] == INET_AF_INET6) ?	\
      (1 + 2 + 16) : localaddrlen(data)))
#else
#define addrlen(data)					\
    ((((unsigned char*)(data))[0] == INET_AF_INET) ?	\
     (1 + 2 + 4) : localaddrlen(data))
#endif
typedef struct {
    ErlDrvSizeT  tag_len;
    char        *tag_buf;
} CallerRef;
typedef int (MultiTimerFunction)(ErlDrvData drv_data, ErlDrvTermData caller);
typedef struct _multi_timer_data {
    ErlDrvTime when;
    ErlDrvTermData caller;
    MultiTimerFunction *timeout_function;
    struct _multi_timer_data *next;
    struct _multi_timer_data *prev;
} MultiTimerData;
static MultiTimerData *add_multi_timer(tcp_descriptor *desc, ErlDrvPort port,
                                       ErlDrvTermData caller, unsigned timeout,
                                       MultiTimerFunction *timeout_fun);
static void fire_multi_timers(tcp_descriptor *desc, ErlDrvPort port,
			      ErlDrvData data);
static void remove_multi_timer(tcp_descriptor *desc, ErlDrvPort port, MultiTimerData *p);
static void cancel_multi_timer(tcp_descriptor *desc, ErlDrvPort port,
                               MultiTimerFunction *timeout_fun);
static int tcp_inet_multi_timeout(ErlDrvData e, ErlDrvTermData caller);
static void clean_multi_timers(tcp_descriptor *desc, ErlDrvPort port);
typedef struct {
    int            id;
    ErlDrvTermData caller;
    int            req;
    union {
	unsigned       value;
	MultiTimerData *mtd;
    } tmo;
    ErlDrvMonitor monitor;
} inet_async_op;
typedef struct inet_async_multi_op_ {
    inet_async_op op;
    struct inet_async_multi_op_ *next;
} inet_async_multi_op;
typedef struct subs_list_ {
  ErlDrvTermData subscriber;
  struct subs_list_ *next;
} subs_list;
#define NO_PROCESS 0
#define NO_SUBSCRIBERS(SLP) ((SLP)->subscriber == NO_PROCESS)
static void send_to_subscribers(ErlDrvTermData, subs_list *, int,
				ErlDrvTermData [], int);
static void free_subscribers(subs_list*);
static int save_subscriber(subs_list *, ErlDrvTermData);
typedef struct {
    SOCKET s;
    HANDLE event;
    long  event_mask;
#ifdef __WIN32__
    long forced_events;
    int err;
    int send_would_block;
#endif
    ErlDrvPort  port;
    ErlDrvTermData dport;
    int   state;
    int   prebound;
    int   mode;
    int   exitf;
    int   deliver;
#ifdef __WIN32__
    int bsd_compat;
#endif
    ErlDrvTermData  caller;
    CallerRef       caller_ref;
    inet_async_op* oph;
    inet_async_op* opt;
    inet_async_op  op_queue[INET_MAX_ASYNC];
    int op_ref;
    int   active;
    Sint16 active_count;
    int   stype;
    int   sprotocol;
    int   sfamily;
    enum PacketParseType htype;
    unsigned int psize;
    inet_address remote;
    inet_address peer_addr;
    inet_address name_addr;
    inet_address* peer_ptr;
    inet_address* name_ptr;
    unsigned int peer_addr_len;
    unsigned int name_addr_len;
    int   bufsz;
    unsigned int hsz;
#ifdef ARCH_64
    Uint64        recv_oct;
#else
    Uint32        recv_oct[2];
#endif
    unsigned long recv_cnt;
    unsigned long recv_max;
    double recv_avg;
    double recv_dvi;
#ifdef ARCH_64
    Uint64        send_oct;
#else
    Uint32        send_oct[2];
#endif
    char          delimiter;
    unsigned long send_cnt;
    unsigned long send_max;
    double send_avg;
    subs_list empty_out_q_subs;
    int flags;
#ifdef HAVE_SETNS
    char *netns;
#endif
    int recv_cmsgflags;
    int debug;
} inet_descriptor;
#define TCP_MAX_PACKET_SIZE 0x4000000
#if defined(__WIN32__)
#define MAX_VSIZE 1024
#elif !defined(NO_SYSCONF)
static int iov_max;
#define MAX_VSIZE iov_max
#elif defined(IOV_MAX)
#define MAX_VSIZE IOV_MAX
#else
#define MAX_VSIZE 16
#endif
static int tcp_inet_init(void);
static void tcp_inet_stop(ErlDrvData);
static void tcp_inet_command(ErlDrvData, char*, ErlDrvSizeT);
static void tcp_inet_commandv(ErlDrvData, ErlIOVec*);
static void tcp_inet_flush(ErlDrvData drv_data);
static void tcp_inet_drv_input(ErlDrvData, ErlDrvEvent);
static void tcp_inet_drv_output(ErlDrvData data, ErlDrvEvent event);
static ErlDrvData tcp_inet_start(ErlDrvPort, char* command);
static ErlDrvSSizeT tcp_inet_ctl(ErlDrvData, unsigned int,
				 char*, ErlDrvSizeT, char**, ErlDrvSizeT);
static void tcp_inet_timeout(ErlDrvData);
static void tcp_inet_process_exit(ErlDrvData, ErlDrvMonitor *);
static void inet_stop_select(ErlDrvEvent, void*);
static void inet_emergency_close(ErlDrvData);
#ifdef __WIN32__
static void tcp_inet_event(ErlDrvData, ErlDrvEvent);
static void find_dynamic_functions(void);
#endif
static struct erl_drv_entry tcp_inet_driver_entry =
{
    tcp_inet_init,
    tcp_inet_start,
    tcp_inet_stop,
    tcp_inet_command,
#ifdef __WIN32__
    tcp_inet_event,
    NULL,
#else
    tcp_inet_drv_input,
    tcp_inet_drv_output,
#endif
    "tcp_inet",
    NULL,
    NULL,
    tcp_inet_ctl,
    tcp_inet_timeout,
    tcp_inet_commandv,
    NULL,
    tcp_inet_flush,
    NULL,
    NULL,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING|ERL_DRV_FLAG_SOFT_BUSY,
    NULL,
    tcp_inet_process_exit,
    inet_stop_select,
    inet_emergency_close
};
#ifdef HAVE_UDP
#   define IS_UDP(desc)((desc)->sprotocol==IPPROTO_UDP)
static int        packet_inet_init(void);
static void       packet_inet_stop(ErlDrvData);
static void       packet_inet_command(ErlDrvData, char*, ErlDrvSizeT);
static void       packet_inet_drv_input(ErlDrvData data, ErlDrvEvent event);
static void       packet_inet_drv_output(ErlDrvData data, ErlDrvEvent event);
static ErlDrvData udp_inet_start(ErlDrvPort, char* command);
#ifdef HAVE_SCTP
static ErlDrvData sctp_inet_start(ErlDrvPort, char* command);
#endif
static ErlDrvSSizeT packet_inet_ctl(ErlDrvData, unsigned int, char*,
				    ErlDrvSizeT, char**, ErlDrvSizeT);
static void       packet_inet_timeout(ErlDrvData);
#ifdef __WIN32__
static void       packet_inet_event(ErlDrvData, ErlDrvEvent);
static SOCKET     make_noninheritable_handle(SOCKET s);
static int        winsock_event_select(inet_descriptor *, int, int);
#endif
static struct erl_drv_entry udp_inet_driver_entry =
{
    packet_inet_init,
    udp_inet_start,
    packet_inet_stop,
    packet_inet_command,
#ifdef __WIN32__
    packet_inet_event,
    NULL,
#else
    packet_inet_drv_input,
    packet_inet_drv_output,
#endif
    "udp_inet",
    NULL,
    NULL,
    packet_inet_ctl,
    packet_inet_timeout,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL,
    NULL,
    inet_stop_select,
    inet_emergency_close
};
#else
#   define IS_UDP(desc) 0
#endif
#ifdef HAVE_SCTP
static struct erl_drv_entry sctp_inet_driver_entry =
{
    packet_inet_init,
    sctp_inet_start,
    packet_inet_stop,
    packet_inet_command,
#ifdef __WIN32__
    packet_inet_event,
    NULL,
#else
    packet_inet_drv_input,
    packet_inet_drv_output,
#endif
    "sctp_inet",
    NULL,
    NULL,
    packet_inet_ctl,
    packet_inet_timeout,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    ERL_DRV_FLAG_USE_PORT_LOCKING,
    NULL,
    NULL,
    inet_stop_select,
    inet_emergency_close
};
#endif
struct _tcp_descriptor {
    inet_descriptor inet;
    int   high;
    int   low;
    int   send_timeout;
    int   send_timeout_close;
    int   busy_on_send;
    int   i_bufsz;
    ErlDrvBinary* i_buf;
    char*         i_ptr;
    char*         i_ptr_start;
    int           i_remain;
    int           tcp_add_flags;
    int           http_state;
    inet_async_multi_op *multi_first;
    inet_async_multi_op *multi_last;
    MultiTimerData *mtd;
    MultiTimerData *mtd_cache;
#ifdef HAVE_SENDFILE
    struct {
        ErlDrvSizeT ioq_skip;
        int dup_file_fd;
        Uint64 bytes_sent;
        Uint64 offset;
        Uint64 length;
    } sendfile;
#endif
};
static int tcp_send(tcp_descriptor* desc, char* ptr, ErlDrvSizeT len);
static int tcp_sendv(tcp_descriptor* desc, ErlIOVec* ev);
static int tcp_recv(tcp_descriptor* desc, int request_len);
static int tcp_deliver(tcp_descriptor* desc, int len);
static int tcp_shutdown_error(tcp_descriptor* desc, int err);
#ifdef HAVE_SENDFILE
static int tcp_inet_sendfile(tcp_descriptor* desc);
static int tcp_sendfile_aborted(tcp_descriptor* desc, int socket_error);
#endif
static int tcp_inet_output(tcp_descriptor* desc, HANDLE event);
static int tcp_inet_input(tcp_descriptor* desc, HANDLE event);
static void tcp_desc_close(tcp_descriptor*);
#ifdef HAVE_UDP
typedef struct {
    inet_descriptor inet;
    int read_packets;
    int i_bufsz;
    ErlDrvBinary* i_buf;
    char* i_ptr;
} udp_descriptor;
static int packet_inet_input(udp_descriptor* udesc, HANDLE event);
#endif
#define INETP(d) (&(d)->inet)
#define NEW_ASYNC_ID(desc) ((desc)->op_ref++ & 0xffff)
#define INET_CHECK_ACTIVE_TO_PASSIVE(inet)                              \
    do {                                                                \
        if ((inet)->active == INET_ONCE)                                \
            (inet)->active = INET_PASSIVE;                              \
        else if ((inet)->active == INET_MULTI && --((inet)->active_count) == 0) { \
            (inet)->active = INET_PASSIVE;                              \
            packet_passive_message(inet);                               \
        }                                                               \
    } while (0)
static ErlDrvTermData am_ok;
static ErlDrvTermData am_undefined;
static ErlDrvTermData am_unspec;
static ErlDrvTermData am_tcp;
static ErlDrvTermData am_error;
static ErlDrvTermData am_einval;
static ErlDrvTermData am_inet_async;
static ErlDrvTermData am_inet_reply;
static ErlDrvTermData am_timeout;
static ErlDrvTermData am_closed;
static ErlDrvTermData am_tcp_passive;
static ErlDrvTermData am_tcp_closed;
static ErlDrvTermData am_tcp_error;
static ErlDrvTermData am_empty_out_q;
static ErlDrvTermData am_ssl_tls;
#ifdef HAVE_UDP
static ErlDrvTermData am_udp;
static ErlDrvTermData am_udp_passive;
static ErlDrvTermData am_udp_error;
#endif
#ifdef HAVE_SYS_UN_H
static ErlDrvTermData am_local;
#endif
#ifndef __WIN32__
static ErlDrvTermData am_tos;
static ErlDrvTermData am_tclass;
static ErlDrvTermData am_ttl;
#endif
#ifdef HAVE_SCTP
static ErlDrvTermData am_sctp;
static ErlDrvTermData am_sctp_passive;
static ErlDrvTermData am_sctp_error;
static ErlDrvTermData am_true;
static ErlDrvTermData am_false;
static ErlDrvTermData am_buffer;
static ErlDrvTermData am_mode;
static ErlDrvTermData am_list;
static ErlDrvTermData am_binary;
static ErlDrvTermData am_active;
static ErlDrvTermData am_once;
static ErlDrvTermData am_multi;
static ErlDrvTermData am_buffer;
static ErlDrvTermData am_linger;
static ErlDrvTermData am_recbuf;
static ErlDrvTermData am_sndbuf;
static ErlDrvTermData am_reuseaddr;
static ErlDrvTermData am_reuseport;
static ErlDrvTermData am_reuseport_lb;
static ErlDrvTermData am_exclusiveaddruse;
static ErlDrvTermData am_dontroute;
static ErlDrvTermData am_priority;
static ErlDrvTermData am_recvtos;
static ErlDrvTermData am_recvtclass;
static ErlDrvTermData am_recvttl;
static ErlDrvTermData am_ipv6_v6only;
static ErlDrvTermData am_netns;
static ErlDrvTermData am_bind_to_device;
#endif
#ifdef HAVE_SENDFILE
static ErlDrvTermData am_sendfile;
#endif
static char str_eafnosupport[] = "eafnosupport";
static char str_eprotonosupport[] = "eprotonosupport";
static char str_einval[] = "einval";
#define EXBADPORT "exbadport"
#define EXBADSEQ  "exbadseq"
static int inet_init(void);
static ErlDrvSSizeT ctl_reply(int, char*, ErlDrvSizeT, char**, ErlDrvSizeT);
struct erl_drv_entry inet_driver_entry =
{
    inet_init,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "inet",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ERL_DRV_EXTENDED_MARKER,
    ERL_DRV_EXTENDED_MAJOR_VERSION,
    ERL_DRV_EXTENDED_MINOR_VERSION,
    0,
    NULL,
    NULL,
    NULL,
};
#if HAVE_IN6
#  if ! defined(HAVE_IN6ADDR_ANY) || ! HAVE_IN6ADDR_ANY
#    if HAVE_DECL_IN6ADDR_ANY_INIT
static const struct in6_addr in6addr_any = { { IN6ADDR_ANY_INIT } };
#    else
static const struct in6_addr in6addr_any =
    { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 } } };
#    endif
#  endif
#  if ! defined(HAVE_IN6ADDR_LOOPBACK) || ! HAVE_IN6ADDR_LOOPBACK
#    if HAVE_DECL_IN6ADDR_LOOPBACK_INIT
static const struct in6_addr in6addr_loopback =
    { { IN6ADDR_LOOPBACK_INIT } };
#    else
static const struct in6_addr in6addr_loopback =
    { { { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 } } };
#    endif
#  endif
#endif
void erts_exit(int n, const char*, ...);
#ifdef FATAL_MALLOC
static void *alloc_wrapper(ErlDrvSizeT size){
    void *ret = driver_alloc(size);
    if(ret == NULL)
	erts_exit(ERTS_ERROR_EXIT,"Out of virtual memory in malloc (%s)", __FILE__);
    return ret;
}
#define ALLOC(X) alloc_wrapper(X)
static void *realloc_wrapper(void *current, ErlDrvSizeT size){
    void *ret = driver_realloc(current,size);
    if(ret == NULL)
	erts_exit(ERTS_ERROR_EXIT,"Out of virtual memory in realloc (%s)", __FILE__);
    return ret;
}
#define REALLOC(X,Y) realloc_wrapper(X,Y)
#define FREE(P)      driver_free((P))
#else
#define ALLOC(X)     driver_alloc((X))
#define REALLOC(X,Y) driver_realloc((X), (Y))
#define FREE(P)      driver_free((P))
#endif
#define INIT_ATOM(NAME) am_ ## NAME = driver_mk_atom(#NAME)
#define LOAD_ATOM_CNT 2
#define LOAD_ATOM(vec, i, atom) \
  (((vec)[(i)] = ERL_DRV_ATOM), \
  ((vec)[(i)+1] = (atom)), \
  ((i)+LOAD_ATOM_CNT))
#define LOAD_INT_CNT 2
#define LOAD_INT(vec, i, val) \
  (((vec)[(i)] = ERL_DRV_INT), \
  ((vec)[(i)+1] = (ErlDrvTermData)(val)), \
  ((i)+LOAD_INT_CNT))
#define LOAD_UINT_CNT 2
#define LOAD_UINT(vec, i, val) \
  (((vec)[(i)] = ERL_DRV_UINT), \
  ((vec)[(i)+1] = (ErlDrvTermData)(val)), \
  ((i)+LOAD_UINT_CNT))
#define LOAD_PORT_CNT 2
#define LOAD_PORT(vec, i, port) \
  (((vec)[(i)] = ERL_DRV_PORT), \
  ((vec)[(i)+1] = (port)), \
  ((i)+LOAD_PORT_CNT))
#define LOAD_PID_CNT 2
#define LOAD_PID(vec, i, pid) \
  (((vec)[(i)] = ERL_DRV_PID), \
  ((vec)[(i)+1] = (pid)), \
  ((i)+LOAD_PID_CNT))
#define LOAD_BINARY_CNT 4
#define LOAD_BINARY(vec, i, bin, offs, len) \
  (((vec)[(i)] = ERL_DRV_BINARY), \
  ((vec)[(i)+1] = (ErlDrvTermData)(bin)), \
  ((vec)[(i)+2] = (len)), \
  ((vec)[(i)+3] = (offs)), \
  ((i)+LOAD_BINARY_CNT))
#define LOAD_BUF2BINARY_CNT 3
#define LOAD_BUF2BINARY(vec, i, buf, len) \
  (((vec)[(i)] = ERL_DRV_BUF2BINARY), \
  ((vec)[(i)+1] = (ErlDrvTermData)(buf)), \
  ((vec)[(i)+2] = (len)), \
  ((i)+LOAD_BUF2BINARY_CNT))
#define LOAD_STRING_CNT 3
#define LOAD_STRING(vec, i, str, len) \
  (((vec)[(i)] = ERL_DRV_STRING), \
  ((vec)[(i)+1] = (ErlDrvTermData)(str)), \
  ((vec)[(i)+2] = (len)), \
  ((i)+LOAD_STRING_CNT))
#define LOAD_STRING_CONS_CNT 3
#define LOAD_STRING_CONS(vec, i, str, len) \
  (((vec)[(i)] = ERL_DRV_STRING_CONS), \
  ((vec)[(i)+1] = (ErlDrvTermData)(str)), \
  ((vec)[(i)+2] = (len)), \
  ((i)+LOAD_STRING_CONS_CNT))
#define LOAD_TUPLE_CNT 2
#define LOAD_TUPLE(vec, i, size) \
  (((vec)[(i)] = ERL_DRV_TUPLE), \
  ((vec)[(i)+1] = (size)), \
  ((i)+LOAD_TUPLE_CNT))
#define LOAD_NIL_CNT 1
#define LOAD_NIL(vec, i) \
  (((vec)[(i)] = ERL_DRV_NIL), \
  ((i)+LOAD_NIL_CNT))
#define LOAD_LIST_CNT 2
#define LOAD_LIST(vec, i, size) \
  (((vec)[(i)] = ERL_DRV_LIST), \
  ((vec)[(i)+1] = (size)), \
  ((i)+LOAD_LIST_CNT))
#define LOAD_EXT_CNT 3
#define LOAD_EXT(vec, i, buf, len) \
  (((vec)[(i)] = ERL_DRV_EXT2TERM), \
  ((vec)[(i)+1] = (ErlDrvTermData)(buf)), \
  ((vec)[(i)+2] = (len)), \
  ((i)+LOAD_EXT_CNT))
static void iov_memmove(char *buf, SysIOVec **iov_pp, ErlDrvSizeT size) {
    int i;
    for (i = 0;  size != 0;  i++) {
        if ((*iov_pp)[i].iov_len < size) {
            if ((*iov_pp)[i].iov_len == 0) continue;
            sys_memcpy(buf, (*iov_pp)[i].iov_base, (*iov_pp)[i].iov_len);
            buf += (*iov_pp)[i].iov_len;
            size -= (*iov_pp)[i].iov_len;
            (*iov_pp)[i].iov_base =
                (char*)((*iov_pp)[i].iov_base) + (*iov_pp)[i].iov_len;
            (*iov_pp)[i].iov_len = 0;
        }
        else {
            sys_memcpy(buf, (*iov_pp)[i].iov_base, size);
            (*iov_pp)[i].iov_base = (char*)((*iov_pp)[i].iov_base) + size;
            (*iov_pp)[i].iov_len -= size;
            break;
        }
    }
}
static int init_caller_iov(ErlDrvTermData *caller_p,
                           CallerRef      *cref_p,
                           ErlDrvPort      port,
                           SysIOVec      **iov_pp,
                           ErlDrvSizeT    *size_p) {
    ErlDrvTermData  caller;
    char            buf[2];
    ErlDrvUInt      n;
    caller = driver_caller(port);
    if (is_not_internal_pid(caller)) {
        return TRUE;
    }
    *caller_p = caller;
    if (*size_p < 2) return FALSE;
    iov_memmove(buf, iov_pp, 2);
    *size_p -= 2;
    n = get_int16(buf);
    if (*size_p < n) return FALSE;
    cref_p->tag_buf = ALLOC(n);
    iov_memmove(cref_p->tag_buf, iov_pp, n);
    *size_p -= n;
    cref_p->tag_len = n;
    return TRUE;
}
static int init_caller(ErlDrvTermData *caller_p,
                       CallerRef      *cref_p,
                       ErlDrvPort      port,
                       char          **buf_p,
                       ErlDrvSizeT    *size_p) {
    ErlDrvTermData  caller;
    ErlDrvUInt      n;
    caller = driver_caller(port);
    if (is_not_internal_pid(caller)) {
        return TRUE;
    }
    *caller_p = caller;
    if (*size_p < 2) return FALSE;
    n = get_int16(*buf_p);
    *buf_p += 2;
    *size_p -= 2;
    if (*size_p < n) return FALSE;
    cref_p->tag_buf = ALLOC(n);
    sys_memcpy(cref_p->tag_buf, *buf_p, n);
    *buf_p += n;
    *size_p -= n;
    cref_p->tag_len = n;
    return TRUE;
}
static CallerRef no_caller_ref(void) {
    CallerRef ret = {0, NULL};
    return ret;
}
static void end_caller_ref(CallerRef *cref_p) {
    if (cref_p->tag_buf != NULL) FREE(cref_p->tag_buf);
    *cref_p = no_caller_ref();
}
#ifdef HAVE_SCTP
#   define IS_SCTP(desc)((desc)->sprotocol==IPPROTO_SCTP)
#   define GET_ASSOC_ID		get_int32
#   define PUT_ASSOC_ID		put_int32
#   define ASSOC_ID_LEN		4
#   define LOAD_ASSOC_ID        LOAD_UINT
#   define LOAD_ASSOC_ID_CNT    LOAD_UINT_CNT
#else
#   define IS_SCTP(desc) 0
#endif
#define ANC_BUFF_SIZE           1024
#ifdef HAVE_UDP
static int load_address(ErlDrvTermData* spec, int i, char* buf)
{
    int n;
    switch (*buf++) {
    case INET_AF_INET: {
        for (n = 2;  n < 2+4;  n++) {
	    spec[i++] = ERL_DRV_INT;
	    spec[i++] = (ErlDrvTermData) ((unsigned char)buf[n]);
	}
	spec[i++] = ERL_DRV_TUPLE;
	spec[i++] = 4;
	spec[i++] = ERL_DRV_INT;
	spec[i++] = (ErlDrvTermData) get_int16(buf);
	break;
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    case INET_AF_INET6: {
	for (n = 2;  n < 2+16;  n += 2) {
	    spec[i++] = ERL_DRV_INT;
	    spec[i++] = (ErlDrvTermData) get_int16(buf+n);
	}
	spec[i++] = ERL_DRV_TUPLE;
	spec[i++] = 8;
	spec[i++] = ERL_DRV_INT;
	spec[i++] = (ErlDrvTermData) get_int16(buf);
	break;
    }
#endif
#ifdef HAVE_SYS_UN_H
    case INET_AF_LOCAL: {
	int len = *(unsigned char*)buf++;
	i = LOAD_ATOM(spec, i, am_local);
	i = LOAD_BUF2BINARY(spec, i, buf, len);
	spec[i++] = ERL_DRV_TUPLE;
	spec[i++] = 2;
	spec[i++] = ERL_DRV_INT;
	spec[i++] = 0;
	break;
    }
#endif
    case INET_AF_UNSPEC: {
        i = LOAD_ATOM(spec, i, am_unspec);
	i = LOAD_BUF2BINARY(spec, i, buf, 0);
	spec[i++] = ERL_DRV_TUPLE;
	spec[i++] = 2;
	spec[i++] = ERL_DRV_INT;
	spec[i++] = 0;
	break;
    }
    default: {
        i = LOAD_ATOM(spec, i, am_undefined);
	i = LOAD_BUF2BINARY(spec, i, buf, 0);
	spec[i++] = ERL_DRV_TUPLE;
	spec[i++] = 2;
	spec[i++] = ERL_DRV_INT;
	spec[i++] = 0;
	break;
    }
    }
    return i;
 }
static struct sockaddr disassoc_sa;
static size_t          disassoc_sa_size;
#endif
#ifdef HAVE_SCTP
static int inet_get_address(char* dst, inet_address* src, unsigned int* len);
#define LOAD_INET_GET_ADDRESS_CNT					\
        (8*LOAD_INT_CNT + LOAD_TUPLE_CNT + LOAD_INT_CNT + LOAD_TUPLE_CNT)
static int load_inet_get_address
           (ErlDrvTermData* spec,    int i, inet_descriptor* desc,
	    struct sockaddr_storage* addr)
{
    unsigned int len  = sizeof(struct sockaddr_storage);
    unsigned int alen = len;
    char         abuf  [len];
    int res = inet_get_address(abuf, (inet_address*) addr, &alen);
    ASSERT(res==0); (void)res;
    i = load_address(spec, i, abuf);
    i = LOAD_TUPLE     (spec, i, 2);
    return i;
}
#define LOAD_BOOL_CNT LOAD_ATOM_CNT
#define LOAD_BOOL(spec,   i,   flag)                          \
	LOAD_ATOM((spec), (i), (flag) ? am_true : am_false);
#endif
#define INET_DRV_CACHE_LINE_SIZE ((ErlDrvUInt) 64)
#define INET_DRV_CACHE_LINE_MASK (INET_DRV_CACHE_LINE_SIZE - 1)
#define BUFFER_STACK_SIZE 14
#define BUFFER_STACK_MAX_MEM_SIZE (1024*1024)
ErlDrvTSDKey buffer_stack_key;
typedef struct {
    int mem_size;
    int pos;
    ErlDrvBinary* stk[BUFFER_STACK_SIZE];
} InetDrvBufStkBase;
typedef struct {
    InetDrvBufStkBase buf;
    char align[(((sizeof(InetDrvBufStkBase) - 1) / INET_DRV_CACHE_LINE_SIZE) + 1)
	       * INET_DRV_CACHE_LINE_SIZE];
} InetDrvBufStk;
static InetDrvBufStk *get_bufstk(void)
{
    InetDrvBufStk *bs = erl_drv_tsd_get(buffer_stack_key);
    if (bs)
	return bs;
    bs = driver_alloc(sizeof(InetDrvBufStk)
		      + INET_DRV_CACHE_LINE_SIZE - 1);
    if (!bs)
	return NULL;
    if ((((ErlDrvUInt) bs) & INET_DRV_CACHE_LINE_MASK) != 0)
	bs = ((InetDrvBufStk *)
	      ((((ErlDrvUInt) bs) & ~INET_DRV_CACHE_LINE_MASK)
	       + INET_DRV_CACHE_LINE_SIZE));
    erl_drv_tsd_set(buffer_stack_key, bs);
    bs->buf.pos = 0;
    bs->buf.mem_size = 0;
    ASSERT(bs == erl_drv_tsd_get(buffer_stack_key));
    return bs;
}
static ErlDrvBinary* alloc_buffer(ErlDrvSizeT minsz)
{
    InetDrvBufStk *bs = get_bufstk();
    DEBUGF(("alloc_buffer: "LLU"\r\n", (llu_t)minsz));
    if (bs && bs->buf.pos > 0) {
	long size;
	ErlDrvBinary* buf = bs->buf.stk[--bs->buf.pos];
	size = buf->orig_size;
	bs->buf.mem_size -= size;
	ASSERT(0 <= bs->buf.mem_size
	       && bs->buf.mem_size <= BUFFER_STACK_MAX_MEM_SIZE);
	if (size >= minsz)
	    return buf;
	driver_free_binary(buf);
    }
    ASSERT(!bs || bs->buf.pos != 0 || bs->buf.mem_size == 0);
    return driver_alloc_binary(minsz);
}
#ifdef CHECK_DOUBLE_RELEASE
static void
check_double_release(InetDrvBufStk *bs, ErlDrvBinary* buf)
{
#ifdef __GNUC__
#warning CHECK_DOUBLE_RELEASE is enabled, this is a custom build emulator
#endif
    int i;
    for (i = 0; i < bs->buf.pos; ++i) {
	if (bs->buf.stk[i] == buf) {
	    erts_exit(ERTS_ABORT_EXIT,
		     "Multiple buffer release in inet_drv, this "
		     "is a bug, save the core and send it to "
		     "support@erlang.ericsson.se!");
	}
    }
}
#endif
static void release_buffer(ErlDrvBinary* buf)
{
    InetDrvBufStk *bs;
    long size;
    DEBUGF(("release_buffer: %ld\r\n", (buf==NULL) ? 0 : buf->orig_size));
    if (!buf)
	return;
    size = buf->orig_size;
    if (size > BUFFER_STACK_MAX_MEM_SIZE)
	goto free_binary;
    bs = get_bufstk();
    if (!bs
	|| (bs->buf.mem_size + size > BUFFER_STACK_MAX_MEM_SIZE)
	|| (bs->buf.pos >= BUFFER_STACK_SIZE)) {
    free_binary:
	driver_free_binary(buf);
    }
    else {
#ifdef CHECK_DOUBLE_RELEASE
	check_double_release(bs, buf);
#endif
	ASSERT(bs->buf.pos != 0 || bs->buf.mem_size == 0);
	bs->buf.mem_size += size;
	bs->buf.stk[bs->buf.pos++] = buf;
	ASSERT(0 <= bs->buf.mem_size
	       && bs->buf.mem_size <= BUFFER_STACK_MAX_MEM_SIZE);
    }
}
#ifdef HAVE_UDP
static ErlDrvBinary* realloc_buffer(ErlDrvBinary* buf, ErlDrvSizeT newsz)
{
    DEBUGF(("realloc_buffer: %ld -> %ld\r\n", (buf==NULL) ? 0 : buf->orig_size, newsz));
    return driver_realloc_binary(buf, newsz);
}
#endif
static void free_buffer(ErlDrvBinary* buf)
{
    DEBUGF(("free_buffer: %ld\r\n", (buf==NULL) ? 0 : buf->orig_size));
    if (buf != NULL) {
	if (driver_binary_get_refc(buf) == 1)
	    release_buffer(buf);
	else
	    driver_free_binary(buf);
    }
}
#ifdef __WIN32__
static ErlDrvData dummy_start(ErlDrvPort port, char* command)
{
    return (ErlDrvData)port;
}
static ErlDrvSSizeT dummy_ctl(ErlDrvData data, unsigned int cmd,
			      char* buf, ErlDrvSizeT len, char** rbuf,
			      ErlDrvSizeT rsize)
{
    static char error[] = "no_winsock2";
    driver_failure_atom((ErlDrvPort)data, error);
    return ctl_reply(INET_REP_ERROR, error, sizeof(error), rbuf, rsize);
}
static void dummy_command(ErlDrvData data, char* buf, ErlDrvSizeT len)
{
}
static struct erl_drv_entry dummy_tcp_driver_entry =
{
    NULL,
    dummy_start,
    NULL,
    dummy_command,
    NULL,
    NULL,
    "tcp_inet",
    NULL,
    NULL,
    dummy_ctl,
    NULL,
    NULL
};
static struct erl_drv_entry dummy_udp_driver_entry =
{
    NULL,
    dummy_start,
    NULL,
    dummy_command,
    NULL,
    NULL,
    "udp_inet",
    NULL,
    NULL,
    dummy_ctl,
    NULL,
    NULL
};
#ifdef HAVE_SCTP
static struct erl_drv_entry dummy_sctp_driver_entry =
{
    NULL,
    dummy_start,
    NULL,
    dummy_command,
    NULL,
    NULL,
    "sctp_inet",
    NULL,
    NULL,
    dummy_ctl,
    NULL,
    NULL
};
#endif
#endif
static char *errno_str(int err)
{
    switch (err) {
    case INET_ERRNO_SYSTEM_LIMIT:
	return "system_limit";
    default:
	return erl_errno_id(err);
    }
}
static ErlDrvSSizeT ctl_reply(int rep, char* buf, ErlDrvSizeT len,
			      char** rbuf, ErlDrvSizeT rsize)
{
    char* ptr;
    if ((len+1) > rsize) {
	ptr = ALLOC(len+1);
	*rbuf = ptr;
    }
    else
	ptr = *rbuf;
    *ptr++ = rep;
    if (buf) {
        memcpy(ptr, buf, len);
    } else {
        ASSERT(len == 0);
    }
    return len+1;
}
static ErlDrvSSizeT ctl_reply_ok(char** rbuf, ErlDrvSizeT rsize)
{
    return ctl_reply(INET_REP_OK, NULL, 0, rbuf, rsize);
}
#ifdef HAVE_SCTP
static ErlDrvSSizeT ctl_reply_ok_maybe_assoc_id(sctp_assoc_t* p_assoc_id,
                                                char** rbuf, ErlDrvSizeT rsize)
{
    if (p_assoc_id) {
        char buf[4];
        put_int32(*p_assoc_id, buf);
        return ctl_reply(INET_REP_OK, buf, sizeof(buf), rbuf, rsize);
    }
    else
        return ctl_reply_ok(rbuf, rsize);
}
#endif
static ErlDrvSSizeT ctl_error(int err, char** rbuf, ErlDrvSizeT rsize)
{
    char* s = errno_str(err);
    return ctl_reply(INET_REP_ERROR, s, strlen(s), rbuf, rsize);
}
static ErlDrvSSizeT ctl_xerror(char* xerr, char** rbuf, ErlDrvSizeT rsize)
{
    int n = strlen(xerr);
    return ctl_reply(INET_REP_ERROR, xerr, n, rbuf, rsize);
}
static ErlDrvTermData error_atom(int err)
{
    return driver_mk_atom(errno_str(err));
}
static void enq_old_multi_op(tcp_descriptor *desc, int id, int req,
			     ErlDrvTermData caller, MultiTimerData *timeout,
			     ErlDrvMonitor *monitorp)
{
    inet_async_multi_op *opp;
    opp = ALLOC(sizeof(inet_async_multi_op));
    opp->op.id = id;
    opp->op.caller = caller;
    opp->op.req = req;
    opp->op.tmo.mtd = timeout;
    memcpy(&(opp->op.monitor), monitorp, sizeof(ErlDrvMonitor));
    opp->next = NULL;
    if (desc->multi_first == NULL) {
	desc->multi_first = opp;
    } else {
	desc->multi_last->next = opp;
    }
    desc->multi_last = opp;
}
static void enq_multi_op(tcp_descriptor *desc, char *buf, int req,
			 ErlDrvTermData caller, MultiTimerData *timeout,
			 ErlDrvMonitor *monitorp)
{
    int id = NEW_ASYNC_ID(INETP(desc));
    enq_old_multi_op(desc,id,req,caller,timeout,monitorp);
    if (buf != NULL)
	put_int16(id, buf);
}
static int deq_multi_op(tcp_descriptor *desc, int *id_p, int *req_p,
			ErlDrvTermData *caller_p, MultiTimerData **timeout_p,
			ErlDrvMonitor *monitorp)
{
    inet_async_multi_op *opp;
    opp = desc->multi_first;
    if (!opp) {
	return -1;
    }
    desc->multi_first = opp->next;
    if (desc->multi_first == NULL) {
	desc->multi_last = NULL;
    }
    *id_p = opp->op.id;
    *req_p = opp->op.req;
    *caller_p = opp->op.caller;
    if (timeout_p != NULL) {
	*timeout_p = opp->op.tmo.mtd;
    }
    if (monitorp != NULL) {
	memcpy(monitorp,&(opp->op.monitor),sizeof(ErlDrvMonitor));
    }
    FREE(opp);
    return 0;
}
static int remove_multi_op(tcp_descriptor *desc, int *id_p, int *req_p,
			   ErlDrvTermData caller, MultiTimerData **timeout_p,
			   ErlDrvMonitor *monitorp)
{
    inet_async_multi_op *opp, *slap;
    for (opp = desc->multi_first, slap = NULL;
	 opp != NULL && opp->op.caller != caller;
	 slap = opp, opp = opp->next)
	;
    if (!opp) {
	return -1;
    }
    if (slap == NULL) {
	desc->multi_first = opp->next;
    } else {
	slap->next = opp->next;
    }
    if (desc->multi_last == opp) {
	desc->multi_last = slap;
    }
    *id_p = opp->op.id;
    *req_p = opp->op.req;
    if (timeout_p != NULL) {
	*timeout_p = opp->op.tmo.mtd;
    }
    if (monitorp != NULL) {
	memcpy(monitorp,&(opp->op.monitor),sizeof(ErlDrvMonitor));
    }
    FREE(opp);
    return 0;
}
static int enq_async_w_tmo(inet_descriptor* desc, char* buf, int req, unsigned timeout,
			   ErlDrvMonitor *monitorp)
{
    int id = NEW_ASYNC_ID(desc);
    inet_async_op* opp;
    if ((opp = desc->oph) == NULL)
	opp = desc->oph = desc->opt = desc->op_queue;
    else if (desc->oph == desc->opt) {
	DEBUGF(("enq(%p): queue full\r\n", desc->port));
	return -1;
    }
    opp->id = id;
    opp->caller = driver_caller(desc->port);
    opp->req = req;
    opp->tmo.value = timeout;
    if (monitorp != NULL) {
	memcpy(&(opp->monitor),monitorp,sizeof(ErlDrvMonitor));
    }
    DEBUGF(("enq(%p): %d %ld %d\r\n",
	    desc->port, opp->id, opp->caller, opp->req));
    opp++;
    if (opp >= desc->op_queue + INET_MAX_ASYNC)
	desc->oph = desc->op_queue;
    else
	desc->oph = opp;
    if (buf != NULL)
	put_int16(id, buf);
    return 0;
}
static int enq_async(inet_descriptor* desc, char* buf, int req)
{
    return enq_async_w_tmo(desc,buf,req,INET_INFINITY, NULL);
}
static int deq_async_w_tmo(inet_descriptor* desc, int* ap, ErlDrvTermData* cp,
			   int* rp, unsigned *tp, ErlDrvMonitor *monitorp)
{
    inet_async_op* opp;
    if ((opp = desc->opt) == NULL) {
	DEBUGF(("deq(%p): queue empty\r\n", desc->port));
	return -1;
    }
    *ap = opp->id;
    *cp = opp->caller;
    *rp = opp->req;
    if (tp != NULL) {
	*tp = opp->tmo.value;
    }
    if (monitorp != NULL) {
	memcpy(monitorp,&(opp->monitor),sizeof(ErlDrvMonitor));
    }
    DEBUGF(("deq(%p): %d %ld %d\r\n",
	    desc->port, opp->id, opp->caller, opp->req));
    opp++;
    if (opp >= desc->op_queue + INET_MAX_ASYNC)
	desc->opt = desc->op_queue;
    else
	desc->opt = opp;
    if (desc->opt == desc->oph)
	desc->opt = desc->oph = NULL;
    return 0;
}
static int deq_async(inet_descriptor* desc, int* ap, ErlDrvTermData* cp, int* rp)
{
    return deq_async_w_tmo(desc,ap,cp,rp,NULL,NULL);
}
static int
send_async_ok(ErlDrvTermData Port, int Ref,ErlDrvTermData recipient)
{
    ErlDrvTermData spec[2*LOAD_ATOM_CNT + LOAD_PORT_CNT +
			LOAD_INT_CNT + LOAD_TUPLE_CNT];
    int i = 0;
    i = LOAD_ATOM(spec, i, am_inet_async);
    i = LOAD_PORT(spec, i, Port);
    i = LOAD_INT(spec, i, Ref);
    i = LOAD_ATOM(spec, i, am_ok);
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    return erl_drv_send_term(Port, recipient, spec, i);
}
static int
send_async_ok_port(ErlDrvTermData Port, int Ref,
		   ErlDrvTermData recipient, ErlDrvTermData Port2)
{
    ErlDrvTermData spec[2*LOAD_ATOM_CNT + 2*LOAD_PORT_CNT +
			LOAD_INT_CNT + 2*LOAD_TUPLE_CNT];
    int i = 0;
    i = LOAD_ATOM(spec, i, am_inet_async);
    i = LOAD_PORT(spec, i, Port);
    i = LOAD_INT(spec, i, Ref);
    {
	i = LOAD_ATOM(spec, i, am_ok);
	i = LOAD_PORT(spec, i, Port2);
	i = LOAD_TUPLE(spec, i, 2);
    }
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    return erl_drv_send_term(Port, recipient, spec, i);
}
static int
send_async_error(ErlDrvTermData Port, int Ref,
		 ErlDrvTermData recipient, ErlDrvTermData Reason)
{
    ErlDrvTermData spec[3*LOAD_ATOM_CNT + LOAD_PORT_CNT +
			LOAD_INT_CNT + 2*LOAD_TUPLE_CNT];
    int i = 0;
    i = LOAD_ATOM(spec, i, am_inet_async);
    i = LOAD_PORT(spec, i, Port);
    i = LOAD_INT(spec, i, Ref);
    {
	i = LOAD_ATOM(spec, i, am_error);
	i = LOAD_ATOM(spec, i, Reason);
	i = LOAD_TUPLE(spec, i, 2);
    }
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    DEBUGF(("send_async_error %ld %ld\r\n", recipient, Reason));
    return erl_drv_send_term(Port, recipient, spec, i);
}
static int async_ok(inet_descriptor* desc)
{
    int req;
    int aid;
    ErlDrvTermData caller;
    if (deq_async(desc, &aid, &caller, &req) < 0)
	return -1;
    return send_async_ok(desc->dport, aid, caller);
}
static int async_ok_port(inet_descriptor* desc, ErlDrvTermData Port2)
{
    int req;
    int aid;
    ErlDrvTermData caller;
    if (deq_async(desc, &aid, &caller, &req) < 0)
	return -1;
    return send_async_ok_port(desc->dport, aid, caller, Port2);
}
static int async_error_am(inet_descriptor* desc, ErlDrvTermData reason)
{
    int req;
    int aid;
    ErlDrvTermData caller;
    if (deq_async(desc, &aid, &caller, &req) < 0)
	return -1;
    return send_async_error(desc->dport, aid, caller, reason);
}
static int async_error_am_all(inet_descriptor* desc, ErlDrvTermData reason)
{
    int req;
    int aid;
    ErlDrvTermData caller;
    while (deq_async(desc, &aid, &caller, &req) == 0) {
	send_async_error(desc->dport, aid, caller, reason);
    }
    return 0;
}
static int async_error(inet_descriptor* desc, int err)
{
    return async_error_am(desc, error_atom(err));
}
static int
inet_reply_finish(inet_descriptor *desc, ErlDrvTermData *spec, int i) {
    int          ret;
    ErlDrvSizeT  tuple_size;
    CallerRef   *cref_p;
    ret = 0;
    cref_p = &desc->caller_ref;
    if (i == 0) goto done;
    if (cref_p->tag_buf != NULL) {
        tuple_size = spec[i - 1];
        i = LOAD_EXT(spec, i - LOAD_TUPLE_CNT,
                     cref_p->tag_buf, cref_p->tag_len);
        i = LOAD_TUPLE(spec, i, tuple_size + 1);
    }
    ret = erl_drv_send_term(desc->dport, desc->caller, spec, i);
 done:
    desc->caller = am_undefined;
    end_caller_ref(cref_p);
    return ret;
}
static int inet_reply_caller_ref(inet_descriptor* desc)
{
    ErlDrvTermData
        spec[2*LOAD_ATOM_CNT + LOAD_PORT_CNT + LOAD_TUPLE_CNT +
             LOAD_EXT_CNT];
    CallerRef *cref_p = &desc->caller_ref;
    int i = 0;
    if (is_not_internal_pid(desc->caller)) return 0;
    i = LOAD_ATOM(spec, i, am_inet_reply);
    i = LOAD_PORT(spec, i, desc->dport);
    if (cref_p->tag_buf != NULL) {
        i = LOAD_EXT(spec, i, cref_p->tag_buf, cref_p->tag_len);
    }
    else {
        i = LOAD_ATOM(spec, i, am_undefined);
    }
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(sizeof(spec)/sizeof(*spec) >= i);
    return erl_drv_send_term(desc->dport, desc->caller, spec, i);
}
static int inet_reply_am(inet_descriptor* desc, ErlDrvTermData am)
{
    ErlDrvTermData
        spec[2*LOAD_ATOM_CNT + LOAD_PORT_CNT + LOAD_TUPLE_CNT +
             LOAD_EXT_CNT];
    int i = 0;
    if (is_not_internal_pid(desc->caller)) goto done;
    i = LOAD_ATOM(spec, i, am_inet_reply);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_ATOM(spec, i, am);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec) - LOAD_EXT_CNT);
 done:
    return inet_reply_finish(desc, spec, i);
}
static int inet_reply_ok(inet_descriptor* desc)
{
    return inet_reply_am(desc, am_ok);
}
#ifdef HAVE_SCTP
static int inet_reply_ok_port(inet_descriptor* desc, ErlDrvTermData dport)
{
    ErlDrvTermData
        spec[2*LOAD_ATOM_CNT + 2*LOAD_PORT_CNT + 2*LOAD_TUPLE_CNT +
             LOAD_EXT_CNT];
    int i = 0;
    if (is_not_internal_pid(desc->caller)) goto done;
    i = LOAD_ATOM(spec, i, am_inet_reply);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_ATOM(spec, i, am_ok);
    i = LOAD_PORT(spec, i, dport);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec) - LOAD_EXT_CNT);
 done:
    return inet_reply_finish(desc, spec, i);
}
#endif
static int inet_reply_error_am(inet_descriptor* desc, ErlDrvTermData reason)
{
    ErlDrvTermData
        spec[3*LOAD_ATOM_CNT + LOAD_PORT_CNT + 2*LOAD_TUPLE_CNT +
             LOAD_EXT_CNT];
    int i = 0;
    if (is_not_internal_pid(desc->caller)) goto done;
    i = LOAD_ATOM(spec, i, am_inet_reply);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_ATOM(spec, i, am_error);
    i = LOAD_ATOM(spec, i, reason);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec) - LOAD_EXT_CNT);
    DEBUGF(("inet_reply_error_am %ld %ld\r\n",
            desc->caller, reason));
 done:
    return inet_reply_finish(desc, spec, i);
}
static int inet_reply_error(inet_descriptor* desc, int err)
{
    return inet_reply_error_am(desc, error_atom(err));
}
static int inet_port_data(inet_descriptor* desc, const char* buf, int len)
{
    unsigned int hsz = desc->hsz;
    DEBUGF(("inet_port_data(%p): len = %d\r\n",
            desc->port, len));
    if ((desc->mode == INET_MODE_LIST) || (hsz > len))
	return driver_output2(desc->port, (char*)buf, len, NULL, 0);
    else if (hsz > 0)
	return driver_output2(desc->port, (char*)buf, hsz, (char*)buf+hsz, len-hsz);
    else
	return driver_output(desc->port, (char*)buf, len);
}
static int
inet_port_binary_data(inet_descriptor* desc, ErlDrvBinary* bin, int offs, int len)
{
    unsigned int hsz = desc->hsz;
    DEBUGF(("inet_port_binary_data(%p): offs=%d, len = %d\r\n",
	    desc->port, offs, len));
    if ((desc->mode == INET_MODE_LIST) || (hsz > len))
	return driver_output2(desc->port, bin->orig_bytes+offs, len, NULL, 0);
    else
	return driver_output_binary(desc->port, bin->orig_bytes+offs, hsz,
				    bin, offs+hsz, len-hsz);
}
static ErlDrvTermData am_http_eoh;
static ErlDrvTermData am_http_header;
static ErlDrvTermData am_http_request;
static ErlDrvTermData am_http_response;
static ErlDrvTermData am_http_error;
static ErlDrvTermData am_abs_path;
static ErlDrvTermData am_absoluteURI;
static ErlDrvTermData am_star;
static ErlDrvTermData am_http;
static ErlDrvTermData am_https;
static ErlDrvTermData am_scheme;
static int http_load_string(tcp_descriptor* desc, ErlDrvTermData* spec, int i,
			    const char* str, int len)
{
    if (desc->inet.htype >= TCP_PB_HTTP_BIN) {
	ASSERT(desc->inet.htype == TCP_PB_HTTP_BIN ||
	       desc->inet.htype == TCP_PB_HTTPH_BIN);
	i = LOAD_BUF2BINARY(spec, i, str, len);
    } else {
	i = LOAD_STRING(spec, i, str, len);
    }
    return i;
}
static int http_response_inetdrv(void *arg, int major, int minor,
				 int status, const char* phrase, int phrase_len)
{
    tcp_descriptor* desc = (tcp_descriptor*) arg;
    int i = 0;
    ErlDrvTermData spec[27];
    ErlDrvTermData caller = am_undefined;
    if (desc->inet.active == INET_PASSIVE) {
        int req;
        int aid;
        if (deq_async(INETP(desc), &aid, &caller, &req) < 0)
            return -1;
        i = LOAD_ATOM(spec, i,  am_inet_async);
        i = LOAD_PORT(spec, i,  desc->inet.dport);
        i = LOAD_INT(spec, i,   aid);
        i = LOAD_ATOM(spec, i,  am_ok);
    }
    else {
        i = LOAD_ATOM(spec, i, am_http);
        i = LOAD_PORT(spec, i, desc->inet.dport);
    }
    i = LOAD_ATOM(spec, i,  am_http_response);
    i = LOAD_INT(spec, i, major);
    i = LOAD_INT(spec, i, minor);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_INT(spec, i, status);
    i = http_load_string(desc, spec, i, phrase, phrase_len);
    i = LOAD_TUPLE(spec, i, 4);
    if (desc->inet.active == INET_PASSIVE) {
        i = LOAD_TUPLE(spec, i, 2);
        i = LOAD_TUPLE(spec, i, 4);
        ASSERT(i<=27);
        return erl_drv_send_term(desc->inet.dport, caller, spec, i);
    }
    else {
        i = LOAD_TUPLE(spec, i, 3);
        ASSERT(i<=27);
        return erl_drv_output_term(desc->inet.dport, spec, i);
    }
}
static int http_load_uri(tcp_descriptor* desc, ErlDrvTermData* spec, int i,
			 const PacketHttpURI* uri)
{
    ErlDrvTermData scheme;
    switch (uri->type) {
    case URI_STAR:
        i = LOAD_ATOM(spec, i, am_star);
        break;
    case URI_ABS_PATH:
        i = LOAD_ATOM(spec, i, am_abs_path);
        i = http_load_string(desc, spec, i, uri->s1_ptr, uri->s1_len);
        i = LOAD_TUPLE(spec, i, 2);
        break;
    case URI_HTTP:
        scheme = am_http;
        goto http_common;
    case URI_HTTPS:
        scheme = am_https;
    http_common:
        i = LOAD_ATOM(spec, i, am_absoluteURI);
        i = LOAD_ATOM(spec, i, scheme);
        i = http_load_string(desc, spec, i, uri->s1_ptr, uri->s1_len);
        if (uri->port == 0) {
            i = LOAD_ATOM(spec, i, am_undefined);
        } else {
            i = LOAD_INT(spec, i, uri->port);
        }
        i = http_load_string(desc, spec, i, uri->s2_ptr, uri->s2_len);
        i = LOAD_TUPLE(spec, i, 5);
        break;
    case URI_STRING:
        i = http_load_string(desc, spec, i, uri->s1_ptr, uri->s1_len);
        break;
    case URI_SCHEME:
        i = LOAD_ATOM(spec, i, am_scheme);
        i = http_load_string(desc, spec, i, uri->s1_ptr, uri->s1_len);
        i = http_load_string(desc, spec, i, uri->s2_ptr, uri->s2_len);
        i = LOAD_TUPLE(spec, i, 3);
    }
    return i;
}
static int
http_request_inetdrv(void* arg, const http_atom_t* meth, const char* meth_ptr,
		     int meth_len, const PacketHttpURI* uri,
		     int major, int minor)
{
    tcp_descriptor* desc = (tcp_descriptor*) arg;
    int i = 0;
    ErlDrvTermData spec[43];
    ErlDrvTermData caller = am_undefined;
    if (desc->inet.active == INET_PASSIVE) {
        int req;
        int aid;
        if (deq_async(INETP(desc), &aid, &caller, &req) < 0)
            return -1;
        i = LOAD_ATOM(spec, i,  am_inet_async);
        i = LOAD_PORT(spec, i,  desc->inet.dport);
        i = LOAD_INT(spec, i,   aid);
        i = LOAD_ATOM(spec, i,  am_ok);
    }
    else {
        i = LOAD_ATOM(spec, i, am_http);
        i = LOAD_PORT(spec, i, desc->inet.dport);
    }
    i = LOAD_ATOM(spec, i,  am_http_request);
    if (meth != NULL)
      i = LOAD_ATOM(spec, i, meth->atom);
    else
      i = http_load_string(desc, spec, i, meth_ptr, meth_len);
    i = http_load_uri(desc, spec, i, uri);
    i = LOAD_INT(spec, i, major);
    i = LOAD_INT(spec, i, minor);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 4);
    if (desc->inet.active == INET_PASSIVE) {
        i = LOAD_TUPLE(spec, i, 2);
        i = LOAD_TUPLE(spec, i, 4);
        ASSERT(i <= 43);
        return erl_drv_send_term(desc->inet.dport, caller, spec, i);
    }
    else {
        i = LOAD_TUPLE(spec, i, 3);
        ASSERT(i <= 43);
        return erl_drv_output_term(desc->inet.dport, spec, i);
    }
}
static int
http_header_inetdrv(void* arg, const http_atom_t* name,
		    const char* name_ptr, int name_len,
		    const char* oname_ptr, int oname_len,
		    const char* value_ptr, int value_len)
{
    tcp_descriptor* desc = (tcp_descriptor*) arg;
    int i = 0;
    ErlDrvTermData spec[27];
    ErlDrvTermData caller = am_undefined;
    if (desc->inet.active == INET_PASSIVE) {
        int req;
        int aid;
        if (deq_async(INETP(desc), &aid, &caller, &req) < 0)
            return -1;
        i = LOAD_ATOM(spec, i,  am_inet_async);
        i = LOAD_PORT(spec, i,  desc->inet.dport);
        i = LOAD_INT(spec, i,   aid);
        i = LOAD_ATOM(spec, i,  am_ok);
    }
    else {
        i = LOAD_ATOM(spec, i, am_http);
        i = LOAD_PORT(spec, i, desc->inet.dport);
    }
    i = LOAD_ATOM(spec, i,  am_http_header);
    if (name != NULL) {
      i = LOAD_INT(spec, i,  name->index+1);
      i = LOAD_ATOM(spec, i, name->atom);
    }
    else {
      i = LOAD_INT(spec, i,  0);
      i = http_load_string(desc, spec, i, name_ptr, name_len);
    }
    i = http_load_string(desc, spec, i, oname_ptr, oname_len);
    i = http_load_string(desc, spec, i, value_ptr, value_len);
    i = LOAD_TUPLE(spec, i, 5);
    if (desc->inet.active == INET_PASSIVE) {
        i = LOAD_TUPLE(spec, i, 2);
        i = LOAD_TUPLE(spec, i, 4);
        ASSERT(i <= 27);
        return erl_drv_send_term(desc->inet.dport, caller, spec, i);
    }
    else {
        i = LOAD_TUPLE(spec, i, 3);
        ASSERT(i <= 27);
        return erl_drv_output_term(desc->inet.dport, spec, i);
    }
}
static int http_eoh_inetdrv(void* arg)
{
  tcp_descriptor* desc = (tcp_descriptor*) arg;
  int i = 0;
  ErlDrvTermData spec[14];
  if (desc->inet.active == INET_PASSIVE) {
    int req;
    int aid;
    ErlDrvTermData caller;
    if (deq_async(INETP(desc), &aid, &caller, &req) < 0)
      return -1;
    i = LOAD_ATOM(spec, i,  am_inet_async);
    i = LOAD_PORT(spec, i,  desc->inet.dport);
    i = LOAD_INT(spec, i,   aid);
    i = LOAD_ATOM(spec, i,  am_ok);
    i = LOAD_ATOM(spec, i,  am_http_eoh);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i <= 14);
    return erl_drv_send_term(desc->inet.dport, caller, spec, i);
  }
  else {
      i = LOAD_ATOM(spec, i,  am_http);
      i = LOAD_PORT(spec, i,  desc->inet.dport);
      i = LOAD_ATOM(spec, i,  am_http_eoh);
      i = LOAD_TUPLE(spec, i, 3);
      ASSERT(i <= 14);
      return erl_drv_output_term(desc->inet.dport, spec, i);
  }
}
static int http_error_inetdrv(void* arg, const char* buf, int len)
{
  tcp_descriptor* desc = (tcp_descriptor*) arg;
  int i = 0;
  ErlDrvTermData spec[19];
  if (desc->inet.active == INET_PASSIVE) {
    int req;
    int aid;
    ErlDrvTermData caller;
    if (deq_async(INETP(desc), &aid, &caller, &req) < 0)
      return -1;
    i = LOAD_ATOM(spec, i,  am_inet_async);
    i = LOAD_PORT(spec, i,  desc->inet.dport);
    i = LOAD_INT(spec, i,   aid);
    i = LOAD_ATOM(spec, i,  am_ok);
    i = LOAD_ATOM(spec, i,  am_http_error);
    i = http_load_string(desc, spec, i, buf, len);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i <= 19);
    return erl_drv_send_term(desc->inet.dport, caller, spec, i);
  }
  else {
      i = LOAD_ATOM(spec, i,  am_http);
      i = LOAD_PORT(spec, i,  desc->inet.dport);
      i = LOAD_ATOM(spec, i,  am_http_error);
      i = http_load_string(desc, spec, i, buf, len);
      i = LOAD_TUPLE(spec, i, 2);
      i = LOAD_TUPLE(spec, i, 3);
      ASSERT(i <= 19);
      return erl_drv_output_term(desc->inet.dport, spec, i);
  }
}
static
int ssl_tls_inetdrv(void* arg, unsigned type, unsigned major, unsigned minor,
                    const char* buf, int len, const char* prefix, int plen)
{
    tcp_descriptor* desc = (tcp_descriptor*) arg;
    int i = 0;
    ErlDrvTermData spec[30];
    ErlDrvTermData caller = am_undefined;
    ErlDrvBinary* bin;
    int ret;
    if ((bin = driver_alloc_binary(plen+len)) == NULL)
        return async_error(&desc->inet, ENOMEM);
    memcpy(bin->orig_bytes+plen, buf, len);
    if (plen) {
        memcpy(bin->orig_bytes, prefix, plen);
        len += plen;
    }
    if (desc->inet.active == INET_PASSIVE) {
        int req;
        int aid;
        if (deq_async(INETP(desc), &aid, &caller, &req) < 0) {
            ret = -1;
            goto done;
        }
        i = LOAD_ATOM(spec, i,  am_inet_async);
        i = LOAD_PORT(spec, i,  desc->inet.dport);
        i = LOAD_INT(spec, i,   aid);
        i = LOAD_ATOM(spec, i,  am_ok);
    }
    i = LOAD_ATOM(spec, i,  am_ssl_tls);
    i = LOAD_PORT(spec, i,  desc->inet.dport);
    i = LOAD_INT(spec, i,   type);
    i = LOAD_INT(spec, i,   major);
    i = LOAD_INT(spec, i,   minor);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_BINARY(spec, i, bin, 0, len);
    i = LOAD_TUPLE(spec, i, 5);
    if (desc->inet.active == INET_PASSIVE) {
        i = LOAD_TUPLE(spec, i, 2);
        i = LOAD_TUPLE(spec, i, 4);
        ASSERT(i <= sizeof(spec)/sizeof(*spec));
        ret = erl_drv_send_term(desc->inet.dport, caller, spec, i);
    }
    else {
        ASSERT(i <= sizeof(spec)/sizeof(*spec));
        ret = erl_drv_output_term(desc->inet.dport, spec, i);
    }
done:
    driver_free_binary(bin);
    return ret;
}
static PacketCallbacks packet_callbacks =
{
    http_response_inetdrv,
    http_request_inetdrv,
    http_eoh_inetdrv,
    http_header_inetdrv,
    http_error_inetdrv,
    ssl_tls_inetdrv
};
static int inet_async_data(inet_descriptor* desc, const char* buf, int len)
{
    unsigned int hsz = desc->hsz;
    ErlDrvTermData spec[20];
    ErlDrvTermData caller;
    int req;
    int aid;
    int i = 0;
    DEBUGF(("inet_async_data(%p): len = %d\r\n",
            desc->port, len));
    if (deq_async(desc, &aid, &caller, &req) < 0)
	return -1;
    i = LOAD_ATOM(spec, i, am_inet_async);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_INT(spec, i, aid);
    i = LOAD_ATOM(spec, i, am_ok);
    if ((desc->mode == INET_MODE_LIST) || (hsz > len)) {
	i = LOAD_STRING(spec, i, buf, len);
	i = LOAD_TUPLE(spec, i, 2);
	i = LOAD_TUPLE(spec, i, 4);
	ASSERT(i == 15);
	return erl_drv_send_term(desc->dport, caller, spec, i);
    }
    else {
	int sz = len - hsz;
	int code;
	i = LOAD_BUF2BINARY(spec, i, buf+hsz, sz);
	if (hsz > 0)
	    i = LOAD_STRING_CONS(spec, i, buf, hsz);
	i = LOAD_TUPLE(spec, i, 2);
	i = LOAD_TUPLE(spec, i, 4);
	ASSERT(i <= 20);
	code = erl_drv_send_term(desc->dport, caller, spec, i);
	return code;
    }
}
#ifndef __WIN32__
static int load_cmsg_int(ErlDrvTermData *spec, int i,
                    struct cmsghdr *cmsg) {
    union u {
        byte uint8;
        Uint16 uint16;
        Uint32 uint32;
        Uint64 uint64;
    } *p;
    p = (union u*) CMSG_DATA(cmsg);
    switch (LEN_CMSG_DATA(cmsg) * CHAR_BIT) {
    case 8:
        return LOAD_INT(spec, i, p->uint8);
    case 16:
        return LOAD_INT(spec, i, p->uint16);
    case 32:
        return LOAD_INT(spec, i, p->uint32);
    case 64:
        return LOAD_INT(spec, i, p->uint64);
    }
    return LOAD_INT(spec, i, 0);
}
static int parse_ancillary_data_item(ErlDrvTermData *spec, int i,
                                     struct cmsghdr *cmsg, int *n) {
#define LOAD_CMSG_INT(proto, type, am)              \
    if (cmsg->cmsg_level == (proto) &&          \
        cmsg->cmsg_type == (type)) {            \
        i = LOAD_ATOM(spec, i, (am));           \
        i = load_cmsg_int(spec, i, cmsg);            \
        i = LOAD_TUPLE(spec, i, 2);             \
        (*n)++;                                 \
        return i;                               \
    }
#if defined(IPPROTO_IP) && defined(IP_TOS)
    LOAD_CMSG_INT(IPPROTO_IP, IP_TOS, am_tos);
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_TCLASS)
    LOAD_CMSG_INT(IPPROTO_IPV6, IPV6_TCLASS, am_tclass);
#endif
#if defined(IPPROTO_IP) && defined(IP_TTL)
    LOAD_CMSG_INT(IPPROTO_IP, IP_TTL, am_ttl);
#endif
#if defined(IPPROTO_IP) && defined(IP_RECVTOS)
    LOAD_CMSG_INT(IPPROTO_IP, IP_RECVTOS, am_tos);
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_RECVTCLASS)
    LOAD_CMSG_INT(IPPROTO_IPV6, IPV6_RECVTCLASS, am_tclass);
#endif
#if defined(IPPROTO_IP) && defined(IP_RECVTTL)
    LOAD_CMSG_INT(IPPROTO_IP, IP_RECVTTL, am_ttl);
#endif
#undef LOAD_CMSG_INT
    return i;
}
#endif
#ifdef HAVE_SCTP
static ErlDrvTermData   am_sctp_rtoinfo,
    am_sctp_associnfo,                 am_sctp_initmsg,
    am_sctp_autoclose,                 am_sctp_nodelay,
    am_sctp_disable_fragments,         am_sctp_i_want_mapped_v4_addr,
    am_sctp_maxseg,                    am_sctp_set_peer_primary_addr,
    am_sctp_primary_addr,              am_sctp_adaptation_layer,
    am_sctp_peer_addr_params,          am_sctp_default_send_param,
    am_sctp_events,                    am_sctp_delayed_ack_time,
    am_sctp_status,                    am_sctp_get_peer_addr_info,
    am_sctp_sndrcvinfo,                am_sctp_assoc_change,
    am_sctp_paddr_change,              am_sctp_remote_error,
    am_sctp_send_failed,               am_sctp_shutdown_event,
    am_sctp_adaptation_event,          am_sctp_pdapi_event,
    am_sctp_assocparams,               am_sctp_prim,
    am_sctp_setpeerprim,               am_sctp_setadaptation,
    am_sctp_paddrparams,               am_sctp_event_subscribe,
    am_sctp_assoc_value,               am_sctp_paddrinfo,
    am_unordered,                      am_addr_over,
    am_abort,                          am_eof,
    am_comm_up,                        am_comm_lost,
    am_restart,                        am_shutdown_comp,
    am_cant_assoc,
    am_addr_available,                 am_addr_unreachable,
    am_addr_removed,                   am_addr_added,
    am_addr_made_prim,                 am_addr_confirmed,
    am_short_recv,                     am_wrong_anc_data,
    am_partial_delivery_aborted,
    am_hb_enable,                      am_hb_disable,
    am_hb_demand,                      am_pmtud_enable,
    am_pmtud_disable,                  am_sackdelay_enable,
    am_sackdelay_disable,
    am_active,                         am_inactive,
#    if HAVE_DECL_SCTP_UNCONFIRMED
    am_unconfirmed,
#    endif
#    if HAVE_DECL_SCTP_EMPTY
    am_empty,
#    endif
#    if HAVE_DECL_SCTP_BOUND
    am_bound,
#    endif
#    if HAVE_DECL_SCTP_LISTEN
    am_listen,
#    endif
    am_cookie_wait,                    am_cookie_echoed,
    am_established,                    am_shutdown_pending,
    am_shutdown_sent,                  am_shutdown_received,
    am_shutdown_ack_sent;
#define SCTP_PARSE_SNDRCVINFO_CNT                            \
        (5*LOAD_ATOM_CNT + 5*LOAD_INT_CNT + 2*LOAD_UINT_CNT + \
	 LOAD_NIL_CNT + LOAD_LIST_CNT + LOAD_ASSOC_ID_CNT + LOAD_TUPLE_CNT)
static int sctp_parse_sndrcvinfo
	   (ErlDrvTermData * spec, int i, struct sctp_sndrcvinfo * sri)
{
    int n;
    i = LOAD_ATOM	(spec, i, am_sctp_sndrcvinfo);
    i = LOAD_INT	(spec, i, sri->sinfo_stream);
    i = LOAD_INT	(spec, i, sri->sinfo_ssn);
    n = 0;
    if (sri->sinfo_flags & SCTP_UNORDERED)
	{ i = LOAD_ATOM (spec, i, am_unordered);     n++; }
    if (sri->sinfo_flags & SCTP_ADDR_OVER)
	{ i = LOAD_ATOM (spec, i, am_addr_over);     n++; }
    if (sri->sinfo_flags & SCTP_ABORT)
	{ i = LOAD_ATOM (spec, i, am_abort);	     n++; }
    if (sri->sinfo_flags & SCTP_EOF)
	{ i = LOAD_ATOM (spec, i, am_eof);	     n++; }
    i = LOAD_NIL	(spec, i);
    i = LOAD_LIST	(spec, i, n+1);
    i = LOAD_INT	(spec, i, sock_ntohl(sri->sinfo_ppid));
    i = LOAD_INT	(spec, i, sri->sinfo_context);
    i = LOAD_INT	(spec, i, sri->sinfo_timetolive);
    i = LOAD_UINT	(spec, i, sri->sinfo_tsn);
    i = LOAD_UINT	(spec, i, sri->sinfo_cumtsn);
    i = LOAD_ASSOC_ID	(spec, i, sri->sinfo_assoc_id);
    i = LOAD_TUPLE	(spec, i, 10);
    return i;
}
static int sctp_parse_ancillary_data
	   (ErlDrvTermData * spec, int i, struct msghdr * mptr)
{
    struct cmsghdr * cmsg, * frst_msg = CMSG_FIRSTHDR(mptr);
    int    s = 0;
    for (cmsg = frst_msg; cmsg != NULL; cmsg = CMSG_NXTHDR(mptr,cmsg))
    {
	struct sctp_sndrcvinfo * sri;
#ifndef __WIN32
        int old_s;
        old_s = s;
        i = parse_ancillary_data_item(spec, i, cmsg, &s);
        if (s > old_s) continue;
	if (cmsg->cmsg_level != IPPROTO_SCTP ||
	    cmsg->cmsg_type  != SCTP_SNDRCV)
	continue;
#endif
	if (((char*)cmsg + cmsg->cmsg_len) - (char*)frst_msg >
	    mptr->msg_controllen)
	    break;
	sri = (struct sctp_sndrcvinfo*) CMSG_DATA(cmsg);
	i = sctp_parse_sndrcvinfo (spec, i, sri);
	s ++;
    }
    i = LOAD_NIL (spec, i);
    i = LOAD_LIST(spec, i, s+1);
    return i;
}
static int sctp_parse_error_chunk
       (ErlDrvTermData * spec, int i, char * chunk, int chlen)
{
    char *causes, *cause;
    int coff,
	ccode,
	clen,
	s;
    int len = sock_ntohs (*((uint16_t*)(chunk+2)));
    ASSERT(len >= 4 && len <= chlen);
    causes = chunk + 4;
    coff   = 0;
    len -= 4;
    cause  = causes;
    s      = 0;
    while (coff < len)
    {
	ccode = sock_ntohs (*((uint16_t*)(cause)));
	clen  = sock_ntohs (*((uint16_t*)(cause + 2)));
	if (clen <= 0)
	    break;
	i = LOAD_INT (spec, i, ccode);
	cause += clen;
	coff  += clen;
	s ++;
    }
    i = LOAD_NIL (spec, i);
    i = LOAD_LIST(spec, i, s+1);
    return i;
}
static int sctp_parse_async_event
      (ErlDrvTermData * spec, int i,    int ok_pos,
       ErlDrvTermData   error_atom,     inet_descriptor* desc,
       ErlDrvBinary   * bin,  int offs, int sz)
{
    char* body			   = bin->orig_bytes + offs;
    union sctp_notification * nptr = (union sctp_notification *) body;
    switch (nptr->sn_header.sn_type)
    {
	case SCTP_ASSOC_CHANGE:
	{
	    struct sctp_assoc_change* sptr = &(nptr->sn_assoc_change);
	    ASSERT(sptr->sac_length <= sz);
	    i = LOAD_ATOM (spec, i, am_sctp_assoc_change);
	    switch (sptr->sac_state)
	    {
	    case SCTP_COMM_UP:
		i = LOAD_ATOM (spec, i, am_comm_up);
		break;
	    case SCTP_COMM_LOST:
		i = LOAD_ATOM (spec, i, am_comm_lost);
		break;
	    case SCTP_RESTART:
		i = LOAD_ATOM (spec, i, am_restart);
		break;
	    case SCTP_SHUTDOWN_COMP:
		i = LOAD_ATOM (spec, i, am_shutdown_comp);
		break;
	    case SCTP_CANT_STR_ASSOC:
		i = LOAD_ATOM (spec, i, am_cant_assoc);
		break;
	    default:
		ASSERT(0);
	    }
	    i = LOAD_INT (spec, i, sptr->sac_error);
	    i = LOAD_INT (spec, i, sptr->sac_outbound_streams);
	    i = LOAD_INT (spec, i, sptr->sac_inbound_streams);
	    i = LOAD_INT (spec, i, sptr->sac_assoc_id);
	    i = LOAD_TUPLE (spec, i, 6);
	    break;
	}
	case SCTP_PEER_ADDR_CHANGE:
	{
	    struct sctp_paddr_change* sptr = &(nptr->sn_paddr_change);
	    ASSERT(sptr->spc_length <= sz);
	    i = LOAD_ATOM	(spec, i, am_sctp_paddr_change);
            PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    i = load_inet_get_address(spec, i, desc, (struct sockaddr_storage *)&sptr->spc_aaddr);
            POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    switch (sptr->spc_state)
	    {
	    case SCTP_ADDR_AVAILABLE:
		i = LOAD_ATOM (spec, i, am_addr_available);
		break;
	    case SCTP_ADDR_UNREACHABLE:
		i = LOAD_ATOM (spec, i, am_addr_unreachable);
		break;
	    case SCTP_ADDR_REMOVED:
		i = LOAD_ATOM (spec, i, am_addr_removed);
		break;
	    case SCTP_ADDR_ADDED:
		i = LOAD_ATOM (spec, i, am_addr_added);
		break;
	    case SCTP_ADDR_MADE_PRIM:
		i = LOAD_ATOM (spec, i, am_addr_made_prim);
		break;
#if HAVE_DECL_SCTP_ADDR_CONFIRMED
	    case SCTP_ADDR_CONFIRMED:
		i = LOAD_ATOM (spec, i, am_addr_confirmed);
		break;
#endif
	    default:
		ASSERT(0);
	    }
	    i = LOAD_INT   (spec, i, sptr->spc_error);
	    i = LOAD_INT   (spec, i, sptr->spc_assoc_id);
	    i = LOAD_TUPLE (spec, i, 5);
	    break;
	}
	case SCTP_REMOTE_ERROR:
	{
	    char *chunk;
	    int chlen;
	    struct sctp_remote_error * sptr = &(nptr->sn_remote_error);
	    ASSERT(sptr->sre_length <= sz);
	    (void)LOAD_ATOM(spec, ok_pos, error_atom);
	    i = LOAD_ATOM  (spec, i, am_sctp_remote_error);
	    i = LOAD_INT   (spec, i, sock_ntohs(sptr->sre_error));
	    i = LOAD_INT   (spec, i, sptr->sre_assoc_id);
#	    ifdef HAVE_STRUCT_SCTP_REMOTE_ERROR_SRE_DATA
	    chunk = (char*) (&(sptr->sre_data));
#	    else
	    chunk = ((char*) &(sptr->sre_assoc_id))
		+ sizeof(sptr->sre_assoc_id);
#	    endif
	    chlen = sptr->sre_length  - (chunk - (char *)sptr);
	    i = sctp_parse_error_chunk(spec, i, chunk, chlen);
	    i = LOAD_TUPLE (spec, i, 4);
	    break;
	}
	case SCTP_SEND_FAILED:
	{
	    char *chunk;
	    int chlen, choff;
	    struct sctp_send_failed * sptr = &(nptr->sn_send_failed);
	    ASSERT(sptr->ssf_length <= sz);
	    (void)LOAD_ATOM(spec, ok_pos, error_atom);
	    i = LOAD_ATOM  (spec, i, am_sctp_send_failed);
	    switch (sptr->ssf_flags) {
	    case SCTP_DATA_SENT:
		i = LOAD_ATOM (spec, i, am_true);
		break;
	    case SCTP_DATA_UNSENT:
		i = LOAD_ATOM (spec, i, am_false);
		break;
	    default:
		ASSERT(0);
	    }
	    i = LOAD_INT      (spec, i, sptr->ssf_error);
	    i = sctp_parse_sndrcvinfo (spec, i, &sptr->ssf_info);
	    i = LOAD_ASSOC_ID (spec, i, sptr->ssf_assoc_id);
#	    ifdef HAVE_STRUCT_SCTP_SEND_FAILED_SSF_DATA
	    chunk = (char*) (&(sptr->ssf_data));
#	    else
	    chunk = ((char*) &(sptr->ssf_assoc_id))
		+ sizeof(sptr->ssf_assoc_id);
#	    endif
	    chlen = sptr->ssf_length - (chunk - (char*) sptr);
	    choff = chunk - bin->orig_bytes;
	    i = LOAD_BINARY(spec, i, bin, choff, chlen);
	    i = LOAD_TUPLE (spec, i, 6);
	    break;
	}
	case SCTP_SHUTDOWN_EVENT:
	{
	    struct sctp_shutdown_event * sptr = &(nptr->sn_shutdown_event);
	    ASSERT (sptr->sse_length == sizeof(struct sctp_shutdown_event) &&
		    sptr->sse_length <= sz);
	    i = LOAD_ATOM  (spec, i, am_sctp_shutdown_event);
	    i = LOAD_INT   (spec, i, sptr->sse_assoc_id);
	    i = LOAD_TUPLE (spec, i, 2);
	    break;
	}
	case SCTP_ADAPTATION_INDICATION:
	{
	    struct sctp_adaptation_event * sptr =
		&(nptr->sn_adaptation_event);
	    ASSERT (sptr->sai_length == sizeof(struct sctp_adaptation_event)
		    && sptr->sai_length <= sz);
	    i = LOAD_ATOM  (spec, i, am_sctp_adaptation_event);
	    i = LOAD_INT   (spec, i, sock_ntohl(sptr->sai_adaptation_ind));
	    i = LOAD_INT   (spec, i, sptr->sai_assoc_id);
	    i = LOAD_TUPLE (spec, i, 3);
	    break;
	}
	case SCTP_PARTIAL_DELIVERY_EVENT:
	{
	    struct sctp_pdapi_event * sptr;
	    (void) LOAD_ATOM  (spec, ok_pos, error_atom);
	    sptr = &(nptr->sn_pdapi_event);
	    ASSERT (sptr->pdapi_length == sizeof(struct sctp_pdapi_event) &&
		    sptr->pdapi_length <= sz);
	    i = LOAD_ATOM  (spec, i, am_sctp_pdapi_event);
	    ASSERT (sptr->pdapi_indication == SCTP_PARTIAL_DELIVERY_ABORTED);
	    i = LOAD_ATOM  (spec, i, am_partial_delivery_aborted);
	    i = LOAD_INT   (spec, i, sptr->pdapi_assoc_id);
	    i = LOAD_TUPLE (spec, i, 3);
	    break;
	}
	default:   ASSERT(0);
    }
    return i;
}
#endif
#ifndef __WIN32__
static int udp_parse_ancillary_data(ErlDrvTermData *spec, int i,
                                struct msghdr *mptr) {
    struct cmsghdr *cmsg;
    int n;
    n = 0;
    for (cmsg = CMSG_FIRSTHDR(mptr);
         cmsg != NULL;
         cmsg = CMSG_NXTHDR(mptr, cmsg)) {
        i = parse_ancillary_data_item(spec, i, cmsg, &n);
    }
    i = LOAD_NIL(spec, i);
    return LOAD_LIST(spec, i, n+1);
}
static int compile_ancillary_data(struct msghdr *mhdr,
                                  char *ptr, ErlDrvSizeT anc_len) {
    struct cmsghdr *cmsg;
    size_t controllen = 0;
    cmsg = CMSG_FIRSTHDR(mhdr);
    for (;;) {
        if (anc_len == 0) {
            mhdr->msg_controllen = controllen;
            return 0;
        }
        if (cmsg == NULL) {
            return 1;
        }
#define COMPILE_ANCILLARY_DATA_ITEM(Level, Opt, Type, Get, Size)        \
        do {                                                            \
            if (anc_len < (Size)) return 1;                             \
            sys_memset(cmsg, '\0', CMSG_SPACE(sizeof(Type)));           \
            cmsg->cmsg_level = Level;                                   \
            cmsg->cmsg_type = Opt;                                      \
            cmsg->cmsg_len = CMSG_LEN(sizeof(Type));                    \
            *((Type *) CMSG_DATA(cmsg)) = Get(ptr);                     \
            controllen += CMSG_SPACE(sizeof(Type));                     \
            cmsg = CMSG_NXTHDR(mhdr, cmsg);                             \
            ptr += 4;                                                   \
            anc_len -= 4;                                               \
        } while (0)
#define SIZEOF_ANCILLARY_DATA (2 * CMSG_SPACE(sizeof(int)))
        switch (anc_len--, *ptr++) {
        case INET_OPT_TOS: {
#if defined(IPPROTO_IP) && defined(IP_TOS)
            COMPILE_ANCILLARY_DATA_ITEM(IPPROTO_IP, IP_TOS, int, get_int32, 4);
#else
            return 1;
#endif
            break;
        }
        case INET_OPT_TTL: {
#if defined(IPPROTO_IP) && defined(IP_TTL)
            COMPILE_ANCILLARY_DATA_ITEM(IPPROTO_IP, IP_TTL, int, get_int32, 4);
#else
            return 1;
#endif
            break;
        }
        case INET_OPT_TCLASS: {
#if defined(IPPROTO_IPV6) && defined(IPV6_TCLASS)
            COMPILE_ANCILLARY_DATA_ITEM(IPPROTO_IPV6, IPV6_TCLASS, int, get_int32, 4);
#else
            return 1;
#endif
            break;
        }
        default:
            return 1;
        }
#undef COMPILE_ANCILLARY_DATA_ITEM
    }
}
#endif
static int
inet_async_binary_data
	(inet_descriptor* desc, unsigned  int phsz,
	 ErlDrvBinary   * bin,  int offs, int len, void *mp)
{
    unsigned int hsz = desc->hsz + phsz;
    ErlDrvTermData spec [PACKET_ERL_DRV_TERM_DATA_LEN];
    ErlDrvTermData caller;
    int aid;
    int req;
    int i = 0;
#ifdef HAVE_SCTP
    int ok_pos;
#endif
    DEBUGF(("inet_async_binary_data(%p): offs=%d, len=%d\r\n",
	    desc->port, offs, len));
    if (deq_async(desc, &aid, &caller, &req) < 0)
	return -1;
    i = LOAD_ATOM(spec, i, am_inet_async);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_INT (spec, i, aid);
#ifdef HAVE_SCTP
    ok_pos = i;
#endif
    i = LOAD_ATOM(spec, i, am_ok);
#ifdef HAVE_SCTP
    if (IS_SCTP(desc))
    {
	int sz;
        struct msghdr *mptr;
        mptr = mp;
        ASSERT (mptr);
	ASSERT (hsz == phsz && hsz != 0);
	sz = len - hsz;
	i = LOAD_STRING(spec, i, bin->orig_bytes+offs, hsz);
	i = sctp_parse_ancillary_data (spec, i, mptr);
	if (mptr->msg_flags & MSG_NOTIFICATION)
	    i = sctp_parse_async_event
		(spec, i, ok_pos, am_error, desc, bin, offs+hsz, sz);
        else
	    if (desc->mode == INET_MODE_LIST)
		i = LOAD_STRING(spec, i, bin->orig_bytes+offs+hsz, sz);
	    else
		i = LOAD_BINARY(spec, i, bin, offs+hsz, sz);
	i = LOAD_TUPLE (spec, i, 3);
    }
    else
#endif
    {
        if ((desc->mode == INET_MODE_LIST) || (hsz > len)) {
            i = LOAD_STRING(spec, i, bin->orig_bytes+offs, len);
        }
        else {
            int sz = len - hsz;
            i = LOAD_BINARY(spec, i, bin, offs+hsz, sz);
            if (hsz > 0)
                i = LOAD_STRING_CONS(spec, i, bin->orig_bytes+offs, hsz);
        }
#ifndef __WIN32__
        if (mp) {
            i = udp_parse_ancillary_data(spec, i, (struct msghdr*)mp);
            i = LOAD_TUPLE(spec, i, 2);
        }
#endif
    }
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 4);
    ASSERT(i <= PACKET_ERL_DRV_TERM_DATA_LEN);
    return erl_drv_send_term(desc->dport, caller, spec, i);
}
static int tcp_message(inet_descriptor* desc, const char* buf, int len)
{
    unsigned int hsz = desc->hsz;
    ErlDrvTermData spec[20];
    int i = 0;
    DEBUGF(("tcp_message(%p): len = %d\r\n",
            desc->port, len));
    i = LOAD_ATOM(spec, i, am_tcp);
    i = LOAD_PORT(spec, i, desc->dport);
    if ((desc->mode == INET_MODE_LIST) || (hsz > len)) {
	i = LOAD_STRING(spec, i, buf, len);
	i = LOAD_TUPLE(spec, i, 3);
	ASSERT(i <= 20);
	return erl_drv_output_term(desc->dport, spec, i);
    }
    else {
	int sz = len - hsz;
	int code;
	i = LOAD_BUF2BINARY(spec, i, buf+hsz, sz);
	if (hsz > 0)
	    i = LOAD_STRING_CONS(spec, i, buf, hsz);
	i = LOAD_TUPLE(spec, i, 3);
	ASSERT(i <= 20);
	code = erl_drv_output_term(desc->dport, spec, i);
	return code;
    }
}
static int
tcp_binary_message(inet_descriptor* desc, ErlDrvBinary* bin, int offs, int len)
{
    unsigned int hsz = desc->hsz;
    ErlDrvTermData spec[20];
    int i = 0;
    DEBUGF(("tcp_binary_message(%p): len = %d\r\n",
            desc->port, len));
    i = LOAD_ATOM(spec, i, am_tcp);
    i = LOAD_PORT(spec, i, desc->dport);
    if ((desc->mode == INET_MODE_LIST) || (hsz > len)) {
	i = LOAD_STRING(spec, i, bin->orig_bytes+offs, len);
    }
    else {
	int sz = len - hsz;
	i = LOAD_BINARY(spec, i, bin, offs+hsz, sz);
	if (hsz > 0)
	    i = LOAD_STRING_CONS(spec, i, bin->orig_bytes+offs, hsz);
    }
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i <= 20);
    return erl_drv_output_term(desc->dport, spec, i);
}
static int tcp_closed_message(tcp_descriptor* desc)
{
    ErlDrvTermData spec[6];
    int i = 0;
    DEBUGF(("tcp_closed_message(%p):\r\n", desc->inet.port));
    if (!(desc->tcp_add_flags & TCP_ADDF_CLOSE_SENT)) {
	desc->tcp_add_flags |= TCP_ADDF_CLOSE_SENT;
	i = LOAD_ATOM(spec, i, am_tcp_closed);
	i = LOAD_PORT(spec, i, desc->inet.dport);
	i = LOAD_TUPLE(spec, i, 2);
	ASSERT(i <= 6);
	return erl_drv_output_term(desc->inet.dport, spec, i);
    }
    return 0;
}
static int tcp_error_message(tcp_descriptor* desc, int err)
{
    ErlDrvTermData spec[8];
    ErlDrvTermData am_err = error_atom(err);
    int i = 0;
    DEBUGF(("tcp_error_message(%p): %d\r\n",
            desc->inet.port, err));
    i = LOAD_ATOM(spec, i, am_tcp_error);
    i = LOAD_PORT(spec, i, desc->inet.dport);
    i = LOAD_ATOM(spec, i, am_err);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i <= 8);
    return erl_drv_output_term(desc->inet.dport, spec, i);
}
#ifdef HAVE_UDP
static int packet_binary_message(inet_descriptor* desc,
                                 ErlDrvBinary* bin, int offs, int len,
                                 void *mp)
{
    unsigned int hsz = desc->hsz;
    ErlDrvTermData spec [PACKET_ERL_DRV_TERM_DATA_LEN];
    int i = 0;
    int alen;
    char* data = bin->orig_bytes+offs;
    DEBUGF(("packet_binary_message(%p): len = %d\r\n",
            desc->port, len));
#   ifdef HAVE_SCTP
    i = LOAD_ATOM(spec, i, IS_SCTP(desc) ? am_sctp : am_udp);
#   else
    i = LOAD_ATOM(spec, i, am_udp );
#   endif
    i = LOAD_PORT(spec, i, desc->dport);
    alen = addrlen(data);
    i = load_address(spec, i, data);
    offs += alen;
    len  -= alen;
#   ifdef HAVE_SCTP
    if (!IS_SCTP(desc))
#   endif
    {
#ifndef __WIN32__
        if (mp) i = udp_parse_ancillary_data(spec, i, (struct msghdr*)mp);
#endif
	if ((desc->mode == INET_MODE_LIST) || (hsz > len))
	    i = LOAD_STRING(spec, i, bin->orig_bytes+offs, len);
	else {
	    int sz = len - hsz;
	    i = LOAD_BINARY(spec, i, bin, offs+hsz, sz);
	    if (hsz > 0)
		i = LOAD_STRING_CONS(spec, i, bin->orig_bytes+offs, hsz);
	}
#ifndef __WIN32__
        if (mp) i = LOAD_TUPLE(spec, i, 6);
        else
#endif
            i = LOAD_TUPLE(spec, i, 5);
    }
#   ifdef HAVE_SCTP
    else
    {
        struct msghdr *mptr;
        mptr = mp;
	ASSERT(hsz == 0);
	i = sctp_parse_ancillary_data (spec, i, mptr);
	if (mptr->msg_flags & MSG_NOTIFICATION)
	    i = sctp_parse_async_event
		(spec, i, 0, am_sctp_error, desc, bin, offs, len);
        else
	    if (desc->mode == INET_MODE_LIST)
		i = LOAD_STRING(spec, i, bin->orig_bytes+offs, len);
	    else
		i = LOAD_BINARY(spec, i, bin, offs, len);
	i = LOAD_TUPLE (spec, i, 2);
        i = LOAD_TUPLE(spec, i, 5);
    }
#   endif
    ASSERT(i <= PACKET_ERL_DRV_TERM_DATA_LEN);
    return erl_drv_output_term(desc->dport, spec, i);
}
#endif
 static int packet_passive_message(inet_descriptor* desc)
 {
     ErlDrvTermData spec[6];
     int i = 0;
     DEBUGF(("packet_passive_message(%p):\r\n", desc->port));
#if !defined(HAVE_UDP) && !defined(HAVE_SCTP)
     i = LOAD_ATOM(spec, i, am_tcp_passive);
#else
     if (desc->sprotocol == IPPROTO_TCP)
         i = LOAD_ATOM(spec, i, am_tcp_passive);
     else {
#ifdef HAVE_SCTP
         i = LOAD_ATOM(spec, i, IS_SCTP(desc) ? am_sctp_passive : am_udp_passive);
#else
         i = LOAD_ATOM(spec, i, am_udp_passive);
#endif
     }
#endif
     i = LOAD_PORT(spec, i, desc->dport);
     i = LOAD_TUPLE(spec, i, 2);
     ASSERT(i <= 6);
     return erl_drv_output_term(desc->dport, spec, i);
 }
#ifdef HAVE_UDP
static int packet_error_message(udp_descriptor* udesc, int err)
{
    inet_descriptor* desc = INETP(udesc);
    ErlDrvTermData spec[2*LOAD_ATOM_CNT + LOAD_PORT_CNT + LOAD_TUPLE_CNT];
    ErlDrvTermData am_err = error_atom(err);
    int i = 0;
    DEBUGF(("packet_error_message(%p): %d\r\n",
            desc->port, err));
#   ifdef HAVE_SCTP
    if (IS_SCTP(desc) )
    	i = LOAD_ATOM(spec, i, am_sctp_error);
    else
#   endif
	i = LOAD_ATOM(spec, i, am_udp_error);
    i = LOAD_PORT(spec, i, desc->dport);
    i = LOAD_ATOM(spec, i, am_err);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    return erl_drv_output_term(desc->dport, spec, i);
}
#endif
static int tcp_reply_data(tcp_descriptor* desc, char* buf, int len)
{
    int code;
    const char* body = buf;
    int bodylen = len;
    packet_get_body(desc->inet.htype, &body, &bodylen);
    if (desc->inet.deliver == INET_DELIVER_PORT) {
        code = inet_port_data(INETP(desc), body, bodylen);
    }
    else if ((code=packet_parse(desc->inet.htype, buf, len,
                                &desc->http_state, &packet_callbacks,
                                desc)) == 0) {
        if (desc->inet.active == INET_PASSIVE)
            return inet_async_data(INETP(desc), body, bodylen);
        else
            code = tcp_message(INETP(desc), body, bodylen);
    }
    if (code < 0)
	return code;
    INET_CHECK_ACTIVE_TO_PASSIVE(INETP(desc));
    return code;
}
static int
tcp_reply_binary_data(tcp_descriptor* desc, ErlDrvBinary* bin, int offs, int len)
{
    int code;
    const char* buf = bin->orig_bytes + offs;
    const char* body = buf;
    int bodylen = len;
    packet_get_body(desc->inet.htype, &body, &bodylen);
    offs = body - bin->orig_bytes;
    if (desc->inet.deliver == INET_DELIVER_PORT)
        code = inet_port_binary_data(INETP(desc), bin, offs, bodylen);
    else if ((code=packet_parse(desc->inet.htype, buf, len, &desc->http_state,
                                     &packet_callbacks,desc)) == 0) {
        if (desc->inet.active == INET_PASSIVE)
            return inet_async_binary_data(INETP(desc), 0, bin, offs, bodylen, NULL);
        else
            code = tcp_binary_message(INETP(desc), bin, offs, bodylen);
    }
    if (code < 0)
	return code;
    INET_CHECK_ACTIVE_TO_PASSIVE(INETP(desc));
    return code;
}
#ifdef HAVE_UDP
static int
packet_reply_binary_data(inet_descriptor* desc, unsigned  int hsz,
			 ErlDrvBinary   * bin,  int offs, int len,
                         void *mp)
{
    int code;
    if (desc->active == INET_PASSIVE)
	return inet_async_binary_data(desc, hsz, bin, offs, len, mp);
    else
    {
	if (desc->deliver == INET_DELIVER_PORT)
	    code = inet_port_binary_data(desc, bin, offs, len);
	else
	    code = packet_binary_message(desc, bin, offs, len, mp);
	if (code < 0)
	    return code;
        INET_CHECK_ACTIVE_TO_PASSIVE(desc);
	return code;
    }
}
#endif
static int
sock_init(void)
{
#ifdef __WIN32__
    WORD wVersionRequested;
    WSADATA wsaData;
    static int res = -1;
    if (res >= 0)
	return res;
    wVersionRequested = MAKEWORD(2,0);
    if (WSAStartup(wVersionRequested, &wsaData) != 0)
	goto error;
    if ((LOBYTE(wsaData.wVersion) != 2) || (HIBYTE(wsaData.wVersion) != 0))
	goto error;
    find_dynamic_functions();
    return res = 1;
 error:
    WSACleanup();
    return res = 0;
#else
    return 1;
#endif
}
#ifdef HAVE_SCTP
static void inet_init_sctp(void) {
    INIT_ATOM(sctp);
    INIT_ATOM(sctp_passive);
    INIT_ATOM(sctp_error);
    INIT_ATOM(true);
    INIT_ATOM(false);
    INIT_ATOM(buffer);
    INIT_ATOM(mode);
    INIT_ATOM(list);
    INIT_ATOM(binary);
    INIT_ATOM(active);
    INIT_ATOM(once);
    INIT_ATOM(multi);
    INIT_ATOM(buffer);
    INIT_ATOM(linger);
    INIT_ATOM(recbuf);
    INIT_ATOM(sndbuf);
    INIT_ATOM(reuseaddr);
    INIT_ATOM(reuseport);
    INIT_ATOM(reuseport_lb);
    INIT_ATOM(exclusiveaddruse);
    INIT_ATOM(dontroute);
    INIT_ATOM(priority);
    INIT_ATOM(recvtos);
    INIT_ATOM(recvtclass);
    INIT_ATOM(recvttl);
    INIT_ATOM(ipv6_v6only);
    INIT_ATOM(netns);
    INIT_ATOM(bind_to_device);
    INIT_ATOM(sctp_rtoinfo);
    INIT_ATOM(sctp_associnfo);
    INIT_ATOM(sctp_initmsg);
    INIT_ATOM(sctp_autoclose);
    INIT_ATOM(sctp_nodelay);
    INIT_ATOM(sctp_disable_fragments);
    INIT_ATOM(sctp_i_want_mapped_v4_addr);
    INIT_ATOM(sctp_maxseg);
    INIT_ATOM(sctp_set_peer_primary_addr);
    INIT_ATOM(sctp_primary_addr);
    INIT_ATOM(sctp_adaptation_layer);
    INIT_ATOM(sctp_peer_addr_params);
    INIT_ATOM(sctp_default_send_param);
    INIT_ATOM(sctp_events);
    INIT_ATOM(sctp_delayed_ack_time);
    INIT_ATOM(sctp_status);
    INIT_ATOM(sctp_get_peer_addr_info);
    INIT_ATOM(sctp_sndrcvinfo);
    INIT_ATOM(sctp_assoc_change);
    INIT_ATOM(sctp_paddr_change);
    INIT_ATOM(sctp_remote_error);
    INIT_ATOM(sctp_send_failed);
    INIT_ATOM(sctp_shutdown_event);
    INIT_ATOM(sctp_adaptation_event);
    INIT_ATOM(sctp_pdapi_event);
    INIT_ATOM(sctp_assocparams);
    INIT_ATOM(sctp_prim);
    INIT_ATOM(sctp_setpeerprim);
    INIT_ATOM(sctp_setadaptation);
    INIT_ATOM(sctp_paddrparams);
    INIT_ATOM(sctp_event_subscribe);
    INIT_ATOM(sctp_assoc_value);
    INIT_ATOM(sctp_paddrinfo);
    INIT_ATOM(unordered);
    INIT_ATOM(addr_over);
    INIT_ATOM(abort);
    INIT_ATOM(eof);
    INIT_ATOM(comm_up);
    INIT_ATOM(comm_lost);
    INIT_ATOM(restart);
    INIT_ATOM(shutdown_comp);
    INIT_ATOM(cant_assoc);
    INIT_ATOM(addr_available);
    INIT_ATOM(addr_unreachable);
    INIT_ATOM(addr_removed);
    INIT_ATOM(addr_added);
    INIT_ATOM(addr_made_prim);
    INIT_ATOM(addr_confirmed);
    INIT_ATOM(short_recv);
    INIT_ATOM(wrong_anc_data);
    INIT_ATOM(partial_delivery_aborted);
    INIT_ATOM(hb_enable);
    INIT_ATOM(hb_disable);
    INIT_ATOM(hb_demand);
    INIT_ATOM(pmtud_enable);
    INIT_ATOM(pmtud_disable);
    INIT_ATOM(sackdelay_enable);
    INIT_ATOM(sackdelay_disable);
    INIT_ATOM(active);
    INIT_ATOM(inactive);
#    if HAVE_DECL_SCTP_UNCONFIRMED
    INIT_ATOM(unconfirmed);
#    endif
#    if HAVE_DECL_SCTP_EMPTY
    INIT_ATOM(empty);
#    endif
#    if HAVE_DECL_SCTP_BOUND
    INIT_ATOM(bound);
#    endif
#    if HAVE_DECL_SCTP_LISTEN
    INIT_ATOM(listen);
#    endif
    INIT_ATOM(cookie_wait);
    INIT_ATOM(cookie_echoed);
    INIT_ATOM(established);
    INIT_ATOM(shutdown_pending);
    INIT_ATOM(shutdown_sent);
    INIT_ATOM(shutdown_received);
    INIT_ATOM(shutdown_ack_sent);
}
#endif
static int inet_init(void)
{
    if (!sock_init())
	goto error;
#if !defined(__WIN32__) && !defined(NO_SYSCONF)
    iov_max = (int) sysconf(_SC_IOV_MAX);
    if (iov_max < 0) {
#ifdef IOV_MAX
        iov_max = IOV_MAX;
#else
        iov_max = 16;
#endif
    }
    ASSERT(iov_max >= 16);
#endif
    if (0 != erl_drv_tsd_key_create("inet_buffer_stack_key", &buffer_stack_key))
	goto error;
    ERTS_CT_ASSERT(sizeof(struct in_addr) == 4);
#   if defined(HAVE_IN6) && defined(AF_INET6)
    ERTS_CT_ASSERT(sizeof(struct in6_addr) == 16);
#   endif
    INIT_ATOM(ok);
    INIT_ATOM(undefined);
    INIT_ATOM(unspec);
    INIT_ATOM(tcp);
#ifdef HAVE_UDP
    INIT_ATOM(udp);
#endif
    INIT_ATOM(error);
    INIT_ATOM(einval);
    INIT_ATOM(inet_async);
    INIT_ATOM(inet_reply);
    INIT_ATOM(timeout);
    INIT_ATOM(closed);
    INIT_ATOM(tcp_passive);
    INIT_ATOM(tcp_closed);
    INIT_ATOM(tcp_error);
#ifdef HAVE_UDP
    INIT_ATOM(udp_passive);
    INIT_ATOM(udp_error);
#endif
#ifdef HAVE_SYS_UN_H
    INIT_ATOM(local);
#endif
    INIT_ATOM(empty_out_q);
    INIT_ATOM(ssl_tls);
#ifndef __WIN32__
    INIT_ATOM(tos);
    INIT_ATOM(tclass);
    INIT_ATOM(ttl);
#endif
    INIT_ATOM(http_eoh);
    INIT_ATOM(http_header);
    INIT_ATOM(http_request);
    INIT_ATOM(http_response);
    INIT_ATOM(http_error);
    INIT_ATOM(abs_path);
    INIT_ATOM(absoluteURI);
    am_star = driver_mk_atom("*");
    INIT_ATOM(http);
    INIT_ATOM(https);
    INIT_ATOM(scheme);
#ifdef HAVE_SENDFILE
    INIT_ATOM(sendfile);
#endif
    add_driver_entry(&tcp_inet_driver_entry);
#ifdef HAVE_UDP
    add_driver_entry(&udp_inet_driver_entry);
#endif
#ifdef HAVE_SCTP
    ERTS_CT_ASSERT(sizeof(sctp_assoc_t)==ASSOC_ID_LEN);
#   if defined(HAVE_SCTP_BINDX)
    p_sctp_bindx = sctp_bindx;
#     if defined(HAVE_SCTP_PEELOFF)
    p_sctp_peeloff = sctp_peeloff;
#     else
    p_sctp_peeloff = NULL;
#     endif
#     if defined(HAVE_SCTP_GETLADDRS) && defined(HAVE_SCTP_FREELADDRS)
    p_sctp_getladdrs = sctp_getladdrs;
    p_sctp_freeladdrs = sctp_freeladdrs;
#     else
    p_sctp_getladdrs = NULL;
    p_sctp_freeladdrs = NULL;
#     endif
#     if defined(HAVE_SCTP_GETPADDRS) && defined(HAVE_SCTP_FREEPADDRS)
    p_sctp_getpaddrs = sctp_getpaddrs;
    p_sctp_freepaddrs = sctp_freepaddrs;
#     else
    p_sctp_getpaddrs = NULL;
    p_sctp_freepaddrs = NULL;
#     endif
#     if defined(HAVE_SCTP_CONNECTX)
    p_sctp_connectx = sctp_connectx;
#     else
    p_sctp_connectx = NULL;
#     endif
    inet_init_sctp();
    add_driver_entry(&sctp_inet_driver_entry);
#   else
#       ifndef LIBSCTP
#           error LIBSCTP not defined
#       endif
    {
	static void *h_libsctp = NULL;
	if (erts_sys_ddll_open_noext(STRINGIFY(LIBSCTP), &h_libsctp, NULL)
	    == 0) {
	    void *ptr;
	    if (erts_sys_ddll_sym(h_libsctp, "sctp_bindx", &ptr) == 0) {
		p_sctp_bindx = ptr;
		if (erts_sys_ddll_sym(h_libsctp, "sctp_peeloff", &ptr) == 0) {
		    p_sctp_peeloff = ptr;
		}
		else p_sctp_peeloff = NULL;
		if (erts_sys_ddll_sym(h_libsctp, "sctp_getladdrs", &ptr) == 0) {
		    p_sctp_getladdrs = ptr;
		}
		else p_sctp_getladdrs = NULL;
		if (erts_sys_ddll_sym(h_libsctp, "sctp_freeladdrs", &ptr) == 0) {
		    p_sctp_freeladdrs = ptr;
		}
		else {
		    p_sctp_freeladdrs = NULL;
		    p_sctp_getladdrs = NULL;
		}
		if (erts_sys_ddll_sym(h_libsctp, "sctp_getpaddrs", &ptr) == 0) {
		    p_sctp_getpaddrs = ptr;
		}
		else p_sctp_getpaddrs = NULL;
		if (erts_sys_ddll_sym(h_libsctp, "sctp_freepaddrs", &ptr) == 0) {
		    p_sctp_freepaddrs = ptr;
		}
		else {
		    p_sctp_freepaddrs = NULL;
		    p_sctp_getpaddrs = NULL;
		}
		if (erts_sys_ddll_vsym(h_libsctp, "sctp_connectx", "VERS_3", &ptr) == 0) {
		    p_sctp_connectx = ptr;
		}
		else p_sctp_connectx = NULL;
		inet_init_sctp();
		add_driver_entry(&sctp_inet_driver_entry);
	    }
	    else p_sctp_bindx = NULL;
	}
    }
#   endif
#endif
    remove_driver_entry(&inet_driver_entry);
    return 0;
 error:
    remove_driver_entry(&inet_driver_entry);
    return -1;
}
static char* inet_set_address(int family, inet_address* dst,
			      char* *src, ErlDrvSizeT* len)
{
    short port;
    switch (family) {
    case AF_INET: {
        if (*len < 2+4) return str_einval;
	sys_memzero((char*)dst, sizeof(struct sockaddr_in));
	port = get_int16(*src);
#ifndef NO_SA_LEN
	dst->sai.sin_len    = sizeof(struct sockaddr_in);
#endif
	dst->sai.sin_family = family;
	dst->sai.sin_port   = sock_htons(port);
	sys_memcpy(&dst->sai.sin_addr, (*src)+2, 4);
	*len = sizeof(struct sockaddr_in);
	*src += 2 + 4;
	return NULL;
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    case AF_INET6: {
        if (*len < 2+16) return str_einval;
	sys_memzero((char*)dst, sizeof(struct sockaddr_in6));
	port = get_int16(*src);
        *src += 2;
#ifndef NO_SA_LEN
	dst->sai6.sin6_len    = sizeof(struct sockaddr_in6);
#endif
	dst->sai6.sin6_family = family;
	dst->sai6.sin6_port   = sock_htons(port);
	sys_memcpy(&dst->sai6.sin6_addr, *src, 16);
	*src += 16;
        dst->sai6.sin6_flowinfo = get_int32(*src);
	*src += 4;
        dst->sai6.sin6_scope_id = get_int32(*src);
	*src += 4;
	*len = sizeof(struct sockaddr_in6);
	return NULL;
    }
#endif
#ifdef HAVE_SYS_UN_H
    case AF_UNIX: {
        int n;
        if (*len == 0) return str_einval;
	n = *((unsigned char*)(*src));
	if (*len < 1+n) return str_einval;
	if (n +
#ifdef __linux__
            ((*len) > 1 && (*src)[1] == '\0' ? 0 : 1)
#else
            1
#endif
            > sizeof(dst->sal.sun_path)) {
	    return str_einval;
	}
	sys_memzero((char*)dst, sizeof(struct sockaddr_un));
	dst->sal.sun_family = family;
	sys_memcpy(dst->sal.sun_path, (*src)+1, n);
	*len = offsetof(struct sockaddr_un, sun_path) + n;
#ifndef NO_SA_LEN
        dst->sal.sun_len = *len;
#endif
	*src += 1 + n;
	return NULL;
    }
#endif
    }
    return str_eafnosupport;
}
static char *inet_set_faddress(int family, inet_address* dst,
			       char* *src, ErlDrvSizeT* len) {
    int tag;
    if (*len < 1) return str_einval;
    (*len) --;
    tag = *((*src) ++);
    switch (tag) {
    case INET_AF_INET:
	family = AF_INET;
	break;
#   if defined(HAVE_IN6) && defined(AF_INET6)
    case INET_AF_INET6:
	family = AF_INET6;
	break;
#   endif
#   ifdef HAVE_SYS_UN_H
    case INET_AF_LOCAL: {
        family = AF_UNIX;
        break;
    }
#   endif
    case INET_AF_ANY:
    case INET_AF_LOOPBACK: {
	int port;
	if (*len < 2) return str_einval;
	port = get_int16(*src);
	switch (family) {
	case AF_INET: {
	    struct in_addr addr;
	    switch (tag) {
	    case INET_AF_ANY:
		addr.s_addr = sock_htonl(INADDR_ANY);
		break;
	    case INET_AF_LOOPBACK:
		addr.s_addr = sock_htonl(INADDR_LOOPBACK);
		break;
	    default:
		return str_einval;
	    }
	    sys_memzero((char*)dst, sizeof(struct sockaddr_in));
#ifndef NO_SA_LEN
	    dst->sai.sin_len         = sizeof(struct sockaddr_in6);
#endif
	    dst->sai.sin_family      = family;
	    dst->sai.sin_port        = sock_htons(port);
	    dst->sai.sin_addr.s_addr = addr.s_addr;
	    *len = sizeof(struct sockaddr_in);
	}   break;
#       if defined(HAVE_IN6) && defined(AF_INET6)
	case AF_INET6: {
	    const struct in6_addr* paddr;
	    switch (tag) {
	    case INET_AF_ANY:
		paddr = &in6addr_any;
		break;
	    case INET_AF_LOOPBACK:
		paddr = &in6addr_loopback;
		break;
	    default:
		return str_einval;
	    }
	    sys_memzero((char*)dst, sizeof(struct sockaddr_in6));
#ifndef NO_SA_LEN
	    dst->sai6.sin6_len    = sizeof(struct sockaddr_in6);
#endif
	    dst->sai6.sin6_family = family;
	    dst->sai6.sin6_port   = sock_htons(port);
	    dst->sai6.sin6_flowinfo = 0;
	    dst->sai6.sin6_scope_id = 0;
	    dst->sai6.sin6_addr = *paddr;
	    *len = sizeof(struct sockaddr_in6);
	}   break;
#       endif
	default:
	    return str_einval;
	}
	*src += 2;
	return NULL;
    }   break;
    default:
	return str_eafnosupport;
    }
    return inet_set_address(family, dst, src, len);
}
static int inet_get_address(char* dst, inet_address* src, unsigned int* len)
{
    int family;
    short port;
    family = src->sa.sa_family;
    if ((family == AF_INET) && (*len >= sizeof(struct sockaddr_in))) {
	dst[0] = INET_AF_INET;
	port = sock_ntohs(src->sai.sin_port);
	put_int16(port, dst+1);
	sys_memcpy(dst+3, (char*)&src->sai.sin_addr, sizeof(struct in_addr));
	*len = 3 + sizeof(struct in_addr);
	return 0;
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    else if ((family == AF_INET6) && (*len >= sizeof(struct sockaddr_in6))) {
	dst[0] = INET_AF_INET6;
	port = sock_ntohs(src->sai6.sin6_port);
	put_int16(port, dst+1);
	sys_memcpy(dst+3, (char*)&src->sai6.sin6_addr,sizeof(struct in6_addr));
	*len = 3 + sizeof(struct in6_addr);
	return 0;
    }
#endif
#ifdef HAVE_SYS_UN_H
    else if (family == AF_UNIX) {
        size_t n, m;
        if (*len < offsetof(struct sockaddr_un, sun_path)) return -1;
        n = *len - offsetof(struct sockaddr_un, sun_path);
        if (255 < n) return -1;
        m = my_strnlen(src->sal.sun_path, n);
#ifdef __linux__
	if (m == 0)  m = n;
#endif
        dst[0] = INET_AF_LOCAL;
        dst[1] = (char) ((unsigned char) m);
        sys_memcpy(dst+2, src->sal.sun_path, m);
        *len = 1 + 1 + m;
        return 0;
      }
#endif
    else if (family == AF_UNSPEC) {
        dst[0] = INET_AF_UNSPEC;
	*len = 1;
    }
    else {
        dst[0] = INET_AF_UNDEFINED;
	*len = 1;
    }
    return -1;
}
static int
inet_address_to_erlang(char *dst, inet_address **src, SOCKLEN_T sz) {
    short port;
    switch ((*src)->sa.sa_family) {
    case AF_INET:
	if (dst) {
	    dst[0] = INET_AF_INET;
	    port = sock_ntohs((*src)->sai.sin_port);
	    put_int16(port, dst+1);
	    sys_memcpy(dst+1+2, (char *) &(*src)->sai.sin_addr, 4);
	}
	(*src) = (inet_address *) (&(*src)->sai + 1);
	return 1 + 2 + 4;
#if defined(HAVE_IN6) && defined(AF_INET6)
    case AF_INET6:
	if (dst) {
	    dst[0] = INET_AF_INET6;
	    port = sock_ntohs((*src)->sai6.sin6_port);
	    put_int16(port, dst+1);
            VALGRIND_MAKE_MEM_DEFINED(&(*src)->sai6.sin6_addr,16);
	    sys_memcpy(dst+1+2, (char *) &(*src)->sai6.sin6_addr, 16);
	}
	(*src) = (inet_address *) (&(*src)->sai6 + 1);
	return 1 + 2 + 16;
#endif
#ifdef HAVE_SYS_UN_H
    case AF_UNIX: {
        size_t n, m;
	if (sz < offsetof(struct sockaddr_un, sun_path)) return -1;
	n = sz - offsetof(struct sockaddr_un, sun_path);
	if (255 < n) return -1;
        m = my_strnlen((*src)->sal.sun_path, n);
#ifdef __linux__
	if (m == 0)  m = n;
#endif
	if (dst) {
	    dst[0] = INET_AF_LOCAL;
	    dst[1] = (char) ((unsigned char) m);
            sys_memcpy(dst+2, (*src)->sal.sun_path, m);
	}
	(*src) = (inet_address *) (&(*src)->sal + 1);
	return 1 + 1 + m;
    }
#endif
    default:
	return -1;
    }
}
static ErlDrvSizeT reply_inet_addrs
(int n, inet_address *addrs, char **rbuf, ErlDrvSizeT rsize, SOCKLEN_T sz) {
    inet_address *ia;
    int i, s;
    ErlDrvSizeT rlen;
    if (IS_SOCKET_ERROR(n)) return ctl_error(sock_errno(), rbuf, rsize);
    if (n == 0) return ctl_reply_ok(rbuf, rsize);
    rlen = 1;
    ia = addrs;
    for (i = 0;  i < n;  i++) {
        s = inet_address_to_erlang(NULL, &ia, sz);
	if (s < 0) break;
	rlen += s;
    }
    if (rlen > rsize) (*rbuf) = ALLOC(rlen);
    (*rbuf)[0] = INET_REP_OK;
    rlen = 1;
    ia = addrs;
    for (i = 0;  i < n;  i++) {
        s = inet_address_to_erlang((*rbuf)+rlen, &ia, sz);
	if (s < 0) break;
	rlen += s;
    }
    return rlen;
}
static void desc_close(inet_descriptor* desc)
{
    if (desc->s != INVALID_SOCKET) {
#ifdef __WIN32__
	winsock_event_select(desc, FD_READ|FD_WRITE|FD_CLOSE, 0);
	sock_close(desc->s);
	desc->forced_events = 0;
	desc->send_would_block = 0;
#endif
	if (!INET_IGNORED(desc))
	    driver_select(desc->port,(ErlDrvEvent)(SWord)desc->event,
			  ERL_DRV_USE, 0);
	else
	  inet_stop_select((ErlDrvEvent)(SWord)desc->event,NULL);
	desc->event = INVALID_EVENT;
	desc->s = INVALID_SOCKET;
	desc->event_mask = 0;
	desc->state &= ~INET_STATE_CONNECTED;
    }
}
static void desc_close_read(inet_descriptor* desc)
{
    if (desc->s != INVALID_SOCKET) {
#ifdef __WIN32__
#endif
	sock_select(desc, FD_READ | FD_CLOSE, 0);
    }
}
static int erl_inet_close(inet_descriptor* desc)
{
    free_subscribers(&desc->empty_out_q_subs);
    if ((desc->prebound == 0) && (desc->state & INET_F_OPEN)) {
	desc_close(desc);
	desc->state = INET_STATE_CLOSED;
    } else if (desc->prebound && (desc->s != INVALID_SOCKET)) {
	sock_select(desc, FD_READ | FD_WRITE | FD_CLOSE | ERL_DRV_USE_NO_CALLBACK, 0);
	desc->event_mask = 0;
#ifdef __WIN32__
	desc->forced_events = 0;
	desc->send_would_block = 0;
#endif
    }
    return 0;
}
static
ErlDrvSSizeT inet_ctl_open(inet_descriptor* desc,
                           int domain, int type, int protocol,
                           char** rbuf, ErlDrvSizeT rsize)
{
    int save_errno;
#ifdef HAVE_SETNS
    int current_ns, new_ns;
    current_ns = new_ns = 0;
#endif
    save_errno = 0;
    if (desc->state != INET_STATE_CLOSED)
	return ctl_xerror(EXBADSEQ, rbuf, rsize);
#ifdef HAVE_SETNS
    if (desc->netns != NULL) {
	current_ns = open("/proc/self/ns/net", O_RDONLY);
	if (current_ns == INVALID_SOCKET)
	    return ctl_error(sock_errno(), rbuf, rsize);
	new_ns = open(desc->netns, O_RDONLY);
	if (new_ns == INVALID_SOCKET) {
	    save_errno = sock_errno();
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    return ctl_error(save_errno, rbuf, rsize);
	}
	if (setns(new_ns, CLONE_NEWNET) != 0) {
	    save_errno = sock_errno();
	    while (close(new_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    return ctl_error(save_errno, rbuf, rsize);
	}
	else {
	    while (close(new_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	}
    }
#endif
#ifdef HAVE_SYS_UN_H
    if (domain == AF_UNIX) protocol = 0;
#endif
    if ((desc->s = sock_open(domain, type, protocol)) == INVALID_SOCKET)
	save_errno = sock_errno();
#ifdef HAVE_SETNS
    if (desc->netns != NULL) {
	if (setns(current_ns, CLONE_NEWNET) != 0) {
	    if (desc->s != INVALID_SOCKET)
		save_errno = sock_errno();
	    while (close(desc->s) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    desc->s = INVALID_SOCKET;
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    return ctl_error(save_errno, rbuf, rsize);
	}
	else {
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	}
    }
#endif
    if (desc->s == INVALID_SOCKET)
	return ctl_error(save_errno, rbuf, rsize);
    if ((desc->event = sock_create_event(desc)) == INVALID_EVENT) {
	save_errno = sock_errno();
	while (close(desc->s) == INVALID_SOCKET &&
	       sock_errno() == EINTR)
	    ;
	desc->s = INVALID_SOCKET;
	return ctl_error(save_errno, rbuf, rsize);
    }
    SET_NONBLOCKING(desc->s);
#ifdef __WIN32__
    driver_select(desc->port, desc->event, ERL_DRV_READ, 1);
#endif
    desc->state = INET_STATE_OPEN;
    desc->stype = type;
    desc->sfamily = domain;
    return ctl_reply_ok(rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_fdopen(inet_descriptor* desc, int domain, int type,
				    SOCKET s, Uint32 bound,
                                    char** rbuf, ErlDrvSizeT rsize)
{
    inet_address name;
    SOCKLEN_T sz;
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "inet_ctl_fdopen -> entry with"
          "\r\n   socket:  %d"
          "\r\n   bound:   %s"
          "\r\n   domain:  %d"
          "\r\n   type:    %d"
          "\r\n",
          __LINE__,
          desc->s, driver_caller(desc->port),
          s, B2S(bound), domain, type) );
    if (bound) {
        sz = sizeof(name);
	sys_memzero((char *) &name, sz);
        if (IS_SOCKET_ERROR(sock_name(s, (struct sockaddr*) &name, &sz)))
            return ctl_error(sock_errno(), rbuf, rsize);
        if (name.sa.sa_family != domain)
            return ctl_error(EINVAL, rbuf, rsize);
    }
    desc->s = s;
    if ((desc->event = sock_create_event(desc)) == INVALID_EVENT)
	return ctl_error(sock_errno(), rbuf, rsize);
    SET_NONBLOCKING(desc->s);
#ifdef __WIN32__
    driver_select(desc->port, desc->event, ERL_DRV_READ, 1);
#endif
    desc->state = INET_STATE_OPEN;
    if (type == SOCK_STREAM) {
	sz = sizeof(name);
	if (!IS_SOCKET_ERROR(sock_peer(s, (struct sockaddr*) &name, &sz))) {
	    desc->state = INET_STATE_CONNECTED;
        }
    }
    desc->prebound = 1;
    desc->stype = type;
    desc->sfamily = domain;
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "inet_ctl_fdopen -> done\r\n",
          __LINE__, desc->s, driver_caller(desc->port)) );
    return ctl_reply_ok(rbuf, rsize);
}
struct addr_if {
    char name[INET_IFNAMSIZ];
    long           flags;
    struct in_addr addr;
    struct in_addr baddr;
    struct in_addr mask;
};
#ifndef SIOCGIFNETMASK
static struct in_addr net_mask(struct in_addr in)
{
    register u_long i = sock_ntohl(in.s_addr);
    if (IN_CLASSA(i))
	in.s_addr = sock_htonl(IN_CLASSA_NET);
    else if (IN_CLASSB(i))
	in.s_addr = sock_htonl(IN_CLASSB_NET);
    else
	in.s_addr = sock_htonl(IN_CLASSC_NET);
    return in;
}
#endif
#if defined(__WIN32__) && defined(SIO_GET_INTERFACE_LIST)
static char* fmt_addr(unsigned long x, char* ptr)
{
    int i;
    for (i = 0; i < 4; i++) {
	int nb[3];
	int y = (x >> 24) & 0xff;
	x <<= 8;
	nb[0] = y % 10; y /= 10;
	nb[1] = y % 10; y /= 10;
	nb[2] = y % 10; y /= 10;
	switch((nb[2] ? 3 : (nb[1] ? 2 : 1))) {
	case 3:  *ptr++ = nb[2] + '0';
	case 2:  *ptr++ = nb[1] + '0';
	case 1:  *ptr++ = nb[0] + '0';
	}
	*ptr++ = '.';
    }
    *(ptr-1) = '\0';
    return ptr;
}
static int parse_addr(char* ptr, int n, long* x)
{
    long addr = 0;
    int  dots = 0;
    int  digs = 0;
    int  v  = 0;
    while(n--) {
	switch(*ptr) {
	case '0': case '1': case '2':case '3':case '4':case '5':
	case '6': case '7': case '8':case '9':
	    v = v*10 + *ptr - '0';
	    if (++digs > 3) return -1;
	    break;
	case '.':
	    if ((dots>2) || (digs==0) || (digs > 3) || (v > 0xff)) return -1;
	    dots++;
	    digs = 0;
	    addr = (addr << 8) | v;
	    v = 0;
	    break;
	default:
	    return -1;
	}
	ptr++;
    }
    if ((dots!=3) || (digs==0) || (digs > 3) || (v > 0xff)) return -1;
    addr = (addr << 8) | v;
    *x = addr;
    return 0;
}
#endif
#define buf_check(ptr, end, n) \
do { if ((end)-(ptr) < (n)) goto error; } while(0)
static char* sockaddr_to_buf(struct sockaddr* addr, char* ptr, char* end)
{
    if (addr->sa_family == AF_INET || addr->sa_family == 0) {
	struct in_addr *p = &(((struct sockaddr_in*) addr)->sin_addr);
	buf_check(ptr, end, 1 + sizeof(struct in_addr));
	*ptr = INET_AF_INET;
	sys_memcpy(ptr+1, (char*)p, sizeof(struct in_addr));
	return ptr + 1 + sizeof(struct in_addr);
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    else if (addr->sa_family == AF_INET6) {
	struct in6_addr *p = &(((struct sockaddr_in6*) addr)->sin6_addr);
	buf_check(ptr, end, 1 + sizeof(struct in6_addr));
	*ptr = INET_AF_INET6;
	sys_memcpy(ptr+1, (char*)p, sizeof(struct in6_addr));
	return ptr + 1 + sizeof(struct in6_addr);
    }
#endif
#if defined(AF_LINK)
    else if (addr->sa_family == AF_LINK) {
	struct sockaddr_dl *sdl_p = (struct sockaddr_dl*) addr;
	buf_check(ptr, end, 2 + sdl_p->sdl_alen);
	put_int16(sdl_p->sdl_alen, ptr); ptr += 2;
	sys_memcpy(ptr, sdl_p->sdl_data + sdl_p->sdl_nlen, sdl_p->sdl_alen);
	return ptr + sdl_p->sdl_alen;
    }
#endif
#if defined(AF_PACKET) && defined(HAVE_NETPACKET_PACKET_H)
    else if(addr->sa_family == AF_PACKET) {
	struct sockaddr_ll *sll_p = (struct sockaddr_ll*) addr;
	buf_check(ptr, end, 2 + sll_p->sll_halen);
	put_int16(sll_p->sll_halen, ptr); ptr += 2;
	sys_memcpy(ptr, sll_p->sll_addr, sll_p->sll_halen);
	return ptr + sll_p->sll_halen;
    }
#endif
    return ptr;
 error:
    return NULL;
}
static size_t sockaddr_bufsz_need(struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET || addr->sa_family == 0) {
	return 1 + sizeof(struct in_addr);
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    else if (addr->sa_family == AF_INET6) {
	return 1 + sizeof(struct in6_addr);
    }
#endif
#if defined(AF_LINK)
    if (addr->sa_family == AF_LINK) {
	struct sockaddr_dl *sdl_p = (struct sockaddr_dl*) addr;
	return 2 + sdl_p->sdl_alen;
    }
#endif
#if defined(AF_PACKET) && defined(HAVE_NETPACKET_PACKET_H)
    else if(addr->sa_family == AF_PACKET) {
	struct sockaddr_ll *sll_p = (struct sockaddr_ll*) addr;
	return 2 + sll_p->sll_halen;
    }
#endif
    return 0;
}
static char* buf_to_sockaddr(char* ptr, char* end, struct sockaddr* addr)
{
    buf_check(ptr,end,1);
    switch (*ptr++) {
    case INET_AF_INET: {
	struct in_addr *p = &((struct sockaddr_in*)addr)->sin_addr;
	buf_check(ptr,end,sizeof(struct in_addr));
	sys_memcpy((char*) p, ptr, sizeof(struct in_addr));
	addr->sa_family = AF_INET;
	return ptr + sizeof(struct in_addr);
    }
#if defined(HAVE_IN6) && defined(AF_INET6)
    case INET_AF_INET6: {
	struct in6_addr *p = &((struct sockaddr_in6*)addr)->sin6_addr;
	buf_check(ptr,end,sizeof(struct in6_addr));
	sys_memcpy((char*) p, ptr, sizeof(struct in6_addr));
	addr->sa_family = AF_INET6;
	return ptr + sizeof(struct in6_addr);
    }
#endif
    }
 error:
    return NULL;
}
#if defined (IFF_POINTOPOINT)
#define IFGET_FLAGS(cflags) IFGET_FLAGS_P2P(cflags, IFF_POINTOPOINT)
#elif defined IFF_POINTTOPOINT
#define IFGET_FLAGS(cflags) IFGET_FLAGS_P2P(cflags, IFF_POINTTOPOINT)
#endif
#define IFGET_FLAGS_P2P(cflags, iff_ptp)				\
    ((((cflags) & IFF_UP) ? INET_IFF_UP : 0) |				\
     (((cflags) & IFF_BROADCAST) ? INET_IFF_BROADCAST : 0) |		\
     (((cflags) & IFF_LOOPBACK) ? INET_IFF_LOOPBACK : 0) |		\
     (((cflags) & iff_ptp) ? INET_IFF_POINTTOPOINT : 0) |		\
     (((cflags) & IFF_UP) ? INET_IFF_RUNNING : 0) |   \
     (((cflags) & IFF_MULTICAST) ? INET_IFF_MULTICAST : 0))
#if defined(__WIN32__) && defined(SIO_GET_INTERFACE_LIST)
static ErlDrvSSizeT inet_ctl_getiflist(inet_descriptor* desc,
				       char** rbuf, ErlDrvSizeT rsize)
{
    char ifbuf[BUFSIZ];
    char sbuf[BUFSIZ];
    char* sptr;
    INTERFACE_INFO* ifp;
    DWORD len;
    ErlDrvSizeT n;
    int err;
    ifp = (INTERFACE_INFO*) ifbuf;
    len = 0;
    err = WSAIoctl(desc->s, SIO_GET_INTERFACE_LIST, NULL, 0,
		   (LPVOID) ifp, BUFSIZ, (LPDWORD) &len,
		   NULL, NULL);
    if (err == SOCKET_ERROR)
	return ctl_error(sock_errno(), rbuf, rsize);
    n = (len + sizeof(INTERFACE_INFO) - 1) / sizeof(INTERFACE_INFO);
    sptr = sbuf;
    while(n--) {
	if (((struct sockaddr*)&ifp->iiAddress)->sa_family == desc->sfamily) {
	    struct in_addr sina = ((struct sockaddr_in*)&ifp->iiAddress)->sin_addr;
	    if (sina.s_addr != INADDR_ANY)
		sptr = fmt_addr(sock_ntohl(sina.s_addr), sptr);
	}
	ifp++;
    }
    return ctl_reply(INET_REP_OK, sbuf, sptr - sbuf, rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_ifget(inet_descriptor* desc, char* buf,
				   ErlDrvSizeT len, char** rbuf, ErlDrvSizeT rsize)
{
    char ifbuf[BUFSIZ];
    int  n;
    char sbuf[BUFSIZ];
    char* sptr;
    char* s_end = sbuf + BUFSIZ;
    int namlen;
    int   err;
    INTERFACE_INFO* ifp;
    long namaddr;
    if ((len == 0) || ((namlen = get_int8(buf)) > len))
	goto error;
    if (parse_addr(buf+1, namlen, &namaddr) < 0)
	goto error;
    namaddr = sock_ntohl(namaddr);
    buf += (namlen+1);
    len -= (namlen+1);
    ifp = (INTERFACE_INFO*) ifbuf;
    err = WSAIoctl(desc->s, SIO_GET_INTERFACE_LIST, NULL, 0,
			      (LPVOID) ifp, BUFSIZ, (LPDWORD) &n,
			      NULL, NULL);
    if (err == SOCKET_ERROR) {
	return ctl_error(sock_errno(), rbuf, rsize);
    }
    n = (n + sizeof(INTERFACE_INFO) - 1) / sizeof(INTERFACE_INFO);
    while(n) {
	if (((struct sockaddr_in*)&ifp->iiAddress)->sin_addr.s_addr == namaddr)
	    break;
	ifp++;
	n--;
    }
    if (n == 0)
	goto error;
    sptr = sbuf;
    while (len--) {
	switch(*buf++) {
	case INET_IFOPT_ADDR:
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_ADDR;
	    if ((sptr = sockaddr_to_buf((struct sockaddr *)&ifp->iiAddress,
					sptr, s_end)) == NULL)
		goto error;
	    break;
	case INET_IFOPT_HWADDR:
	    break;
	case INET_IFOPT_BROADADDR:
#ifdef SIOCGIFBRDADDR
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_BROADADDR;
	    if ((sptr=sockaddr_to_buf((struct sockaddr *)
				      &ifp->iiBroadcastAddress,sptr,s_end))
		== NULL)
		goto error;
#endif
	    break;
	case INET_IFOPT_DSTADDR:
	    break;
	case INET_IFOPT_NETMASK:
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_NETMASK;
	    if ((sptr = sockaddr_to_buf((struct sockaddr *)
					&ifp->iiNetmask,sptr,s_end)) == NULL)
		goto error;
	    break;
	case INET_IFOPT_MTU:
	    break;
	case INET_IFOPT_FLAGS: {
	    int flags = ifp->iiFlags;
	    buf_check(sptr, s_end, 5);
	    *sptr++ = INET_IFOPT_FLAGS;
	    put_int32(IFGET_FLAGS(flags), sptr);
	    sptr += 4;
	    break;
	}
	default:
	    goto error;
	}
    }
    return ctl_reply(INET_REP_OK, sbuf, sptr - sbuf, rbuf, rsize);
 error:
    return ctl_error(EINVAL, rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_ifset(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** rbuf, ErlDrvSizeT rsize)
{
    return ctl_reply_ok(rbuf, rsize);
}
#elif defined(SIOCGIFCONF) && defined(SIOCSIFFLAGS)
#define VOIDP(x) ((void*)(x))
#if defined(AF_LINK) && !defined(NO_SA_LEN)
#define SIZEA(p) (((p).sa_len > sizeof(p)) ? (p).sa_len : sizeof(p))
#else
#define SIZEA(p) (sizeof (p))
#endif
static int get_ifconf(SOCKET s, struct ifconf *ifcp) {
    int ifc_len = 0;
    int buflen = 100 * sizeof(struct ifreq);
    char *buf = ALLOC(buflen);
    for (;;) {
	ifcp->ifc_len = buflen;
	ifcp->ifc_buf = buf;
	if (ioctl(s, SIOCGIFCONF, (char *)ifcp) < 0) {
	    int res = sock_errno();
	    if (res != EINVAL || ifc_len) {
		FREE(buf);
		return -1;
	    }
	} else {
	    if (ifcp->ifc_len == ifc_len) break;
	    ifc_len = ifcp->ifc_len;
	}
	buflen += 10 * sizeof(struct ifreq);
	buf = (char *)REALLOC(buf, buflen);
    }
    return 0;
}
static void free_ifconf(struct ifconf *ifcp) {
    FREE(ifcp->ifc_buf);
}
static ErlDrvSSizeT inet_ctl_getiflist(inet_descriptor* desc,
				       char** rbuf, ErlDrvSizeT rsize)
{
    struct ifconf ifc;
    struct ifreq *ifrp;
    char *sbuf, *sp;
    ErlDrvSizeT i;
    if (get_ifconf(desc->s, &ifc) < 0) {
	return ctl_error(sock_errno(), rbuf, rsize);
    }
    sp = sbuf = ALLOC(ifc.ifc_len+1);
    *sp++ = INET_REP_OK;
    i = 0;
    for (;;) {
	ErlDrvSizeT n;
	ifrp = (struct ifreq *) VOIDP(ifc.ifc_buf + i);
	n = sizeof(ifrp->ifr_name) + SIZEA(ifrp->ifr_addr);
	if (n < sizeof(*ifrp)) n = sizeof(*ifrp);
	if (i+n > ifc.ifc_len) break;
	i += n;
	switch (ifrp->ifr_addr.sa_family) {
#if defined(HAVE_IN6) && defined(AF_INET6)
	case AF_INET6:
#endif
	case AF_INET:
	    ASSERT(sp+IFNAMSIZ+1 < sbuf+ifc.ifc_len+1);
	    strncpy(sp, ifrp->ifr_name, IFNAMSIZ);
	    sp[IFNAMSIZ] = '\0';
	    sp += strlen(sp), ++sp;
	}
	if (i >= ifc.ifc_len) break;
    }
    free_ifconf(&ifc);
    *rbuf = sbuf;
    return sp - sbuf;
}
#ifdef HAVE_LIBDLPI_H
#include <libdlpi.h>
static int hwaddr_libdlpi_lookup(const char *ifnm,
                                 uchar_t *addr, size_t *alen)
{
    dlpi_handle_t handle;
    dlpi_info_t linkinfo;
    int ret = -1;
    if (dlpi_open(ifnm, &handle, 0) != DLPI_SUCCESS) {
        return -1;
    }
    if (dlpi_get_physaddr(handle, DL_CURR_PHYS_ADDR,
                          addr, alen) == DLPI_SUCCESS &&
        dlpi_info(handle, &linkinfo, 0) == DLPI_SUCCESS)
    {
        ret = 0;
    }
    dlpi_close(handle);
    return ret;
}
#endif
#ifdef HAVE_GETIFADDRS
static int call_getifaddrs(inet_descriptor* desc_p, struct ifaddrs **ifa_pp)
{
    int result, save_errno;
#ifdef HAVE_SETNS
    int current_ns;
    current_ns = 0;
    if (desc_p->netns != NULL) {
        int new_ns;
	current_ns = open("/proc/self/ns/net", O_RDONLY);
	if (current_ns == INVALID_SOCKET) {
            save_errno = sock_errno();
            ASSERT(save_errno != 0);
	    return save_errno;
        }
	new_ns = open(desc_p->netns, O_RDONLY);
	if (new_ns == INVALID_SOCKET) {
	    save_errno = sock_errno();
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
            ASSERT(save_errno != 0);
	    return save_errno;
	}
	if (setns(new_ns, CLONE_NEWNET) != 0) {
	    save_errno = sock_errno();
	    while (close(new_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	    while (close(current_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
            ASSERT(save_errno != 0);
	    return save_errno;
	}
	else {
	    while (close(new_ns) == INVALID_SOCKET &&
		   sock_errno() == EINTR)
		;
	}
    }
#endif
    save_errno = 0;
    result = getifaddrs(ifa_pp);
    if (result  < 0)
        save_errno = sock_errno();
#ifdef HAVE_SETNS
    if (desc_p->netns != NULL) {
        if (setns(current_ns, CLONE_NEWNET) != 0) {
            if (result >= 0) {
                save_errno = sock_errno();
                ASSERT(save_errno != 0);
                freeifaddrs(*ifa_pp);
            }
	}
        while (close(current_ns) == INVALID_SOCKET &&
               sock_errno() == EINTR)
	    ;
    }
#endif
    return save_errno;
}
#endif
#ifndef IFHWADDRLEN
#define IFHWADDRLEN 6
#endif
static ErlDrvSSizeT inet_ctl_ifget(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** rbuf, ErlDrvSizeT rsize)
{
    char sbuf[BUFSIZ];
    char* sptr;
    char* s_end = sbuf + BUFSIZ;
    struct ifreq ifreq;
    int namlen;
    if ((len == 0) || ((namlen = get_int8(buf)) > len))
	goto error;
    sys_memset(ifreq.ifr_name, '\0', IFNAMSIZ);
    sys_memcpy(ifreq.ifr_name, buf+1,
	       (namlen >= IFNAMSIZ) ? IFNAMSIZ-1 : namlen);
    buf += (namlen+1);
    len -= (namlen+1);
    sptr = sbuf;
    while (len--) {
	switch(*buf++) {
	case INET_IFOPT_ADDR:
	    if (ioctl(desc->s, SIOCGIFADDR, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_ADDR;
	    if ((sptr = sockaddr_to_buf(&ifreq.ifr_addr, sptr, s_end)) == NULL)
		goto error;
	    break;
	case INET_IFOPT_HWADDR: {
#ifdef HAVE_LIBDLPI_H
	    uchar_t addr[DLPI_PHYSADDR_MAX];
	    size_t alen = sizeof(addr);
	    if (hwaddr_libdlpi_lookup(ifreq.ifr_name, addr, &alen) == 0) {
		buf_check(sptr, s_end, 1+2+alen);
		*sptr++ = INET_IFOPT_HWADDR;
		put_int16(alen, sptr);
                    sptr += 2;
                    sys_memcpy(sptr, addr, alen);
                    sptr += alen;
	    }
#elif defined(SIOCGIFHWADDR) && defined(HAVE_STRUCT_IFREQ_IFR_HWADDR)
	    if (ioctl(desc->s, SIOCGIFHWADDR, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1+2+IFHWADDRLEN);
	    *sptr++ = INET_IFOPT_HWADDR;
	    put_int16(IFHWADDRLEN, sptr); sptr += 2;
	    sys_memcpy(sptr, (char*)(&ifreq.ifr_hwaddr.sa_data), IFHWADDRLEN);
	    sptr += IFHWADDRLEN;
#elif defined(SIOCGENADDR) && defined(HAVE_STRUCT_IFREQ_IFR_ENADDR)
	    if (ioctl(desc->s, SIOCGENADDR, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1+2+sizeof(ifreq.ifr_enaddr));
	    *sptr++ = INET_IFOPT_HWADDR;
	    put_int16(sizeof(ifreq.ifr_enaddr), sptr); sptr += 2;
	    sys_memcpy(sptr, (char*)(&ifreq.ifr_enaddr),
		       sizeof(ifreq.ifr_enaddr));
	    sptr += sizeof(ifreq.ifr_enaddr);
#elif defined(HAVE_GETIFADDRS) && defined(AF_LINK)
	    struct ifaddrs *ifa, *ifp;
	    struct sockaddr_dl *sdlp;
	    int found = 0;
            if (call_getifaddrs(desc, &ifa) != 0)
                goto error;
	    for (ifp = ifa; ifp; ifp = ifp->ifa_next) {
		if ((ifp->ifa_addr->sa_family == AF_LINK) &&
		    (sys_strcmp(ifp->ifa_name, ifreq.ifr_name) == 0)) {
		    found = 1;
		    break;
		}
	    }
	    if (found == 0) {
		freeifaddrs(ifa);
		break;
	    }
	    sdlp = (struct sockaddr_dl *)ifp->ifa_addr;
	    buf_check(sptr, s_end, 1+2+sdlp->sdl_alen);
	    *sptr++ = INET_IFOPT_HWADDR;
	    put_int16(sdlp->sdl_alen, sptr); sptr += 2;
	    sys_memcpy(sptr,
		       sdlp->sdl_data + sdlp->sdl_nlen,
		       sdlp->sdl_alen);
	    sptr += sdlp->sdl_alen;
	    freeifaddrs(ifa);
#endif
	    break;
	}
	case INET_IFOPT_BROADADDR:
#ifdef SIOCGIFBRDADDR
	    if (ioctl(desc->s, SIOCGIFBRDADDR, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_BROADADDR;
	    if ((sptr=sockaddr_to_buf(&ifreq.ifr_broadaddr,sptr,s_end)) == NULL)
		goto error;
#endif
	    break;
	case INET_IFOPT_DSTADDR:
#ifdef SIOCGIFDSTADDR
	    if (ioctl(desc->s, SIOCGIFDSTADDR, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_DSTADDR;
	    if ((sptr = sockaddr_to_buf(&ifreq.ifr_dstaddr,sptr,s_end)) == NULL)
		goto error;
#endif
	    break;
	case INET_IFOPT_NETMASK:
#if defined(SIOCGIFNETMASK)
	    if (ioctl(desc->s, SIOCGIFNETMASK, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 1);
	    *sptr++ = INET_IFOPT_NETMASK;
#if defined(ifr_netmask)
	    sptr = sockaddr_to_buf(&ifreq.ifr_netmask,sptr,s_end);
#else
	    sptr = sockaddr_to_buf(&ifreq.ifr_addr,sptr,s_end);
#endif
	    if (sptr == NULL)
		goto error;
#else
	    if (ioctl(desc->s, SIOCGIFADDR, (char *)&ifreq) < 0)
		break;
	    else {
		struct sockadd_in* ap;
		buf_check(sptr, s_end, 1);
		*sptr++ = INET_IFOPT_NETMASK;
		ap = (struct sockaddr_in*) VOIDP(&ifreq.ifr_addr);
		ap->sin_addr = net_mask(ap->sin_addr);
		if ((sptr = sockaddr_to_buf(&ifreq.ifr_addr,sptr,s_end)) == NULL)
		    goto error;
	    }
#endif
	    break;
	case INET_IFOPT_MTU: {
#if defined(SIOCGIFMTU) && defined(ifr_mtu)
	    int n;
	    if (ioctl(desc->s, SIOCGIFMTU, (char *)&ifreq) < 0)
		break;
	    buf_check(sptr, s_end, 5);
	    *sptr++ = INET_IFOPT_MTU;
	    n = ifreq.ifr_mtu;
	    put_int32(n, sptr);
	    sptr += 4;
#endif
	    break;
	}
	case INET_IFOPT_FLAGS: {
	    int flags;
	    if (ioctl(desc->s, SIOCGIFFLAGS, (char*)&ifreq) < 0)
		flags = 0;
	    else
		flags = ifreq.ifr_flags;
	    buf_check(sptr, s_end, 5);
	    *sptr++ = INET_IFOPT_FLAGS;
	    put_int32(IFGET_FLAGS(flags), sptr);
	    sptr += 4;
	    break;
	}
	default:
	    goto error;
	}
    }
    return ctl_reply(INET_REP_OK, sbuf, sptr - sbuf, rbuf, rsize);
 error:
    return ctl_error(EINVAL, rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_ifset(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** rbuf, ErlDrvSizeT rsize)
{
    struct ifreq ifreq;
    int namlen;
    char* b_end = buf + len;
    if ((len == 0) || ((namlen = get_int8(buf)) > len))
	goto error;
    sys_memset(ifreq.ifr_name, '\0', IFNAMSIZ);
    sys_memcpy(ifreq.ifr_name, buf+1,
	       (namlen >= IFNAMSIZ) ? IFNAMSIZ-1 : namlen);
    buf += (namlen+1);
    len -= (namlen+1);
    while(buf < b_end) {
	switch(*buf++) {
	case INET_IFOPT_ADDR:
	    if ((buf = buf_to_sockaddr(buf, b_end, &ifreq.ifr_addr)) == NULL)
		goto error;
	    (void) ioctl(desc->s, SIOCSIFADDR, (char*)&ifreq);
	    break;
	case INET_IFOPT_HWADDR: {
	    unsigned int hwalen;
	    buf_check(buf, b_end, 2);
	    hwalen = get_int16(buf); buf += 2;
	    buf_check(buf, b_end, hwalen);
#ifdef SIOCSIFHWADDR
	    sys_memset((char*)(&ifreq.ifr_hwaddr.sa_data),
		       '\0', sizeof(ifreq.ifr_hwaddr.sa_data));
	    sys_memcpy((char*)(&ifreq.ifr_hwaddr.sa_data), buf, hwalen);
	    (void) ioctl(desc->s, SIOCSIFHWADDR, (char *)&ifreq);
#endif
	    buf += hwalen;
	    break;
	}
	case INET_IFOPT_BROADADDR:
#ifdef SIOCSIFBRDADDR
	    if ((buf = buf_to_sockaddr(buf, b_end, &ifreq.ifr_broadaddr)) == NULL)
		goto error;
	    (void) ioctl(desc->s, SIOCSIFBRDADDR, (char *)&ifreq);
#endif
	    break;
	case INET_IFOPT_DSTADDR:
#ifdef SIOCSIFDSTADDR
	    if ((buf = buf_to_sockaddr(buf, b_end, &ifreq.ifr_dstaddr)) == NULL)
		goto error;
	    (void) ioctl(desc->s, SIOCSIFDSTADDR, (char *)&ifreq);
#endif
	    break;
	case INET_IFOPT_NETMASK:
#ifdef SIOCSIFNETMASK
#if defined(ifr_netmask)
	    buf = buf_to_sockaddr(buf,b_end, &ifreq.ifr_netmask);
#else
	    buf = buf_to_sockaddr(buf,b_end, &ifreq.ifr_addr);
#endif
	    if (buf == NULL)
		goto error;
	    (void) ioctl(desc->s, SIOCSIFNETMASK, (char *)&ifreq);
#endif
	    break;
	case INET_IFOPT_MTU:
	    buf_check(buf, b_end, 4);
#if defined(SIOCSIFMTU) && defined(ifr_mtu)
	    ifreq.ifr_mtu = get_int32(buf);
	    (void) ioctl(desc->s, SIOCSIFMTU, (char *)&ifreq);
#endif
	    buf += 4;
	    break;
	case INET_IFOPT_FLAGS: {
	    int flags0;
	    int flags;
	    int eflags;
	    buf_check(buf, b_end, 4);
	    eflags = get_int32(buf);
	    if (ioctl(desc->s, SIOCGIFFLAGS, (char*)&ifreq) < 0)
		flags0 = flags = 0;
	    else
		flags0 = flags = ifreq.ifr_flags;
	    if (eflags & INET_IFF_UP)            flags |= IFF_UP;
	    if (eflags & INET_IFF_DOWN)          flags &= ~IFF_UP;
	    if (eflags & INET_IFF_BROADCAST)     flags |= IFF_BROADCAST;
	    if (eflags & INET_IFF_NBROADCAST)    flags &= ~IFF_BROADCAST;
	    if (eflags & INET_IFF_POINTTOPOINT)  flags |= IFF_POINTOPOINT;
	    if (eflags & INET_IFF_NPOINTTOPOINT) flags &= ~IFF_POINTOPOINT;
	    if (flags != flags0) {
		ifreq.ifr_flags = flags;
		(void) ioctl(desc->s, SIOCSIFFLAGS, (char*)&ifreq);
	    }
	    buf += 4;
	    break;
	}
	default:
	    goto error;
	}
    }
    return ctl_reply_ok(rbuf, rsize);
 error:
    return ctl_error(EINVAL, rbuf, rsize);
}
#else
static ErlDrvSSizeT inet_ctl_getiflist(inet_descriptor* desc,
				       char** rbuf, ErlDrvSizeT rsize)
{
    return ctl_reply_ok(rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_ifget(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** rbuf, ErlDrvSizeT rsize)
{
    return ctl_reply_ok(rbuf, rsize);
}
static ErlDrvSSizeT inet_ctl_ifset(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** rbuf, ErlDrvSizeT rsize)
{
    return ctl_reply_ok(rbuf, rsize);
}
#endif
#if defined(__WIN32__) || defined(HAVE_GETIFADDRS)
static int utf8_len(const char *c, int m) {
    int l;
    for (l = 0;  m;  c++, l++, m--) {
	if (*c == '\0') break;
	if ((*c & 0x7f) != *c) l++;
    }
    return l;
}
static void utf8_encode(const char *c, int m, char *p) {
    for (;  m;  c++, m--) {
	if (*c == '\0') break;
	if ((*c & 0x7f) != *c) {
	    *p++ = (char) (0xC0 | (0x03 & (*c >> 6)));
	    *p++ = (char) (0x80 | (0x3F & *c));
	} else {
	    *p++ = (char) *c;
	}
    }
}
#endif
#if defined(__WIN32__)
static void set_netmask_bytes(char *c, int len, int pref_len) {
    int i, m;
    for (i = 0, m = pref_len >> 3;  i < m && i < len;  i++) c[i] = '\xFF';
    if (i < len) c[i++] = 0xFF << (8 - (pref_len & 7));
    for (;  i < len;  i++) c[i] = '\0';
}
int eq_masked_bytes(char *a, char *b, int pref_len) {
    int i, m;
    for (i = 0, m = pref_len >> 3;  i < m;  i++) {
	if (a[i] != b[i]) return 0;
    }
    m = pref_len & 7;
    if (m) {
	m = 0xFF & (0xFF << (8 - m));
	if ((a[i] & m) != (b[i] & m)) return 0;
    }
    return !0;
}
static ErlDrvSSizeT inet_ctl_getifaddrs(inet_descriptor* desc_p,
					char **rbuf_pp, ErlDrvSizeT rsize)
{
    int i;
    DWORD ret, n;
    IP_INTERFACE_INFO *info_p = NULL;
    MIB_IPADDRTABLE *ip_addrs_p = NULL;
    IP_ADAPTER_ADDRESSES *ip_adaddrs_p = NULL;
    IP_ADAPTER_ADDRESSES *ia_p = NULL;
    char *buf_p;
    char *buf_alloc_p;
    ErlDrvSizeT buf_size = 512;
#   define BUF_ENSURE(Size)						\
    do {								\
	int NEED_, GOT_ = buf_p - buf_alloc_p;				\
	NEED_ = GOT_ + (Size);						\
	if (NEED_ > buf_size) {						\
	    buf_size = NEED_ + 512;					\
	    buf_alloc_p = REALLOC(buf_alloc_p, buf_size);		\
	    buf_p = buf_alloc_p + GOT_;					\
	}								\
    } while(0)
#   define SOCKADDR_TO_BUF(opt, sa)					\
    do {								\
	if (sa) {							\
	    char *P_;							\
	    *buf_p++ = (opt);						\
	    while (! (P_ = sockaddr_to_buf((sa), buf_p,			\
					   buf_alloc_p+buf_size))) {    \
		int GOT_ = buf_p - buf_alloc_p;				\
		buf_size += 512;					\
		buf_alloc_p = REALLOC(buf_alloc_p, buf_size);		\
		buf_p = buf_alloc_p + GOT_;				\
	    }								\
	    if (P_ == buf_p) {						\
		buf_p--;						\
	    } else {							\
		buf_p = P_;						\
	    }								\
	}								\
    } while (0)
    {
	unsigned long ip_adaddrs_size = 16 * 1024;
	ULONG family = AF_UNSPEC;
	ULONG flags =
	    GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_SKIP_ANYCAST |
	    GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME |
	    GAA_FLAG_SKIP_MULTICAST;
	ULONG (WINAPI *fpGetAdaptersAddresses)
	    (ULONG, ULONG, PVOID, PIP_ADAPTER_ADDRESSES, PULONG);
	HMODULE iphlpapi = GetModuleHandle("iphlpapi");
	fpGetAdaptersAddresses = (void *)
	    (iphlpapi ?
		GetProcAddress(iphlpapi, "GetAdaptersAddresses") :
		NULL);
	if (fpGetAdaptersAddresses) {
	    ip_adaddrs_p = ALLOC(ip_adaddrs_size);
	    for (i = 17;  i;  i--) {
		ret = fpGetAdaptersAddresses(
		    family, flags, NULL, ip_adaddrs_p, &ip_adaddrs_size);
		ip_adaddrs_p = REALLOC(ip_adaddrs_p, ip_adaddrs_size);
		if (ret == NO_ERROR) break;
		if (ret == ERROR_BUFFER_OVERFLOW) continue;
		i = 0;
		break;
	    }
	    if (! i) {
		FREE(ip_adaddrs_p);
                ip_adaddrs_p = NULL;
	    }
	} else ip_adaddrs_p = NULL;
    }
    {
	unsigned long info_size = 4 * 1024;
	info_p = ALLOC(info_size);
	for (i = 17;  i;  i--) {
	    ret = GetInterfaceInfo(info_p, &info_size);
	    info_p = REALLOC(info_p, info_size);
	    if (ret == NO_ERROR) break;
	    if (ret == ERROR_INSUFFICIENT_BUFFER) continue;
	    i = 0;
	    break;
	}
	if (! i) {
            FREE(info_p);
            info_p = NULL;
        }
    }
    if (! ip_adaddrs_p) {
	unsigned long ip_addrs_size = 16 * sizeof(*ip_addrs_p);
	ip_addrs_p = ALLOC(ip_addrs_size);
	for (i = 17;  i;  i--) {
	    ret = GetIpAddrTable(ip_addrs_p, &ip_addrs_size, FALSE);
	    ip_addrs_p = REALLOC(ip_addrs_p, ip_addrs_size);
	    if (ret == NO_ERROR) break;
	    if (ret == ERROR_INSUFFICIENT_BUFFER) continue;
	    i = 0;
	    break;
	}
	if (! i) {
	    if (info_p) FREE(info_p);
	    FREE(ip_addrs_p);
	    return ctl_reply_ok(rbuf_pp, rsize);
	}
    } else ip_addrs_p = NULL;
    buf_p = buf_alloc_p = ALLOC(buf_size);
    *buf_p++ = INET_REP_OK;
    ia_p = NULL;
    for (ip_addrs_p ? (void)(i = 0) : (void)(ia_p = ip_adaddrs_p);
	 ip_addrs_p ? (i < ip_addrs_p->dwNumEntries) : (ia_p != NULL);
	 ip_addrs_p ? (void)(i++) : (void)(ia_p = ia_p->Next)) {
	MIB_IPADDRROW *ipaddrrow_p = NULL;
	DWORD flags = INET_IFF_MULTICAST;
	DWORD index = 0;
	WCHAR *wname_p = NULL;
	MIB_IFROW ifrow;
	if (ip_addrs_p) {
	    ipaddrrow_p = ip_addrs_p->table + i;
	    index = ipaddrrow_p->dwIndex;
	} else {
	    index = ia_p->IfIndex;
	    if (ia_p->Flags & IP_ADAPTER_NO_MULTICAST) {
		flags &= ~INET_IFF_MULTICAST;
	    }
	}
index:
	if (! index) goto done;
	sys_memzero(&ifrow, sizeof(ifrow));
	ifrow.dwIndex = index;
	if (GetIfEntry(&ifrow) != NO_ERROR) break;
	if (ifrow.wszName[0] != 0) {
	    wname_p = ifrow.wszName;
	} else {
	    int j;
	    if (info_p) {
	        for (j = 0;  j < info_p->NumAdapters;  j++) {
		    if (info_p->Adapter[j].Index == (ULONG) ifrow.dwIndex) {
		        if (info_p->Adapter[j].Name[0] != 0) {
			    wname_p = info_p->Adapter[j].Name;
		        }
		        break;
		    }
                }
	    }
	}
	if (wname_p) {
	    int len;
	    len =
		WideCharToMultiByte(
		    CP_UTF8, 0, wname_p, -1, NULL, 0, NULL, NULL);
	    if (! len) break;
	    BUF_ENSURE(len);
	    WideCharToMultiByte(
		CP_UTF8, 0, wname_p, -1, buf_p, len, NULL, NULL);
	    buf_p += len;
	} else {
	    int l;
	    l = utf8_len(ifrow.bDescr, ifrow.dwDescrLen);
	    BUF_ENSURE(9 + l+1);
	    buf_p +=
		erts_sprintf(
		    buf_p, "%lu: ", (unsigned long) ifrow.dwIndex);
	    utf8_encode(ifrow.bDescr, ifrow.dwDescrLen, buf_p);
	    buf_p += l;
	    *buf_p++ = '\0';
	}
	switch (ifrow.dwType) {
	case IF_TYPE_ETHERNET_CSMACD:
	    flags |= INET_IFF_BROADCAST;
	    break;
	case IF_TYPE_SOFTWARE_LOOPBACK:
	    flags |= INET_IFF_LOOPBACK;
	    flags &= ~INET_IFF_MULTICAST;
	    break;
	default:
	    flags &= ~INET_IFF_MULTICAST;
	    break;
	}
	if (ifrow.dwAdminStatus) {
	    flags |= INET_IFF_UP;
	    switch (ifrow.dwOperStatus) {
	    case IF_OPER_STATUS_CONNECTING:
		flags |= INET_IFF_POINTTOPOINT;
		break;
	    case IF_OPER_STATUS_CONNECTED:
		flags |= INET_IFF_RUNNING | INET_IFF_POINTTOPOINT;
		break;
	    case IF_OPER_STATUS_OPERATIONAL:
		flags |= INET_IFF_RUNNING;
		break;
	    }
	}
	BUF_ENSURE(1 + 4);
	*buf_p++ = INET_IFOPT_FLAGS;
	put_int32(flags, buf_p); buf_p += 4;
	if (ipaddrrow_p) {
	    struct sockaddr_in sin;
	    sys_memzero(&sin, sizeof(sin));
	    sin.sin_family = AF_INET;
	    sin.sin_addr.s_addr = ipaddrrow_p->dwAddr;
	    BUF_ENSURE(1);
	    SOCKADDR_TO_BUF(INET_IFOPT_ADDR, (struct sockaddr *) &sin);
	    sin.sin_addr.s_addr = ipaddrrow_p->dwMask;
	    BUF_ENSURE(1);
	    SOCKADDR_TO_BUF(INET_IFOPT_NETMASK, (struct sockaddr *) &sin);
	    if (flags & INET_IFF_BROADCAST) {
		sin.sin_addr.s_addr = ipaddrrow_p->dwAddr;
		sin.sin_addr.s_addr |= ~ipaddrrow_p->dwMask;
		BUF_ENSURE(1);
		SOCKADDR_TO_BUF(
		    INET_IFOPT_BROADADDR, (struct sockaddr *) &sin);
	    }
	} else {
	    IP_ADAPTER_UNICAST_ADDRESS *p;
	    for (p = ia_p->FirstUnicastAddress;
		p;
		p = p->Next)
	    {
		IP_ADAPTER_PREFIX *q;
		ULONG shortest_length;
		struct sockaddr *shortest_p, *sa_p = p->Address.lpSockaddr;
		BUF_ENSURE(1);
		SOCKADDR_TO_BUF(INET_IFOPT_ADDR, sa_p);
		shortest_p = NULL;
		shortest_length = 0;
		for (q = ia_p->FirstPrefix;
		     q;
		     q = q->Next) {
		    struct sockaddr *sp_p = q->Address.lpSockaddr;
		    if (sa_p->sa_family != sp_p->sa_family) continue;
		    switch (sa_p->sa_family) {
		    case AF_INET: {
			struct sockaddr_in sin;
			DWORD sa, sp, mask;
			sa = ntohl((DWORD)
				   ((struct sockaddr_in *)
				    sa_p)->sin_addr.s_addr);
			sp = ntohl((DWORD)
				   ((struct sockaddr_in *)
				    sp_p)->sin_addr.s_addr);
			mask = 0xFFFFFFFF << (32 - q->PrefixLength);
			if ((sa & mask) != (sp & mask)) continue;
			if ((! shortest_p)
			    || q->PrefixLength < shortest_length) {
			    shortest_p = sp_p;
			    shortest_length = q->PrefixLength;
			}
		    }   break;
		    case AF_INET6: {
			struct sockaddr_in6 sin6;
			if (!eq_masked_bytes((char *)
					     &((struct sockaddr_in6 *)
					       sa_p)->sin6_addr,
					     (char *)
					     &((struct sockaddr_in6 *)
					       sp_p)->sin6_addr,
					     q->PrefixLength)) {
			    continue;
			}
			if ((! shortest_p)
			    || q->PrefixLength < shortest_length) {
			    shortest_p = sp_p;
			    shortest_length = q->PrefixLength;
			}
		    }   break;
		    }
		}
		if (! shortest_p) {
		    shortest_p = sa_p;
		    switch (shortest_p->sa_family) {
		    case AF_INET: {
			DWORD addr = ntohl(((struct sockaddr_in *)shortest_p)
					   ->sin_addr.s_addr);
			if (! (addr & 0x800000)) {
			    shortest_length = 8;
			} else if (! (addr & 0x400000)) {
			    shortest_length = 16;
			} else if (! (addr & 0x200000)) {
			    shortest_length = 24;
			} else {
			    shortest_length = 32;
			}
		    }   break;
		    case AF_INET6: {
			shortest_length = 128;
		    }   break;
		    }
		}
		switch (shortest_p->sa_family) {
		case AF_INET: {
		    struct sockaddr_in sin;
		    DWORD mask = 0xFFFFFFFF << (32 - shortest_length);
		    sys_memzero(&sin, sizeof(sin));
		    sin.sin_family = shortest_p->sa_family;
		    sin.sin_addr.s_addr = htonl(mask);
		    BUF_ENSURE(1);
		    SOCKADDR_TO_BUF(INET_IFOPT_NETMASK,
				    (struct sockaddr *) &sin);
		    if (flags & INET_IFF_BROADCAST) {
			DWORD sp =
			    ntohl((DWORD)
				  ((struct sockaddr_in *)shortest_p)
				  -> sin_addr.s_addr);
			sin.sin_addr.s_addr = htonl(sp | ~mask);
			BUF_ENSURE(1);
			SOCKADDR_TO_BUF(INET_IFOPT_BROADADDR,
					(struct sockaddr *) &sin);
		    }
		}   break;
		case AF_INET6: {
		    struct sockaddr_in6 sin6;
		    sys_memzero(&sin6, sizeof(sin6));
		    sin6.sin6_family = shortest_p->sa_family;
		    set_netmask_bytes((char *) &sin6.sin6_addr,
				      16,
				      shortest_length);
		    BUF_ENSURE(1);
		    SOCKADDR_TO_BUF(INET_IFOPT_NETMASK,
				    (struct sockaddr *) &sin6);
		}   break;
		}
	    }
	}
	if (ifrow.dwPhysAddrLen) {
	    BUF_ENSURE(1 + 2 + ifrow.dwPhysAddrLen);
	    *buf_p++ = INET_IFOPT_HWADDR;
	    put_int16(ifrow.dwPhysAddrLen, buf_p); buf_p += 2;
	    sys_memcpy(buf_p, ifrow.bPhysAddr, ifrow.dwPhysAddrLen);
	    buf_p += ifrow.dwPhysAddrLen;
	}
done:
	BUF_ENSURE(1);
	*buf_p++ = '\0';
	if (ia_p &&
	    ia_p->Ipv6IfIndex &&
	    ia_p->Ipv6IfIndex != index)
	{
	    index = ia_p->Ipv6IfIndex;
	    goto index;
	}
    }
    if (ip_adaddrs_p) FREE(ip_adaddrs_p);
    if (info_p) FREE(info_p);
    if (ip_addrs_p) FREE(ip_addrs_p);
    buf_size = buf_p - buf_alloc_p;
    buf_alloc_p = REALLOC(buf_alloc_p, buf_size);
    *rbuf_pp = buf_alloc_p;
    return buf_size;
#   undef BUF_ENSURE
}
#elif defined(HAVE_GETIFADDRS)
#ifdef  DEBUG
#define GETIFADDRS_BUFSZ (1)
#else
#define GETIFADDRS_BUFSZ (512)
#endif
static ErlDrvSSizeT inet_ctl_getifaddrs(inet_descriptor* desc_p,
					char **rbuf_pp, ErlDrvSizeT rsize)
{
    struct ifaddrs *ifa_p, *ifa_free_p;
    ErlDrvSizeT buf_size;
    char *buf_p;
    char *buf_alloc_p;
    int save_errno;
    buf_size = GETIFADDRS_BUFSZ;
    buf_alloc_p = ALLOC(GETIFADDRS_BUFSZ);
    buf_p = buf_alloc_p;
#   define BUF_ENSURE(Size)						\
    do {								\
	int NEED_, GOT_ = buf_p - buf_alloc_p;				\
	NEED_ = GOT_ + (Size);						\
	if (NEED_ > buf_size) {						\
	    buf_size = NEED_ + GETIFADDRS_BUFSZ;			\
	    buf_alloc_p = REALLOC(buf_alloc_p, buf_size);		\
	    buf_p = buf_alloc_p + GOT_;					\
	}								\
    } while (0)
#   define SOCKADDR_TO_BUF(opt, sa)				        \
    do {    						                \
	if (sa) {                                                       \
	    char *P_;							\
	    *buf_p++ = (opt);						\
	    while (! (P_ = sockaddr_to_buf((sa), buf_p,			\
					   buf_alloc_p+buf_size))) {	\
		int GOT_ = buf_p - buf_alloc_p;				\
		buf_size += GETIFADDRS_BUFSZ;				\
		buf_alloc_p = REALLOC(buf_alloc_p, buf_size);		\
		buf_p = buf_alloc_p + GOT_;				\
	    }								\
	    if (P_ == buf_p) {						\
		buf_p--;						\
	    } else {							\
		buf_p = P_;						\
	    }								\
	}                                                               \
    } while (0)
    DDBG(desc_p,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "%s -> get if addrs"
          "\r\n",
          __LINE__, desc_p->s, driver_caller(desc_p->port), __FUNCTION__) );
    if ((save_errno = call_getifaddrs(desc_p, &ifa_p)) != 0)
        return ctl_error(save_errno, rbuf_pp, rsize);
    ifa_free_p = ifa_p;
    *buf_p++ = INET_REP_OK;
    for (;  ifa_p;  ifa_p = ifa_p->ifa_next) {
	int len = utf8_len(ifa_p->ifa_name, -1);
        DDBG(desc_p,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "%s -> process if-addr %s"
              "\r\n   flags: 0x%X"
              "\r\n",
              __LINE__, desc_p->s, driver_caller(desc_p->port), __FUNCTION__,
              ifa_p->ifa_name, ifa_p->ifa_flags) );
	BUF_ENSURE(len+1 + 1+4 + 1);
	utf8_encode(ifa_p->ifa_name, -1, buf_p);
	buf_p += len;
	*buf_p++ = '\0';
	*buf_p++ = INET_IFOPT_FLAGS;
	put_int32(IFGET_FLAGS(ifa_p->ifa_flags), buf_p); buf_p += 4;
	if (ifa_p->ifa_addr) {
            DDBG(desc_p,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "%s -> family: %d (%s)"
                  "\r\n",
                  __LINE__, desc_p->s, driver_caller(desc_p->port), __FUNCTION__,
                  ifa_p->ifa_addr->sa_family,
                  FAM2S(ifa_p->ifa_addr->sa_family)) );
	    if (ifa_p->ifa_addr->sa_family == AF_INET
#if defined(AF_INET6)
		|| ifa_p->ifa_addr->sa_family == AF_INET6
#endif
		) {
		SOCKADDR_TO_BUF(INET_IFOPT_ADDR, ifa_p->ifa_addr);
		if (ifa_p->ifa_netmask) {
		    BUF_ENSURE(1);
		    SOCKADDR_TO_BUF(INET_IFOPT_NETMASK, ifa_p->ifa_netmask);
		}
		if (ifa_p->ifa_dstaddr &&
		    (ifa_p->ifa_flags & IFF_POINTOPOINT)) {
		    BUF_ENSURE(1);
		    SOCKADDR_TO_BUF(INET_IFOPT_DSTADDR, ifa_p->ifa_dstaddr);
		} else if (ifa_p->ifa_broadaddr &&
			   (ifa_p->ifa_flags & IFF_BROADCAST)) {
		    BUF_ENSURE(1);
		    SOCKADDR_TO_BUF(INET_IFOPT_BROADADDR, ifa_p->ifa_broadaddr);
		}
	    }
#if defined(AF_LINK) || defined(AF_PACKET)
	    else if (
#if defined(AF_LINK)
		     ifa_p->ifa_addr->sa_family == AF_LINK
#else
		     0
#endif
#if defined(AF_PACKET)
		     || ifa_p->ifa_addr->sa_family == AF_PACKET
#endif
		     ) {
		size_t need = sockaddr_bufsz_need(ifa_p->ifa_addr);
		if (need > 3) {
		    BUF_ENSURE(1 + need);
		    SOCKADDR_TO_BUF(INET_IFOPT_HWADDR, ifa_p->ifa_addr);
		}
	    }
#endif
	}
	BUF_ENSURE(1);
	*buf_p++ = '\0';
    }
    buf_size = buf_p - buf_alloc_p;
    buf_alloc_p = REALLOC(buf_alloc_p, buf_size);
    freeifaddrs(ifa_free_p);
    *rbuf_pp = buf_alloc_p;
    DDBG(desc_p,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "%s -> done when buffer size: %d"
          "\r\n",
          __LINE__, desc_p->s, driver_caller(desc_p->port), __FUNCTION__,
          buf_size) );
    return buf_size;
#   undef BUF_ENSURE
}
#undef GETIFADDRS_BUFSZ
#else
static ErlDrvSSizeT inet_ctl_getifaddrs(inet_descriptor* desc_p,
					char **rbuf_pp, ErlDrvSizeT rsize)
{
    return ctl_error(ENOTSUP, rbuf_pp, rsize);
}
#endif
#if  defined(IP_TOS) && defined(IPPROTO_IP)             \
    && defined(SO_PRIORITY) && !defined(__WIN32__)
static int setopt_prio_tos_trick
	(int fd, int proto, int type, char* arg_ptr, int arg_sz, int propagate)
{
    int          tmp_ival_prio;
    int          tmp_ival_tos;
    int          res;
    int          res_prio;
    int          res_tos;
    SOCKLEN_T    tmp_arg_sz_prio = sizeof(tmp_ival_prio);
    SOCKLEN_T    tmp_arg_sz_tos  = sizeof(tmp_ival_tos);
    res_prio = sock_getopt(fd, SOL_SOCKET, SO_PRIORITY,
		      (char *) &tmp_ival_prio, &tmp_arg_sz_prio);
    res_tos = sock_getopt(fd, IPPROTO_IP, IP_TOS,
		      (char *) &tmp_ival_tos, &tmp_arg_sz_tos);
	    res = sock_setopt(fd, proto, type, arg_ptr, arg_sz);
	    if (res == 0) {
		if (type != SO_PRIORITY) {
	    if (type != IP_TOS && res_tos == 0) {
		res_tos = sock_setopt(fd,
					  IPPROTO_IP,
					  IP_TOS,
					  (char *) &tmp_ival_tos,
					  tmp_arg_sz_tos);
		if (propagate)
		    res = res_tos;
		    }
	    if (res == 0 && res_prio == 0) {
		res_prio = sock_setopt(fd,
					   SOL_SOCKET,
					   SO_PRIORITY,
					   (char *) &tmp_ival_prio,
					   tmp_arg_sz_prio);
		if (propagate) {
		    if (res_prio != 0 && sock_errno() == EPERM) {
			res = 0;
		    } else {
			res = res_prio;
		    }
		}
	    }
	}
    }
    return (res);
}
#endif
static
int inet_setopt(int fd,
                int proto, int type,
                char* arg_ptr, int arg_sz,
                int propagate)
{
    int res;
#if defined(IP_TOS) && defined(IPPROTO_IP)             \
    && defined(SO_PRIORITY) && !defined(__WIN32__)
    DEBUGF(("inet_setopt -> try trick setopt with"
             "\r\n   fd:        %d"
             "\r\n   proto:     %d"
             "\r\n   type:      %d"
             "\r\n   sz:        %d"
             "\r\n   propagate: %d"
             "\r\n", fd, proto, type, arg_sz, propagate));
    res = setopt_prio_tos_trick (fd, proto, type, arg_ptr, arg_sz, propagate);
#else
    DEBUGF(("inet_setopt -> try std setopt with"
             "\r\n   fd:        %d"
             "\r\n   proto:     %d"
             "\r\n   type:      %d"
             "\r\n   sz:        %d"
             "\r\n", fd, proto, type, arg_sz));
    res = sock_setopt	    (fd, proto, type, arg_ptr, arg_sz);
#endif
    return res;
}
#ifdef HAVE_SCTP
static int sctp_set_opts(inet_descriptor* desc, char* ptr, int len);
#endif
static int inet_set_opts(inet_descriptor* desc, char* ptr, int len)
{
    int type;
    int proto;
    int opt;
    struct linger li_val;
#if defined(HAVE_MULTICAST_SUPPORT)
#if defined(IPPROTO_IP)
#if defined(HAVE_STRUCT_IP_MREQN)
    struct ip_mreqn  mreq4;
#else
    struct ip_mreq   mreq4;
#endif
#endif
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(IPPROTO_IPV6)
    struct ipv6_mreq mreq6;
#endif
    unsigned int mreqSz = 0;
#endif
    int   ival;
    char* arg_ptr;
    int   arg_sz;
#ifdef SO_BINDTODEVICE
    char ifname[IFNAMSIZ];
#endif
    enum PacketParseType old_htype = desc->htype;
    int old_active = desc->active;
    int propagate;
    int res;
#ifdef HAVE_SCTP
    if (IS_SCTP(desc))
	return sctp_set_opts(desc, ptr, len);
#endif
    while(len >= 5) {
        int recv_cmsgflags;
#ifdef __WIN32__
        int bsd_compat_set = 0;
        int bsd_compat_unset = 0;
#endif
	opt = *ptr++;
	ival = get_int32(ptr);
	ptr += 4;
	len -= 5;
	arg_ptr = (char*) &ival;
	arg_sz = sizeof(ival);
	proto = SOL_SOCKET;
	propagate = 0;
        recv_cmsgflags = desc->recv_cmsgflags;
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_set_opts -> opt: %d\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), opt) );
 	switch(opt) {
	case INET_LOPT_HEADER:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(header) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    desc->hsz = ival;
	    continue;
	case INET_LOPT_MODE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(mode) -> %s (%d)\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  M2S(ival), ival) );
	    desc->mode = ival;
	    continue;
	case INET_LOPT_DELIVER:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(deliver) -> %s (%d)\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  D2S(ival), ival) );
	    desc->deliver = ival;
	    continue;
	case INET_LOPT_BUFFER:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(buffer) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (ival < INET_BUFFER_MIN) ival = INET_BUFFER_MIN;
	    desc->bufsz = ival;
            desc->flags |= INET_FLG_BUFFER_SET;
	    continue;
	case INET_LOPT_ACTIVE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(active) -> active (ival): %s (%d)\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  A2S(ival), ival) );
	    desc->active = ival;
            if (desc->active == INET_MULTI) {
                long   ac   = desc->active_count;
                Sint16 nval = get_int16(ptr);
                ptr += 2;
                len -= 2;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(active) -> nval: %d\r\n",
                      __LINE__, desc->s, driver_caller(desc->port), nval) );
                ac += nval;
                if (ac > INT16_MAX || ac < INT16_MIN)
                    return -1;
                desc->active_count += nval;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(active) -> count => %d\r\n",
                      __LINE__, desc->s, driver_caller(desc->port),
                      desc->active_count) );
                if (desc->active_count < 0)
                    desc->active_count = 0;
                if (desc->active_count == 0) {
                    desc->active = INET_PASSIVE;
                    packet_passive_message(desc);
                }
            } else
                desc->active_count = 0;
	    if ((desc->stype == SOCK_STREAM) && (desc->active != INET_PASSIVE) &&
		(desc->state == INET_STATE_CLOSED)) {
		tcp_descriptor *tdesc = (tcp_descriptor *) desc;
		if (tdesc->tcp_add_flags & TCP_ADDF_DELAYED_ECONNRESET) {
		    tdesc->tcp_add_flags &= ~TCP_ADDF_DELAYED_ECONNRESET;
		    tcp_error_message(tdesc, ECONNRESET);
		}
		tcp_closed_message(tdesc);
		if (desc->exitf) {
		    driver_exit(desc->port, 0);
		    return 0;
		} else {
		    desc_close_read(desc);
		}
	    }
	    continue;
	case INET_LOPT_PACKET:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(packet) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    desc->htype = ival;
	    continue;
	case INET_LOPT_PACKET_SIZE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(packet-size) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    desc->psize = (unsigned int)ival;
	    continue;
	case INET_LOPT_EXITONCLOSE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(exit-on-close) -> %d (%s)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), ival, B2S(ival)) );
	    desc->exitf = ival;
	    continue;
	case INET_LOPT_TCP_HIWTRMRK:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(hiwtrmrk) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (desc->stype == SOCK_STREAM) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (ival < 0) ival = 0;
		if (tdesc->low > ival)
		    tdesc->low = ival;
		tdesc->high = ival;
	    }
	    continue;
	case INET_LOPT_TCP_LOWTRMRK:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(lowtrmrk) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (desc->stype == SOCK_STREAM) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (ival < 0) ival = 0;
		if (tdesc->high < ival)
		    tdesc->high = ival;
		tdesc->low = ival;
	    }
	    continue;
	case INET_LOPT_MSGQ_HIWTRMRK: {
	    ErlDrvSizeT high;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(msgq-hiwtrmrk) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (ival < ERL_DRV_BUSY_MSGQ_LIM_MIN
		|| ERL_DRV_BUSY_MSGQ_LIM_MAX < ival)
		return -1;
	    high = (ErlDrvSizeT) ival;
	    erl_drv_busy_msgq_limits(desc->port, NULL, &high);
	    continue;
	}
	case INET_LOPT_MSGQ_LOWTRMRK: {
	    ErlDrvSizeT low;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(msgq-lowtrmrk) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (ival < ERL_DRV_BUSY_MSGQ_LIM_MIN
		|| ERL_DRV_BUSY_MSGQ_LIM_MAX < ival)
		return -1;
	    low = (ErlDrvSizeT) ival;
	    erl_drv_busy_msgq_limits(desc->port, &low, NULL);
	    continue;
	}
	case INET_LOPT_TCP_SEND_TIMEOUT:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(send-timeout) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (desc->stype == SOCK_STREAM) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		tdesc->send_timeout = ival;
	    }
	    continue;
	case INET_LOPT_TCP_SEND_TIMEOUT_CLOSE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(send-timeout-close) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (desc->stype == SOCK_STREAM) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		tdesc->send_timeout_close = ival;
	    }
	    continue;
	case INET_LOPT_TCP_DELAY_SEND:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(delay-send) -> %d (%s)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), ival, B2S(ival)) );
	    if (desc->stype == SOCK_STREAM) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (ival)
		    tdesc->tcp_add_flags |= TCP_ADDF_DELAY_SEND;
		else
		    tdesc->tcp_add_flags &= ~TCP_ADDF_DELAY_SEND;
	    }
	    continue;
#ifdef HAVE_UDP
	case INET_LOPT_UDP_READ_PACKETS:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(read-packets) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (desc->stype == SOCK_DGRAM) {
		udp_descriptor* udesc = (udp_descriptor*) desc;
		if (ival <= 0) return -1;
		udesc->read_packets = ival;
	    }
	    continue;
#endif
#ifdef HAVE_SETNS
	case INET_LOPT_NETNS:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(netns) -> %d, %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival, len) );
	    if (ival < 0) return -1;
	    if (len < ival) return -1;
	    if (desc->netns != NULL) FREE(desc->netns);
	    desc->netns = ALLOC(((unsigned int) ival) + 1);
	    memcpy(desc->netns, ptr, ival);
	    desc->netns[ival] = '\0';
	    ptr += ival;
	    len -= ival;
	    continue;
#endif
	case INET_LOPT_TCP_SHOW_ECONNRESET:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(show-econnreset) -> %d (%s)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), ival, B2S(ival)) );
	    if (desc->sprotocol == IPPROTO_TCP) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (ival)
		    tdesc->tcp_add_flags |= TCP_ADDF_SHOW_ECONNRESET;
		else
		    tdesc->tcp_add_flags &= ~TCP_ADDF_SHOW_ECONNRESET;
	    }
	    continue;
	case INET_LOPT_TCP_READ_AHEAD:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(read_ahead) -> %d (%s)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), ival, B2S(ival)) );
	    if (desc->sprotocol == IPPROTO_TCP) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (! ival)
		    tdesc->tcp_add_flags |= TCP_ADDF_NO_READ_AHEAD;
		else
		    tdesc->tcp_add_flags &= ~TCP_ADDF_NO_READ_AHEAD;
	    }
	    continue;
	case INET_LOPT_LINE_DELIM:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(line-delimiter) -> %c (%d)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), (char) ival, ival) );
	    desc->delimiter = (char)ival;
	    continue;
        case INET_OPT_EXCLUSIVEADDRUSE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(exclusiveaddruse) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
#ifdef __WIN32__
            type = SO_EXCLUSIVEADDRUSE;
#else
            continue;
#endif
            break;
        case INET_OPT_REUSEPORT_LB:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(reuseport_lb) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
#if defined(__WIN32__)
            continue;
#elif defined(__linux__) && defined(SO_REUSEPORT)
            type = SO_REUSEPORT;
            break;
#elif defined(SO_REUSEPORT_LB)
            type = SO_REUSEPORT_LB;
            break;
#else
            continue;
#endif
        case INET_OPT_REUSEPORT:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(reuseport) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
#if defined(__WIN32__)
            ERTS_FALLTHROUGH();
#elif defined(SO_REUSEPORT)
            type = SO_REUSEPORT;
            break;
#else
            continue;
#endif
	case INET_OPT_REUSEADDR:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(reuseaddr) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
#ifdef __WIN32__
            {
                int old_ra, new_ra, compat, ra_bits, opt_bit;
                ra_bits = (1<<INET_OPT_REUSEADDR) | (1<<INET_OPT_REUSEPORT);
                opt_bit = (1<<opt);
                compat = desc->bsd_compat;
                old_ra = (compat & ra_bits) == ra_bits;
                if (ival) {
                    bsd_compat_set = opt_bit;
                    compat |= opt_bit;
                }
                else {
                    bsd_compat_unset = opt_bit;
                    compat &= ~opt_bit;
                }
                new_ra = (compat & ra_bits) == ra_bits;
                desc->bsd_compat = compat;
                if (old_ra == new_ra)
                    continue;
            }
#endif
            type = SO_REUSEADDR;
	    break;
	case INET_OPT_KEEPALIVE: type = SO_KEEPALIVE;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(keepalive) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    break;
	case INET_OPT_DONTROUTE: type = SO_DONTROUTE;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(dontroute) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    break;
	case INET_OPT_BROADCAST: type = SO_BROADCAST;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(broadcast) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    break;
	case INET_OPT_OOBINLINE: type = SO_OOBINLINE;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(oobinline) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    break;
	case INET_OPT_SNDBUF:    type = SO_SNDBUF;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(sndbuf) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    break;
	case INET_OPT_RCVBUF:    type = SO_RCVBUF;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(rcvbuf) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
            if (!(desc->flags & INET_FLG_BUFFER_SET)) {
                if ((ival > INET_UDP_THEORETICAL_MAX_MTU) &&
                    (desc->stype == SOCK_DGRAM) &&
                    !IS_SCTP(desc)) {
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(rcvbuf) -> "
                          "force update of 'buffer' to max-mtu (%d)\r\n",
                          __LINE__, desc->s, driver_caller(desc->port),
                          INET_UDP_THEORETICAL_MAX_MTU) );
                    desc->bufsz = INET_UDP_THEORETICAL_MAX_MTU;
                } else if (ival > desc->bufsz) {
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(rcvbuf) -> "
                          "force update of 'buffer'\r\n",
                          __LINE__, desc->s, driver_caller(desc->port)) );
                    desc->bufsz = ival;
                }
            }
	    break;
	case INET_OPT_LINGER:    type = SO_LINGER;
	    if (len < 4)
		return -1;
	    li_val.l_onoff = ival;
	    li_val.l_linger = get_int32(ptr);
	    ptr += 4;
	    len -= 4;
	    arg_ptr = (char*) &li_val;
	    arg_sz = sizeof(li_val);
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(linger) -> %d, %d\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port),
                  li_val.l_onoff, li_val.l_linger) );
	    if (desc->sprotocol == IPPROTO_TCP) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		if (li_val.l_onoff && li_val.l_linger == 0)
		    tdesc->tcp_add_flags |= TCP_ADDF_LINGER_ZERO;
		else
		    tdesc->tcp_add_flags &= ~TCP_ADDF_LINGER_ZERO;
	    }
	    break;
	case INET_OPT_PRIORITY:
#ifdef SO_PRIORITY
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(prio) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    type = SO_PRIORITY;
	    propagate = 1;
	    break;
#else
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(prio) -> SKIP\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    continue;
#endif
	case INET_OPT_TOS:
#if defined(IP_TOS) && defined(IPPROTO_IP)
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(tos) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IP;
	    type = IP_TOS;
	    propagate = 1;
	    break;
#else
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(tos) -> SKIP\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    continue;
#endif
#if defined(IPV6_TCLASS) && defined(IPPROTO_IPV6)
	case INET_OPT_TCLASS:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(tclass) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IPV6;
	    type = IPV6_TCLASS;
	    propagate = 1;
	    break;
#endif
#if defined(IP_TTL) && defined(IPPROTO_IP)
	case INET_OPT_TTL:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(ttl) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IP;
	    type = IP_TTL;
	    propagate = 1;
	    break;
#endif
#if defined(IP_RECVTOS) && defined(IPPROTO_IP)
	case INET_OPT_RECVTOS:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(recvtos) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IP;
	    type = IP_RECVTOS;
	    propagate = 1;
            recv_cmsgflags =
                ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTOS) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTOS);
	    break;
#endif
#if defined(IPV6_RECVTCLASS) && defined(IPPROTO_IPV6)
	case INET_OPT_RECVTCLASS:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(recvtclass) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IPV6;
	    type = IPV6_RECVTCLASS;
	    propagate = 1;
            recv_cmsgflags =
                ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTCLASS) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTCLASS);
	    break;
#endif
#if defined(IP_RECVTTL) && defined(IPPROTO_IP)
	case INET_OPT_RECVTTL:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(recvttl) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IP;
	    type = IP_RECVTTL;
	    propagate = 1;
            recv_cmsgflags =
                ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTTL) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTTL);
	    break;
#endif
	case TCP_OPT_NODELAY:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(nodelay) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    proto = IPPROTO_TCP;
	    type = TCP_NODELAY;
	    break;
	case TCP_OPT_NOPUSH:
#if defined(INET_TCP_NOPUSH)
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(nopush) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_TCP;
	    type = INET_TCP_NOPUSH;
	    break;
#else
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(nopush) -> SKIP\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    continue;
#endif
#if defined(TCP_KEEPCNT)
	case TCP_OPT_KEEPCNT:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(keepcnt) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPCNT;
	    break;
#endif
#if defined(TCP_KEEPIDLE)
	case TCP_OPT_KEEPIDLE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(keepidle) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPIDLE;
	    break;
#endif
#if defined(TCP_KEEPINTVL)
	case TCP_OPT_KEEPINTVL:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(keepintvl) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPINTVL;
	    break;
#endif
#if defined(TCP_USER_TIMEOUT)
	case TCP_OPT_USER_TIMEOUT:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(user_timeout) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_TCP;
	    type = TCP_USER_TIMEOUT;
	    break;
#endif
#if defined(HAVE_MULTICAST_SUPPORT) && defined(IPPROTO_IP)
	case UDP_OPT_MULTICAST_TTL:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(multicast-ttl) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    proto = IPPROTO_IP;
	    type = IP_MULTICAST_TTL;
	    break;
	case UDP_OPT_MULTICAST_LOOP:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(multicast-loop) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    proto = IPPROTO_IP;
	    type = IP_MULTICAST_LOOP;
	    break;
	case UDP_OPT_MULTICAST_IF:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(multicast-if) -> %d (%d)\r\n",
                  __LINE__,
                  desc->s, driver_caller(desc->port), ival, sock_htonl(ival)) );
            if (desc->sfamily == AF_INET) {
                proto = IPPROTO_IP;
                type  = IP_MULTICAST_IF;
                DEBUGF(("inet_set_opts(%p): s=%d, IP_MULTICAST_IF = %x\r\n",
                        desc->port, desc->s, ival));
                ival = sock_htonl(ival);
            }
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(IPPROTO_IPV6)
            else if (desc->sfamily == AF_INET6) {
                proto = IPPROTO_IPV6;
                type  = IPV6_MULTICAST_IF;
                DEBUGF(("inet_set_opts(%p): s=%d, IPV6_MULTICAST_IF = %x\r\n",
                        desc->port, desc->s, ival));
                ival = sock_htonl(ival);
            } else {
                return -1;
            }
#else
            else {
                return -1;
            }
#endif
	    break;
	case UDP_OPT_ADD_MEMBERSHIP:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(add-membership) -> domain: %d (%s)\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  ival, DOM2S(ival)) );
            if (ival == INET_AF_INET) {
                proto = IPPROTO_IP;
                type  = IP_ADD_MEMBERSHIP;
            }
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(IPPROTO_IPV6)
            else if (ival == INET_AF_INET6) {
                proto = IPPROTO_IPV6;
#if defined(INET_ADD_MEMBERSHIP)
                type  = INET_ADD_MEMBERSHIP;
#else
                return -1;
#endif
            }
#endif
            else {
                return -1;
            }
            goto L_init_mreq;
	case UDP_OPT_DROP_MEMBERSHIP:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(drop-membership) -> %d (%s)\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  ival, DOM2S(ival)) );
            if (ival == INET_AF_INET) {
                proto = IPPROTO_IP;
                type  = IP_DROP_MEMBERSHIP;
            }
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(IPPROTO_IPV6)
            else if (ival == INET_AF_INET6) {
                proto = IPPROTO_IPV6;
#if defined(INET_DROP_MEMBERSHIP)
                type  = INET_DROP_MEMBERSHIP;
#else
                return -1;
#endif
            }
#endif
            else {
                return -1;
            }
        L_init_mreq:
            {
                int domain = ival;
                ival = get_int32(ptr);
                ptr += 4;
                len -= 4;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(L_init_mreq) -> "
                      "\r\n   proto:  %d"
                      "\r\n   type:   %d"
                      "\r\n   domain: %d (%s)"
                      "\r\n   if:     %d"
                      "\r\n",
                      __LINE__, desc->s, driver_caller(desc->port),
                      proto, type, domain, DOM2S(domain), ival));
                if ((domain == INET_AF_INET) && (desc->sfamily == AF_INET)) {
                    mreqSz = sizeof(mreq4);
#if defined(HAVE_STRUCT_IP_MREQN)
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq,inet) -> mreqn\r\n",
                          __LINE__, desc->s, driver_caller(desc->port)));
                    mreq4.imr_ifindex          = ival;
                    ival = get_int32(ptr);
                    ptr += 4;
                    len -= 4;
                    mreq4.imr_multiaddr.s_addr = sock_htonl(ival);
                    ival = get_int32(ptr);
                    ptr += 4;
                    len -= 4;
                    mreq4.imr_address.s_addr   = sock_htonl(ival);
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq,inet) -> "
                          "try setopt: "
                          "\r\n   maddr: %x"
                          "\r\n   addr:  %x"
                          "\r\n   if:    %d"
                          "\r\n   sz:    %d"
                          "\r\n",
                          __LINE__, desc->s, driver_caller(desc->port),
                          mreq4.imr_multiaddr.s_addr,
                          mreq4.imr_address.s_addr,
                          mreq4.imr_ifindex,
                          mreqSz));
#else
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq,inet) -> mreq\r\n",
                          __LINE__, desc->s, driver_caller(desc->port)));
                    ival = get_int32(ptr);
                    ptr += 4;
                    len -= 4;
                    mreq4.imr_multiaddr.s_addr = sock_htonl(ival);
                    ival = get_int32(ptr);
                    ptr += 4;
                    len -= 4;
                    mreq4.imr_interface.s_addr = sock_htonl(ival);
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq,inet) -> "
                          "try setopt: "
                          "\r\n   maddr: %x"
                          "\r\n   if:    %x"
                          "\r\n   sz:    %d"
                          "\r\n",
                          __LINE__, desc->s, driver_caller(desc->port),
                          mreq4.imr_multiaddr.s_addr,
                          mreq4.imr_interface.s_addr,
                          mreqSz));
#endif
                    arg_ptr = (char*)&mreq4;
                    arg_sz  = mreqSz;
                }
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(IPPROTO_IPV6)
                else if ((domain == INET_AF_INET6) &&
                         (desc->sfamily == AF_INET6)) {
                    mreqSz = sizeof(mreq6);
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq,inet6) -> mreq\r\n",
                          __LINE__, desc->s, driver_caller(desc->port)));
                    sys_memcpy(&mreq6.ipv6mr_multiaddr, ptr, 16);
                    ptr += 16;
                    len -= len;
                    mreq6.ipv6mr_interface = ival;
                    arg_ptr = (char*)&mreq6;
                    arg_sz  = mreqSz;
                }
#endif
                else {
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "inet_set_opts(L_init_mreq) -> "
                          "invalid domain: "
                          "\r\n   domain:  %d (%d, %d)"
                          "\r\n   sfmaily: %d (%d, %d)"
                          "\r\n",
                          __LINE__, desc->s, driver_caller(desc->port),
                          domain, INET_AF_INET, INET_AF_INET6,
                          desc->sfamily, AF_INET, AF_INET6));
                    return -1;
                }
            }
	    break;
#endif
	case INET_OPT_IPV6_V6ONLY:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(ipv6-v6only) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
#if HAVE_DECL_IPV6_V6ONLY && defined(IPPROTO_IPV6)
	    proto = IPPROTO_IPV6;
	    type = IPV6_V6ONLY;
	    propagate = 1;
	    break;
#elif defined(__WIN32__) && defined(HAVE_IN6) && defined(AF_INET6)
	    if (ival != 0) continue;
	    else return -1;
	    break;
#else
	    continue;
#endif
	case INET_OPT_RAW:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(raw) -> %d, %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival, len) );
	    if (len < 8) {
		return -1;
	    }
	    proto = ival;
	    type = get_int32(ptr);
	    ptr += 4;
	    arg_sz = get_int32(ptr);
	    ptr += 4;
	    len -= 8;
	    if (len < arg_sz) {
		return -1;
	    }
	    arg_ptr = ptr;
	    ptr += arg_sz;
	    len -= arg_sz;
	    break;
#ifdef SO_BINDTODEVICE
	case INET_OPT_BIND_TO_DEVICE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(bindtodevice) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    if (ival < 0) return -1;
	    if (len < ival) return -1;
	    if (ival > sizeof(ifname)) {
		return -1;
	    }
	    memcpy(ifname, ptr, ival);
	    ifname[ival] = '\0';
	    ptr += ival;
	    len -= ival;
	    proto = SOL_SOCKET;
	    type = SO_BINDTODEVICE;
	    arg_ptr = (char*)&ifname;
	    arg_sz = sizeof(ifname);
	    propagate = 1;
	    break;
#endif
	case INET_LOPT_NON_BLOCK_SEND:
#ifdef HAVE_SCTP
            if (IS_SCTP(desc)) {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(non-block-send) -> %s\r\n",
                      __LINE__, desc->s,
                      driver_caller(desc->port), B2S(ival)) );
                if (ival) {
                    desc->flags |= INET_FLG_NON_BLOCK_SEND;
                } else {
                    desc->flags  = (desc->flags) & ~(INET_FLG_NON_BLOCK_SEND);
                }
            } else {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(non-block-send) -> IGNORE\r\n",
                      __LINE__, desc->s,
                      driver_caller(desc->port)) );
            }
#else
            {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "inet_set_opts(non-block-send) -> IGNORE\r\n",
                      __LINE__, desc->s,
                      driver_caller(desc->port)) );
            }
#endif
            continue;
	case INET_LOPT_DEBUG:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_set_opts(debug) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
            if (ival) {
                desc->debug = TRUE;
            } else {
                desc->debug = FALSE;
            }
            continue;
 	default:
	    return -1;
	}
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_set_opts -> try set opt (%d) %d\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), proto, type) );
        res = inet_setopt(desc->s, proto, type, arg_ptr, arg_sz, propagate);
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_set_opts -> set opt result: %d\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), res) );
        if (res == 0) {
            desc->recv_cmsgflags = recv_cmsgflags;
        }
        else {
#ifdef __WIN32__
            if (bsd_compat_set)
                desc->bsd_compat &= ~bsd_compat_set;
            if (bsd_compat_unset)
                desc->bsd_compat |= bsd_compat_unset;
#endif
            if (propagate)
                return -1;
	}
    }
    if ( ((desc->stype == SOCK_STREAM) && IS_CONNECTED(desc)) ||
	((desc->stype == SOCK_DGRAM) && IS_OPEN(desc))) {
        int trigger_recv;
        trigger_recv =
            (desc->stype==SOCK_STREAM) &&
            !old_active &&
            (desc->active == INET_ONCE || desc->active == INET_MULTI) &&
            (desc->htype == old_htype);
        if (trigger_recv) {
            return 2;
        }
	if (desc->active != old_active) {
            if (desc->active == INET_PASSIVE && desc->stype == SOCK_DGRAM)
                driver_cancel_timer(desc->port);
            sock_select(desc, (FD_READ|FD_CLOSE), (desc->active>0));
        }
	if ((desc->stype==SOCK_STREAM) && desc->active) {
	    if (! old_active) {
		return 1;
	    }
	    if (desc->htype != old_htype) {
                tcp_descriptor *tdesc = (tcp_descriptor *) desc;
                tdesc->i_remain = 0;
		return 1;
	    }
	    return 0;
	}
    }
    return 0;
}
#ifdef HAVE_SCTP
#define SCTP_GET_INITMSG_LEN (4*2)
static char* sctp_get_initmsg(struct sctp_initmsg* ini, char* curr)
{
    ini->sinit_num_ostreams   = get_int16 (curr);	curr += 2;
    ini->sinit_max_instreams  = get_int16 (curr);	curr += 2;
    ini->sinit_max_attempts   = get_int16 (curr);	curr += 2;
    ini->sinit_max_init_timeo = get_int16 (curr);	curr += 2;
    return curr;
}
#define SCTP_GET_SENDPARAMS_LEN (2*2 + 3*4 + ASSOC_ID_LEN)
static char* sctp_get_sendparams (struct sctp_sndrcvinfo* sri, char* curr)
{
    int eflags;
    int cflags;
    sri->sinfo_stream       = get_int16(curr);		curr += 2;
    sri->sinfo_ssn	    = 0;
    eflags		    = get_int16(curr);		curr += 2;
    cflags		    = 0;
    if (eflags & SCTP_FLAG_UNORDERED) cflags |= SCTP_UNORDERED;
    if (eflags & SCTP_FLAG_ADDR_OVER) cflags |= SCTP_ADDR_OVER;
    if (eflags & SCTP_FLAG_ABORT)     cflags |= SCTP_ABORT;
    if (eflags & SCTP_FLAG_EOF)	      cflags |= SCTP_EOF;
    sri->sinfo_flags	    = cflags;
    sri->sinfo_ppid         = sock_htonl(get_int32(curr));
							curr += 4;
    sri->sinfo_context      = get_int32(curr);		curr += 4;
    sri->sinfo_timetolive   = get_int32(curr);		curr += 4;
    sri->sinfo_tsn	    = 0;
    sri->sinfo_cumtsn	    = 0;
    sri->sinfo_assoc_id	    = GET_ASSOC_ID  (curr);	curr += ASSOC_ID_LEN;
    return curr;
}
static int sctp_set_opts(inet_descriptor* desc, char* ptr, int len)
{
#   define CHKLEN(Ptr, Len)                        \
    do {                                           \
	if ((Ptr) + (Len) > ptr + len) return -1; \
    } while (0)
    char * curr = ptr;
    int    proto, type, res;
    union  opts_union
    {
	int			    ival;
	struct sctp_rtoinfo	    rtoi;
	struct sctp_assocparams	    ap;
	struct sctp_initmsg	    im;
	struct linger		    lin;
	struct sctp_setpeerprim	    prim;
	struct sctp_setadaptation   ad;
	struct sctp_paddrparams	    pap;
	struct sctp_sndrcvinfo	    sri;
	struct sctp_event_subscribe es;
#if defined(HAVE_DECL_SCTP_DELAYED_ACK_TIME) && defined(SCTP_ASSOC_VALUE_ASSOC_ID)
	struct sctp_assoc_value     av;
#endif
#	ifdef SO_BINDTODEVICE
	char ifname[IFNAMSIZ];
#	endif
    }
    arg;
    char * arg_ptr = NULL;
    int    arg_sz  = 0;
    int    old_active = desc->active;
    while (curr < ptr + len)
    {
        int recv_cmsgflags;
	int eopt;
#ifdef __WIN32__
        int bsd_compat_set = 0;
        int bsd_compat_unset = 0;
#endif
        eopt = *curr;
	curr++;
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "sctp_set_opts -> eopt: %d\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), eopt) );
        recv_cmsgflags = desc->recv_cmsgflags;
	CHKLEN(curr, 4);
	switch(eopt)
	{
	case INET_LOPT_BUFFER:
            {
                int ival = get_int32(curr);		curr += 4;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "sctp_set_opts(buffer) -> %d\r\n",
                      __LINE__, desc->s, driver_caller(desc->port), ival) );
                desc->bufsz  = ival;
                if (desc->bufsz < INET_BUFFER_MIN)
                    desc->bufsz = INET_BUFFER_MIN;
                desc->flags |= INET_FLG_BUFFER_SET;
                res = 0;
            }
	    continue;
	case INET_LOPT_MODE:
            {
                int ival = get_int32(curr);		curr += 4;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "sctp_set_opts(mode) -> %s (%d)\r\n",
                      __LINE__, desc->s, driver_caller(desc->port),
                      M2S(ival), ival) );
                desc->mode = ival;
                res        = 0;
            }
	    continue;
	case INET_LOPT_ACTIVE:
            {
                int ival = get_int32(curr);		curr += 4;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "sctp_set_opts(active) -> %s (%d)\r\n",
                      __LINE__, desc->s, driver_caller(desc->port),
                      A2S(ival), ival) );
                desc->active = ival;
                if (desc->active == INET_MULTI) {
                    long   ac   = desc->active_count;
                    Sint16 nval = get_int16(curr);          curr += 2;
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "sctp_set_opts(active) -> nval: %d\r\n",
                      __LINE__, desc->s, driver_caller(desc->port), nval) );
                    ac += nval;
                    if (ac > INT16_MAX || ac < INT16_MIN)
                        return -1;
                    desc->active_count += nval;
                    if (desc->active_count < 0)
                        desc->active_count = 0;
                    if (desc->active_count == 0) {
                        desc->active = INET_PASSIVE;
                        packet_passive_message(desc);
                    }
                } else
                    desc->active_count = 0;
                res = 0;
            }
	    continue;
#ifdef HAVE_SETNS
	case INET_LOPT_NETNS:
	{
	    size_t ns_len;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> NETNS\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    ns_len = get_int32(curr);                   curr += 4;
	    CHKLEN(curr, ns_len);
	    if (desc->netns != NULL) FREE(desc->netns);
	    desc->netns = ALLOC(ns_len + 1);
	    memcpy(desc->netns, curr, ns_len);
	    desc->netns[ns_len] = '\0';
	    curr += ns_len;
	}
	    continue;
#endif
	case SCTP_OPT_RTOINFO:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> RTOINFO\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, ASSOC_ID_LEN + 3*4);
	    arg.rtoi.srto_assoc_id = GET_ASSOC_ID(curr);  curr += ASSOC_ID_LEN;
	    arg.rtoi.srto_initial  = get_int32   (curr);  curr += 4;
	    arg.rtoi.srto_max      = get_int32   (curr);  curr += 4;
	    arg.rtoi.srto_min      = get_int32   (curr);  curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_RTOINFO;
	    arg_ptr = (char*) (&arg.rtoi);
	    arg_sz  = sizeof  ( arg.rtoi);
	    break;
	}
	case SCTP_OPT_ASSOCINFO:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> ASSOCINFO\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, ASSOC_ID_LEN + 2*2 + 3*4);
	    arg.ap.sasoc_assoc_id    = GET_ASSOC_ID(curr); curr += ASSOC_ID_LEN;
	    arg.ap.sasoc_asocmaxrxt  = get_int16   (curr); curr += 2;
	    arg.ap.sasoc_number_peer_destinations =
				       get_int16   (curr); curr += 2;
	    arg.ap.sasoc_peer_rwnd   = get_int32   (curr); curr += 4;
	    arg.ap.sasoc_local_rwnd  = get_int32   (curr); curr += 4;
	    arg.ap.sasoc_cookie_life = get_int32   (curr); curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_ASSOCINFO;
	    arg_ptr = (char*) (&arg.ap);
	    arg_sz  = sizeof  ( arg.ap);
	    break;
	}
	case SCTP_OPT_INITMSG:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> INITMSG\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, SCTP_GET_INITMSG_LEN);
	    curr  = sctp_get_initmsg (&arg.im, curr);
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_INITMSG;
	    arg_ptr = (char*) (&arg.im);
	    arg_sz  = sizeof  ( arg.im);
	    break;
	}
	case INET_OPT_LINGER:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> LINGER\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, 2*4);
	    arg.lin.l_onoff  = get_int32 (curr);  curr += 4;
	    arg.lin.l_linger = get_int32 (curr);  curr += 4;
	    proto   = SOL_SOCKET;
	    type    = SO_LINGER;
	    arg_ptr = (char*) (&arg.lin);
	    arg_sz  = sizeof  ( arg.lin);
	    break;
	}
	case SCTP_OPT_NODELAY:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> NODELAY\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    arg.ival= get_int32 (curr);	  curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_NODELAY;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
	}
	case INET_OPT_RCVBUF:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(rcvbuf) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival= ival;
	    proto   = SOL_SOCKET;
	    type    = SO_RCVBUF;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    if (desc->bufsz <= arg.ival)
		desc->bufsz  = arg.ival;
            desc->flags |= INET_FLG_BUFFER_SET;
	    break;
	}
	case INET_OPT_SNDBUF:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(sndbuf) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival= ival;
	    proto   = SOL_SOCKET;
	    type    = SO_SNDBUF;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
	}
        case INET_OPT_EXCLUSIVEADDRUSE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> EXCLUSIVEADDRUSE\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
#ifdef __WIN32__
	    arg.ival= get_int32 (curr);	  curr += 4;
	    proto   = SOL_SOCKET;
	    type    = SO_EXCLUSIVEADDRUSE;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
#else
            curr += 4;
            continue;
#endif
	case INET_OPT_REUSEPORT_LB:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> REUSEPORT_LB\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
#if defined(__WIN32__)
            curr += 4;
            continue;
#elif defined(SO_REUSEPORT_LB) || (defined(__linux__) && defined(SO_REUSEPORT))
	    arg.ival= get_int32 (curr);	  curr += 4;
	    proto   = SOL_SOCKET;
#if defined(__linux__)
	    type    = SO_REUSEPORT;
#else
	    type    = SO_REUSEPORT_LB;
#endif
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
#else
            curr += 4;
            continue;
#endif
	}
	case INET_OPT_REUSEPORT:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> REUSEPORT\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
#if defined(__WIN32__)
            ERTS_FALLTHROUGH();
#elif defined(SO_REUSEPORT)
	    arg.ival= get_int32 (curr);	  curr += 4;
	    proto   = SOL_SOCKET;
	    type    = SO_REUSEPORT;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
#else
            curr += 4;
            continue;
#endif
	}
	case INET_OPT_REUSEADDR:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> REUSEADDR\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
#ifdef __WIN32__
            {
                int old_ra, new_ra, compat, ra_bits, opt_bit;
                ra_bits = (1<<INET_OPT_REUSEADDR) | (1<<INET_OPT_REUSEPORT);
                opt_bit = (1<<opt);
                compat = desc->bsd_compat;
                old_ra = (compat & ra_bits) == ra_bits;
                if (ival) {
                    bsd_compat_set = opt_bit;
                    compat |= opt_bit;
                }
                else {
                    bsd_compat_unset = opt_bit;
                    compat &= ~opt_bit;
                }
                new_ra = (compat & ra_bits) == ra_bits;
                desc->bsd_compat = compat;
                if (old_ra == new_ra) {
                    curr += 4;
                    continue;
                }
            }
#endif
	    arg.ival= get_int32 (curr);	  curr += 4;
	    proto   = SOL_SOCKET;
	    type    = SO_REUSEADDR;
	    arg_ptr = (char*) (&arg.ival);
	    arg_sz  = sizeof  ( arg.ival);
	    break;
	}
	case INET_OPT_DONTROUTE:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(dontroute) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = SOL_SOCKET;
	    type     = SO_DONTROUTE;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
	case INET_OPT_PRIORITY:
#	ifdef SO_PRIORITY
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(prio) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = SOL_SOCKET;
	    type     = SO_PRIORITY;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
#	else
            curr += 4;
            continue;
#	endif
	case INET_OPT_TOS:
#	if defined(IP_TOS) && defined(IPPROTO_IP)
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(tos) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = IPPROTO_IP;
	    type     = IP_TOS;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
#	else
            curr += 4;
            continue;
#	endif
#       if defined(IPV6_TCLASS) && defined(IPPROTO_IPV6)
	case INET_OPT_TCLASS:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(tclass) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = IPPROTO_IPV6;
	    type     = IPV6_TCLASS;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
#	endif
#       if defined(IP_TTL) && defined(IPPROTO_IP)
	case INET_OPT_TTL:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(ttl) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = IPPROTO_IP;
	    type     = IP_TTL;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
#	endif
#	if defined(IP_RECVTOS) && defined(IPPROTO_IP)
	case INET_OPT_RECVTOS:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(recvtos) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_IP;
	    type     = IP_RECVTOS;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
            recv_cmsgflags =
                arg.ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTOS) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTOS);
	    break;
	}
#	endif
#       if defined(IPV6_RECVTCLASS) && defined(IPPROTO_IPV6)
	case INET_OPT_RECVTCLASS:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(recvtclass) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_IPV6;
	    type     = IPV6_RECVTCLASS;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
            recv_cmsgflags =
                arg.ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTCLASS) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTCLASS);
	    break;
	}
#	endif
#	if defined(IP_RECVTTL) && defined(IPPROTO_IP)
	case INET_OPT_RECVTTL:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(recvttl) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_IP;
	    type     = IP_RECVTTL;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
            recv_cmsgflags =
                arg.ival ?
                (desc->recv_cmsgflags | INET_CMSG_RECVTTL) :
                (desc->recv_cmsgflags & ~INET_CMSG_RECVTTL);
	    break;
	}
#	endif
	case INET_OPT_IPV6_V6ONLY:
#       if HAVE_DECL_IPV6_V6ONLY && defined(IPPROTO_IPV6)
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(ipv6_v6only) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_IPV6;
	    type     = IPV6_V6ONLY;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
#       elif defined(__WIN32__) && defined(HAVE_IN6) && defined(AF_INET6)
#           error Here is a fix for Win IPv6 SCTP missing
#       else
	    curr += 4;
            continue;
#       endif
#ifdef SO_BINDTODEVICE
	case INET_OPT_BIND_TO_DEVICE:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> BIND_TO_DEVICE\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    arg_sz = get_int32(curr);			curr += 4;
	    CHKLEN(curr, arg_sz);
	    if (arg_sz >= sizeof(arg.ifname))
		return -1;
	    memcpy(arg.ifname, curr, arg_sz);
	    arg.ifname[arg_sz] = '\0';
	    curr += arg_sz;
	    proto   = SOL_SOCKET;
	    type    = SO_BINDTODEVICE;
	    arg_ptr = (char*) (&arg.ifname);
	    arg_sz  = sizeof  ( arg.ifname);
	    break;
#endif
	case SCTP_OPT_AUTOCLOSE:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(autoclose) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = IPPROTO_SCTP;
	    type     = SCTP_AUTOCLOSE;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
	case SCTP_OPT_DISABLE_FRAGMENTS:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(disable_fragments) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_SCTP;
	    type     = SCTP_DISABLE_FRAGMENTS;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
	case SCTP_OPT_I_WANT_MAPPED_V4_ADDR:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(i_want_mapped_v4_addr) -> %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), B2S(ival)) );
	    arg.ival = ival;
	    proto    = IPPROTO_SCTP;
	    type     = SCTP_I_WANT_MAPPED_V4_ADDR;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
	case SCTP_OPT_MAXSEG:
	{
            int ival = get_int32 (curr);	  curr += 4;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts(maxseg) -> %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), ival) );
	    arg.ival = ival;
	    proto    = IPPROTO_SCTP;
	    type     = SCTP_MAXSEG;
	    arg_ptr  = (char*) (&arg.ival);
	    arg_sz   = sizeof  ( arg.ival);
	    break;
	}
	case SCTP_OPT_PRIMARY_ADDR:
	case SCTP_OPT_SET_PEER_PRIMARY_ADDR:
	{
	    ErlDrvSizeT alen;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> PRIMARY_ADDR | SET_PEER_PRIMARY_ADDR\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, ASSOC_ID_LEN);
	    arg.prim.sspp_assoc_id = GET_ASSOC_ID(curr); curr += ASSOC_ID_LEN;
	    alen  = ptr + len - curr;
	    if (inet_set_faddress
		(desc->sfamily, (inet_address*) (&arg.prim.sspp_addr),
		 &curr,  &alen) != NULL) return -1;
	    proto = IPPROTO_SCTP;
	    if (eopt == SCTP_OPT_PRIMARY_ADDR)
		type =  SCTP_PRIMARY_ADDR;
	    else
		type =  SCTP_SET_PEER_PRIMARY_ADDR;
	    arg_ptr  =  (char*) (&arg.prim);
	    arg_sz   =  sizeof  ( arg.prim);
	    break;
	}
	case SCTP_OPT_ADAPTATION_LAYER:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> ADAPTATION_LAYER\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    arg.ad.ssb_adaptation_ind = sock_htonl (get_int32(curr));  curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_ADAPTATION_LAYER;
	    arg_ptr = (char*) (&arg.ad);
	    arg_sz  = sizeof  ( arg.ad);
	    break;
	}
	case SCTP_OPT_PEER_ADDR_PARAMS:
	{
	    ErlDrvSizeT alen;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_FLAGS
	    int eflags, cflags, hb_enable, hb_disable,
		pmtud_enable, pmtud_disable;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_SACKDELAY
	    int
		sackdelay_enable, sackdelay_disable;
#           endif
#           endif
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> PEER_ADDR_PARAMS\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, ASSOC_ID_LEN);
	    arg.pap.spp_assoc_id = GET_ASSOC_ID(curr);	curr += ASSOC_ID_LEN;
	    alen  = ptr + len - curr;
	    if (inet_set_faddress
		(desc->sfamily, (inet_address*) (&arg.pap.spp_address),
		 &curr,  &alen) != NULL) return -1;
	    CHKLEN(curr, 4 + 2 + 3*4);
	    arg.pap.spp_hbinterval = get_int32(curr);	curr += 4;
	    arg.pap.spp_pathmaxrxt = get_int16(curr);	curr += 2;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_PATHMTU
	    arg.pap.spp_pathmtu    = get_int32(curr);
#           endif
	    curr += 4;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_SACKDELAY
	    arg.pap.spp_sackdelay  = get_int32(curr);
#           endif
	    curr += 4;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_FLAGS
	    eflags	       = get_int32(curr);
	    cflags	       = 0;
	    hb_enable      = eflags & SCTP_FLAG_HB_ENABLE;
	    hb_disable     = eflags & SCTP_FLAG_HB_DISABLE;
	    if (hb_enable && hb_disable)
		return -1;
	    if (hb_enable)	 		cflags |= SPP_HB_ENABLE;
	    if (hb_disable)	 		cflags |= SPP_HB_DISABLE;
	    if (eflags & SCTP_FLAG_HB_DEMAND)	cflags |= SPP_HB_DEMAND;
	    pmtud_enable   = eflags & SCTP_FLAG_PMTUD_ENABLE;
	    pmtud_disable  = eflags & SCTP_FLAG_PMTUD_DISABLE;
	    if (pmtud_enable && pmtud_disable)
		return -1;
	    if (pmtud_enable)			cflags |= SPP_PMTUD_ENABLE;
	    if (pmtud_disable)			cflags |= SPP_PMTUD_DISABLE;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_SACKDELAY
	    sackdelay_enable =eflags& SCTP_FLAG_SACDELAY_ENABLE;
	    sackdelay_disable=eflags& SCTP_FLAG_SACDELAY_DISABLE;
	    if (sackdelay_enable && sackdelay_disable)
		return -1;
	    if (sackdelay_enable)		cflags |= SPP_SACKDELAY_ENABLE;
	    if (sackdelay_disable)		cflags |= SPP_SACKDELAY_DISABLE;
#           endif
	    arg.pap.spp_flags  = cflags;
#	    endif
	    curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_PEER_ADDR_PARAMS;
	    arg_ptr = (char*) (&arg.pap);
	    arg_sz  = sizeof  ( arg.pap);
	    break;
	}
	case SCTP_OPT_DEFAULT_SEND_PARAM:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> DEFAULT_SEND_PARAM\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, SCTP_GET_SENDPARAMS_LEN);
	    curr = sctp_get_sendparams (&arg.sri, curr);
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_DEFAULT_SEND_PARAM;
	    arg_ptr = (char*) (&arg.sri);
	    arg_sz  = sizeof  ( arg.sri);
	    VALGRIND_MAKE_MEM_DEFINED(arg_ptr, arg_sz);
	    break;
	}
	case SCTP_OPT_EVENTS:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> EVENTS\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, 9);
	    memset (&arg.es, 0, sizeof(arg.es));
	    arg.es.sctp_data_io_event          = get_int8(curr);   curr++;
	    arg.es.sctp_association_event      = get_int8(curr);   curr++;
	    arg.es.sctp_address_event	       = get_int8(curr);   curr++;
	    arg.es.sctp_send_failure_event     = get_int8(curr);   curr++;
	    arg.es.sctp_peer_error_event       = get_int8(curr);   curr++;
	    arg.es.sctp_shutdown_event	       = get_int8(curr);   curr++;
	    arg.es.sctp_partial_delivery_event = get_int8(curr);   curr++;
	    arg.es.sctp_adaptation_layer_event = get_int8(curr);   curr++;
	     curr++;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_EVENTS;
	    arg_ptr = (char*) (&arg.es);
#if defined(__linux__)
            arg_sz  = offsetof(struct sctp_event_subscribe,
                               sctp_adaptation_layer_event) + 1;
#else
	    arg_sz  = sizeof  ( arg.es);
#endif
	    break;
	}
#if defined(HAVE_DECL_SCTP_DELAYED_ACK_TIME) && defined(SCTP_ASSOC_VALUE_ASSOC_ID)
	case SCTP_OPT_DELAYED_ACK_TIME:
	{
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_set_opts -> DELAYED_ACK_TIME\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    CHKLEN(curr, ASSOC_ID_LEN + 4);
	    arg.av.assoc_id    = GET_ASSOC_ID(curr);	curr += ASSOC_ID_LEN;
	    arg.av.assoc_value = get_int32(curr);	curr += 4;
	    proto   = IPPROTO_SCTP;
	    type    = SCTP_DELAYED_ACK_TIME;
	    arg_ptr = (char*) (&arg.av);
	    arg_sz  = sizeof  ( arg.av);
	    break;
	}
#endif
	case INET_LOPT_NON_BLOCK_SEND:
            if (IS_SCTP(desc))
                {
                    int ival = get_int32(curr); curr += 4;
                    DDBG(desc,
                         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                          "sctp_set_opts(non-block-send) -> %s\r\n",
                          __LINE__, desc->s, driver_caller(desc->port),
                          B2S(ival)) );
                    if (ival) {
                        desc->flags |= INET_FLG_NON_BLOCK_SEND;
                    } else {
                        desc->flags  = (desc->flags) & ~(INET_FLG_NON_BLOCK_SEND);
                    }
                    res = 0;
                }
            else {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "sctp_set_opts(non-block-send) -> IGNORE\r\n",
                      __LINE__, desc->s,
                      driver_caller(desc->port)) );
            }
            continue;
	case INET_LOPT_DEBUG:
            {
                int ival = get_int32(curr); curr += 4;
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "sctp_set_opts(debug) -> %s\r\n",
                      __LINE__, desc->s, driver_caller(desc->port),
                      B2S(ival)) );
                if (ival) {
                    desc->debug = TRUE;
                } else {
                    desc->debug = FALSE;
                }
                res = 0;
            }
            continue;
	default:
	    return -1;
	}
        res = inet_setopt(desc->s, proto, type, arg_ptr, arg_sz, 1);
	ASSERT(res == 0 || res == -1);
        if (res == 0) {
            desc->recv_cmsgflags = recv_cmsgflags;
        }
#ifdef __WIN32__
        else {
            if (bsd_compat_set)
                desc->bsd_compat &= ~bsd_compat_set;
            if (bsd_compat_unset)
                desc->bsd_compat |= bsd_compat_unset;
        }
#endif
	if (res == -1)
	{
#	   ifdef SOLARIS10
	   if (type != SO_SNDBUF && type != SO_RCVBUF)
#	   endif
	   return res;
	}
    }
    if (IS_OPEN(desc) && desc->active != old_active) {
	sock_select(desc, (FD_READ|FD_CLOSE), (desc->active > 0));
    }
    return 0;
#   undef CHKLEN
}
#endif
#ifndef __WIN32__
static void put_cmsg_int32(struct cmsghdr *cmsg, char *ptr) {
    union u {
        byte uint8;
        Uint16 uint16;
        Uint32 uint32;
        Uint64 uint64;
    } *p;
    p = (union u*) CMSG_DATA(cmsg);
    switch (LEN_CMSG_DATA(cmsg) * CHAR_BIT) {
    case 8:
        put_int32((Uint32) p->uint8, ptr);
        break;
    case 16:
        put_int32((Uint32) p->uint16, ptr);
        break;
    case 32:
        put_int32(p->uint32, ptr);
        break;
    case 64:
        put_int32((Uint32) p->uint64, ptr);
        break;
    default:
        put_int32(0, ptr);
    }
    return;
}
#endif
static ErlDrvSSizeT inet_fill_opts(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT len,
				   char** dest, ErlDrvSizeT destlen)
{
    int type;
    int proto;
    int opt;
    struct linger li_val;
    int ival;
    char* arg_ptr;
    unsigned int arg_sz;
    char *ptr = NULL;
    ErlDrvSizeT dest_used = 0;
    ErlDrvSizeT dest_allocated = destlen;
    char *orig_dest = *dest;
#ifdef SO_BINDTODEVICE
    char ifname[IFNAMSIZ];
#endif
#define RETURN_ERROR()				\
    do {					\
	if (dest_allocated > destlen) {		\
	    FREE(*dest);			\
	    *dest = orig_dest;			\
	}					\
	return -1;				\
    } while(0)
#define PLACE_FOR(Size,Ptr)						   \
    do {								   \
	ErlDrvSizeT need = dest_used + (Size);					   \
	if (need > INET_MAX_OPT_BUFFER) {				   \
	    RETURN_ERROR();						   \
	}								   \
	if (need > dest_allocated) {					   \
	    char *new_buffer;						   \
	    if (dest_allocated == destlen) {				   \
		new_buffer = ALLOC((dest_allocated = need + 10));	   \
		memcpy(new_buffer,*dest,dest_used);			   \
	    } else {							   \
		new_buffer = REALLOC(*dest, (dest_allocated = need + 10)); \
	    }								   \
	    *dest = new_buffer;						   \
	}								   \
	(Ptr) = (*dest) + dest_used;					   \
	dest_used = need;						   \
    } while (0)
#define TRUNCATE_TO(Size,Ptr)				\
    do {						\
	ErlDrvSizeT new_need = ((Ptr) - (*dest)) + (Size);	\
	if (new_need > dest_used) {			\
	    erts_exit(ERTS_ERROR_EXIT,"Internal error in inet_drv, "	\
		     "miscalculated buffer size");	\
	}						\
	dest_used = new_need;				\
    } while(0)
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "inet_fill_opts -> entry\r\n",
          __LINE__, desc->s, driver_caller(desc->port)) );
    PLACE_FOR(1,ptr);
    *ptr = INET_REP_OK;
    while(len--) {
	opt = *buf++;
	proto = SOL_SOCKET;
	ival = 0;
	arg_sz = sizeof(ival);
	arg_ptr = (char*) &ival;
	PLACE_FOR(5,ptr);
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_fill_opts -> opt %d\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), opt) );
	switch(opt) {
	case INET_LOPT_BUFFER:
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_fill_opts -> buffer: %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), desc->bufsz) );
	    *ptr++ = opt;
	    put_int32(desc->bufsz, ptr);
	    continue;
	case INET_LOPT_HEADER:
	    *ptr++ = opt;
	    put_int32(desc->hsz, ptr);
	    continue;
	case INET_LOPT_MODE:
	    *ptr++ = opt;
	    put_int32(desc->mode, ptr);
	    continue;
	case INET_LOPT_DELIVER:
	    *ptr++ = opt;
	    put_int32(desc->deliver, ptr);
	    continue;
	case INET_LOPT_ACTIVE:
	    *ptr++ = opt;
	    put_int32(desc->active, ptr);
            if (desc->active == INET_MULTI) {
                PLACE_FOR(2,ptr);
                put_int16(desc->active_count, ptr);
                ptr += 2;
            }
	    continue;
	case INET_LOPT_PACKET:
	    *ptr++ = opt;
	    put_int32(desc->htype, ptr);
	    continue;
	case INET_LOPT_PACKET_SIZE:
	    *ptr++ = opt;
	    put_int32(desc->psize, ptr);
	    continue;
	case INET_LOPT_EXITONCLOSE:
	    *ptr++ = opt;
	    put_int32(desc->exitf, ptr);
	    continue;
	case INET_LOPT_TCP_HIWTRMRK:
	    if (desc->stype == SOCK_STREAM) {
		*ptr++ = opt;
		ival = ((tcp_descriptor*)desc)->high;
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_LOPT_TCP_LOWTRMRK:
	    if (desc->stype == SOCK_STREAM) {
		*ptr++ = opt;
		ival = ((tcp_descriptor*)desc)->low;
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_LOPT_MSGQ_HIWTRMRK: {
	    ErlDrvSizeT high = ERL_DRV_BUSY_MSGQ_READ_ONLY;
	    *ptr++ = opt;
	    erl_drv_busy_msgq_limits(desc->port, NULL, &high);
	    ival = high > INT_MAX ? INT_MAX : (int) high;
	    put_int32(ival, ptr);
	    continue;
	}
	case INET_LOPT_MSGQ_LOWTRMRK: {
	    ErlDrvSizeT low = ERL_DRV_BUSY_MSGQ_READ_ONLY;
	    *ptr++ = opt;
	    erl_drv_busy_msgq_limits(desc->port, &low, NULL);
	    ival = low > INT_MAX ? INT_MAX : (int) low;
	    put_int32(ival, ptr);
	    continue;
	}
	case INET_LOPT_TCP_SEND_TIMEOUT:
	    if (desc->stype == SOCK_STREAM) {
		*ptr++ = opt;
		ival = ((tcp_descriptor*)desc)->send_timeout;
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_LOPT_TCP_SEND_TIMEOUT_CLOSE:
	    if (desc->stype == SOCK_STREAM) {
		*ptr++ = opt;
		ival = ((tcp_descriptor*)desc)->send_timeout_close;
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_LOPT_TCP_DELAY_SEND:
	    if (desc->stype == SOCK_STREAM) {
		*ptr++ = opt;
		ival = !!(((tcp_descriptor*)desc)->tcp_add_flags & TCP_ADDF_DELAY_SEND);
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
#ifdef HAVE_UDP
	case INET_LOPT_UDP_READ_PACKETS:
	    if (desc->stype == SOCK_DGRAM) {
		*ptr++ = opt;
		ival = ((udp_descriptor*)desc)->read_packets;
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
#endif
#ifdef HAVE_SETNS
	case INET_LOPT_NETNS:
	    if (desc->netns != NULL) {
		size_t netns_len;
		netns_len = strlen(desc->netns);
		*ptr++ = opt;
		put_int32(netns_len, ptr);
		PLACE_FOR(netns_len, ptr);
		memcpy(ptr, desc->netns, netns_len);
		ptr += netns_len;
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
#endif
	case INET_LOPT_TCP_SHOW_ECONNRESET:
	    if (desc->sprotocol == IPPROTO_TCP) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		*ptr++ = opt;
		ival = !!(tdesc->tcp_add_flags & TCP_ADDF_SHOW_ECONNRESET);
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_LOPT_TCP_READ_AHEAD:
	    if (desc->sprotocol == IPPROTO_TCP) {
		tcp_descriptor* tdesc = (tcp_descriptor*) desc;
		*ptr++ = opt;
		ival = !(tdesc->tcp_add_flags & TCP_ADDF_NO_READ_AHEAD);
		put_int32(ival, ptr);
	    } else {
		TRUNCATE_TO(0,ptr);
	    }
	    continue;
	case INET_OPT_PRIORITY:
#ifdef SO_PRIORITY
	    type = SO_PRIORITY;
	    break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
	case INET_OPT_TOS:
#if defined(IP_TOS) && defined(IPPROTO_IP)
	    proto = IPPROTO_IP;
	    type = IP_TOS;
	    break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
	case INET_OPT_TCLASS:
#if defined(IPV6_TCLASS) && defined(IPPROTO_IPV6)
	    proto = IPPROTO_IPV6;
	    type = IPV6_TCLASS;
	    break;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_TTL:
#if defined(IP_TTL) && defined(IPPROTO_IP)
	    proto = IPPROTO_IP;
	    type = IP_TTL;
	    break;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_RECVTOS:
#if defined(IP_RECVTOS) && defined(IPPROTO_IP)
	    proto = IPPROTO_IP;
	    type = IP_RECVTOS;
	    break;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_RECVTCLASS:
#if defined(IPV6_RECVTCLASS) && defined(IPPROTO_IPV6)
	    proto = IPPROTO_IPV6;
	    type = IPV6_RECVTCLASS;
	    break;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_RECVTTL:
#if defined(IP_RECVTTL) && defined(IPPROTO_IP)
	    proto = IPPROTO_IP;
	    type = IP_RECVTTL;
	    break;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_REUSEADDR: {
#if defined(__WIN32__)
            int res = !!(desc->bsd_compat & (1<<INET_OPT_REUSEADDR));
	    *ptr++ = opt;
	    put_int32(res, ptr);
            continue;
#else
	    type = SO_REUSEADDR;
#endif
	    break;
        }
	case INET_OPT_REUSEPORT_LB: {
#if defined(__linux__) && defined(SO_REUSEPORT)
	    type = SO_REUSEPORT;
	    break;
#elif defined(SO_REUSEPORT_LB)
	    type = SO_REUSEPORT_LB;
	    break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
        }
	case INET_OPT_REUSEPORT: {
#if defined(__WIN32__)
            int res = !!(desc->bsd_compat & (1<<INET_OPT_REUSEPORT));
	    *ptr++ = opt;
	    put_int32(res, ptr);
            continue;
#elif defined(SO_REUSEPORT)
	    type = SO_REUSEPORT;
	    break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
        }
        case INET_OPT_EXCLUSIVEADDRUSE:
#ifdef __WIN32__
            type = SO_EXCLUSIVEADDRUSE;
            break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
	case INET_OPT_KEEPALIVE:
	    type = SO_KEEPALIVE;
	    break;
	case INET_OPT_DONTROUTE:
	    type = SO_DONTROUTE;
	    break;
	case INET_OPT_BROADCAST:
	    type = SO_BROADCAST;
	    break;
	case INET_OPT_OOBINLINE:
	    type = SO_OOBINLINE;
	    break;
	case INET_OPT_SNDBUF:
	    type = SO_SNDBUF;
	    break;
	case INET_OPT_RCVBUF:
	    type = SO_RCVBUF;
	    break;
	case TCP_OPT_NODELAY:
	    proto = IPPROTO_TCP;
	    type = TCP_NODELAY;
	    break;
	case TCP_OPT_NOPUSH:
#if defined(INET_TCP_NOPUSH)
	    proto = IPPROTO_TCP;
	    type = INET_TCP_NOPUSH;
	    break;
#else
	    *ptr++ = opt;
	    put_int32(0, ptr);
	    continue;
#endif
#if defined(TCP_KEEPCNT)
	case TCP_OPT_KEEPCNT:
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPCNT;
	    break;
#endif
#if defined(TCP_KEEPIDLE)
	case TCP_OPT_KEEPIDLE:
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPIDLE;
	    break;
#endif
#if defined(TCP_KEEPINTVL)
	case TCP_OPT_KEEPINTVL:
	    proto = IPPROTO_TCP;
	    type = TCP_KEEPINTVL;
	    break;
#endif
#if defined(TCP_USER_TIMEOUT)
	case TCP_OPT_USER_TIMEOUT:
	    proto = IPPROTO_TCP;
	    type = TCP_USER_TIMEOUT;
	    break;
#endif
#if defined(HAVE_MULTICAST_SUPPORT) && defined(IPPROTO_IP)
	case UDP_OPT_MULTICAST_TTL:
	    proto = IPPROTO_IP;
	    type = IP_MULTICAST_TTL;
	    break;
	case UDP_OPT_MULTICAST_LOOP:
	    proto = IPPROTO_IP;
	    type = IP_MULTICAST_LOOP;
	    break;
	case UDP_OPT_MULTICAST_IF:
            {
                int            mif   = 0;
                unsigned int   mifSz = sizeof(mif);
                *ptr++ = opt;
                PLACE_FOR(4,ptr);
                if (desc->sfamily == AF_INET) {
                    put_int32(INET_AF_INET, ptr);
                    ptr  += 4;
                    proto = IPPROTO_IP;
                    type  = IP_MULTICAST_IF;
                } else if (desc->sfamily == AF_INET6) {
                    put_int32(INET_AF_INET6, ptr);
                    ptr  += 4;
                    proto = IPPROTO_IPV6;
                    type  = IPV6_MULTICAST_IF;
                } else {
                    RETURN_ERROR();
                }
                if (IS_SOCKET_ERROR(sock_getopt(desc->s,
                                                proto, type,
                                                (void *) &mif,
                                                (void *) &mifSz))) {
                    TRUNCATE_TO(0,ptr);
                    continue;
                }
                put_int32(mif, ptr);
            }
	    continue;
	case INET_OPT_LINGER:
	    arg_sz = sizeof(li_val);
	    sys_memzero((void *) &li_val, sizeof(li_val));
	    arg_ptr = (char*) &li_val;
	    type = SO_LINGER;
	    break;
#endif
	case INET_OPT_IPV6_V6ONLY:
#if HAVE_DECL_IPV6_V6ONLY && defined(IPPROTO_IPV6)
	    proto = IPPROTO_IPV6;
	    type = IPV6_V6ONLY;
	    break;
#elif defined(__WIN32__) && defined(HAVE_IN6) && defined(AF_INET6)
	    *ptr++ = opt;
	    put_int32(1, ptr);
	    ptr += 4;
	    continue;
#else
	    TRUNCATE_TO(0,ptr);
	    continue;
#endif
	case INET_OPT_RAW:
	    {
		int data_provided;
		if (len < 13) {
		    RETURN_ERROR();
		}
		len -= 13;
		proto = get_int32(buf);
		buf += 4;
		type = get_int32(buf);
		buf += 4;
		data_provided = (int) *buf++;
		arg_sz = get_int32(buf);
		if (arg_sz > INET_MAX_OPT_BUFFER) {
		    RETURN_ERROR();
		}
		buf += 4;
		TRUNCATE_TO(0,ptr);
		PLACE_FOR(13 + arg_sz,ptr);
		arg_ptr = ptr + 13;
		if (data_provided) {
		    if (len < arg_sz) {
			RETURN_ERROR();
		    }
		    memcpy(arg_ptr,buf,arg_sz);
		    buf += arg_sz;
		    len -= arg_sz;
		}
		if (IS_SOCKET_ERROR(sock_getopt(desc->s,proto,type,
						arg_ptr,&arg_sz))) {
		    TRUNCATE_TO(0,ptr);
		    continue;
		}
		TRUNCATE_TO(arg_sz + 13,ptr);
		*ptr++ = opt;
		put_int32(proto,ptr);
		ptr += 4;
		put_int32(type,ptr);
		ptr += 4;
		put_int32(arg_sz,ptr);
		continue;
	    }
#ifdef SO_BINDTODEVICE
	case INET_OPT_BIND_TO_DEVICE:
	    arg_sz = sizeof(ifname);
	    TRUNCATE_TO(0,ptr);
	    PLACE_FOR(5 + arg_sz,ptr);
	    arg_ptr = ptr + 5;
	    if (IS_SOCKET_ERROR(sock_getopt(desc->s,SOL_SOCKET,SO_BINDTODEVICE,
						arg_ptr,&arg_sz))) {
		    TRUNCATE_TO(0,ptr);
		    continue;
		}
	    arg_sz = my_strnlen(arg_ptr, arg_sz);
	    TRUNCATE_TO(arg_sz + 5,ptr);
	    *ptr++ = opt;
	    put_int32(arg_sz,ptr);
	    ptr += arg_sz;
	    continue;
#endif
#ifndef __WIN32__
        case INET_OPT_PKTOPTIONS: {
            struct cmsghdr *cmsg, *cmsg_top;
            SOCKLEN_T cmsg_sz;
            union {
                struct cmsghdr hdr;
                char buf[2*CMSG_SPACE(sizeof(int))];
            } cmsgbuf;
            switch (desc->sfamily) {
#if defined(IPPROTO_IP) && defined(IP_PKTOPTIONS)
            case AF_INET: {
                proto = IPPROTO_IP;
                type = IP_PKTOPTIONS;
            }
                break;
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_PKTOPTIONS) && defined(AF_INET6)
            case AF_INET6: {
                proto = IPPROTO_IPV6;
                type = IPV6_PKTOPTIONS;
            }
                break;
#endif
            default: {
                RETURN_ERROR();
            }
            }
            TRUNCATE_TO(0, ptr);
            cmsg_sz = sizeof(cmsgbuf.buf);
            if (IS_SOCKET_ERROR(sock_getopt(desc->s, proto, type,
                                            cmsgbuf.buf, &cmsg_sz))) {
                continue;
            }
            PLACE_FOR(1+4, ptr);
            *ptr++ = opt;
            arg_ptr = ptr;
            arg_sz = 0;
            for (cmsg_top = (struct cmsghdr*)(cmsgbuf.buf + cmsg_sz),
                     cmsg = (struct cmsghdr*)cmsgbuf.buf;
                 cmsg < cmsg_top;
                 cmsg = NXT_CMSG_HDR(cmsg)) {
#define PUT_CMSG_INT32(CMSG_LEVEL, CMSG_TYPE, OPT)      \
                if ((cmsg->cmsg_level == CMSG_LEVEL) && \
                    (cmsg->cmsg_type == CMSG_TYPE)) {   \
                    PLACE_FOR(1+4, ptr);                \
                    *ptr++ = OPT;                       \
                    put_cmsg_int32(cmsg, ptr);          \
                    arg_sz += 1+4;                      \
                    continue;                           \
                }
#if defined(IPPROTO_IP) && defined(IP_TOS)
                PUT_CMSG_INT32(IPPROTO_IP, IP_TOS, INET_OPT_TOS);
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_TCLASS)
                PUT_CMSG_INT32(IPPROTO_IPV6, IPV6_TCLASS, INET_OPT_TCLASS);
#endif
#if defined(IPPROTO_IP) && defined(IP_TTL)
                PUT_CMSG_INT32(IPPROTO_IP, IP_TTL, INET_OPT_TTL);
#endif
#if defined(IPPROTO_IP) && defined(IP_RECVTOS)
                PUT_CMSG_INT32(IPPROTO_IP, IP_RECVTOS, INET_OPT_TOS);
#endif
#if defined(IPPROTO_IPV6) && defined(IPV6_RECVTCLASS)
                PUT_CMSG_INT32(IPPROTO_IPV6, IPV6_RECVTCLASS, INET_OPT_TCLASS);
#endif
#if defined(IPPROTO_IP) && defined(IP_RECVTTL)
                PUT_CMSG_INT32(IPPROTO_IP, IP_RECVTTL, INET_OPT_TTL);
#endif
#undef PUT_CMSG_INT32
            }
            put_int32(arg_sz, arg_ptr);
            continue;
        }
#endif
	case INET_LOPT_NON_BLOCK_SEND:
            *ptr++ = opt;
            if (IS_NON_BLOCK_SEND(desc))
                ival = TRUE;
            else
                ival = FALSE;
            put_int32(ival, ptr);
            continue;
	case INET_LOPT_DEBUG:
            *ptr++ = opt;
            ival = desc->debug;
            put_int32(ival, ptr);
            continue;
	default:
	    RETURN_ERROR();
	}
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_fill_opts -> try get opt (proto %d, type %d)\r\n",
	      __LINE__, desc->s, driver_caller(desc->port), proto, type) );
	if (IS_SOCKET_ERROR(sock_getopt(desc->s,proto,type,arg_ptr,&arg_sz))) {
            int save_errno = sock_errno();
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "inet_fill_opts -> failed get option (%d) %d: "
                  "\r\n   errno: %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  proto, type, save_errno) );
	    TRUNCATE_TO(0,ptr);
	    continue;
	}
	*ptr++ = opt;
	if (arg_ptr == (char*)&ival) {
	    put_int32(ival, ptr);
	}
	else {
	    put_int32(((Uint32) li_val.l_onoff), ptr);
	    PLACE_FOR(4,ptr);
	    put_int32(((Uint32) li_val.l_linger), ptr);
	}
    }
    return (dest_used);
#undef PLACE_FOR
#undef TRUNCATE_TO
#undef RETURN_ERROR
}
#ifdef HAVE_SCTP
#define LOAD_PADDRINFO_CNT                                            \
        (2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT + LOAD_INET_GET_ADDRESS_CNT + \
	 4*LOAD_INT_CNT + LOAD_TUPLE_CNT)
static int load_paddrinfo (ErlDrvTermData * spec, int i,
			   inet_descriptor* desc, struct sctp_paddrinfo* pai)
{
    i = LOAD_ATOM	(spec, i, am_sctp_paddrinfo);
    i = LOAD_ASSOC_ID	(spec, i, pai->spinfo_assoc_id);
    PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
    i = load_inet_get_address(spec, i, desc, (struct sockaddr_storage *)&pai->spinfo_address);
    POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
    switch(pai->spinfo_state)
    {
    case SCTP_ACTIVE:
	i = LOAD_ATOM	(spec, i, am_active);
	break;
    case SCTP_INACTIVE:
	i = LOAD_ATOM	(spec, i, am_inactive);
	break;
#   if HAVE_DECL_SCTP_UNCONFIRMED
    case SCTP_UNCONFIRMED:
      i = LOAD_ATOM	(spec, i, am_unconfirmed);
      break;
#   endif
    default:
      i = LOAD_ATOM	(spec, i, am_undefined);
    }
    i = LOAD_INT	(spec, i, pai->spinfo_cwnd);
    i = LOAD_INT	(spec, i, pai->spinfo_srtt);
    i = LOAD_INT	(spec, i, pai->spinfo_rto );
    i = LOAD_INT	(spec, i, pai->spinfo_mtu );
    i = LOAD_TUPLE	(spec, i, 8);
    return i;
}
static ErlDrvSSizeT sctp_fill_opts(inet_descriptor* desc,
				   char* buf, ErlDrvSizeT buflen,
				   char** dest, ErlDrvSizeT destlen)
{
    ErlDrvTermData *spec;
    int i      = 0;
    int length = 0;
    int spec_allocated = PACKET_ERL_DRV_TERM_DATA_LEN;
    spec = ALLOC(sizeof(* spec) * spec_allocated);
#   define RETURN_ERROR(Spec, Errno) \
    do {                    \
	FREE(Spec);        \
	return (Errno);     \
    } while(0)
#   define PLACE_FOR(Spec, Index, N)                            \
    do {                                                        \
	int need;                                               \
	if ((Index) > spec_allocated) {                         \
	    erts_exit(ERTS_ERROR_EXIT,"Internal error in inet_drv, "           \
		     "miscalculated buffer size");              \
	}                                                       \
	need = (Index) + (N);                                   \
	if (need > INET_MAX_OPT_BUFFER/sizeof(ErlDrvTermData)) {\
	    RETURN_ERROR((Spec), -ENOMEM);                      \
	}                                                       \
	if (need > spec_allocated) {                            \
	    (Spec) = REALLOC((Spec),                            \
			     sizeof(* (Spec))                   \
			     * (spec_allocated = need + 20));   \
	}                                                       \
    } while (0)
    PLACE_FOR(spec, i, 2*LOAD_ATOM_CNT + LOAD_PORT_CNT);
    i = LOAD_ATOM (spec, i, am_inet_reply);
    i = LOAD_PORT (spec, i, desc->dport);
    i = LOAD_ATOM (spec, i, am_ok);
    while (buflen > 0) {
	int eopt = *buf;
	buf ++; buflen --;
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "sctp_fill_opts -> opt: %d\r\n",
              __LINE__, desc->s, driver_caller(desc->port), eopt) );
	switch(eopt)
	{
	case INET_LOPT_NON_BLOCK_SEND:
        {
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_fill_opts -> NON_BLOCK_SEND: %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  B2S(IS_NON_BLOCK_SEND(desc))) );
            if (IS_NON_BLOCK_SEND(desc))
                i = LOAD_ATOM (spec, i, am_true);
            else
                i = LOAD_ATOM (spec, i, am_false);
            break;
        }
        case INET_LOPT_DEBUG:
        {
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "sctp_fill_opts -> DEBUG: %s\r\n",
                  __LINE__, desc->s, driver_caller(desc->port),
                  B2S(desc->debug)) );
            if (desc->debug)
                i = LOAD_ATOM (spec, i, am_true);
            else
                i = LOAD_ATOM (spec, i, am_false);
            break;
        }
	case INET_LOPT_BUFFER:
	{
	    PLACE_FOR(spec, i, LOAD_ATOM_CNT + LOAD_INT_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM (spec, i, am_buffer);
	    i = LOAD_INT  (spec, i, desc->bufsz);
	    i = LOAD_TUPLE(spec, i, 2);
	    break;
	}
	case INET_LOPT_MODE:
	{
	    PLACE_FOR(spec, i, 2*LOAD_ATOM_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM (spec, i, am_mode);
	    switch (desc->mode)
	    {
	    	case INET_MODE_LIST  :
		{ i = LOAD_ATOM (spec, i, am_list);   break; }
		case INET_MODE_BINARY:
		{ i = LOAD_ATOM (spec, i, am_binary); break; }
		default: ASSERT (0);
	    }
	    i = LOAD_TUPLE (spec, i, 2);
	    break;
	}
	case INET_LOPT_ACTIVE:
	{
            if (desc->active == INET_MULTI)
                PLACE_FOR(spec, i, LOAD_ATOM_CNT + LOAD_INT_CNT + LOAD_TUPLE_CNT);
            else
                PLACE_FOR(spec, i, 2*LOAD_ATOM_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM (spec, i, am_active);
	    switch (desc->active)
	    {
		case INET_ACTIVE :
		{ i = LOAD_ATOM (spec, i, am_true);  break; }
		case INET_PASSIVE:
		{ i = LOAD_ATOM (spec, i, am_false); break; }
		case INET_ONCE   :
		{ i = LOAD_ATOM (spec, i, am_once);  break; }
                case INET_MULTI  :
                { i = LOAD_INT(spec, i, desc->active_count); break; }
		default: ASSERT (0);
	    }
	    i = LOAD_TUPLE (spec, i, 2);
	    break;
	}
#ifdef HAVE_SETNS
	case INET_LOPT_NETNS:
	    if (desc->netns != NULL) {
		PLACE_FOR
		    (spec, i,
		     LOAD_ATOM_CNT + LOAD_BUF2BINARY_CNT + LOAD_TUPLE_CNT);
		i = LOAD_ATOM (spec, i, am_netns);
		i = LOAD_BUF2BINARY
		    (spec, i, desc->netns, strlen(desc->netns));
		i = LOAD_TUPLE (spec, i, 2);
		break;
	    }
	    else
		continue;
#endif
	case SCTP_OPT_RTOINFO:
	{
	    struct       sctp_rtoinfo rti;
	    unsigned int sz  = sizeof(rti);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    rti.srto_assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_RTOINFO,
			    &rti, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      3*LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_rtoinfo);
	    i = LOAD_ATOM	(spec, i, am_sctp_rtoinfo);
	    i = LOAD_ASSOC_ID	(spec, i, rti.srto_assoc_id);
	    i = LOAD_INT	(spec, i, rti.srto_initial);
	    i = LOAD_INT	(spec, i, rti.srto_max);
	    i = LOAD_INT	(spec, i, rti.srto_min);
	    i = LOAD_TUPLE	(spec, i, 5);
	    i = LOAD_TUPLE (spec, i, 2);
	    break;
	}
	case SCTP_OPT_ASSOCINFO:
	{
	    struct       sctp_assocparams ap;
	    unsigned int sz  = sizeof(ap);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    ap.sasoc_assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_ASSOCINFO,
			    &ap, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      5*LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_associnfo);
	    i = LOAD_ATOM	(spec, i, am_sctp_assocparams);
	    i = LOAD_ASSOC_ID	(spec, i, ap.sasoc_assoc_id);
	    i = LOAD_INT	(spec, i, ap.sasoc_asocmaxrxt);
	    i = LOAD_INT	(spec, i, ap.sasoc_number_peer_destinations);
	    i = LOAD_INT	(spec, i, ap.sasoc_peer_rwnd);
	    i = LOAD_INT	(spec, i, ap.sasoc_local_rwnd);
	    i = LOAD_INT	(spec, i, ap.sasoc_cookie_life);
	    i = LOAD_TUPLE	(spec, i, 7);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_INITMSG:
	{
	    struct       sctp_initmsg im;
	    unsigned int sz = sizeof(im);
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_INITMSG,
			    &im, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT +
		      4*LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_initmsg);
	    i = LOAD_ATOM	(spec, i, am_sctp_initmsg);
	    i = LOAD_INT	(spec, i, im.sinit_num_ostreams);
	    i = LOAD_INT	(spec, i, im.sinit_max_instreams);
	    i = LOAD_INT	(spec, i, im.sinit_max_attempts);
	    i = LOAD_INT	(spec, i, im.sinit_max_init_timeo);
	    i = LOAD_TUPLE	(spec, i, 5);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case INET_OPT_LINGER:
	{
	    struct linger lg;
	    unsigned int  sz = sizeof(lg);
	    if (sock_getopt(desc->s, SOL_SOCKET, SO_LINGER,
			    &lg, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      LOAD_ATOM_CNT + LOAD_BOOL_CNT +
		      LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_linger);
	    i = LOAD_BOOL	(spec, i, lg.l_onoff);
	    i = LOAD_INT	(spec, i, lg.l_linger);
	    i = LOAD_TUPLE	(spec, i, 2);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
#ifdef SO_BINDTODEVICE
	case INET_OPT_BIND_TO_DEVICE: {
	    char ifname[IFNAMSIZ];
	    unsigned int  sz = sizeof(ifname);
	    if (sock_getopt(desc->s, SOL_SOCKET, SO_BINDTODEVICE,
			    &ifname, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      LOAD_ATOM_CNT + LOAD_BUF2BINARY_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM (spec, i, am_bind_to_device);
	    i = LOAD_BUF2BINARY(spec, i, ifname, my_strnlen(ifname, sz));
	    i = LOAD_TUPLE (spec, i, 2);
	    break;
	}
#endif
	case INET_OPT_RCVBUF   :
	case INET_OPT_SNDBUF   :
	case INET_OPT_DONTROUTE:
	case INET_OPT_PRIORITY :
	case INET_OPT_TOS      :
	case INET_OPT_TCLASS   :
	case INET_OPT_TTL      :
	case INET_OPT_IPV6_V6ONLY:
	case SCTP_OPT_AUTOCLOSE:
	case SCTP_OPT_MAXSEG   :
	case INET_OPT_REUSEADDR:
	case INET_OPT_REUSEPORT:
	case INET_OPT_REUSEPORT_LB:
        case INET_OPT_EXCLUSIVEADDRUSE:
	case SCTP_OPT_NODELAY  :
	case SCTP_OPT_DISABLE_FRAGMENTS:
	case SCTP_OPT_I_WANT_MAPPED_V4_ADDR:
	case INET_OPT_RECVTOS  :
	case INET_OPT_RECVTCLASS :
	case INET_OPT_RECVTTL :
	{
	    int res   = 0;
	    unsigned int sz = sizeof(res);
	    int proto = 0, type = 0, is_int = 0;
	    ErlDrvTermData tag = am_sctp_error;
	    switch(eopt)
	    {
	    case INET_OPT_RCVBUF   :
	    {
		proto  = SOL_SOCKET;
		type   = SO_RCVBUF;
		is_int = 1;
		tag    = am_recbuf;
		break;
	    }
	    case INET_OPT_SNDBUF   :
	    {
		proto  = SOL_SOCKET;
		type   = SO_SNDBUF;
		is_int = 1;
		tag    = am_sndbuf;
		break;
	    }
	    case INET_OPT_EXCLUSIVEADDRUSE:
	    {
#if defined(__WIN32__)
		proto  = SOL_SOCKET;
		type   = SO_EXCLUSIVEADDRUSE;
		is_int = 0;
		tag    = am_exclusiveaddruse;
		break;
#else
                continue;
#endif
	    }
	    case INET_OPT_REUSEPORT_LB:
	    {
#if defined(SO_REUSEPORT_LB) || (defined(__linux__) && defined(SO_REUSEPORT))
		proto  = SOL_SOCKET;
#if defined(__linux__)
		type   = SO_REUSEPORT;
#else
                type   = SO_REUSEPORT_LB;
#endif
		is_int = 0;
		tag    = am_reuseport_lb;
		break;
#else
                continue;
#endif
	    }
	    case INET_OPT_REUSEPORT:
	    {
#if defined(__WIN32__)
                res = !!(desc->bsd_compat & (1<<INET_OPT_REUSEPORT));
		is_int = 0;
		tag    = am_reuseaddr;
                goto form_result;
#elif defined(SO_REUSEPORT)
		proto  = SOL_SOCKET;
		type   = SO_REUSEPORT;
		is_int = 0;
		tag    = am_reuseport;
		break;
#else
                continue;
#endif
	    }
	    case INET_OPT_REUSEADDR:
	    {
#if defined(__WIN32__)
                res = !!(desc->bsd_compat & (1<<INET_OPT_REUSEADDR));
		is_int = 0;
		tag    = am_reuseaddr;
                goto form_result;
#else
		proto  = SOL_SOCKET;
		type   = SO_REUSEADDR;
		is_int = 0;
		tag    = am_reuseaddr;
		break;
#endif
	    }
	    case INET_OPT_DONTROUTE:
	    {
		proto  = SOL_SOCKET;
		type   = SO_DONTROUTE;
		is_int = 0;
		tag    = am_dontroute;
		break;
	    }
	    case INET_OPT_PRIORITY:
	    {
#	    if defined(SO_PRIORITY)
		proto  = SOL_SOCKET;
		type   = SO_PRIORITY;
		is_int = 1;
		tag    = am_priority;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_TOS:
	    {
#	    if defined(IP_TOS) && defined(IPPROTO_IP)
		proto  = IPPROTO_IP;
		type   = IP_TOS;
		is_int = 1;
		tag    = am_tos;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_TCLASS:
	    {
#           if defined(IPV6_TCLASS) && defined(IPPROTO_IPV6)
		proto  = IPPROTO_IPV6;
		type   = IPV6_TCLASS;
		is_int = 1;
		tag    = am_tclass;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_TTL:
	    {
#           if defined(IP_TTL) && defined(IPPROTO_IP)
		proto  = IPPROTO_IP;
		type   = IP_TTL;
		is_int = 1;
		tag    = am_ttl;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_RECVTOS:
	    {
#	    if defined(IP_RECVTOS) && defined(IPPROTO_IP)
		proto  = IPPROTO_IP;
		type   = IP_RECVTOS;
		is_int = 0;
		tag    = am_recvtos;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_RECVTCLASS:
	    {
#           if defined(IPV6_RECVTCLASS) && defined(IPPROTO_IPV6)
		proto  = IPPROTO_IPV6;
		type   = IPV6_RECVTCLASS;
		is_int = 0;
		tag    = am_recvtclass;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_RECVTTL:
	    {
#	    if defined(IP_RECVTTL) && defined(IPPROTO_IP)
		proto  = IPPROTO_IP;
		type   = IP_RECVTTL;
		is_int = 0;
		tag    = am_recvttl;
		break;
#	    else
		continue;
#	    endif
	    }
	    case INET_OPT_IPV6_V6ONLY:
#           if HAVE_DECL_IPV6_V6ONLY && defined(IPPROTO_IPV6)
	    {
		proto  = IPPROTO_IPV6;
		type   = IPV6_V6ONLY;
		tag    = am_ipv6_v6only;
		break;
	    }
#           elif defined(__WIN32__) && defined(HAVE_IN6) && defined(AF_INET6)
#               error Here is a fix for Win IPv6 SCTP needed
#           else
		continue;
#           endif
	    case SCTP_OPT_AUTOCLOSE:
	    {
		proto  = IPPROTO_SCTP;
		type   = SCTP_AUTOCLOSE;
		is_int = 1;
		tag    = am_sctp_autoclose;
		break;
	    }
	    case SCTP_OPT_MAXSEG   :
	   {
		proto  = IPPROTO_SCTP;
		type   = SCTP_MAXSEG;
		is_int = 1;
		tag    = am_sctp_maxseg;
		break;
	    }
	    case SCTP_OPT_NODELAY  :
	   {
		proto  = IPPROTO_SCTP;
		type   = SCTP_NODELAY;
		is_int = 0;
		tag    = am_sctp_nodelay;
		break;
	    }
	    case SCTP_OPT_DISABLE_FRAGMENTS:
	    {
		proto  = IPPROTO_SCTP;
		type   = SCTP_DISABLE_FRAGMENTS;
		is_int = 0;
		tag    = am_sctp_disable_fragments;
		break;
	    }
	    case SCTP_OPT_I_WANT_MAPPED_V4_ADDR:
	    {
		proto  = IPPROTO_SCTP;
		type   = SCTP_I_WANT_MAPPED_V4_ADDR;
		is_int = 0;
		tag    = am_sctp_i_want_mapped_v4_addr;
		break;
	    }
	    default:	 ASSERT(0);
	    }
	    if (sock_getopt (desc->s, proto, type, &res, &sz) < 0) continue;
#ifdef __WIN32__
form_result:
#endif
	    PLACE_FOR(spec, i, LOAD_ATOM_CNT +
		      (is_int ? LOAD_INT_CNT : LOAD_BOOL_CNT) +
		      LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, tag);
	    if (is_int)
	    	i = LOAD_INT	(spec, i, res);
	    else
	    	i = LOAD_BOOL	(spec, i, res);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_PRIMARY_ADDR:
	case SCTP_OPT_SET_PEER_PRIMARY_ADDR:
	{
	    struct       sctp_setpeerprim sp;
	    unsigned int sz = sizeof(sp);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    sp.sspp_assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP,
			    (eopt == SCTP_OPT_PRIMARY_ADDR) ?
			    SCTP_PRIMARY_ADDR : SCTP_SET_PEER_PRIMARY_ADDR,
			    &sp, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      LOAD_INET_GET_ADDRESS_CNT + 2*LOAD_TUPLE_CNT);
	    switch (eopt) {
	    case SCTP_OPT_PRIMARY_ADDR:
		i = LOAD_ATOM(spec, i, am_sctp_primary_addr);
		i = LOAD_ATOM(spec, i, am_sctp_prim);
		break;
	    case SCTP_OPT_SET_PEER_PRIMARY_ADDR:
		i = LOAD_ATOM(spec, i, am_sctp_set_peer_primary_addr);
		i = LOAD_ATOM(spec, i, am_sctp_setpeerprim);
		break;
	    default:
		ASSERT(0);
	    }
	    i = LOAD_ASSOC_ID	(spec, i, sp.sspp_assoc_id);
            PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    i = load_inet_get_address(spec, i, desc, (struct sockaddr_storage *)&sp.sspp_addr);
            POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    i = LOAD_TUPLE	(spec, i, 3);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_ADAPTATION_LAYER:
	{
	    struct       sctp_setadaptation ad;
	    unsigned int sz  = sizeof (ad);
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_ADAPTATION_LAYER,
			    &ad, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_adaptation_layer);
	    i = LOAD_ATOM	(spec, i, am_sctp_setadaptation);
	    i = LOAD_INT	(spec, i, sock_ntohl(ad.ssb_adaptation_ind));
	    i = LOAD_TUPLE	(spec, i, 2);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_PEER_ADDR_PARAMS:
	{
	    struct sctp_paddrparams  ap;
	    unsigned int             sz = sizeof(ap);
	    int                      n;
	    char                    *before, *xerror;
	    ErlDrvSizeT              alen;
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    ap.spp_assoc_id = GET_ASSOC_ID(buf);
	    buf += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    alen = buflen;
	    before = buf;
	    xerror =
	      inet_set_faddress
	      (desc->sfamily, (inet_address*) (&ap.spp_address),
	       &buf, &alen);
	    if (xerror != NULL) {
#ifdef EAFNOSUPPORT
	        if (xerror == str_eafnosupport) {
		    RETURN_ERROR(spec, -EAFNOSUPPORT);
		}
#else
		RETURN_ERROR(spec, -EINVAL);
#endif
	    }
	    buflen -= buf - before;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS,
			    &ap, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      LOAD_INET_GET_ADDRESS_CNT + 4*LOAD_INT_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_peer_addr_params);
	    i = LOAD_ATOM	(spec, i, am_sctp_paddrparams);
	    i = LOAD_ASSOC_ID	(spec, i, ap.spp_assoc_id);
            PUSH_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    i = load_inet_get_address(spec, i, desc, (struct sockaddr_storage *)&ap.spp_address);
            POP_SUPPRESS_ADDRESS_OF_PACKED_MEMBER();
	    i = LOAD_INT	(spec, i, ap.spp_hbinterval);
	    i = LOAD_INT	(spec, i, ap.spp_pathmaxrxt);
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_PATHMTU
	    i = LOAD_INT	(spec, i, ap.spp_pathmtu);
#           else
	    i = LOAD_INT	(spec, i, 0);
#           endif
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_SACKDELAY
	    i = LOAD_INT	(spec, i, ap.spp_sackdelay);
#           else
	    i = LOAD_INT	(spec, i, 0);
#           endif
	    n = 0;
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_FLAGS
	    PLACE_FOR(spec, i, 7*LOAD_ATOM_CNT);
	    if (ap.spp_flags & SPP_HB_ENABLE)
	    	{ i = LOAD_ATOM	(spec, i, am_hb_enable); 	     n++; }
	    if (ap.spp_flags & SPP_HB_DISABLE)
		{ i = LOAD_ATOM (spec, i, am_hb_disable); 	     n++; }
	    if (ap.spp_flags & SPP_HB_DEMAND)
		{ i = LOAD_ATOM (spec, i, am_hb_demand);	     n++; }
	    if (ap.spp_flags & SPP_PMTUD_ENABLE)
		{ i = LOAD_ATOM (spec, i, am_pmtud_enable);          n++; }
	    if (ap.spp_flags & SPP_PMTUD_DISABLE)
		{ i = LOAD_ATOM (spec, i, am_pmtud_disable);         n++; }
#	    ifdef HAVE_STRUCT_SCTP_PADDRPARAMS_SPP_SACKDELAY
	    if (ap.spp_flags & SPP_SACKDELAY_ENABLE)
		{ i = LOAD_ATOM (spec, i, am_sackdelay_enable);      n++; }
	    if (ap.spp_flags & SPP_SACKDELAY_DISABLE)
		{ i = LOAD_ATOM (spec, i, am_sackdelay_disable);     n++; }
#	    endif
#	    endif
	    PLACE_FOR(spec, i,
		      LOAD_NIL_CNT + LOAD_LIST_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_NIL	(spec, i);
	    i = LOAD_LIST	(spec, i, n+1);
	    i = LOAD_TUPLE	(spec, i, 8);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_DEFAULT_SEND_PARAM:
	{
	    struct       sctp_sndrcvinfo sri;
	    unsigned int sz  = sizeof(sri);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    sri.sinfo_assoc_id = GET_ASSOC_ID(buf);
	    buf += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_DEFAULT_SEND_PARAM,
			    &sri, &sz) < 0) continue;
	    PLACE_FOR(spec, i, LOAD_ATOM_CNT +
		      SCTP_PARSE_SNDRCVINFO_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM(spec, i, am_sctp_default_send_param);
	    i = sctp_parse_sndrcvinfo(spec, i, &sri);
	    i = LOAD_TUPLE(spec, i, 2);
	    break;
	}
	case SCTP_OPT_EVENTS:
	{
	    struct       sctp_event_subscribe evs;
	    unsigned int sz  = sizeof(evs);
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_EVENTS,
			    &evs, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      2*LOAD_ATOM_CNT + 9*LOAD_BOOL_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_events);
	    i = LOAD_ATOM	(spec, i, am_sctp_event_subscribe);
	    i = LOAD_BOOL	(spec, i, evs.sctp_data_io_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_association_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_address_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_send_failure_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_peer_error_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_shutdown_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_partial_delivery_event);
	    i = LOAD_BOOL	(spec, i, evs.sctp_adaptation_layer_event);
	    i = LOAD_BOOL	(spec, i, 0);
	    i = LOAD_TUPLE	(spec, i, 10);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
#if defined(HAVE_DECL_SCTP_DELAYED_ACK_TIME) && defined(SCTP_ASSOC_VALUE_ASSOC_ID)
	case SCTP_OPT_DELAYED_ACK_TIME:
	{
	    struct       sctp_assoc_value av;
	    unsigned int sz  = sizeof(av);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    av.assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_DELAYED_ACK_TIME,
			    &av, &sz) < 0) continue;
	    PLACE_FOR(spec, i, 2*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      LOAD_INT_CNT + 2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_delayed_ack_time);
	    i = LOAD_ATOM	(spec, i, am_sctp_assoc_value);
	    i = LOAD_ASSOC_ID	(spec, i, av.assoc_id);
	    i = LOAD_INT	(spec, i, av.assoc_value);
	    i = LOAD_TUPLE	(spec, i, 3);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
#	endif
	case SCTP_OPT_STATUS:
	{
	    struct       sctp_status  st;
	    unsigned int sz  = sizeof(st);
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    st.sstat_assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_STATUS,
			    &st, &sz) < 0) continue;
	    PLACE_FOR(spec, i, 3*LOAD_ATOM_CNT + LOAD_ASSOC_ID_CNT +
		      6*LOAD_INT_CNT + LOAD_PADDRINFO_CNT +
		      2*LOAD_TUPLE_CNT);
	    i = LOAD_ATOM	(spec, i, am_sctp_status);
	    i = LOAD_ATOM	(spec, i, am_sctp_status);
	    i = LOAD_ASSOC_ID   (spec, i, st.sstat_assoc_id);
	    switch(st.sstat_state)
	    {
#	    if HAVE_DECL_SCTP_EMPTY
	    case SCTP_EMPTY:
		i = LOAD_ATOM	(spec, i, am_empty);
		break;
#	    endif
	    case SCTP_CLOSED:
		i = LOAD_ATOM   (spec, i, am_closed);
		break;
#           if HAVE_DECL_SCTP_BOUND
	    case SCTP_BOUND:
		i = LOAD_ATOM	(spec, i, am_bound);
		break;
#           endif
#           if HAVE_DECL_SCTP_LISTEN
	    case SCTP_LISTEN:
		i = LOAD_ATOM	(spec, i, am_listen);
		break;
#           endif
	    case SCTP_COOKIE_WAIT:
		i = LOAD_ATOM	(spec, i, am_cookie_wait);
		break;
	    case SCTP_COOKIE_ECHOED:
		i = LOAD_ATOM	(spec, i, am_cookie_echoed);
		break;
	    case SCTP_ESTABLISHED:
		i = LOAD_ATOM	(spec, i, am_established);
		break;
	    case SCTP_SHUTDOWN_PENDING:
		i = LOAD_ATOM	(spec, i, am_shutdown_pending);
		break;
	    case SCTP_SHUTDOWN_SENT:
		i = LOAD_ATOM	(spec, i, am_shutdown_sent);
		break;
	    case SCTP_SHUTDOWN_RECEIVED:
		i = LOAD_ATOM	(spec, i, am_shutdown_received);
		break;
	    case SCTP_SHUTDOWN_ACK_SENT:
		i = LOAD_ATOM	(spec, i, am_shutdown_ack_sent);
		break;
	    default:
		i = LOAD_ATOM	(spec, i, am_undefined);
		break;
	    }
	    i = LOAD_INT	(spec, i, st.sstat_rwnd);
	    i = LOAD_INT	(spec, i, st.sstat_unackdata);
	    i = LOAD_INT	(spec, i, st.sstat_penddata);
	    i = LOAD_INT	(spec, i, st.sstat_instrms);
	    i = LOAD_INT	(spec, i, st.sstat_outstrms);
	    i = LOAD_INT	(spec, i, st.sstat_fragmentation_point);
	    i = load_paddrinfo	(spec, i, desc, &st.sstat_primary);
	    i = LOAD_TUPLE	(spec, i, 10);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	case SCTP_OPT_GET_PEER_ADDR_INFO:
	{
	    struct sctp_paddrinfo  pai;
	    unsigned int           sz = sizeof(pai);
	    char                  *before, *xerror;
	    ErlDrvSizeT            alen;
	    if (buflen < ASSOC_ID_LEN) RETURN_ERROR(spec, -EINVAL);
	    pai.spinfo_assoc_id = GET_ASSOC_ID(buf);
	    buf    += ASSOC_ID_LEN;
	    buflen -= ASSOC_ID_LEN;
	    alen = buflen;
	    before = buf;
	    xerror =
	      inet_set_faddress
	      (desc->sfamily, (inet_address*) (&pai.spinfo_address),
	       &buf, &alen);
	    if (xerror != NULL) {
#ifdef EAFNOSUPPORT
	        if (xerror == str_eafnosupport) {
		    RETURN_ERROR(spec, -EAFNOSUPPORT);
		}
#else
		RETURN_ERROR(spec, -EINVAL);
#endif
	    }
	    buflen -= buf - before;
	    if (sock_getopt(desc->s, IPPROTO_SCTP, SCTP_GET_PEER_ADDR_INFO,
			    &pai, &sz) < 0) continue;
	    PLACE_FOR(spec, i,
		      LOAD_ATOM_CNT + LOAD_PADDRINFO_CNT + LOAD_TUPLE_CNT);
	    i = LOAD_ATOM       (spec, i, am_sctp_get_peer_addr_info);
	    i = load_paddrinfo	(spec, i, desc, &pai);
	    i = LOAD_TUPLE	(spec, i, 2);
	    break;
	}
	default:
	    RETURN_ERROR(spec, -EINVAL);
	}
	length ++;
    }
    if (buflen != 0) RETURN_ERROR(spec, -EINVAL);
    PLACE_FOR(spec, i, LOAD_NIL_CNT + LOAD_LIST_CNT + 2*LOAD_TUPLE_CNT);
    i = LOAD_NIL  (spec, i);
    i = LOAD_LIST (spec, i, length+1);
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 3);
    erl_drv_send_term(desc->dport, driver_caller(desc->port), spec, i);
    FREE(spec);
    (*dest)[0] = INET_REP;
    return 1;
#   undef PLACE_FOR
#   undef RETURN_ERROR
}
#endif
static ErlDrvSSizeT inet_fill_stat(inet_descriptor* desc,
				   char* src, ErlDrvSizeT len, char* dst)
{
    unsigned long val;
    int op;
    char* dst_start = dst;
    *dst++ = INET_REP_OK;
    while (len--) {
	op = *src++;
	*dst++ = op;
	switch(op) {
	case INET_STAT_RECV_CNT:
	    val = desc->recv_cnt;
	    break;
	case INET_STAT_RECV_MAX:
	    val = (unsigned long) desc->recv_max;
	    break;
	case INET_STAT_RECV_AVG:
	    val = (unsigned long) desc->recv_avg;
	    break;
	case INET_STAT_RECV_DVI:
	    val = (unsigned long) fabs(desc->recv_dvi);
	    break;
	case INET_STAT_SEND_CNT:
	    val = desc->send_cnt;
	    break;
	case INET_STAT_SEND_MAX:
	    val = desc->send_max;
	    break;
	case INET_STAT_SEND_AVG:
	    val = (unsigned long) desc->send_avg;
	    break;
	case INET_STAT_SEND_PND:
	    val = (unsigned long) driver_sizeq(desc->port);
	    break;
	case INET_STAT_RECV_OCT:
#ifdef ARCH_64
	    put_int64(desc->recv_oct, dst);
#else
	    put_int32(desc->recv_oct[1], dst);
	    put_int32(desc->recv_oct[0], dst+4);
#endif
	    dst += 8;
	    continue;
	case INET_STAT_SEND_OCT:
#ifdef ARCH_64
	    put_int64(desc->send_oct, dst);
#else
	    put_int32(desc->send_oct[1], dst);
	    put_int32(desc->send_oct[0], dst+4);
#endif
	    dst += 8;
	    continue;
	default: return -1;
	}
	put_int32(val, dst);
	dst += 4;
    }
    return dst - dst_start;
}
static void
send_empty_out_q_msgs(inet_descriptor* desc)
{
  ErlDrvTermData msg[6];
  int msg_len = 0;
  if(NO_SUBSCRIBERS(&desc->empty_out_q_subs))
    return;
  msg_len = LOAD_ATOM(msg, msg_len, am_empty_out_q);
  msg_len = LOAD_PORT(msg, msg_len, desc->dport);
  msg_len = LOAD_TUPLE(msg, msg_len, 2);
  ASSERT(msg_len == sizeof(msg)/sizeof(*msg));
  send_to_subscribers(desc->dport,
		      &desc->empty_out_q_subs,
		      1,
		      msg,
		      msg_len);
}
static ErlDrvSSizeT inet_subscribe(inet_descriptor* desc,
				   char* src, ErlDrvSizeT len, char* dst)
{
    unsigned long val;
    int op;
    char* dst_start = dst;
    *dst++ = INET_REP_OK;
    while (len--) {
	op = *src++;
	*dst++ = op;
	switch(op) {
	case INET_SUBS_EMPTY_OUT_Q:
	  val = driver_sizeq(desc->port);
	  if(val > 0)
	    if(!save_subscriber(&desc->empty_out_q_subs,
				driver_caller(desc->port)))
	      return 0;
	  break;
	default: return -1;
	}
	put_int32(val, dst);
	dst += 4;
    }
    return dst - dst_start;
}
static void inet_stop(inet_descriptor* desc)
{
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR "] "
          "inet_stop -> entry\r\n",
          __LINE__, desc->s) );
    erl_inet_close(desc);
#ifdef HAVE_SETNS
    if (desc->netns != NULL)
	FREE(desc->netns);
#endif
    FREE(desc);
}
static void inet_emergency_close(ErlDrvData data)
{
    tcp_descriptor* tcp_desc = (tcp_descriptor*)data;
    inet_descriptor* desc = INETP(tcp_desc);
    DEBUGF(("inet_emergency_close(%p) {s=%d\r\n",
	    desc->port, desc->s));
    if (desc->s != INVALID_SOCKET) {
	sock_close(desc->s);
    }
}
static void set_default_msgq_limits(ErlDrvPort port)
{
    ErlDrvSizeT q_high = INET_HIGH_MSGQ_WATERMARK;
    ErlDrvSizeT q_low = INET_LOW_MSGQ_WATERMARK;
    if (q_low < ERL_DRV_BUSY_MSGQ_LIM_MIN)
	q_low = ERL_DRV_BUSY_MSGQ_LIM_MIN;
    else if (q_low > ERL_DRV_BUSY_MSGQ_LIM_MAX)
	q_low = ERL_DRV_BUSY_MSGQ_LIM_MAX;
    if (q_high < ERL_DRV_BUSY_MSGQ_LIM_MIN)
	q_high = ERL_DRV_BUSY_MSGQ_LIM_MIN;
    else if (q_high > ERL_DRV_BUSY_MSGQ_LIM_MAX)
	q_high = ERL_DRV_BUSY_MSGQ_LIM_MAX;
    erl_drv_busy_msgq_limits(port, &q_low, &q_high);
}
static ErlDrvData inet_start(ErlDrvPort port, int size, int protocol)
{
    inet_descriptor* desc;
    if ((desc = (inet_descriptor*) ALLOC(size)) == NULL)
	return NULL;
    desc->s = INVALID_SOCKET;
    desc->event = INVALID_EVENT;
    desc->event_mask = 0;
#ifdef __WIN32__
    desc->forced_events = 0;
    desc->err = 0;
    desc->send_would_block = 0;
#endif
    desc->port = port;
    desc->dport = driver_mk_port(port);
    desc->state = INET_STATE_CLOSED;
    desc->prebound = 0;
    if (protocol == IPPROTO_TCP)
        desc->bufsz = INET_BUFFER_TCP_DEFAULT;
    else if (protocol == IPPROTO_UDP)
        desc->bufsz = INET_BUFFER_UDP_DEFAULT;
#if defined(HAVE_SCTP)
    else if (protocol == IPPROTO_SCTP)
        desc->bufsz = INET_BUFFER_SCTP_DEFAULT;
#endif
    else
        desc->bufsz = 9999;
    desc->hsz = 0;
    desc->htype = TCP_PB_RAW;
    desc->psize = 0;
    desc->stype = -1;
    desc->sfamily = -1;
    desc->sprotocol = protocol;
    desc->mode    = INET_MODE_LIST;
    desc->exitf   = 1;
    desc->deliver = INET_DELIVER_TERM;
    desc->active  = INET_PASSIVE;
    desc->active_count = 0;
    desc->delimiter    = '\n';
    desc->oph = NULL;
    desc->opt = NULL;
    desc->op_ref = 0;
    desc->caller     = am_undefined;
    desc->caller_ref = no_caller_ref();
    desc->peer_ptr = NULL;
    desc->name_ptr = NULL;
#ifdef ARCH_64
    desc->recv_oct  = 0;
#else
    desc->recv_oct[0] = desc->recv_oct[1] = 0;
#endif
    desc->recv_cnt = 0;
    desc->recv_max = 0;
    desc->recv_avg = 0.0;
    desc->recv_dvi = 0.0;
#ifdef ARCH_64
    desc->send_oct = 0;
#else
    desc->send_oct[0] = desc->send_oct[1] = 0;
#endif
    desc->send_cnt = 0;
    desc->send_max = 0;
    desc->send_avg = 0.0;
    desc->empty_out_q_subs.subscriber = NO_PROCESS;
    desc->empty_out_q_subs.next = NULL;
    sys_memzero((char *)&desc->remote,sizeof(desc->remote));
    desc->flags = 0;
#ifdef HAVE_SETNS
    desc->netns = NULL;
#endif
    desc->recv_cmsgflags = 0;
    desc->debug = DDBG_DEFAULT;
    return (ErlDrvData)desc;
}
#define INET_MAXHOSTNAMELEN 255
static ErlDrvSSizeT inet_ctl(inet_descriptor* desc, int cmd, char* buf,
			     ErlDrvSizeT len, char** rbuf, ErlDrvSizeT rsize)
{
    int save_errno = 0;
    switch (cmd) {
    case INET_REQ_GETSTAT: {
	  char* dst;
	  ErlDrvSizeT i;
	  int dstlen = 1;
          DDBG(desc,
               ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                "inet_ctl -> GETSTAT\r\n",
                __LINE__, desc->s, driver_caller(desc->port)) );
	  for (i = 0; i < len; i++) {
	      switch(buf[i]) {
	      case INET_STAT_SEND_OCT: dstlen += 9; break;
	      case INET_STAT_RECV_OCT: dstlen += 9; break;
	      default: dstlen += 5; break;
	      }
	  }
	  if (dstlen > INET_MAX_OPT_BUFFER)
	      return 0;
	  if (dstlen > rsize) {
	      if ((dst = (char*) ALLOC(dstlen)) == NULL)
		  return 0;
	      *rbuf = dst;
	  }
	  else
	      dst = *rbuf;
	  return inet_fill_stat(desc, buf, len, dst);
      }
    case INET_REQ_SUBSCRIBE: {
	  char* dst;
	  int dstlen = 1  + len*5;
          DDBG(desc,
               ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                "inet_ctl -> SUBSCRIBE\r\n",
                __LINE__, desc->s, driver_caller(desc->port)) );
	  if (dstlen > INET_MAX_OPT_BUFFER)
	      return 0;
	  if (dstlen > rsize) {
	      if ((dst = (char*) ALLOC(dstlen)) == NULL)
		  return 0;
	      *rbuf = dst;
	  }
	  else
	      dst = *rbuf;
	  return inet_subscribe(desc, buf, len, dst);
      }
    case INET_REQ_GETOPTS: {
	ErlDrvSSizeT replen;
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "inet_ctl -> GETOPTS\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
#ifdef HAVE_SCTP
        if (IS_SCTP(desc))
        {
            if ((replen = sctp_fill_opts(desc, buf, len, rbuf, rsize)) < 0)
                return ctl_error(-replen, rbuf, rsize);
        } else
#endif
            if ((replen = inet_fill_opts(desc, buf, len, rbuf, rsize)) < 0) {
                return ctl_error(EINVAL, rbuf, rsize);
        }
	return replen;
    }
    case INET_REQ_GETIFLIST: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETIFLIST\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	return inet_ctl_getiflist(desc, rbuf, rsize);
    }
    case INET_REQ_GETIFADDRS: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETIFADDRS\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	return inet_ctl_getifaddrs(desc, rbuf, rsize);
    }
    case INET_REQ_IFGET: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> IFGET\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	return inet_ctl_ifget(desc, buf, len, rbuf, rsize);
    }
    case INET_REQ_IFSET: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> IFSET\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	return inet_ctl_ifset(desc, buf, len, rbuf, rsize);
    }
    case INET_REQ_SETOPTS:  {
        int sres;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> SETOPTS\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
        sres = inet_set_opts(desc, buf, len);
        if (sres != 0)
            DDBG(desc,
                 ("INET-DRV-DBG[%d][%d,%T] inet_ctl(setopts) -> sres: %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), sres) );
	switch(sres) {
	case -1:
	    return ctl_error(EINVAL, rbuf, rsize);
	case 0:
	    return ctl_reply_ok(rbuf, rsize);
	case 1:
	    tcp_deliver((tcp_descriptor *) desc, 0);
	    return ctl_reply_ok(rbuf, rsize);
	default:
            if ((tcp_recv((tcp_descriptor *) desc, 0) >= 0) && desc->active) {
                sock_select(desc, (FD_READ|FD_CLOSE), 1);
            }
	    return ctl_reply_ok(rbuf, rsize);
	}
    }
    case INET_REQ_GETSTATUS: {
	char tbuf[4];
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETSTATUS\r\n",
              __LINE__, driver_caller(desc->port)) );
	put_int32(desc->state, tbuf);
	return ctl_reply(INET_REP_OK, tbuf, 4, rbuf, rsize);
    }
    case INET_REQ_GETTYPE: {
	char tbuf[8];
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETTYPE\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (desc->sfamily == AF_INET) {
	    put_int32(INET_AF_INET, &tbuf[0]);
	}
#if defined(HAVE_IN6) && defined(AF_INET6)
        else if (desc->sfamily == AF_INET6) {
	    put_int32(INET_AF_INET6, &tbuf[0]);
	}
#endif
#ifdef HAVE_SYS_UN_H
	else if (desc->sfamily == AF_UNIX) {
	    put_int32(INET_AF_LOCAL, &tbuf[0]);
	}
#endif
	else
	    return ctl_error(EINVAL, rbuf, rsize);
	if (desc->stype == SOCK_STREAM) {
	    put_int32(INET_TYPE_STREAM, &tbuf[4]);
	}
	else if (desc->stype == SOCK_DGRAM) {
	    put_int32(INET_TYPE_DGRAM, &tbuf[4]);
	}
#ifdef HAVE_SCTP
	else if (desc->stype == SOCK_SEQPACKET) {
	    put_int32(INET_TYPE_SEQPACKET, &tbuf[4]);
	}
#endif
	else
	    return ctl_error(EINVAL, rbuf, rsize);
	return ctl_reply(INET_REP_OK, tbuf, 8, rbuf, rsize);
    }
    case INET_REQ_GETFD: {
	char tbuf[4];
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> GETFD\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_error(EINVAL, rbuf, rsize);
	put_int32((long)desc->s, tbuf);
	return ctl_reply(INET_REP_OK, tbuf, 4, rbuf, rsize);
    }
    case INET_REQ_GETHOSTNAME: {
	char tbuf[INET_MAXHOSTNAMELEN + 1];
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETHOSTNAME\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len != 0)
	    return ctl_error(EINVAL, rbuf, rsize);
	if (IS_SOCKET_ERROR(sock_hostname(tbuf, INET_MAXHOSTNAMELEN + 1)))
	    return ctl_error(sock_errno(), rbuf, rsize);
	return ctl_reply(INET_REP_OK, tbuf, strlen(tbuf), rbuf, rsize);
    }
    case INET_REQ_GETPADDRS: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETPADDRS\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len != 4) return ctl_error(EINVAL, rbuf, rsize);
	if (! IS_OPEN(desc)) return ctl_xerror(EXBADPORT, rbuf, rsize);
#ifdef HAVE_SCTP
	if (IS_SCTP(desc) && p_sctp_getpaddrs) {
	    struct sockaddr *sa;
	    Uint32 assoc_id;
	    int n;
	    ErlDrvSizeT rlen;
	    assoc_id = get_int32(buf);
	    n = p_sctp_getpaddrs(desc->s, assoc_id, &sa);
	    rlen = reply_inet_addrs(n, (inet_address *) sa, rbuf, rsize, 0);
	    if (n > 0) p_sctp_freepaddrs(sa);
	    return rlen;
	}
#endif
	{
	    inet_address addr;
	    SOCKLEN_T sz;
	    int i;
	    sz = sizeof(addr);
	    i = sock_peer(desc->s, (struct sockaddr *) &addr, &sz);
	    return reply_inet_addrs(i >= 0 ? 1 : i, &addr, rbuf, rsize, sz);
	}
    }
    case INET_REQ_PEER:  {
	char tbuf[sizeof(inet_address)];
	inet_address peer;
	inet_address* ptr;
	unsigned int sz;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> PEER\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (!(desc->state & INET_F_ACTIVE))
	    return ctl_error(ENOTCONN, rbuf, rsize);
	if ((ptr = desc->peer_ptr) != NULL) {
	    sz = desc->peer_addr_len;
	}
	else {
	    ptr = &peer;
            sz = sizeof(peer);
            sys_memzero((char *) &peer, sz);
	    if (IS_SOCKET_ERROR
		(sock_peer
		 (desc->s, (struct sockaddr*)ptr, &sz)))
		return ctl_error(sock_errno(), rbuf, rsize);
	}
	if (inet_get_address(tbuf, ptr, &sz) < 0)
	    return ctl_error(EINVAL, rbuf, rsize);
	return ctl_reply(INET_REP_OK, tbuf, sz, rbuf, rsize);
    }
    case INET_REQ_SETPEER: {
        char *xerror;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> SETPEER\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len == 0) {
	    desc->peer_ptr = NULL;
	    return ctl_reply_ok(rbuf, rsize);
	}
	else if (len < 2)
	    return ctl_error(EINVAL, rbuf, rsize);
	else if ((xerror = inet_set_faddress
		  (desc->sfamily, &desc->peer_addr, &buf, &len)) != NULL)
	    return ctl_xerror(xerror, rbuf, rsize);
	else {
	    desc->peer_ptr = &desc->peer_addr;
	    desc->peer_addr_len = (unsigned int) len;
	    return ctl_reply_ok(rbuf, rsize);
	}
    }
    case INET_REQ_GETLADDRS: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETLADDRS\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len != 4) return ctl_error(EINVAL, rbuf, rsize);
	if (! IS_OPEN(desc)) return ctl_xerror(EXBADPORT, rbuf, rsize);
#ifdef HAVE_SCTP
	if (IS_SCTP(desc) && p_sctp_getladdrs) {
	    struct sockaddr *sa;
	    Uint32 assoc_id;
	    int n;
	    ErlDrvSizeT rlen;
	    assoc_id = get_int32(buf);
	    n = p_sctp_getladdrs(desc->s, assoc_id, &sa);
	    rlen = reply_inet_addrs(n, (inet_address *) sa, rbuf, rsize, 0);
	    if (n > 0) p_sctp_freeladdrs(sa);
	    return rlen;
	}
#endif
	{
	    inet_address addr;
	    SOCKLEN_T sz;
	    int i;
	    sz = sizeof(addr);
	    sys_memzero((char *) &addr, sz);
	    i = sock_name(desc->s, (struct sockaddr *) &addr, &sz);
	    return reply_inet_addrs(i >= 0 ? 1 : i, &addr, rbuf, rsize, sz);
	}
    }
    case INET_REQ_NAME:  {
	char tbuf[sizeof(inet_address)];
	inet_address name;
	inet_address* ptr;
	unsigned int sz;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> NAME\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
	if ((ptr = desc->name_ptr) != NULL) {
	    sz = desc->name_addr_len;
	}
	else {
	    ptr = &name;
	    sz = sizeof(name);
	    sys_memzero((char *) &name, sz);
	    if (IS_SOCKET_ERROR
		(sock_name(desc->s, (struct sockaddr*)ptr, &sz)))
		return ctl_error(sock_errno(), rbuf, rsize);
	}
	if (inet_get_address(tbuf, ptr, &sz) < 0)
	    return ctl_error(EINVAL, rbuf, rsize);
	return ctl_reply(INET_REP_OK, tbuf, sz, rbuf, rsize);
    }
    case INET_REQ_SETNAME: {
        char *xerror;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> SETNAME\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len == 0) {
	    desc->name_ptr = NULL;
	    return ctl_reply_ok(rbuf, rsize);
	}
	else if (len < 2)
	    return ctl_error(EINVAL, rbuf, rsize);
	else if ((xerror = inet_set_faddress
		  (desc->sfamily, &desc->name_addr, &buf, &len)) != NULL)
	    return ctl_xerror(xerror, rbuf, rsize);
	else {
	    desc->name_ptr = &desc->name_addr;
	    desc->name_addr_len = (unsigned int) len;
	    return ctl_reply_ok(rbuf, rsize);
	}
    }
    case INET_REQ_BIND:  {
        char tbuf[2], *xerror;
	inet_address local;
	int port;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> BIND\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
	if (len < 2)
	    return ctl_error(EINVAL, rbuf, rsize);
	if (desc->state != INET_STATE_OPEN)
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	if ((xerror = inet_set_faddress
	     (desc->sfamily, &local, &buf, &len)) != NULL)
	    return ctl_xerror(xerror, rbuf, rsize);
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> try bind\r\n",
              __LINE__, desc->s, driver_caller(desc->port)) );
	if (IS_SOCKET_ERROR(sock_bind(desc->s,(struct sockaddr*) &local, len))) {
            save_errno = sock_errno();
            DDBG(desc,
                 ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> bind failed: %d\r\n",
                  __LINE__, desc->s, driver_caller(desc->port), save_errno) );
	    return ctl_error(save_errno, rbuf, rsize);
        }
	desc->state = INET_STATE_OPEN;
	port = inet_address_port(&local);
	if (port == 0) {
	    SOCKLEN_T adrlen = sizeof(local);
	    sys_memzero((char *) &local, adrlen);
	    sock_name(desc->s, &local.sa, &adrlen);
	    port = inet_address_port(&local);
	}
        else if (port == -1) port = 0;
	put_int16(sock_ntohs((Uint16) port), tbuf);
	return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
    }
    case INET_REQ_IGNOREFD: {
        DDBG(desc,
             ("INET-DRV-DBG[%d][%d,%T] inet_ctl -> IGNOREFD: %s\r\n",
              __LINE__, desc->s, driver_caller(desc->port), B2S(*buf)) );
      if (!IS_CONNECTED(desc))
	  return ctl_error(ENOTCONN, rbuf, rsize);
      if (desc->stype != SOCK_STREAM)
	  return ctl_error(EINVAL, rbuf, rsize);
      if (*buf == 1 && !INET_IGNORED(desc)) {
	  sock_select(desc, (FD_READ|FD_WRITE|FD_CLOSE|ERL_DRV_USE_NO_CALLBACK), 0);
	  if (desc->active)
	    desc->flags |= INET_IGNORE_READ;
	  else
	    desc->flags |= INET_IGNORE_PASSIVE;
      } else if (*buf == 0 && INET_IGNORED(desc)) {
	  int flags = FD_CLOSE;
	  if (desc->flags & INET_IGNORE_READ)
	    flags |= FD_READ;
	  if (desc->flags & INET_IGNORE_WRITE)
	    flags |= FD_WRITE;
	  desc->flags = INET_IGNORE_CLEAR(desc);
	  if (flags != FD_CLOSE)
	    sock_select(desc, flags, 1);
      } else
	  return ctl_error(EINVAL, rbuf, rsize);
      return ctl_reply_ok(rbuf, rsize);
    }
    case INET_REQ_GETSERVBYNAME: {
	char namebuf[256];
	char protobuf[256];
	char tbuf[2];
	struct servent* srv;
	short port;
	int n;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETSERVBYNAME\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len < 2)
	    return ctl_error(EINVAL, rbuf, rsize);
	n = get_int8(buf); buf++; len--;
	if (n >= len)
	    return ctl_error(EINVAL, rbuf, rsize);
	memcpy(namebuf, buf, n);
	namebuf[n] = '\0';
	len -= n; buf += n;
	n = get_int8(buf); buf++; len--;
	if (n > len)
	    return ctl_error(EINVAL, rbuf, rsize);
	memcpy(protobuf, buf, n);
	protobuf[n] = '\0';
	if ((srv = sock_getservbyname(namebuf, protobuf)) == NULL)
	    return ctl_error(EINVAL, rbuf, rsize);
	port = sock_ntohs(srv->s_port);
	put_int16(port, tbuf);
	return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
    }
    case INET_REQ_GETSERVBYPORT: {
	char protobuf[256];
	unsigned short port;
	int n;
	struct servent* srv;
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> GETSERVBYPORT\r\n",
              __LINE__, driver_caller(desc->port)) );
	if (len < 3)
	    return ctl_error(EINVAL, rbuf, rsize);
	port = get_int16(buf);
	port = sock_htons(port);
	buf += 2;
	n = get_int8(buf); buf++; len -= 3;
	if (n > len)
	    return ctl_error(EINVAL, rbuf, rsize);
	memcpy(protobuf, buf, n);
	protobuf[n] = '\0';
	if ((srv = sock_getservbyport(port, protobuf)) == NULL)
	    return ctl_error(EINVAL, rbuf, rsize);
	len = strlen(srv->s_name);
	return ctl_reply(INET_REP_OK, srv->s_name, len, rbuf, rsize);
    }
    default:
        DDBG(desc,
             ("INET-DRV-DBG[%d][%T] inet_ctl -> unknown command\r\n",
              __LINE__, driver_caller(desc->port)) );
	return ctl_xerror(EXBADPORT, rbuf, rsize);
    }
}
static void inet_output_count(inet_descriptor* desc, ErlDrvSizeT len)
{
    unsigned long n = desc->send_cnt + 1;
#ifndef ARCH_64
    Uint32 t = desc->send_oct[0] + len;
    int c = (t < desc->send_oct[0]);
#endif
    double avg = desc->send_avg;
#ifdef ARCH_64
    desc->send_oct += len;
#else
    desc->send_oct[0] = t;
    desc->send_oct[1] += c;
#endif
    if (n == 0)
	n = 1;
    desc->send_avg += (len - avg) / n;
    if (len > desc->send_max)
	desc->send_max = len;
    desc->send_cnt = n;
}
static void inet_input_count(inet_descriptor* desc, ErlDrvSizeT len)
{
    unsigned long n = desc->recv_cnt + 1;
#ifndef ARCH_64
    Uint32 t = (desc->recv_oct[0] + len);
    int c = (t < desc->recv_oct[0]);
#endif
    double avg = desc->recv_avg;
    double dvi;
#ifdef ARCH_64
    desc->recv_oct += len;
#else
    desc->recv_oct[0] = t;
    desc->recv_oct[1] += c;
#endif
    if (n == 0)
	n = 1;
    avg = avg + (len - avg) / n;
    desc->recv_avg = avg;
    if (len > desc->recv_max)
	desc->recv_max = len;
    dvi = desc->recv_dvi;
    desc->recv_dvi = dvi + ((len - avg) - dvi) / n;
    desc->recv_cnt = n;
}
static int tcp_expand_buffer(tcp_descriptor* desc, int len)
{
    ErlDrvBinary* bin;
    int offs1;
    int offs2;
    int used = desc->i_ptr_start - desc->i_buf->orig_bytes;
    int ulen = used + len;
    if (desc->i_bufsz >= ulen)
	return 0;
    else if (desc->i_buf->orig_size >= ulen) {
	desc->i_bufsz = ulen;
	return 0;
    }
    DEBUGF(("tcp_expand_buffer(%p): s=%d, from %ld to %d\r\n",
	    desc->inet.port, desc->inet.s,
            desc->i_buf->orig_size, ulen));
    offs1 = desc->i_ptr_start - desc->i_buf->orig_bytes;
    offs2 = desc->i_ptr - desc->i_ptr_start;
    if ((bin = driver_realloc_binary(desc->i_buf, ulen)) == NULL)
	return -1;
    desc->i_buf = bin;
    desc->i_ptr_start = bin->orig_bytes + offs1;
    desc->i_ptr       = desc->i_ptr_start + offs2;
    desc->i_bufsz     = ulen;
    return 0;
}
static int tcp_push_buffer(tcp_descriptor* desc, char* buf, int len)
{
    ErlDrvBinary* bin;
    if (desc->i_buf == NULL) {
	bin = alloc_buffer(len);
	sys_memcpy(bin->orig_bytes, buf, len);
	desc->i_buf = bin;
	desc->i_bufsz = len;
	desc->i_ptr_start = desc->i_buf->orig_bytes;
	desc->i_ptr = desc->i_ptr_start + len;
    }
    else {
	char* start =  desc->i_buf->orig_bytes;
	int sz_before = desc->i_ptr_start - start;
	int sz_filled = desc->i_ptr - desc->i_ptr_start;
	if (len <= sz_before) {
	    sys_memcpy(desc->i_ptr_start - len, buf, len);
	    desc->i_ptr_start -= len;
	}
	else {
	    bin = alloc_buffer(desc->i_bufsz+len);
	    sys_memcpy(bin->orig_bytes, buf, len);
	    sys_memcpy(bin->orig_bytes+len, desc->i_ptr_start, sz_filled);
	    free_buffer(desc->i_buf);
	    desc->i_bufsz += len;
	    desc->i_buf = bin;
	    desc->i_ptr_start = bin->orig_bytes;
	    desc->i_ptr = desc->i_ptr_start + sz_filled + len;
	}
    }
    desc->i_remain = 0;
    return 0;
}
static void tcp_clear_input(tcp_descriptor* desc)
{
    if (desc->i_buf != NULL)
	free_buffer(desc->i_buf);
    desc->i_buf = NULL;
    desc->i_remain    = 0;
    desc->i_ptr       = NULL;
    desc->i_ptr_start = NULL;
    desc->i_bufsz     = 0;
}
static void tcp_clear_output(tcp_descriptor* desc)
{
    ErlDrvPort ix  = desc->inet.port;
    ErlDrvSizeT qsz = driver_sizeq(ix);
    driver_deq(ix, qsz);
    send_empty_out_q_msgs(INETP(desc));
}
static void tcp_restart_input(tcp_descriptor* desc)
{
    ASSERT( desc->i_buf != NULL );
    if (desc->i_ptr_start != desc->i_buf->orig_bytes) {
	int n;
        ASSERT( (desc->i_ptr_start != NULL) &&
                (desc->i_buf != NULL) &&
                (desc->i_buf->orig_bytes != NULL) );
        n = desc->i_ptr - desc->i_ptr_start;
	DEBUGF(("tcp_restart_input: move %d bytes\r\n", n));
	sys_memmove(desc->i_buf->orig_bytes, desc->i_ptr_start, n);
	desc->i_ptr_start = desc->i_buf->orig_bytes;
	desc->i_ptr = desc->i_ptr_start + n;
    }
}
static int tcp_inet_init(void)
{
    DEBUGF(("tcp_inet_init() {}\r\n"));
    return 0;
}
static ErlDrvData prep_tcp_inet_start(ErlDrvPort port, char* args)
{
    tcp_descriptor* desc;
    DEBUGF(("tcp_inet_start(%p) {\r\n", port));
    desc = (tcp_descriptor*)
	inet_start(port, sizeof(tcp_descriptor), IPPROTO_TCP);
    if (desc == NULL)
	return ERL_DRV_ERROR_ERRNO;
    desc->high = INET_HIGH_WATERMARK;
    desc->low  = INET_LOW_WATERMARK;
    desc->send_timeout = INET_INFINITY;
    desc->send_timeout_close = 0;
    desc->busy_on_send = 0;
    desc->i_buf = NULL;
    desc->i_ptr = NULL;
    desc->i_ptr_start = NULL;
    desc->i_remain = 0;
    desc->i_bufsz = 0;
    desc->tcp_add_flags = 0;
    desc->http_state = 0;
    desc->mtd = NULL;
    desc->mtd_cache = NULL;
    desc->multi_first = desc->multi_last = NULL;
    DEBUGF(("tcp_inet_start(%p) }\r\n", port));
    return (ErlDrvData) desc;
}
static ErlDrvData tcp_inet_start(ErlDrvPort port, char* args)
{
    ErlDrvData data = prep_tcp_inet_start(port, args);
    set_default_msgq_limits(port);
    return data;
}
static tcp_descriptor* tcp_inet_copy(tcp_descriptor* desc,SOCKET s,
				     ErlDrvTermData owner, int* err)
{
    ErlDrvSizeT q_low, q_high;
    ErlDrvPort port = desc->inet.port;
    tcp_descriptor* copy_desc;
    copy_desc = (tcp_descriptor*) prep_tcp_inet_start(port, NULL);
    if ((copy_desc->inet.s = s) != INVALID_SOCKET) {
	if ((copy_desc->inet.event = sock_create_event(INETP(copy_desc))) ==
	    INVALID_EVENT) {
	    *err = sock_errno();
	    FREE(copy_desc);
	    return NULL;
	}
    }
    copy_desc->inet.mode     = desc->inet.mode;
    copy_desc->inet.exitf    = desc->inet.exitf;
    copy_desc->inet.deliver  = desc->inet.deliver;
    copy_desc->inet.htype    = desc->inet.htype;
    copy_desc->inet.psize    = desc->inet.psize;
    copy_desc->inet.stype    = desc->inet.stype;
    copy_desc->inet.sfamily  = desc->inet.sfamily;
    copy_desc->inet.hsz      = desc->inet.hsz;
    copy_desc->inet.bufsz    = desc->inet.bufsz;
    copy_desc->high          = desc->high;
    copy_desc->low           = desc->low;
    copy_desc->send_timeout  = desc->send_timeout;
    copy_desc->send_timeout_close = desc->send_timeout_close;
    copy_desc->tcp_add_flags = desc->tcp_add_flags
        & (TCP_ADDF_SHOW_ECONNRESET | TCP_ADDF_LINGER_ZERO |
           TCP_ADDF_NO_READ_AHEAD);
    port = driver_create_port(port, owner, "tcp_inet", (ErlDrvData) copy_desc);
    if ((SWord)port == -1) {
	*err = INET_ERRNO_SYSTEM_LIMIT;
	FREE(copy_desc);
	return NULL;
    }
    q_low = q_high = ERL_DRV_BUSY_MSGQ_READ_ONLY;
    erl_drv_busy_msgq_limits(desc->inet.port, &q_low, &q_high);
    erl_drv_busy_msgq_limits(port, &q_low, &q_high);
    copy_desc->inet.port = port;
    copy_desc->inet.dport = driver_mk_port(port);
    *err = 0;
    return copy_desc;
}
static void tcp_close_check(tcp_descriptor* desc)
{
    if (desc->inet.state == INET_STATE_ACCEPTING) {
	inet_async_op *this_op = desc->inet.opt;
	sock_select(INETP(desc), FD_ACCEPT, 0);
	desc->inet.state = INET_STATE_LISTENING;
	if (this_op != NULL) {
	    driver_demonitor_process(desc->inet.port, &(this_op->monitor));
	}
	async_error_am(INETP(desc), am_closed);
    }
    else if (desc->inet.state == INET_STATE_MULTI_ACCEPTING) {
	int id,req;
	ErlDrvTermData caller;
	ErlDrvMonitor monitor;
	sock_select(INETP(desc), FD_ACCEPT, 0);
	desc->inet.state = INET_STATE_LISTENING;
	while (deq_multi_op(desc,&id,&req,&caller,NULL,&monitor) == 0) {
	    driver_demonitor_process(desc->inet.port, &monitor);
	    send_async_error(desc->inet.dport, id, caller, am_closed);
	}
    }
    else if (desc->inet.state == INET_STATE_CONNECTING) {
	async_error_am(INETP(desc), am_closed);
    }
    else if (desc->inet.state == INET_STATE_CONNECTED) {
	async_error_am_all(INETP(desc), am_closed);
    }
    clean_multi_timers(desc, desc->inet.port);
}
static void tcp_inet_stop(ErlDrvData e)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    DDBG(INETP(desc),
         ("INET-DRV-DBG[%d][" SOCKET_FSTR "] tcp_inet_stop -> entry\r\n",
          __LINE__, desc->inet.s) );
    tcp_close_check(desc);
    tcp_clear_input(desc);
#ifdef HAVE_SENDFILE
    if(desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
        desc->tcp_add_flags &= ~TCP_ADDF_SENDFILE;
        close(desc->sendfile.dup_file_fd);
        DDBG(INETP(desc),
             ("INET-DRV-DBG[%d][" SOCKET_FSTR "] "
              "tcp_inet_stop -> SENDFILE dup closed %d\r\n",
              __LINE__, desc->inet.s, desc->sendfile.dup_file_fd) );
    }
#endif
    inet_stop(INETP(desc));
}
static void tcp_desc_close(tcp_descriptor* desc)
{
    tcp_clear_input(desc);
    tcp_clear_output(desc);
    erl_inet_close(INETP(desc));
}
static int tcp_inet_recv_timeout(ErlDrvData e, ErlDrvTermData dummy)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    ASSERT(!desc->inet.active);
    sock_select(INETP(desc),(FD_READ|FD_CLOSE),0);
    desc->i_remain = 0;
    async_error_am(INETP(desc), am_timeout);
    return 1;
}
static ErlDrvSSizeT tcp_inet_ctl(ErlDrvData e, unsigned int cmd,
				 char* buf, ErlDrvSizeT len,
				 char** rbuf, ErlDrvSizeT rsize)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    cmd -= ERTS_INET_DRV_CONTROL_MAGIC_NUMBER;
    switch(cmd) {
    case INET_REQ_OPEN: {
	int domain, protocol;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][%T] tcp_inet_ctl -> OPEN\r\n",
	      __LINE__, driver_caller(desc->inet.port)) );
	if (len != 3) return ctl_error(EINVAL, rbuf, rsize);
	switch(buf[0]) {
	case INET_AF_INET:
	    domain = AF_INET;
	    break;
#if defined(HAVE_IN6) && defined(AF_INET6)
	case INET_AF_INET6:
	    domain = AF_INET6;
	    break;
#endif
#ifdef HAVE_SYS_UN_H
	case INET_AF_LOCAL:
	    domain = AF_UNIX;
	    break;
#endif
	default:
	    return ctl_xerror(str_eafnosupport, rbuf, rsize);
	}
	if (buf[1] != INET_TYPE_STREAM) return ctl_error(EINVAL, rbuf, rsize);
        switch(buf[2]) {
        case INET_PROTO_DEFAULT: protocol = 0; break;
        case INET_PROTO_TCP: protocol = IPPROTO_TCP; break;
#ifdef IPPROTO_MPTCP
        case INET_PROTO_MPTCP: protocol = IPPROTO_MPTCP; break;
#endif
        default:
            return ctl_xerror(str_eprotonosupport, rbuf, rsize);
        }
	return
            inet_ctl_open(INETP(desc),
                          domain, SOCK_STREAM, protocol, rbuf, rsize);
	break;
    }
    case INET_REQ_FDOPEN: {
	int domain;
        int bound;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> FDOPEN\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (len != 6 && len != 10) return ctl_error(EINVAL, rbuf, rsize);
	switch(buf[0]) {
	case INET_AF_INET:
	    domain = AF_INET;
	    break;
#if defined(HAVE_IN6) && defined(AF_INET6)
	case INET_AF_INET6:
	    domain = AF_INET6;
	    break;
#endif
#ifdef HAVE_SYS_UN_H
	case INET_AF_LOCAL:
	    domain = AF_UNIX;
	    break;
#endif
	default:
	    return ctl_xerror(str_eafnosupport, rbuf, rsize);
	}
	if (buf[1] != INET_TYPE_STREAM) return ctl_error(EINVAL, rbuf, rsize);
        if (len == 6) bound = 1;
        else bound = get_int32(buf+2+4);
	return inet_ctl_fdopen(INETP(desc), domain, SOCK_STREAM,
                               (SOCKET) get_int32(buf+2),
                               bound, rbuf, rsize);
	break;
    }
    case INET_REQ_LISTEN: {
	int backlog;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> LISTEN\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (desc->inet.state == INET_STATE_CLOSED)
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	if (!IS_OPEN(INETP(desc)))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	if (len != 2)
	    return ctl_error(EINVAL, rbuf, rsize);
	backlog = get_int16(buf);
	if (IS_SOCKET_ERROR(sock_listen(desc->inet.s, backlog)))
	    return ctl_error(sock_errno(), rbuf, rsize);
	desc->inet.state = INET_STATE_LISTENING;
	return ctl_reply_ok(rbuf, rsize);
    }
    case INET_REQ_CONNECT: {
	int code;
	char *xerror;
	unsigned timeout;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> CONNECT\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (!IS_OPEN(INETP(desc)))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
	if (IS_CONNECTED(INETP(desc)))
	    return ctl_error(EISCONN, rbuf, rsize);
	if (IS_CONNECTING(INETP(desc)))
	    return ctl_error(EINVAL, rbuf, rsize);
	if (len < 6)
	    return ctl_error(EINVAL, rbuf, rsize);
	timeout = get_int32(buf);
	buf += 4;
	len -= 4;
	if ((xerror = inet_set_faddress
	     (desc->inet.sfamily, &desc->inet.remote, &buf, &len)) != NULL)
	    return ctl_xerror(xerror, rbuf, rsize);
	code = sock_connect(desc->inet.s,
			    (struct sockaddr*) &desc->inet.remote, len);
	if (IS_SOCKET_ERROR(code) &&
		((sock_errno() == ERRNO_BLOCK) ||
		 (sock_errno() == EINPROGRESS))) {
            char tbuf[2];
            DDBG(INETP(desc),
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "tcp_inet_ctl(connect) -> would block => select\r\n",
                  __LINE__,
                  desc->inet.s, driver_caller(desc->inet.port)) );
            sock_select(INETP(desc), FD_CONNECT, 1);
	    desc->inet.state = INET_STATE_CONNECTING;
	    if (timeout != INET_INFINITY)
		driver_set_timer(desc->inet.port, timeout);
	    enq_async(INETP(desc), tbuf, INET_REQ_CONNECT);
            return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
	}
	else if (code == 0) {
            DDBG(INETP(desc),
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "tcp_inet_ctl(connect) -> connected\r\n",
                  __LINE__,
                  desc->inet.s, driver_caller(desc->inet.port)) );
	    desc->inet.state = INET_STATE_CONNECTED;
	    if (desc->inet.active)
		sock_select(INETP(desc), (FD_READ|FD_CLOSE), 1);
            return ctl_reply_ok(rbuf, rsize);
	}
	else {
            int save_errno = sock_errno();
            DDBG(INETP(desc),
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "tcp_inet_ctl(connect) -> error: %s (%d)\r\n",
                  __LINE__,
                  desc->inet.s, driver_caller(desc->inet.port),
                  errno_str(save_errno), save_errno) );
	    return ctl_error(save_errno, rbuf, rsize);
	}
    }
    case INET_REQ_ACCEPT: {
	char tbuf[2];
	unsigned timeout;
	inet_address remote;
	unsigned int n;
	SOCKET s;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> ACCEPT\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if ((desc->inet.state != INET_STATE_LISTENING && desc->inet.state != INET_STATE_ACCEPTING &&
	     desc->inet.state != INET_STATE_MULTI_ACCEPTING) || len != 4) {
	    return ctl_error(EINVAL, rbuf, rsize);
	}
	timeout = get_int32(buf);
	if (desc->inet.state == INET_STATE_ACCEPTING) {
	    unsigned long time_left = 0;
	    int oid = 0;
	    ErlDrvTermData ocaller = am_undefined;
	    int oreq = 0;
	    unsigned otimeout = 0;
	    ErlDrvTermData caller = driver_caller(desc->inet.port);
	    MultiTimerData *mtd = NULL,*omtd = NULL;
	    ErlDrvMonitor monitor, omonitor;
	    if (driver_monitor_process(desc->inet.port, caller ,&monitor) != 0) {
		return ctl_xerror("noproc", rbuf, rsize);
	    }
	    deq_async_w_tmo(INETP(desc),&oid,&ocaller,&oreq,&otimeout,&omonitor);
	    if (otimeout != INET_INFINITY) {
		driver_read_timer(desc->inet.port, &time_left);
		driver_cancel_timer(desc->inet.port);
		if (time_left <= 0) {
		    time_left = 1;
		}
		omtd = add_multi_timer(desc, desc->inet.port, ocaller,
				       time_left, &tcp_inet_multi_timeout);
	    }
	    enq_old_multi_op(desc, oid, oreq, ocaller, omtd, &omonitor);
	    if (timeout != INET_INFINITY) {
		mtd = add_multi_timer(desc, desc->inet.port, caller,
				      timeout, &tcp_inet_multi_timeout);
	    }
	    enq_multi_op(desc, tbuf, INET_REQ_ACCEPT, caller, mtd, &monitor);
	    desc->inet.state = INET_STATE_MULTI_ACCEPTING;
	    return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
	} else if (desc->inet.state == INET_STATE_MULTI_ACCEPTING) {
	    ErlDrvTermData caller = driver_caller(desc->inet.port);
	    MultiTimerData *mtd = NULL;
	    ErlDrvMonitor monitor;
	    if (driver_monitor_process(desc->inet.port, caller ,&monitor) != 0) {
		return ctl_xerror("noproc", rbuf, rsize);
	    }
	    if (timeout != INET_INFINITY) {
		mtd = add_multi_timer(desc, desc->inet.port, caller,
				      timeout, &tcp_inet_multi_timeout);
	    }
	    enq_multi_op(desc, tbuf, INET_REQ_ACCEPT, caller, mtd, &monitor);
	    return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
 	} else {
	    n = sizeof(desc->inet.remote);
	    sys_memzero((char *) &remote, n);
	    s = sock_accept(desc->inet.s, (struct sockaddr*) &remote, &n);
	    if (s == INVALID_SOCKET) {
		if (sock_errno() == ERRNO_BLOCK) {
		    ErlDrvMonitor monitor;
		    if (driver_monitor_process(desc->inet.port, driver_caller(desc->inet.port),
					       &monitor) != 0) {
			return ctl_xerror("noproc", rbuf, rsize);
		    }
		    enq_async_w_tmo(INETP(desc), tbuf, INET_REQ_ACCEPT, timeout, &monitor);
		    desc->inet.state = INET_STATE_ACCEPTING;
		    sock_select(INETP(desc),FD_ACCEPT,1);
		    if (timeout != INET_INFINITY) {
			driver_set_timer(desc->inet.port, timeout);
		    }
		} else {
		    return ctl_error(sock_errno(), rbuf, rsize);
		}
	    } else {
		ErlDrvTermData owner = driver_caller(desc->inet.port);
		tcp_descriptor* accept_desc;
		int err;
		if ((accept_desc = tcp_inet_copy(desc,s,owner,&err)) == NULL) {
		    sock_close(s);
		    return ctl_error(err, rbuf, rsize);
		}
		accept_desc->inet.remote = remote;
		SET_NONBLOCKING(accept_desc->inet.s);
#ifdef __WIN32__
		driver_select(accept_desc->inet.port, accept_desc->inet.event,
			      ERL_DRV_READ, 1);
#endif
		accept_desc->inet.state = INET_STATE_CONNECTED;
		enq_async(INETP(desc), tbuf, INET_REQ_ACCEPT);
		async_ok_port(INETP(desc), accept_desc->inet.dport);
	    }
	    return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
	}
    }
    case INET_REQ_CLOSE:
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] tcp_inet_ctl -> CLOSE\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	tcp_close_check(desc);
	tcp_desc_close(desc);
	return ctl_reply_ok(rbuf, rsize);
    case TCP_REQ_RECV: {
	unsigned timeout;
	char tbuf[2];
	int n;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] tcp_inet_ctl -> RECV\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (!IS_CONNECTED(INETP(desc))) {
	    if (desc->tcp_add_flags & TCP_ADDF_DELAYED_CLOSE_RECV) {
		desc->tcp_add_flags &= ~(TCP_ADDF_DELAYED_CLOSE_RECV|
					 TCP_ADDF_DELAYED_CLOSE_SEND);
		if (desc->tcp_add_flags & TCP_ADDF_DELAYED_ECONNRESET) {
		    desc->tcp_add_flags &= ~TCP_ADDF_DELAYED_ECONNRESET;
		    return ctl_reply(INET_REP_ERROR, "econnreset", 10, rbuf, rsize);
		} else
		    return ctl_reply(INET_REP_ERROR, "closed", 6, rbuf, rsize);
	    }
	    return ctl_error(ENOTCONN, rbuf, rsize);
	}
	if (desc->inet.active || (len != 8))
	    return ctl_error(EINVAL, rbuf, rsize);
	timeout = get_int32(buf);
	buf += 4;
	n = get_int32(buf);
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl(recv) -> timeout: %d, n: %d\r\n",
	      __LINE__,
              desc->inet.s, driver_caller(desc->inet.port), timeout, n) );
	if ((desc->inet.htype != TCP_PB_RAW) && (n != 0))
	    return ctl_error(EINVAL, rbuf, rsize);
	if (n > TCP_MAX_PACKET_SIZE)
	    return ctl_error(ENOMEM, rbuf, rsize);
	if (enq_async(INETP(desc), tbuf, TCP_REQ_RECV) < 0)
	    return ctl_error(EALREADY, rbuf, rsize);
	if (INET_IGNORED(INETP(desc)) || tcp_recv(desc, n) == 0) {
	    if (timeout == 0)
		async_error_am(INETP(desc), am_timeout);
	    else {
		if (timeout != INET_INFINITY)
                    add_multi_timer(desc, INETP(desc)->port, am_undefined,
                                    timeout, &tcp_inet_recv_timeout);
		if (!INET_IGNORED(INETP(desc)))
		    sock_select(INETP(desc),(FD_READ|FD_CLOSE),1);
		else
		  INETP(desc)->flags |= INET_IGNORE_READ;
	    }
	}
	return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
    }
    case TCP_REQ_UNRECV: {
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] tcp_inet_ctl -> UNRECV\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (!IS_CONNECTED(INETP(desc)))
   	    return ctl_error(ENOTCONN, rbuf, rsize);
	tcp_push_buffer(desc, buf, len);
	if (desc->inet.active)
	    tcp_deliver(desc, 0);
	return ctl_reply_ok(rbuf, rsize);
    }
    case TCP_REQ_SHUTDOWN: {
	int how;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> SHUTDOWN\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
	if (!IS_CONNECTED(INETP(desc))) {
	    return ctl_error(ENOTCONN, rbuf, rsize);
	}
	if (len != 1) {
	    return ctl_error(EINVAL, rbuf, rsize);
	}
	how = buf[0];
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl(shutdown) -> how: %s (%d)\r\n",
	      __LINE__,
              desc->inet.s, driver_caller(desc->inet.port), SH2S(how), how) );
	if (how != TCP_SHUT_RD && driver_sizeq(desc->inet.port) > 0) {
	    if (how == TCP_SHUT_WR) {
		desc->tcp_add_flags |= TCP_ADDF_PENDING_SHUT_WR;
	    } else if (how == TCP_SHUT_RDWR) {
		desc->tcp_add_flags |= TCP_ADDF_PENDING_SHUT_RDWR;
	    }
	    return ctl_reply_ok(rbuf, rsize);
	}
	if (IS_SOCKET_ERROR(sock_shutdown(INETP(desc)->s, how))) {
	    if (how != TCP_SHUT_RD)
		desc->tcp_add_flags |= TCP_ADDF_SHUTDOWN_WR_DONE;
	    return ctl_error(sock_errno(), rbuf, rsize);
	} else {
	    return ctl_reply_ok(rbuf, rsize);
	}
    }
    case TCP_REQ_SENDFILE: {
#ifdef HAVE_SENDFILE
        const ErlDrvSizeT required_len =
            sizeof(desc->sendfile.dup_file_fd) +
            sizeof(Uint64) * 2;
        int raw_file_fd;
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> SENDFILE\r\n",
	      __LINE__, desc->inet.s, driver_caller(desc->inet.port)) );
        if (len != required_len) {
            return ctl_error(EINVAL, rbuf, rsize);
        } else if (!IS_CONNECTED(INETP(desc))) {
            return ctl_error(ENOTCONN, rbuf, rsize);
        } else if (desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
            return ctl_error(EINVAL, rbuf, rsize);
        }
        sys_memcpy(&raw_file_fd, buf, sizeof(raw_file_fd));
        buf += sizeof(raw_file_fd);
        desc->sendfile.dup_file_fd = dup(raw_file_fd);
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl(sendfile) -> dup: %d\r\n",
	      __LINE__,
              desc->inet.s, driver_caller(desc->inet.port),
              desc->sendfile.dup_file_fd) );
        if(desc->sendfile.dup_file_fd == -1) {
            return ctl_error(errno, rbuf, rsize);
        }
        desc->sendfile.offset = get_int64(buf);
        buf += sizeof(Uint64);
        desc->sendfile.length = get_int64(buf);
        buf += sizeof(Uint64);
        ASSERT(desc->sendfile.offset >= 0);
        ASSERT(desc->sendfile.length >= 0);
        desc->sendfile.ioq_skip = driver_sizeq(desc->inet.port);
        desc->sendfile.bytes_sent = 0;
        desc->tcp_add_flags |= TCP_ADDF_SENDFILE;
        if (tcp_inet_sendfile(desc) == 0) {
            if(desc->sendfile.length > 0) {
                sock_select(INETP(desc), FD_WRITE, 1);
            }
        }
        return ctl_reply_ok(rbuf, rsize);
#else
        return ctl_error(ENOTSUP, rbuf, rsize);
#endif
    }
    default:
	DDBG(INETP(desc),
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "tcp_inet_ctl -> (maybe) general command %u\r\n",
	      __LINE__,
              desc->inet.s, driver_caller(desc->inet.port), cmd) );
	return inet_ctl(INETP(desc), cmd, buf, len, rbuf, rsize);
    }
}
static int tcp_inet_send_timeout(ErlDrvData e, ErlDrvTermData dummy)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    ASSERT(IS_BUSY(INETP(desc)));
    ASSERT(desc->busy_on_send);
    desc->inet.state &= ~INET_F_BUSY;
    desc->busy_on_send = 0;
    set_busy_port(desc->inet.port, 0);
    inet_reply_error_am(INETP(desc), am_timeout);
    if (desc->send_timeout_close) {
        tcp_desc_close(desc);
    }
    return 1;
}
static void tcp_inet_timeout(ErlDrvData e)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    int state = desc->inet.state;
    DEBUGF(("tcp_inet_timeout(%p) {s=%d\r\n",
	    desc->inet.port, desc->inet.s));
    if ((state & INET_F_MULTI_CLIENT)) {
	fire_multi_timers(desc, desc->inet.port, e);
    } else if ((state & INET_STATE_CONNECTED) == INET_STATE_CONNECTED) {
        fire_multi_timers(desc, desc->inet.port, e);
    }
    else if ((state & INET_STATE_CONNECTING) == INET_STATE_CONNECTING) {
	tcp_desc_close(desc);
	async_error_am(INETP(desc), am_timeout);
    }
    else if ((state & INET_STATE_ACCEPTING) == INET_STATE_ACCEPTING) {
	inet_async_op *this_op = desc->inet.opt;
	sock_select(INETP(desc), FD_ACCEPT, 0);
	if (this_op != NULL) {
	    driver_demonitor_process(desc->inet.port, &(this_op->monitor));
	}
	desc->inet.state = INET_STATE_LISTENING;
	async_error_am(INETP(desc), am_timeout);
    }
    DEBUGF(("tcp_inet_timeout(%p) }\r\n", desc->inet.port));
}
static int tcp_inet_multi_timeout(ErlDrvData e, ErlDrvTermData caller)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    int id,req;
    ErlDrvMonitor monitor;
    if (remove_multi_op(desc, &id, &req, caller, NULL, &monitor) != 0) {
	return 1;
    }
    driver_demonitor_process(desc->inet.port, &monitor);
    if (desc->multi_first == NULL) {
	sock_select(INETP(desc),FD_ACCEPT,0);
	desc->inet.state = INET_STATE_LISTENING;
    }
    send_async_error(desc->inet.dport, id, caller, am_timeout);
    return 1;
}
static void tcp_inet_command(ErlDrvData e, char *buf, ErlDrvSizeT len)
{
    tcp_descriptor  *desc    = (tcp_descriptor*)e;
    inet_descriptor *inetp   = INETP(desc);
    if (! init_caller(&inetp->caller, &inetp->caller_ref,
                      inetp->port, &buf, &len)) {
        driver_failure_posix(inetp->port, EINVAL);
	return;
    }
    DDBG(inetp,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "tcp_inet_command -> entry\r\n",
          __LINE__, inetp->s, inetp->caller) );
    if (!IS_CONNECTED(inetp))
	inet_reply_error(inetp, ENOTCONN);
    else if (tcp_send(desc, buf, len) == 0)
	inet_reply_ok(inetp);
    DDBG(inetp,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "tcp_inet_command -> done\r\n",
          __LINE__, inetp->s, inetp->caller) );
}
static void tcp_inet_commandv(ErlDrvData e, ErlIOVec *ev)
{
    tcp_descriptor  *desc    = (tcp_descriptor*)e;
    inet_descriptor *inetp   = INETP(desc);
#ifdef INET_DRV_DEBUG
    ErlDrvTermData   caller;
#endif
    if (! init_caller_iov(&inetp->caller, &inetp->caller_ref,
                          inetp->port, &ev->iov, &ev->size)) {
        driver_failure_posix(inetp->port, EINVAL);
        return;
    }
#ifdef INET_DRV_DEBUG
    caller  = inetp->caller;
    DDBG(inetp,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "tcp_inet_commandv -> entry\r\n",
          __LINE__, inetp->s, caller) );
#endif
    if (!IS_CONNECTED(INETP(desc))) {
	if (desc->tcp_add_flags & TCP_ADDF_DELAYED_CLOSE_SEND) {
	    desc->tcp_add_flags &= ~TCP_ADDF_DELAYED_CLOSE_SEND;
	    if (desc->tcp_add_flags & TCP_ADDF_DELAYED_ECONNRESET) {
		inet_reply_error(inetp, ECONNRESET);
	    } else
		inet_reply_error_am(inetp, am_closed);
	}
	else
	    inet_reply_error(inetp, ENOTCONN);
    }
    else if (desc->tcp_add_flags & TCP_ADDF_PENDING_SHUTDOWN)
	tcp_shutdown_error(desc, EPIPE);
    else if (tcp_sendv(desc, ev) == 0)
	inet_reply_ok(inetp);
#ifdef INET_DRV_DEBUG
    DDBG(INETP(desc),
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] tcp_inet_commandv -> done\r\n",
          __LINE__, inetp->s, caller) );
#endif
}
static void tcp_inet_flush(ErlDrvData e)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    int discard_output;
    discard_output = !(desc->inet.event_mask & FD_WRITE);
    discard_output |= desc->tcp_add_flags & TCP_ADDF_LINGER_ZERO;
#ifdef HAVE_SENDFILE
    if(desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
        discard_output = 1;
    }
#endif
    if (discard_output) {
        tcp_clear_output(desc);
    }
}
static void tcp_inet_process_exit(ErlDrvData e, ErlDrvMonitor *monitorp)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    ErlDrvTermData who = driver_get_monitored_process(desc->inet.port,monitorp);
    int            state = desc->inet.state;
    DDBG(INETP(desc),
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%p] "
          "tcp_inet_process_exit -> entry with"
          "\r\n   who: %T\r\n",
          __LINE__, desc->inet.s, desc->inet.port, who) );
    if ((state & INET_STATE_MULTI_ACCEPTING) == INET_STATE_MULTI_ACCEPTING) {
	int id,req;
	MultiTimerData *timeout;
	if (remove_multi_op(desc, &id, &req, who, &timeout, NULL) != 0) {
	    return;
	}
	if (timeout != NULL) {
	    remove_multi_timer(desc, desc->inet.port, timeout);
	}
	if (desc->multi_first == NULL) {
	    sock_select(INETP(desc),FD_ACCEPT,0);
	    desc->inet.state = INET_STATE_LISTENING;
	}
    } else if ((state & INET_STATE_ACCEPTING) == INET_STATE_ACCEPTING) {
	int did,drid;
	ErlDrvTermData dcaller;
	deq_async(INETP(desc), &did, &dcaller, &drid);
	driver_cancel_timer(desc->inet.port);
	sock_select(INETP(desc),FD_ACCEPT,0);
	desc->inet.state = INET_STATE_LISTENING;
    }
    DDBG(INETP(desc),
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%p] "
          "tcp_inet_process_exit -> done\r\n",
          __LINE__, desc->inet.s, desc->inet.port) );
}
static void inet_stop_select(ErlDrvEvent event, void* _)
{
#ifdef __WIN32__
    WSACloseEvent((HANDLE)event);
#else
    sock_close((SOCKET)(long)event);
#endif
}
static int tcp_recv_closed(tcp_descriptor* desc)
{
#ifdef DEBUG
    ErlDrvPort port = desc->inet.port;
#endif
    int blocking_send = 0;
    DEBUGF(("tcp_recv_closed(%p): s=%d, in %s, line %d\r\n",
	    port, desc->inet.s, __LINE__));
    if (IS_BUSY(INETP(desc))) {
	tcp_clear_output(desc);
	if (desc->busy_on_send) {
            cancel_multi_timer(desc, INETP(desc)->port, &tcp_inet_send_timeout);
	    desc->busy_on_send = 0;
	    DEBUGF(("tcp_recv_closed(%p): busy on send\r\n", port));
	}
	desc->inet.state &= ~INET_F_BUSY;
	set_busy_port(desc->inet.port, 0);
	inet_reply_error_am(INETP(desc), am_closed);
	DEBUGF(("tcp_recv_closed(%p): busy reply 'closed'\r\n", port));
        blocking_send = 1;
    }
#ifdef HAVE_SENDFILE
    if (desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
        tcp_sendfile_aborted(desc, ENOTCONN);
        blocking_send = 1;
    }
#endif
    if (!blocking_send) {
        desc->tcp_add_flags |= TCP_ADDF_DELAYED_CLOSE_SEND;
    }
    if (!desc->inet.active) {
        clean_multi_timers(desc, INETP(desc)->port);
	tcp_clear_input(desc);
	if (desc->inet.exitf) {
	    tcp_desc_close(desc);
	} else {
	    desc_close_read(INETP(desc));
	}
	async_error_am_all(INETP(desc), am_closed);
	DEBUGF(("tcp_recv_closed(%p): passive reply all 'closed'\r\n",
                port));
    } else {
	tcp_clear_input(desc);
	tcp_closed_message(desc);
	if (desc->inet.exitf) {
	    driver_exit(desc->inet.port, 0);
	} else {
	    desc_close_read(INETP(desc));
	}
	DEBUGF(("tcp_recv_closed(%p): active close\r\n", port));
    }
    DEBUGF(("tcp_recv_closed(%p): done\r\n", port));
    return -1;
}
static int tcp_recv_error(tcp_descriptor* desc, int err)
{
    if (err != ERRNO_BLOCK) {
	if (IS_BUSY(INETP(desc))) {
	    tcp_clear_output(desc);
	    if (desc->busy_on_send) {
                cancel_multi_timer(desc, INETP(desc)->port, &tcp_inet_send_timeout);
		desc->busy_on_send = 0;
	    }
	    desc->inet.state &= ~INET_F_BUSY;
	    set_busy_port(desc->inet.port, 0);
	    inet_reply_error_am(INETP(desc), am_closed);
	}
#ifdef HAVE_SENDFILE
        if (desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
            tcp_sendfile_aborted(desc, err);
        }
#endif
	if (!desc->inet.active) {
            clean_multi_timers(desc, INETP(desc)->port);
	    tcp_clear_input(desc);
	    if (desc->inet.exitf) {
		tcp_desc_close(desc);
	    } else {
		desc_close_read(INETP(desc));
	    }
	    async_error_am_all(INETP(desc), error_atom(err));
	} else {
	    tcp_clear_input(desc);
	    tcp_error_message(desc, err);
	    tcp_closed_message(desc);
	    if (desc->inet.exitf)
		driver_exit(desc->inet.port, err);
	    else
		desc_close_read(INETP(desc));
	}
	return -1;
    }
    return 0;
}
static int packet_header_length(tcp_descriptor *desc) {
    int n, hlen;
    if (! (desc->tcp_add_flags & TCP_ADDF_NO_READ_AHEAD))
        return 0;
    switch (desc->inet.htype) {
    case TCP_PB_RAW:
        return 0;
    case TCP_PB_1:          hlen = 1; break;
    case TCP_PB_2:          hlen = 2; break;
    case TCP_PB_4:          hlen = 4; break;
    case TCP_PB_RM:         hlen = 4; break;
    case TCP_PB_ASN1:       hlen = 2; break;
    case TCP_PB_SSL_TLS:    hlen = 5; break;
    case TCP_PB_CDR:        hlen = 12; break;
    case TCP_PB_FCGI:       hlen = sizeof(struct fcgi_head); break;
    case TCP_PB_TPKT:       hlen = 4; break;
    default:
        return 1;
    }
    n = desc->i_ptr - desc->i_ptr_start;
    ASSERT(n >= 0);
    if (hlen > n)
        return hlen - n;
    else
        return 1;
}
static int tcp_remain(tcp_descriptor* desc, int* len)
{
    char* ptr = desc->i_ptr_start;
    int nfill = (desc->i_ptr - desc->i_buf->orig_bytes);
    int nsz   = desc->i_bufsz - nfill;
    int n = desc->i_ptr - ptr;
    int tlen;
    tlen = packet_get_length(desc->inet.htype, ptr, n,
                             desc->inet.psize, desc->i_bufsz,
                             desc->inet.delimiter, &desc->http_state);
    DEBUGF(("tcp_remain(%p): s=%d, n=%d, nfill=%d nsz=%d, tlen %d\r\n",
	    desc->inet.port, desc->inet.s, n, nfill, nsz, tlen));
    if (tlen > 0) {
        *len = tlen;
        if (tlen <= n) {
            DEBUGF((" => nothing remain packet=%d\r\n", tlen));
            return 0;
        }
        else {
            int nread;
            if (tcp_expand_buffer(desc, tlen) < 0)
                return -1;
            nread = tlen - n;
            DEBUGF((" => remain=%d\r\n", nread));
            return nread;
        }
    }
    else if (tlen == 0) {
        int nread;
        *len = 0;
        nread = packet_header_length(desc);
        if (nread != 0) {
            tcp_restart_input(desc);
            return nread;
        }
        if (nsz == 0) {
            if (nfill == n) {
                if (desc->inet.psize != 0 && desc->inet.psize > nfill) {
                    if (tcp_expand_buffer(desc, desc->inet.psize) < 0)
                        return -1;
                    return desc->inet.psize - n;;
                }
                else
                    goto error;
            }
            DEBUGF((" => restart more=%d\r\n", nfill - n));
            tcp_restart_input(desc);
            return nfill - n;
        }
        else {
            DEBUGF((" => more=%d \r\n", nsz));
            tcp_restart_input(desc);
            return nsz;
        }
    }
error:
    DEBUGF((" => packet error\r\n"));
    return -1;
}
static int tcp_deliver(tcp_descriptor* desc, int len)
{
    int count = 0;
    int n;
    if (len == 0) {
	if ((desc->i_buf == NULL) || (desc->i_remain > 0))
	    return 0;
	if ((n = tcp_remain(desc, &len)) < 0)
            return n;
        else if (0 < n) {
            if (0 < len)
                desc->i_remain = n;
            return 0;
        }
    }
    while (len > 0) {
	int code;
	inet_input_count(INETP(desc), len);
	if (len >=
            (desc->i_buf->orig_size - (desc->i_buf->orig_size >> 2))) {
	    code = tcp_reply_binary_data(desc, desc->i_buf,
					 (desc->i_ptr_start -
					  desc->i_buf->orig_bytes),
					 len);
	    if (code < 0)
		return code;
	    if (desc->i_ptr_start + len == desc->i_ptr) {
		tcp_clear_input(desc);
	    }
	    else {
		ErlDrvBinary* bin = alloc_buffer(desc->i_bufsz);
		char* ptr_end = desc->i_ptr_start + len;
		int sz = desc->i_ptr - ptr_end;
		memcpy(bin->orig_bytes, ptr_end, sz);
		free_buffer(desc->i_buf);
		desc->i_buf = bin;
		desc->i_ptr_start = desc->i_buf->orig_bytes;
		desc->i_ptr = desc->i_ptr_start + sz;
		desc->i_remain = 0;
	    }
	}
	else {
	    code = tcp_reply_data(desc, desc->i_ptr_start, len);
	    if (code < 0)
		return code;
	    desc->i_ptr_start += len;
	    if (desc->i_ptr_start == desc->i_ptr)
		tcp_clear_input(desc);
	    else
		desc->i_remain = 0;
	}
	count++;
	if (! desc->inet.active) {
            cancel_multi_timer(desc, INETP(desc)->port, &tcp_inet_recv_timeout);
	    sock_select(INETP(desc),(FD_READ|FD_CLOSE),0);
	    if (desc->i_buf != NULL)
		tcp_restart_input(desc);
            len = 0;
	}
	else if (desc->i_buf == NULL) {
            len = 0;
        }
        else if ((n = tcp_remain(desc, &len)) < 0)
            return n;
        else if (0 < n) {
            if (0 < len)
                desc->i_remain = n;
            len = 0;
        }
    }
    return count;
}
static int tcp_recv(tcp_descriptor* desc, int request_len)
{
    int nread;
    if (desc->i_buf == NULL) {
	int sz = (request_len > 0) ? request_len : desc->inet.bufsz;
	if ((desc->i_buf = alloc_buffer(sz)) == NULL)
	    return -1;
	desc->i_bufsz = sz;
	desc->i_ptr_start = desc->i_buf->orig_bytes;
	desc->i_ptr = desc->i_ptr_start;
        if (request_len > 0) {
	    nread = desc->i_remain = request_len;
        }
        else {
            nread = packet_header_length(desc);
            if (nread == 0)
                nread = sz;
	    desc->i_remain = 0;
        }
    }
    else if (request_len > 0) {
        int n = desc->i_ptr - desc->i_ptr_start;
	if (n >= request_len)
	    return tcp_deliver(desc, request_len);
	else if (tcp_expand_buffer(desc, request_len) < 0)
	    return tcp_recv_error(desc, ENOMEM);
        desc->i_remain = nread = request_len - n;
    }
    else if (desc->i_remain == 0) {
        int len;
	if ((nread = tcp_remain(desc, &len)) < 0)
	    return tcp_recv_error(desc, EMSGSIZE);
        else if (nread == 0)
            return tcp_deliver(desc, len);
        else if (0 < len)
            desc->i_remain = nread;
    }
    else
	nread = desc->i_remain;
    for (;;) {
        int n;
        DEBUGF(("tcp_recv(%p): s=%d about to read %d bytes...\r\n",
                desc->inet.port, desc->inet.s, nread));
        n = sock_recv(desc->inet.s, desc->i_ptr, nread, 0);
        if (IS_SOCKET_ERROR(n)) {
            int err = sock_errno();
            if (err == ECONNRESET) {
                DEBUGF((" => detected close (connreset)\r\n"));
                if (desc->tcp_add_flags & TCP_ADDF_SHOW_ECONNRESET)
                    return tcp_recv_error(desc, err);
                else
                    return tcp_recv_closed(desc);
            }
            if (err == ERRNO_BLOCK) {
                DEBUGF((" => would block\r\n"));
                return 0;
            }
            else {
                DEBUGF((" => error: %d\r\n", err));
                return tcp_recv_error(desc, err);
            }
        }
        else if (n == 0) {
            DEBUGF(("  => detected close\r\n"));
            return tcp_recv_closed(desc);
        }
        ASSERT(nread >= n);
        DEBUGF((" => got %d bytes\r\n", n));
        desc->i_ptr += n;
        if (desc->i_remain > 0) {
            desc->i_remain -= n;
            if (desc->i_remain == 0)
                return tcp_deliver(desc, desc->i_ptr - desc->i_ptr_start);
            else
                return 0;
        }
        else {
            int len;
            if ((nread = tcp_remain(desc, &len)) < 0)
                return tcp_recv_error(desc, EMSGSIZE);
            else if (nread == 0)
                return tcp_deliver(desc, len);
            else if (0 < len)
                desc->i_remain = nread;
        }
    }
}
#ifdef __WIN32__
static int winsock_event_select(inet_descriptor *desc, int flags, int on)
{
    int save_event_mask = desc->event_mask;
    desc->forced_events &= FD_CLOSE;
    if (on)
	desc->event_mask |= flags;
    else
	desc->event_mask &= (~flags);
    DEBUGF(("port %d: winsock_event_select: "
	    "flags=%02X, on=%d, event_mask=%02X\n",
	    desc->port, flags, on, desc->event_mask));
    if (WSAEventSelect(desc->s, NULL, 0) != 0) {
	DEBUGF(("port %d: winsock_event_select: "
		"WSAEventSelect returned error, code %d.\n",
		sock_errno()));
	desc->event_mask = save_event_mask;
	return -1;
    }
    if (!ResetEvent(desc->event)) {
	DEBUGF(("port %d: winsock_event_select: "
		"ResetEvent returned error, code %d.\n",
		GetLastError()));
	desc->event_mask = 0;
	return -1;
    }
    if (desc->event_mask != 0) {
	if (WSAEventSelect(desc->s,
			     desc->event,
			     desc->event_mask) != 0) {
	    DEBUGF(("port %d: winsock_event_select: "
		    "WSAEventSelect returned error, code %d.\n",
		    sock_errno()));
	    desc->event_mask = 0;
	    return -1;
	}
	if ((desc->event_mask & FD_WRITE)) {
	    int do_force = 1;
	    if (desc->send_would_block) {
		TIMEVAL tmo = {0,0};
		FD_SET fds;
		int ret;
		FD_ZERO(&fds);
		FD_SET(desc->s,&fds);
		do_force = (select(desc->s+1,0,&fds,0,&tmo) > 0);
	    }
	    if (do_force) {
		SetEvent(desc->event);
		desc->forced_events |= FD_WRITE;
	    }
	}
	if ((desc->event_mask & (FD_READ|FD_CLOSE))) {
	    int readable = 0;
	    int closed = 0;
	    TIMEVAL tmo = {0,0};
	    FD_SET fds;
	    int ret;
	    unsigned long arg;
	    FD_ZERO(&fds);
	    FD_SET(desc->s,&fds);
	    ret = select(desc->s+1,&fds,0,0,&tmo);
	    if (ret > 0) {
		++readable;
		if (ioctlsocket(desc->s,FIONREAD,&arg) != 0) {
		    ++closed;
		} else {
		    closed = (arg == 0);
		}
	    }
	    if ((desc->event_mask & FD_READ) && readable && !closed) {
		SetEvent(desc->event);
		desc->forced_events |= FD_READ;
	    }
	    if ((desc->event_mask & FD_CLOSE) && closed) {
		SetEvent(desc->event);
		desc->forced_events |= FD_CLOSE;
	    }
	}
    }
    return 0;
}
static void tcp_inet_event(ErlDrvData e, ErlDrvEvent event)
{
    tcp_descriptor* desc = (tcp_descriptor*)e;
    WSANETWORKEVENTS netEv;
    int err;
    DEBUGF(("tcp_inet_event(%p) {s=%d\r\n",
	    desc->inet.port, desc->inet.s));
    if (WSAEnumNetworkEvents(desc->inet.s, desc->inet.event,
					&netEv) != 0) {
	DEBUGF((" => EnumNetworkEvents = %d\r\n", sock_errno() ));
	goto error;
    }
    DEBUGF((" => event=%02X, mask=%02X, forced=%02X\r\n",
	    netEv.lNetworkEvents, desc->inet.event_mask,
            desc->inet.forced_events));
    netEv.lNetworkEvents |= desc->inet.forced_events;
    if (desc->inet.forced_events & FD_CLOSE)
        netEv.iErrorCode[FD_CLOSE_BIT] = desc->inet.err;
#ifdef DEBUG
    if ((netEv.lNetworkEvents & ~(desc->inet.event_mask)) != 0) {
	DEBUGF(("port %d:  ... unexpected event: %d\r\n",
		desc->inet.port, netEv.lNetworkEvents & ~(desc->inet.event_mask)));
    }
#endif
    netEv.lNetworkEvents &= desc->inet.event_mask;
    if (netEv.lNetworkEvents & FD_READ) {
	if (tcp_inet_input(desc, event) < 0) {
	    goto error;
	}
	if (netEv.lNetworkEvents & FD_CLOSE) {
            if ((desc->inet.forced_events & FD_CLOSE) == 0) {
                desc->inet.forced_events |= FD_CLOSE;
                desc->inet.err = netEv.iErrorCode[FD_CLOSE_BIT];
            }
	    for (;;) {
                if (!desc->inet.active && desc->inet.opt == NULL) {
                    netEv.lNetworkEvents &= ~FD_CLOSE;
                    break;
                }
		DEBUGF(("Retrying read due to FD_CLOSE\r\n"));
		if (tcp_inet_input(desc, event) < 0) {
		    goto error;
		}
	    }
	}
    }
    if (netEv.lNetworkEvents & FD_WRITE) {
	desc->inet.send_would_block = 0;
	if (tcp_inet_output(desc, event) < 0)
	    goto error;
    }
    if (netEv.lNetworkEvents & FD_CONNECT) {
	if ((err = netEv.iErrorCode[FD_CONNECT_BIT]) != 0) {
	    async_error(INETP(desc), err);
	} else {
	    tcp_inet_output(desc, event);
	}
    } else if (netEv.lNetworkEvents & FD_ACCEPT) {
	if ((err = netEv.iErrorCode[FD_ACCEPT_BIT]) != 0)
	    async_error(INETP(desc), err);
	else
	    tcp_inet_input(desc, event);
    }
    if (netEv.lNetworkEvents & FD_CLOSE) {
	DEBUGF(("Detected close in %s, line %d\r\n", __LINE__));
	if (desc->tcp_add_flags & TCP_ADDF_SHOW_ECONNRESET) {
	    err = netEv.iErrorCode[FD_CLOSE_BIT];
	    if (err == ECONNRESET)
		tcp_recv_error(desc, err);
	    else if (err == ECONNABORTED && IS_CONNECTED(INETP(desc))) {
		tcp_recv_error(desc, ECONNRESET);
	    }
	    else
		tcp_recv_closed(desc);
	}
	else
	    tcp_recv_closed(desc);
    }
    DEBUGF(("tcp_inet_event(%p) }\r\n", desc->inet.port));
    return;
 error:
    DEBUGF(("tcp_inet_event(%p) error}\r\n",
            desc->inet.port));
    return;
}
#endif
static int tcp_inet_input(tcp_descriptor* desc, HANDLE event)
{
    int ret = 0;
#ifdef DEBUG
    void *port = desc->inet.port;
#endif
    ASSERT(!INET_IGNORED(INETP(desc)));
    DEBUGF(("tcp_inet_input(%p) {s=%d\r\n", port, desc->inet.s));
    if (desc->inet.state == INET_STATE_ACCEPTING) {
	SOCKET s;
	unsigned int len;
	inet_address remote;
	inet_async_op *this_op = desc->inet.opt;
	len = sizeof(desc->inet.remote);
	sys_memzero((char *) &remote, len);
	s = sock_accept(desc->inet.s, (struct sockaddr*) &remote, &len);
	if (s == INVALID_SOCKET && sock_errno() == ERRNO_BLOCK) {
	    goto done;
	}
	sock_select(INETP(desc),FD_ACCEPT,0);
	desc->inet.state = INET_STATE_LISTENING;
	if (this_op != NULL) {
	    driver_demonitor_process(desc->inet.port, &(this_op->monitor));
	}
	driver_cancel_timer(desc->inet.port);
	if (s == INVALID_SOCKET) {
	    ret = async_error(INETP(desc), sock_errno());
	    goto done;
	}
	else {
	    ErlDrvTermData caller;
	    tcp_descriptor* accept_desc;
	    int err;
	    if (desc->inet.opt == NULL) {
		sock_close(s);
		ret = async_error(INETP(desc), EINVAL);
		goto done;
	    }
	    caller = desc->inet.opt->caller;
	    if ((accept_desc = tcp_inet_copy(desc,s,caller,&err)) == NULL) {
		sock_close(s);
		ret = async_error(INETP(desc), err);
		goto done;
	    }
	    accept_desc->inet.remote = remote;
	    SET_NONBLOCKING(accept_desc->inet.s);
#ifdef __WIN32__
	    driver_select(accept_desc->inet.port, accept_desc->inet.event,
			  ERL_DRV_READ, 1);
#endif
	    accept_desc->inet.state = INET_STATE_CONNECTED;
	    ret =  async_ok_port(INETP(desc), accept_desc->inet.dport);
	    goto done;
	}
    } else if (desc->inet.state == INET_STATE_MULTI_ACCEPTING) {
	SOCKET s;
	unsigned int len;
	inet_address remote;
	int id,req;
	ErlDrvTermData caller;
	MultiTimerData *timeout;
	ErlDrvMonitor monitor;
#ifdef HARDDEBUG
	int times = 0;
#endif
	while (desc->inet.state == INET_STATE_MULTI_ACCEPTING) {
	    len = sizeof(desc->inet.remote);
	    sys_memzero((char *) &remote, len);
	    s = sock_accept(desc->inet.s, (struct sockaddr*) &remote, &len);
	    if (s == INVALID_SOCKET && sock_errno() == ERRNO_BLOCK) {
		goto done;
	    }
#ifdef HARDDEBUG
	    if (++times > 1) {
		erts_fprintf(stderr,"Accepts in one suite: %d :-)\r\n",times);
	    }
#endif
	    if (deq_multi_op(desc,&id,&req,&caller,&timeout,&monitor) != 0) {
		ret = -1;
		goto done;
	    }
	    if (desc->multi_first == NULL) {
		sock_select(INETP(desc),FD_ACCEPT,0);
		desc->inet.state = INET_STATE_LISTENING;
	    }
	    if (timeout != NULL) {
		remove_multi_timer(desc, desc->inet.port, timeout);
	    }
	    driver_demonitor_process(desc->inet.port, &monitor);
	    if (s == INVALID_SOCKET) {
		ret = send_async_error(desc->inet.dport,
				       id, caller, error_atom(sock_errno()));
		goto done;
	    }
	    else {
		tcp_descriptor* accept_desc;
		int err;
		if ((accept_desc = tcp_inet_copy(desc,s,caller,&err)) == NULL) {
		    sock_close(s);
		    ret = send_async_error(desc->inet.dport,
					   id, caller, error_atom(err));
		    goto done;
		}
		accept_desc->inet.remote = remote;
		SET_NONBLOCKING(accept_desc->inet.s);
#ifdef __WIN32__
		driver_select(accept_desc->inet.port, accept_desc->inet.event,
			      ERL_DRV_READ, 1);
#endif
		accept_desc->inet.state = INET_STATE_CONNECTED;
		ret =  send_async_ok_port(desc->inet.dport,
					  id, caller, accept_desc->inet.dport);
	    }
	}
    }
    else if (IS_CONNECTED(INETP(desc))) {
	ret = tcp_recv(desc, 0);
	goto done;
    }
    else {
	sock_select(INETP(desc),FD_ACCEPT,0);
	DEBUGF(("tcp_inet_input(%p): s=%d bad state: %04x\r\n",
		port, desc->inet.s, desc->inet.state));
    }
 done:
    DEBUGF(("tcp_inet_input(%p) }\r\n", port));
    return ret;
}
static int tcp_send_or_shutdown_error(tcp_descriptor* desc, int err)
{
    int show_econnreset = (err == ECONNRESET
			   && desc->tcp_add_flags & TCP_ADDF_SHOW_ECONNRESET);
    if (IS_BUSY(INETP(desc))) {
	if (desc->busy_on_send) {
            cancel_multi_timer(desc, INETP(desc)->port, &tcp_inet_send_timeout);
	    desc->busy_on_send = 0;
	}
	desc->inet.state &= ~INET_F_BUSY;
	set_busy_port(desc->inet.port, 0);
    }
    DEBUGF(("driver_failure_eof(%p) in %s, line %d\r\n",
	    desc->inet.port, __LINE__));
    if (desc->inet.active) {
        ErlDrvTermData err_atom;
	if (show_econnreset) {
	    tcp_error_message(desc, err);
            err_atom = error_atom(err);
	} else {
            err_atom = am_closed;
	}
        tcp_closed_message(desc);
        if (! (desc->tcp_add_flags & TCP_ADDF_SENDFILE))
            inet_reply_error_am(INETP(desc), err_atom);
	if (desc->inet.exitf)
	    driver_exit(desc->inet.port, 0);
	else
	    tcp_desc_close(desc);
    } else {
	tcp_close_check(desc);
        if (is_not_internal_pid(desc->inet.caller)) {
	    desc->tcp_add_flags |= TCP_ADDF_DELAYED_CLOSE_SEND;
        }
        else if (! (desc->tcp_add_flags & TCP_ADDF_SENDFILE)) {
            if (show_econnreset)
                inet_reply_error(INETP(desc), err);
            else
                inet_reply_error_am(INETP(desc), am_closed);
        }
        tcp_desc_close(desc);
	desc->tcp_add_flags |= TCP_ADDF_DELAYED_CLOSE_RECV;
	if (show_econnreset) {
	    desc->tcp_add_flags |= TCP_ADDF_DELAYED_ECONNRESET;
	}
    }
    return -1;
}
static int tcp_send_error(tcp_descriptor* desc, int err)
{
#ifdef __WIN32__
    if (err == ECONNABORTED)
	err = ECONNRESET;
#endif
    if (err == EPIPE && !(desc->tcp_add_flags & TCP_ADDF_SHUTDOWN_WR_DONE))
	err = ECONNRESET;
    return tcp_send_or_shutdown_error(desc, err);
}
static int tcp_shutdown_error(tcp_descriptor* desc, int err)
{
    return tcp_send_or_shutdown_error(desc, err);
}
static int tcp_inet_delay_send(ErlDrvData data, ErlDrvTermData dummy)
{
    tcp_descriptor *desc = (tcp_descriptor*)data;
    return tcp_inet_output(desc, (HANDLE) INETP(desc)->s);
}
static int tcp_sendv(tcp_descriptor* desc, ErlIOVec* ev)
{
    ErlDrvSizeT sz;
    char buf[4];
    ErlDrvSizeT h_len;
    ssize_t n;
    ErlDrvPort ix = desc->inet.port;
    ErlDrvSizeT len = ev->size;
     switch(desc->inet.htype) {
     case TCP_PB_1:
         put_int8(len, buf);
         h_len = 1;
         break;
     case TCP_PB_2:
         put_int16(len, buf);
         h_len = 2;
         break;
     case TCP_PB_4:
         put_int32(len, buf);
         h_len = 4;
         break;
     default:
         h_len = 0;
         break;
     }
    inet_output_count(INETP(desc), len+h_len);
    if (h_len > 0) {
	ev->iov[0].iov_base = buf;
	ev->iov[0].iov_len = h_len;
	ev->size += h_len;
    }
    sz = driver_sizeq(ix);
    if ((desc->tcp_add_flags & TCP_ADDF_SENDFILE) || sz > 0) {
	driver_enqv(ix, ev, 0);
	if (sz+ev->size >= desc->high) {
            if (desc->send_timeout == 0) {
                inet_reply_error_am(INETP(desc), am_timeout);
                return 1;
            }
	    DEBUGF(("tcp_sendv(%p): s=%d, sender forced busy\r\n",
		    desc->inet.port, desc->inet.s));
	    desc->inet.state |= INET_F_BUSY;
	    set_busy_port(desc->inet.port, 1);
	    if (desc->send_timeout != INET_INFINITY) {
		desc->busy_on_send = 1;
                add_multi_timer(desc, INETP(desc)->port,
                                am_undefined,
                                desc->send_timeout ,
                                &tcp_inet_send_timeout);
	    }
	    return 1;
	}
    }
    else {
	int vsize;
        if ((h_len == 0) && (len == 0))
            return 0;
        vsize = (ev->vsize > MAX_VSIZE) ? MAX_VSIZE : ev->vsize;
        DEBUGF(("tcp_sendv(%p): s=%d, "
                "about to send "LLU","LLU" bytes\r\n",
		desc->inet.port, desc->inet.s,
                (llu_t)h_len, (llu_t)len));
	if (INET_IGNORED(INETP(desc))) {
	    INETP(desc)->flags |= INET_IGNORE_WRITE;
	    n = 0;
	} else if (desc->tcp_add_flags & TCP_ADDF_DELAY_SEND) {
            driver_enqv(ix, ev, 0);
            add_multi_timer(desc, INETP(desc)->port, am_undefined,
                            0, &tcp_inet_delay_send);
	    return 0;
	} else if (IS_SOCKET_ERROR(sock_sendv(desc->inet.s, ev->iov,
					      vsize, &n, 0))) {
	    if ((sock_errno() != ERRNO_BLOCK) && (sock_errno() != EINTR)) {
		int err = sock_errno();
		DEBUGF(("tcp_sendv(%p): s=%d, "
			"sock_sendv(size=2) errno = %d\r\n",
			desc->inet.port, desc->inet.s, err));
		return tcp_send_error(desc, err);
	    }
#ifdef __WIN32__
	    desc->inet.send_would_block = 1;
#endif
	    n = 0;
	}
	else if (n == ev->size) {
	    ASSERT(NO_SUBSCRIBERS(&INETP(desc)->empty_out_q_subs));
	    return 0;
	}
	else {
	    DEBUGF(("tcp_sendv(%p): s=%d, only sent "
		    LLU"/%d of "LLU"/%d bytes/items\r\n",
		    desc->inet.port, desc->inet.s,
		    (llu_t)n, vsize, (llu_t)ev->size, ev->vsize));
	}
	DEBUGF(("tcp_sendv(%p): s=%d, Send failed, queuing\r\n",
		desc->inet.port, desc->inet.s));
	driver_enqv(ix, ev, n);
	if (!INET_IGNORED(INETP(desc)))
	    sock_select(INETP(desc),(FD_WRITE|FD_CLOSE), 1);
    }
    return 0;
}
static int tcp_send(tcp_descriptor* desc, char* ptr, ErlDrvSizeT len)
{
    ErlDrvSizeT sz;
    char buf[4];
    int h_len;
    int n;
    ErlDrvPort ix = desc->inet.port;
    SysIOVec iov[2];
    switch(desc->inet.htype) {
    case TCP_PB_1:
	put_int8(len, buf);
	h_len = 1;
	break;
    case TCP_PB_2:
	put_int16(len, buf);
	h_len = 2;
	break;
    case TCP_PB_4:
	put_int32(len, buf);
	h_len = 4;
	break;
    default:
	h_len = 0;
	break;
    }
    inet_output_count(INETP(desc), len+h_len);
    sz = driver_sizeq(ix);
    if ((desc->tcp_add_flags & TCP_ADDF_SENDFILE) || sz > 0) {
	if (h_len > 0)
	    driver_enq(ix, buf, h_len);
	driver_enq(ix, ptr, len);
	if (sz+h_len+len >= desc->high) {
            if (desc->send_timeout == 0) {
                inet_reply_error_am(INETP(desc), am_timeout);
                return 1;
            }
	    DEBUGF(("tcp_send(%p): s=%d, sender forced busy\r\n",
		    desc->inet.port, desc->inet.s));
	    desc->inet.state |= INET_F_BUSY;
	    set_busy_port(desc->inet.port, 1);
	    if (desc->send_timeout != INET_INFINITY) {
		desc->busy_on_send = 1;
                add_multi_timer(desc, INETP(desc)->port,
                                am_undefined ,
                                desc->send_timeout ,
                                &tcp_inet_send_timeout);
	    }
	    return 1;
	}
    }
    else {
        if ((h_len == 0) && (len == 0))
            return 0;
	iov[0].iov_base = buf;
	iov[0].iov_len = h_len;
	iov[1].iov_base = ptr;
	iov[1].iov_len = len;
	DEBUGF(("tcp_send(%p): s=%d, "
                "about to send "LLU","LLU" bytes\r\n",
		desc->inet.port, desc->inet.s,
                (llu_t)h_len, (llu_t)len));
	if (INET_IGNORED(INETP(desc))) {
	    INETP(desc)->flags |= INET_IGNORE_WRITE;
	    n = 0;
	} else if (desc->tcp_add_flags & TCP_ADDF_DELAY_SEND) {
	    sock_send(desc->inet.s, buf, 0, 0);
	    n = 0;
	} else 	if (IS_SOCKET_ERROR(sock_sendv(desc->inet.s,iov,2,&n,0))) {
	    if ((sock_errno() != ERRNO_BLOCK) && (sock_errno() != EINTR)) {
		int err = sock_errno();
		DEBUGF(("tcp_send(%p): s=%d, "
                        "sock_sendv(size=2) errno = %d\r\n",
			desc->inet.port, desc->inet.s, err));
		return tcp_send_error(desc, err);
	    }
#ifdef __WIN32__
	    desc->inet.send_would_block = 1;
#endif
	    n = 0;
	}
	else if (n == len+h_len) {
	    ASSERT(NO_SUBSCRIBERS(&INETP(desc)->empty_out_q_subs));
	    return 0;
	}
	DEBUGF(("tcp_send(%p): s=%d, Send failed, queuing",
		desc->inet.port, desc->inet.s));
	if (n < h_len) {
	    driver_enq(ix, buf+n, h_len-n);
	    driver_enq(ix, ptr, len);
	}
	else {
	    n -= h_len;
	    driver_enq(ix, ptr+n, len-n);
	}
	if (!INET_IGNORED(INETP(desc)))
	    sock_select(INETP(desc),(FD_WRITE|FD_CLOSE), 1);
    }
    return 0;
}
static void tcp_shutdown_async(tcp_descriptor* desc)
{
    int how;
    how = (desc->tcp_add_flags & TCP_ADDF_PENDING_SHUT_WR) ?
		TCP_SHUT_WR : TCP_SHUT_RDWR;
    if (IS_SOCKET_ERROR(sock_shutdown(INETP(desc)->s, how)))
	tcp_shutdown_error(desc, sock_errno());
    else
	desc->tcp_add_flags |= TCP_ADDF_SHUTDOWN_WR_DONE;
}
static void tcp_inet_drv_output(ErlDrvData data, ErlDrvEvent event)
{
    (void)tcp_inet_output((tcp_descriptor*)data, (HANDLE)event);
}
static void tcp_inet_drv_input(ErlDrvData data, ErlDrvEvent event)
{
    (void)tcp_inet_input((tcp_descriptor*)data, (HANDLE)event);
}
#ifdef HAVE_SENDFILE
static int tcp_sendfile_completed(tcp_descriptor* desc) {
    ErlDrvTermData spec[LOAD_PORT_CNT + LOAD_TUPLE_CNT * 2 +
        LOAD_ATOM_CNT * 2 + LOAD_UINT_CNT * 2];
    Uint32 sent_low, sent_high;
    int i;
    desc->tcp_add_flags &= ~TCP_ADDF_SENDFILE;
    close(desc->sendfile.dup_file_fd);
    DEBUGF(("tcp_sendfile_completed(%p): SENDFILE dup closed %d\r\n",
            desc->inet.port, desc->sendfile.dup_file_fd));
    if (driver_sizeq(desc->inet.port) <= desc->low) {
        if (IS_BUSY(INETP(desc))) {
            desc->inet.state &= ~INET_F_BUSY;
            set_busy_port(desc->inet.port, 0);
            if (desc->busy_on_send) {
                cancel_multi_timer(desc, INETP(desc)->port,
                                   &tcp_inet_send_timeout);
                desc->busy_on_send = 0;
            }
            inet_reply_ok(INETP(desc));
        }
    }
    if (driver_sizeq(desc->inet.port) == 0) {
        sock_select(INETP(desc), FD_WRITE, 0);
        send_empty_out_q_msgs(INETP(desc));
        if (desc->tcp_add_flags & TCP_ADDF_PENDING_SHUTDOWN) {
            tcp_shutdown_async(desc);
        }
    }
    sent_low = ((Uint64)desc->sendfile.bytes_sent >> 0) & 0xFFFFFFFF;
    sent_high = ((Uint64)desc->sendfile.bytes_sent >> 32) & 0xFFFFFFFF;
    i = LOAD_ATOM(spec, 0, am_sendfile);
    i = LOAD_PORT(spec, i, desc->inet.dport);
    i = LOAD_ATOM(spec, i, am_ok);
    i = LOAD_UINT(spec, i, sent_low);
    i = LOAD_UINT(spec, i, sent_high);
    i = LOAD_TUPLE(spec, i, 3);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    return erl_drv_output_term(desc->inet.dport, spec, i);
}
static int tcp_sendfile_aborted(tcp_descriptor* desc, int socket_error) {
    ErlDrvTermData spec[LOAD_PORT_CNT + LOAD_TUPLE_CNT * 2 + LOAD_ATOM_CNT * 3];
    int i;
    i = LOAD_ATOM(spec, 0, am_sendfile);
    i = LOAD_PORT(spec, i, desc->inet.dport);
    i = LOAD_ATOM(spec, i, am_error);
    switch (socket_error) {
    case ECONNRESET:
    case ENOTCONN:
    case EPIPE:
        i = LOAD_ATOM(spec, i, am_closed);
        break;
    default:
        i = LOAD_ATOM(spec, i, error_atom(socket_error));
    }
    i = LOAD_TUPLE(spec, i, 2);
    i = LOAD_TUPLE(spec, i, 3);
    ASSERT(i == sizeof(spec)/sizeof(*spec));
    return erl_drv_output_term(desc->inet.dport, spec, i);
}
static int tcp_inet_sendfile(tcp_descriptor* desc) {
    ErlDrvPort ix = desc->inet.port;
    int result = 0;
    ssize_t n;
    DEBUGF(("tcp_inet_sendfile(%p) {s=%d\r\n",
            ix, desc->inet.s));
    while (desc->sendfile.ioq_skip > 0) {
        ssize_t bytes_to_send;
        SysIOVec* iov;
        int vsize;
        ASSERT(driver_sizeq(ix) >= desc->sendfile.ioq_skip);
        if ((iov = driver_peekq(ix, &vsize)) == NULL) {
            ERTS_INTERNAL_ERROR("ioq empty when sendfile.ioq_skip > 0");
        }
        bytes_to_send = MIN(desc->sendfile.ioq_skip, iov[0].iov_len);
        n = sock_send(desc->inet.s, iov[0].iov_base, bytes_to_send, 0);
        if (!IS_SOCKET_ERROR(n)) {
            desc->sendfile.ioq_skip -= n;
            driver_deq(ix, n);
        } else if (sock_errno() == ERRNO_BLOCK) {
#ifdef __WIN32__
            desc->inet.send_would_block = 1;
#endif
            goto done;
        } else if (sock_errno() != EINTR) {
            goto socket_error;
        }
    }
    while (desc->sendfile.length > 0) {
        const Sint64 SENDFILE_CHUNK_SIZE = ((1UL << 30) - 1);
        ssize_t bytes_to_send = MIN(SENDFILE_CHUNK_SIZE, desc->sendfile.length);
        off_t offset = desc->sendfile.offset;
#if defined(__linux__)
        n = sendfile(desc->inet.s, desc->sendfile.dup_file_fd, &offset,
            bytes_to_send);
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__DARWIN__)
        {
            off_t bytes_sent;
            int error;
    #if defined(__DARWIN__)
            bytes_sent = bytes_to_send;
            error = sendfile(desc->sendfile.dup_file_fd, desc->inet.s, offset,
                &bytes_sent, NULL, 0);
            n = bytes_sent;
    #else
            error = sendfile(desc->sendfile.dup_file_fd, desc->inet.s, offset,
                bytes_to_send, NULL, &bytes_sent, 0);
            n = bytes_sent;
    #endif
            if(error < 0) {
                if((errno != EAGAIN && errno != EINTR) || bytes_sent == 0) {
                    n = -1;
                }
            }
        }
#elif defined(__sun) && defined(__SVR4) && defined(HAVE_SENDFILEV)
        {
            sendfilevec_t sfvec[1];
            size_t bytes_sent;
            ssize_t error;
            sfvec[0].sfv_fd = desc->sendfile.dup_file_fd;
            sfvec[0].sfv_len = bytes_to_send;
            sfvec[0].sfv_off = offset;
            sfvec[0].sfv_flag = 0;
            error = sendfilev(desc->inet.s, sfvec, 1, &bytes_sent);
            n = bytes_sent;
            if(error < 0) {
                if(errno == EINVAL) {
                } else {
                    if((errno != EAGAIN && errno != EINTR) || bytes_sent == 0) {
                        n = -1;
                    }
                }
            }
        }
#else
        #error "Unsupported sendfile syscall; update configure test."
#endif
        if (n > 0) {
            desc->sendfile.bytes_sent += n;
            desc->sendfile.offset += n;
            desc->sendfile.length -= n;
        } else if (n == 0) {
            desc->sendfile.length = 0;
            break;
        } else if (IS_SOCKET_ERROR(n) && sock_errno() != EINTR) {
            if (sock_errno() != ERRNO_BLOCK) {
                goto socket_error;
            }
#ifdef __WIN32__
            desc->inet.send_would_block = 1;
#endif
            break;
        }
    }
    if (desc->sendfile.length == 0) {
        tcp_sendfile_completed(desc);
    }
    goto done;
socket_error: {
        int socket_errno = sock_errno();
        DEBUGF(("tcp_inet_sendfile(%p): send errno = %d (errno %d)\r\n",
            desc->inet.port, socket_errno, errno));
        tcp_sendfile_aborted(desc, socket_errno);
        result = tcp_send_error(desc, socket_errno);
        goto done;
    }
done:
    DEBUGF(("tcp_inet_sendfile(%p) }\r\n", desc->inet.port));
    return result;
}
#endif
static int tcp_inet_output(tcp_descriptor* desc, HANDLE event)
{
    int ret = 0;
    ErlDrvPort ix = desc->inet.port;
    ASSERT(!INET_IGNORED(INETP(desc)));
    DEBUGF(("tcp_inet_output(%p) {s=%d\r\n",
	    desc->inet.port, desc->inet.s));
    if (desc->inet.state == INET_STATE_CONNECTING) {
	sock_select(INETP(desc),FD_CONNECT,0);
	driver_cancel_timer(ix);
#ifndef __WIN32__
#ifndef SO_ERROR
	{
	    int sz, code;
            sz = sizeof(desc->inet.remote);
            sys_memzero((char *) &desc->inet.remote, sz);
	    code = sock_peer(desc->inet.s,
                             (struct sockaddr*) &desc->inet.remote, &sz);
	    if (IS_SOCKET_ERROR(code)) {
		desc->inet.state = INET_STATE_OPEN;
		ret =  async_error(INETP(desc), sock_errno());
		goto done;
	    }
	}
#else
	{
	    int error = 0;
	    unsigned int sz = sizeof(error);
	    int code = sock_getopt(desc->inet.s, SOL_SOCKET, SO_ERROR,
				   (void *)&error, &sz);
	    if ((code < 0) || error) {
		desc->inet.state = INET_STATE_OPEN;
		ret = async_error(INETP(desc), error);
		goto done;
	    }
	}
#endif
#endif
	desc->inet.state = INET_STATE_CONNECTED;
	if (desc->inet.active)
	    sock_select(INETP(desc),(FD_READ|FD_CLOSE),1);
	async_ok(INETP(desc));
    }
    else if (IS_CONNECTED(INETP(desc))) {
#ifdef HAVE_SENDFILE
        if(desc->tcp_add_flags & TCP_ADDF_SENDFILE) {
            return tcp_inet_sendfile(desc);
        }
#endif
        for (;;) {
	    int vsize;
	    ssize_t n;
	    SysIOVec* iov;
	    if ((iov = driver_peekq(ix, &vsize)) == NULL) {
		sock_select(INETP(desc), FD_WRITE, 0);
		send_empty_out_q_msgs(INETP(desc));
		if (desc->tcp_add_flags & TCP_ADDF_PENDING_SHUTDOWN)
		    tcp_shutdown_async(desc);
		goto done;
	    }
	    vsize = vsize > MAX_VSIZE ? MAX_VSIZE : vsize;
	    DEBUGF(("tcp_inet_output(%p): s=%d, "
                    "About to send %d items\r\n",
		    desc->inet.port, desc->inet.s, vsize));
	    if (IS_SOCKET_ERROR(sock_sendv(desc->inet.s, iov, vsize, &n, 0))) {
	    write_error:
		if ((sock_errno() != ERRNO_BLOCK) && (sock_errno() != EINTR)) {
		    DEBUGF(("tcp_inet_output(%p): "
                            "sock_sendv(%d) errno = %d (errno %d)\r\n",
			    desc->inet.port, vsize,
                            sock_errno(), errno));
		    ret =  tcp_send_error(desc, sock_errno());
		    goto done;
		}
#ifdef __WIN32__
		desc->inet.send_would_block = 1;
#endif
                if (desc->tcp_add_flags & TCP_ADDF_DELAY_SEND)
                    sock_select(INETP(desc), FD_WRITE, 1);
		goto done;
	    } else if (n == 0) {
	      size_t howmuch = 0x7FFFFFFF;
	      int x;
	      for(x = 0; x < vsize && iov[x].iov_len == 0; ++x)
		;
	      if (x < vsize) {
		if (howmuch > iov[x].iov_len) {
		  howmuch = iov[x].iov_len;
		}
		n = sock_send(desc->inet.s, iov[x].iov_base,howmuch,0);
		if (IS_SOCKET_ERROR(n)) {
		  goto write_error;
		}
	      }
	    }
	    if (driver_deq(ix, n) <= desc->low) {
		if (IS_BUSY(INETP(desc))) {
		    desc->inet.state &= ~INET_F_BUSY;
		    set_busy_port(desc->inet.port, 0);
		    if (desc->busy_on_send) {
                        cancel_multi_timer(desc, INETP(desc)->port, &tcp_inet_send_timeout);
			desc->busy_on_send = 0;
		    }
		    inet_reply_ok(INETP(desc));
		}
	    }
	}
    }
    else {
	sock_select(INETP(desc),FD_CONNECT,0);
	DEBUGF(("tcp_inet_output(%p): bad state: %04x\r\n",
		desc->inet.port, desc->inet.state));
    }
 done:
    DEBUGF(("tcp_inet_output(%p) }\r\n", desc->inet.port));
    return ret;
}
#if defined(HAVE_SO_BSDCOMPAT)
#if defined(__linux__)
#include <sys/utsname.h>
static int should_use_so_bsdcompat(void)
{
    static int init_done;
    static int so_bsdcompat_is_obsolete;
    if (!init_done) {
	struct utsname utsname;
	unsigned int version, patchlevel;
	init_done = 1;
	if (uname(&utsname) < 0) {
	    fprintf(stderr, "uname: %s\r\n", strerror(sock_errno()));
	    return 1;
	}
	if (sscanf(utsname.release, "%u.%u", &version, &patchlevel) != 2) {
	    fprintf(stderr, "uname: unexpected release '%s'\r\n",
		    utsname.release);
	    return 1;
	}
	if (version > 2 || (version == 2 && patchlevel >= 5))
	    so_bsdcompat_is_obsolete = 1;
    }
    return !so_bsdcompat_is_obsolete;
}
#else
#define should_use_so_bsdcompat() 1
#endif
#endif
#ifdef HAVE_SCTP
static ErlDrvData packet_inet_start(ErlDrvPort port, char* args, int protocol);
static udp_descriptor* sctp_inet_copy(udp_descriptor* desc, SOCKET s,
                                      ErlDrvTermData owner, int* err)
{
    ErlDrvSizeT q_low, q_high;
    ErlDrvPort port = desc->inet.port;
    udp_descriptor* copy_desc;
    copy_desc = (udp_descriptor*) packet_inet_start(port, NULL, IPPROTO_SCTP);
    if ((copy_desc->inet.s = s) != INVALID_SOCKET) {
	if ((copy_desc->inet.event = sock_create_event(INETP(copy_desc))) ==
	    INVALID_EVENT) {
	    *err = sock_errno();
	    FREE(copy_desc);
	    return NULL;
	}
    }
    copy_desc->inet.mode         = desc->inet.mode;
    copy_desc->inet.exitf        = desc->inet.exitf;
    copy_desc->inet.deliver      = desc->inet.deliver;
    copy_desc->inet.htype        = desc->inet.htype;
    copy_desc->inet.psize        = desc->inet.psize;
    copy_desc->inet.stype        = desc->inet.stype;
    copy_desc->inet.sfamily      = desc->inet.sfamily;
    copy_desc->inet.hsz          = desc->inet.hsz;
    copy_desc->inet.bufsz        = desc->inet.bufsz;
    copy_desc->inet.flags = desc->inet.flags & INET_FLG_NON_BLOCK_SEND;
    port = driver_create_port(port, owner, "sctp_inet",
			      (ErlDrvData) copy_desc);
    if ((long)port == -1) {
	*err = ENFILE;
	FREE(copy_desc);
	return NULL;
    }
    q_low = q_high = ERL_DRV_BUSY_MSGQ_READ_ONLY;
    erl_drv_busy_msgq_limits(desc->inet.port, &q_low, &q_high);
    erl_drv_busy_msgq_limits(port, &q_low, &q_high);
    copy_desc->inet.port = port;
    copy_desc->inet.dport = driver_mk_port(port);
    *err = 0;
    return copy_desc;
}
#endif
#ifdef HAVE_UDP
static int packet_inet_init(void)
{
    sys_memzero((char *)&disassoc_sa, sizeof(disassoc_sa));
#ifdef AF_UNSPEC
    disassoc_sa.sa_family = AF_UNSPEC;
#endif
    disassoc_sa_size = offsetof(struct sockaddr, sa_data);
    return 0;
}
static ErlDrvData packet_inet_start(ErlDrvPort port, char* args, int protocol)
{
    ErlDrvData	    drvd = inet_start(port, sizeof(udp_descriptor),
				      protocol);
    udp_descriptor* desc = (udp_descriptor*) drvd;
    if (desc == NULL)
	return ERL_DRV_ERROR_ERRNO;
    desc->read_packets = INET_PACKET_POLL;
    desc->i_bufsz = 0;
    desc->i_buf = NULL;
    desc->i_ptr = NULL;
    return drvd;
}
static ErlDrvData udp_inet_start(ErlDrvPort port, char *args)
{
    ErlDrvData data = packet_inet_start(port, args, IPPROTO_UDP);
    set_default_msgq_limits(port);
    return data;
}
#endif
#ifdef HAVE_SCTP
static ErlDrvData sctp_inet_start(ErlDrvPort port, char *args)
{
    ErlDrvData data = packet_inet_start(port, args, IPPROTO_SCTP);
    set_default_msgq_limits(port);
    return data;
}
#endif
#ifdef HAVE_UDP
static void packet_inet_stop(ErlDrvData e)
{
    udp_descriptor * udesc = (udp_descriptor*) e;
    inet_descriptor* desc  = INETP(udesc);
    if (udesc->i_buf != NULL) {
	release_buffer(udesc->i_buf);
	udesc->i_buf = NULL;
    }
    ASSERT(NO_SUBSCRIBERS(&(desc->empty_out_q_subs)));
    async_error_am_all(desc, am_closed);
    inet_stop(desc);
}
static int packet_error(udp_descriptor* udesc, int err)
{
    inet_descriptor * desc = INETP(udesc);
    if (!desc->active)
	async_error(desc, err);
    driver_failure_posix(desc->port, err);
    return -1;
}
static ErlDrvSSizeT packet_inet_ctl(ErlDrvData e, unsigned int cmd, char* buf,
				    ErlDrvSizeT len, char** rbuf, ErlDrvSizeT rsize)
{
    ErlDrvSSizeT replen;
    udp_descriptor * udesc = (udp_descriptor *) e;
    inet_descriptor* desc  = INETP(udesc);
    int af = AF_INET;
    int type = SOCK_DGRAM;
    int protocol;
    cmd -= ERTS_INET_DRV_CONTROL_MAGIC_NUMBER;
    switch(cmd) {
    case INET_REQ_OPEN:
	DDBG(desc,
	     ("INET-DRV-DBG[%d][%T] packet_inet_ctl -> OPEN\r\n",
	      __LINE__, driver_caller(desc->port)) );
	if (len != 3) {
	    return ctl_error(EINVAL, rbuf, rsize);
	}
	switch (buf[0]) {
	case INET_AF_INET:  af = AF_INET; break;
#if defined(HAVE_IN6) && defined(AF_INET6)
	case INET_AF_INET6: af = AF_INET6; break;
#endif
#ifdef HAVE_SYS_UN_H
	case INET_AF_LOCAL: af = AF_UNIX; break;
#endif
	default:
	    return ctl_xerror(str_eafnosupport, rbuf, rsize);
	}
	switch (buf[1]) {
	case INET_TYPE_STREAM: type = SOCK_STREAM; break;
	case INET_TYPE_DGRAM: type = SOCK_DGRAM; break;
#ifdef HAVE_SCTP
	case INET_TYPE_SEQPACKET: type = SOCK_SEQPACKET; break;
#endif
	default:
	    return ctl_error(EINVAL, rbuf, rsize);
	}
        switch(buf[2]) {
        case INET_PROTO_DEFAULT: protocol = 0; break;
        case INET_PROTO_UDP: protocol = IPPROTO_UDP; break;
#ifdef HAVE_SCTP
        case INET_PROTO_SCTP: protocol = IPPROTO_SCTP; break;
#endif
        default:
            return ctl_xerror(str_eprotonosupport, rbuf, rsize);
        }
	replen = inet_ctl_open(desc, af, type, protocol, rbuf, rsize);
	if ((*rbuf)[0] != INET_REP_ERROR) {
	    if (desc->active)
		sock_select(desc,FD_READ,1);
#ifdef HAVE_SO_BSDCOMPAT
	    if (should_use_so_bsdcompat()) {
		int one = 1;
		sock_setopt(desc->s, SOL_SOCKET, SO_BSDCOMPAT, &one,
			    sizeof(one));
	    }
#endif
	}
	return replen;
    case INET_REQ_FDOPEN: {
	SOCKET s;
        int bound;
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_ctl -> FDOPEN\r\n",
	      __LINE__, desc->s, driver_caller(desc->port)) );
	if (len != 6 && len != 10) {
	    return ctl_error(EINVAL, rbuf, rsize);
	}
	switch (buf[0]) {
	case INET_AF_INET:  af = AF_INET; break;
#if defined(HAVE_IN6) && defined(AF_INET6)
	case INET_AF_INET6: af = AF_INET6; break;
#endif
#ifdef HAVE_SYS_UN_H
	case INET_AF_LOCAL: af = AF_UNIX; break;
#endif
	default:
	    return ctl_xerror(str_eafnosupport, rbuf, rsize);
	}
	switch (buf[1]) {
	case INET_TYPE_STREAM: type = SOCK_STREAM; break;
	case INET_TYPE_DGRAM: type = SOCK_DGRAM; break;
#ifdef HAVE_SCTP
	case INET_TYPE_SEQPACKET: type = SOCK_SEQPACKET; break;
#endif
	default:
	    return ctl_error(EINVAL, rbuf, rsize);
	}
	s = (SOCKET)get_int32(buf+2);
        if (len == 6) bound = 1;
        else bound = get_int32(buf+2+4);
	replen = inet_ctl_fdopen(desc, af, type, s, bound, rbuf, rsize);
	if ((*rbuf)[0] != INET_REP_ERROR) {
	    if (desc->active)
		sock_select(desc,FD_READ,1);
#ifdef HAVE_SO_BSDCOMPAT
	    if (should_use_so_bsdcompat()) {
		int one = 1;
		sock_setopt(desc->s, SOL_SOCKET, SO_BSDCOMPAT, &one,
			    sizeof(one));
	    }
#endif
	}
	return replen;
    }
    case INET_REQ_CLOSE:
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_ctl -> CLOSE\r\n",
	      __LINE__, desc->s, driver_caller(desc->port)) );
	erl_inet_close(desc);
	return ctl_reply_ok(rbuf, rsize);
    case INET_REQ_CONNECT:  {
	int code;
#ifdef HAVE_SCTP
	sctp_assoc_t assoc_id = 0;
	sctp_assoc_t *p_assoc_id = NULL;
#endif
	DDBG(desc,
	     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_ctl -> CONNECT\r\n",
	      __LINE__, desc->s, driver_caller(desc->port)) );
	if (!IS_OPEN(desc))
	    return ctl_xerror(EXBADPORT, rbuf, rsize);
#ifdef HAVE_SCTP
	if (IS_SCTP(desc)) {
	    inet_address remote;
	    char *xerror;
	    if (IS_CONNECTING(desc))
		return ctl_error(EINVAL, rbuf, rsize);
	    if (len < 6)
		return ctl_error(EINVAL, rbuf, rsize);
	    buf += 4;
	    len -= 4;
	    if ((*buf) == INET_AF_LIST) {
	      int addrcnt = 0;
	      buf++; len--;
	      addrcnt = get_int8(buf),
	      buf++; len--;
	      if (! p_sctp_connectx)
		return ctl_error(ENOTSUP, rbuf, rsize);
	      if (addrcnt > 0) {
		char *rabuf = ALLOC(sizeof(inet_address) * addrcnt);
                struct sockaddr *addrs = (struct sockaddr *) rabuf;
		ErlDrvSizeT remote_size;
		char *ep = buf + len;
		for(int ai=0; ai < addrcnt; ai++) {
		  remote_size = ep - buf;
		  if ((xerror = inet_set_faddress
		       (desc->sfamily, &remote, &buf, &remote_size)) != NULL) {
                    FREE((char *)addrs);
		    return ctl_xerror(xerror, rbuf, rsize);
                  }
		  memcpy(rabuf, &remote, remote_size);
		  rabuf += remote_size;
		}
		p_assoc_id = &assoc_id;
		code = p_sctp_connectx(desc->s, addrs, addrcnt, p_assoc_id);
                FREE((char *)addrs);
	      } else {
		return ctl_error(EINVAL, rbuf, rsize);
	      }
	    } else {
	      if ((xerror = inet_set_faddress
		   (desc->sfamily, &remote, &buf, &len)) != NULL)
	        return ctl_xerror(xerror, rbuf, rsize);
	      code = sock_connect(desc->s, &remote.sa, len);
	    }
	    if ((IS_SOCKET_ERROR(code) && (sock_errno() == EINPROGRESS))
                || (! IS_SOCKET_ERROR(code))) {
                return ctl_reply_ok_maybe_assoc_id(p_assoc_id, rbuf, rsize);
	    }
	    else {
		return ctl_error(sock_errno(), rbuf, rsize);
	    }
	}
#endif
	if (len == 0) {
            sock_connect(desc->s, &disassoc_sa, disassoc_sa_size);
	    desc->state &= ~INET_F_ACTIVE;
	    return ctl_reply_ok(rbuf, rsize);
	}
	else if (len < 6)
	    return ctl_error(EINVAL, rbuf, rsize);
	else {
	    char *xerror;
	    buf += 4;
	    len -= 4;
	    if ((xerror = inet_set_faddress
		 (desc->sfamily, &desc->remote, &buf, &len)) != NULL)
	        return ctl_xerror(xerror, rbuf, rsize);
#ifdef __linux__
            if (IS_CONNECTED(desc)) {
                (void) sock_connect(desc->s, &disassoc_sa, disassoc_sa_size);
            }
#endif
	    code = sock_connect(desc->s,
				(struct sockaddr*) &desc->remote, len);
	    if (IS_SOCKET_ERROR(code)) {
                code = sock_errno(),
                (void) sock_connect(desc->s, &disassoc_sa, disassoc_sa_size);
		desc->state &= ~INET_F_ACTIVE;
		return ctl_error(code, rbuf, rsize);
	    }
	    else  {
		desc->state |= INET_F_ACTIVE;
                return ctl_reply_ok(rbuf, rsize);
	    }
	}
    }
#ifdef HAVE_SCTP
    case INET_REQ_LISTEN:
	{
	    int backlog;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "packet_inet_ctl -> LISTEN\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    if (!IS_SCTP(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    if (!IS_OPEN(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    if (len != 2)
		return ctl_error(EINVAL, rbuf, rsize);
	    backlog = get_int16(buf);
	    if (IS_SOCKET_ERROR(sock_listen(desc->s, backlog)))
		return ctl_error(sock_errno(), rbuf, rsize);
	    desc->state = INET_STATE_LISTENING;
	    return ctl_reply_ok(rbuf, rsize);
	}
    case SCTP_REQ_BINDX:
	{
	    inet_address addr;
	    char* curr;
	    int   add_flag, rflag;
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "packet_inet_ctl -> BINDX\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    if (!IS_SCTP(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    curr = buf;
	    add_flag = get_int8(curr);
	    curr++;
	    rflag = add_flag ? SCTP_BINDX_ADD_ADDR : SCTP_BINDX_REM_ADDR;
	    while (curr < buf+len)
		{
		    char *xerror;
		    ErlDrvSizeT alen  = buf + len - curr;
		    xerror = inet_set_faddress
		      (desc->sfamily, &addr, &curr, &alen);
		    if (xerror != NULL)
		        return ctl_xerror(xerror, rbuf, rsize);
		    if (p_sctp_bindx(desc->s, (struct sockaddr *)&addr, 1,
				     rflag) < 0)
			return ctl_error(sock_errno(), rbuf, rsize);
		}
	    desc->state = INET_STATE_OPEN;
	    return ctl_reply_ok(rbuf, rsize);
	}
    case SCTP_REQ_PEELOFF:
	{
	    Uint32 assoc_id;
	    udp_descriptor* new_udesc;
	    int err;
	    SOCKET new_socket;
            ErlDrvTermData caller;
            caller = driver_caller(desc->port);
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "packet_inet_ctl -> PEELOFF\r\n",
                  __LINE__, desc->s, caller) );
	    if (!IS_SCTP(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    if (!IS_OPEN(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    if (! p_sctp_peeloff)
		return ctl_error(ENOTSUP, rbuf, rsize);
	    if (len != 4)
		return ctl_error(EINVAL, rbuf, rsize);
	    assoc_id = get_int32(buf);
	    new_socket = p_sctp_peeloff(desc->s, assoc_id);
	    if (IS_SOCKET_ERROR(new_socket)) {
		return ctl_error(sock_errno(), rbuf, rsize);
	    }
	    if ((new_udesc =
                 sctp_inet_copy(udesc, new_socket, caller, &err)) == NULL) {
		sock_close(new_socket);
		return ctl_error(err, rbuf, rsize);
	    }
            else {
                desc->caller = caller;
            }
	    new_udesc->inet.state = INET_STATE_CONNECTED;
	    new_udesc->inet.stype = SOCK_STREAM;
	    SET_NONBLOCKING(new_udesc->inet.s);
	    inet_reply_ok_port(desc, new_udesc->inet.dport);
	    (*rbuf)[0] = INET_REP;
	    return 1;
	}
#endif
    case PACKET_REQ_RECV:
	{
	    unsigned timeout;
	    char tbuf[2];
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "packet_inet_ctl -> RECV\r\n",
                  __LINE__, desc->s, driver_caller(desc->port)) );
	    if (!IS_OPEN(desc))
		return ctl_xerror(EXBADPORT, rbuf, rsize);
	    if (desc->active || (len != 8))
		return ctl_error(EINVAL, rbuf, rsize);
	    timeout = get_int32(buf);
	    if (enq_async(desc, tbuf, PACKET_REQ_RECV) < 0)
		return ctl_error(EALREADY, rbuf, rsize);
	    if (packet_inet_input(udesc, desc->event) == 0) {
                if (timeout != INET_INFINITY) {
                    driver_set_timer(desc->port, timeout);
                }
	    }
	    return ctl_reply(INET_REP_OK, tbuf, 2, rbuf, rsize);
	}
    default:
	return inet_ctl(desc, cmd, buf, len, rbuf, rsize);
    }
}
static void packet_inet_timeout(ErlDrvData e)
{
    udp_descriptor  * udesc = (udp_descriptor*) e;
    inet_descriptor * desc  = INETP(udesc);
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%p] "
          "packet_inet_timeout -> entry\r\n",
          __LINE__, desc->s, desc->port) );
    ASSERT(! desc->active);
    sock_select(desc, FD_READ, 0);
    async_error_am (desc, am_timeout);
}
static void packet_inet_command(ErlDrvData e, char* buf, ErlDrvSizeT len)
{
    udp_descriptor * udesc= (udp_descriptor*) e;
    inet_descriptor* desc = INETP(udesc);
    char* ptr;
    char* qtr;
    char* xerror;
    ErlDrvSizeT sz;
    long code;
    inet_address other;
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "packet_inet_command -> entry"
          "\r\n",
          __LINE__,
          desc->s, driver_caller(desc->port)) );
    if (! init_caller(&desc->caller, &desc->caller_ref,
                      desc->port, &buf, &len)) {
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_command -> init caller failed"
              "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
        driver_failure_posix(desc->port, EINVAL);
	return;
    }
    ptr = buf;
    if (!IS_OPEN(desc)) {
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_command -> not open"
              "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
	inet_reply_error(desc, EINVAL);
	return;
    }
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "packet_inet_command -> data size: %u"
          "\r\n",
          __LINE__,
          desc->s, driver_caller(desc->port), len) );
#ifdef HAVE_SCTP
    if (IS_SCTP(desc))
    {
	ErlDrvSizeT   data_len;
	struct iovec  iov[1];
	struct msghdr mhdr;
	struct sctp_sndrcvinfo *sri;
	union {
	    struct cmsghdr hdr;
	    char ancd[CMSG_SPACE(sizeof(*sri))];
	} cmsg;
	if (len < SCTP_GET_SENDPARAMS_LEN) goto return_einval;
	sri = (struct sctp_sndrcvinfo *) (CMSG_DATA(&cmsg.hdr));
	ptr  = sctp_get_sendparams(sri, ptr);
	cmsg.hdr.cmsg_level = IPPROTO_SCTP;
	cmsg.hdr.cmsg_type  = SCTP_SNDRCV;
	cmsg.hdr.cmsg_len   = CMSG_LEN(sizeof(*sri));
	data_len = (buf + len) - ptr;
	mhdr.msg_name           = NULL;
	mhdr.msg_namelen        = 0;
	iov[0].iov_len          = data_len;
	iov[0].iov_base         = ptr;
	mhdr.msg_iov            = iov;
	mhdr.msg_iovlen         = 1;
	mhdr.msg_control        = cmsg.ancd;
	mhdr.msg_controllen     = cmsg.hdr.cmsg_len;
	VALGRIND_MAKE_MEM_DEFINED(mhdr.msg_control, mhdr.msg_controllen);
	mhdr.msg_flags          = 0;
	inet_output_count(desc, data_len);
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_command -> try sendmsg"
              "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
	code = sock_sendmsg(desc->s, &mhdr, 0);
        goto check_result_code;
    }
#endif
    {
        ErlDrvSizeT anc_len;
        sz = len;
        qtr = ptr;
        xerror = inet_set_faddress(desc->sfamily, &other, &qtr, &sz);
        if (xerror != NULL) {
            inet_reply_error_am(desc, driver_mk_atom(xerror));
            return;
        }
        len -= (qtr - ptr);
        ptr = qtr;
        if (len < 4) goto return_einval;
        anc_len = get_int32(ptr);
        len -= 4; ptr += 4;
        if (len < anc_len) goto return_einval;
        if (anc_len == 0) {
            inet_output_count(desc, len);
            if (desc->state & INET_F_ACTIVE) {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "packet_inet_command -> try send (connected)"
                      "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
                code = sock_send(desc->s, ptr, len, 0);
            }
            else {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "packet_inet_command -> try sendto"
                      "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
                code = sock_sendto(desc->s, ptr, len, 0, &other.sa, sz);
            }
        }
        else {
#ifdef __WIN32__
            goto return_einval;
#else
            struct iovec iov[1];
            struct msghdr mhdr;
            union {
                struct cmsghdr hdr;
                char ancd[SIZEOF_ANCILLARY_DATA];
            } cmsg;
            sys_memset(&iov, '\0', sizeof(iov));
            sys_memset(&mhdr, '\0', sizeof(mhdr));
            sys_memset(&cmsg, '\0', sizeof(cmsg));
            if (desc->state & INET_F_ACTIVE) {
                mhdr.msg_name = NULL;
                mhdr.msg_namelen = 0;
            }
            else {
                mhdr.msg_name = &other;
                mhdr.msg_namelen = sz;
            }
            mhdr.msg_control = cmsg.ancd;
            mhdr.msg_controllen = sizeof(cmsg.ancd);
            if (compile_ancillary_data(&mhdr, ptr, anc_len) != 0) {
                goto return_einval;
            }
            ASSERT(mhdr.msg_controllen != 0);
            len -= anc_len;
            ptr += anc_len;
            iov[0].iov_len = len;
            iov[0].iov_base = ptr;
            mhdr.msg_iov = iov;
            mhdr.msg_iovlen = 1;
            mhdr.msg_flags = 0;
            inet_output_count(desc, len);
            DDBG(desc,
                 ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                  "packet_inet_command -> try sendmsg"
                  "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
            code = sock_sendmsg(desc->s, &mhdr, 0);
#endif
        }
    }
#ifdef HAVE_SCTP
 check_result_code:
#endif
    if (IS_SOCKET_ERROR(code)) {
        int err = sock_errno();
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_command -> send failed: "
              "\r\n   error: %d (%s)"
              "\r\n", __LINE__,
              desc->s, driver_caller(desc->port), err, errno_str(err)) );
        if (IS_UDP(desc)) {
            inet_reply_error(desc, err);
            return;
        } else {
            if ((err != ERRNO_BLOCK) && (err != EINTR)) {
                inet_reply_error(desc, err);
                return;
            } else if (IS_NON_BLOCK_SEND(desc)) {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "packet_inet_command -> "
                      "[sctp] block|intr when non-block send"
                      "\r\n",
                      __LINE__,
                      desc->s, driver_caller(desc->port)) );
                inet_reply_error(desc, err);
                return;
            } else {
                DDBG(desc,
                     ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
                      "packet_inet_command -> [sctp] block|intr send"
                      "\r\n",
                      __LINE__,
                      desc->s, driver_caller(desc->port)) );
                sock_select(desc, (FD_WRITE|FD_CLOSE), 1);
                set_busy_port(desc->port, 1);
                inet_reply_caller_ref(desc);
                return;
            }
        }
    } else {
        DDBG(desc,
             ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
              "packet_inet_command -> ok"
              "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
        inet_reply_ok(desc);
        return;
    }
 return_einval:
    DDBG(desc,
         ("INET-DRV-DBG[%d][" SOCKET_FSTR ",%T] "
          "packet_inet_command -> einval"
          "\r\n", __LINE__, desc->s, driver_caller(desc->port)) );
    inet_reply_error(desc, EINVAL);
    return;
}
#endif
#ifdef __WIN32__
static void packet_inet_event(ErlDrvData e, ErlDrvEvent event)
{
    udp_descriptor * udesc = (udp_descriptor*)e;
    inet_descriptor* desc  = INETP(udesc);
    WSANETWORKEVENTS netEv;
    if ((WSAEnumNetworkEvents)(desc->s, desc->event, &netEv) != 0) {
	DEBUGF(( "port %d: EnumNetwrokEvents = %d\r\n",
		desc->port, sock_errno() ));
	return;
    }
    netEv.lNetworkEvents |= desc->forced_events;
    if (netEv.lNetworkEvents & FD_READ) {
	packet_inet_input(udesc, (HANDLE)event);
    }
}
#endif
#ifdef HAVE_UDP
static void packet_inet_drv_output(ErlDrvData e, ErlDrvEvent event)
{
    inet_descriptor *desc = INETP((udp_descriptor *) e);
    sock_select(desc, (FD_WRITE|FD_CLOSE), 0);
    set_busy_port(desc->port, 0);
    inet_reply_ok(desc);
}
#endif
#ifdef HAVE_UDP
static void packet_inet_drv_input(ErlDrvData e, ErlDrvEvent event)
{
    (void)  packet_inet_input((udp_descriptor*)e, (HANDLE)event);
}
static int packet_inet_input(udp_descriptor* udesc, HANDLE event)
{
    inet_descriptor* desc = INETP(udesc);
    int n;
    inet_address other;
    char abuf[sizeof(inet_address)];
    int packet_count = udesc->read_packets;
    int count = 0;
#ifndef __WIN32__
    struct msghdr mhdr;
    struct iovec  iov[1];
    char   ancd[ANC_BUFF_SIZE];
#endif
#ifdef HAVE_SCTP
    int have_fragment = FALSE;
#endif
    while(packet_count--) {
	unsigned int len = sizeof(other);
	sys_memzero((char *) &other, sizeof(other));
#ifdef HAVE_SCTP
	if (udesc->i_buf != NULL) {
            ErlDrvBinary* tmp;
	    int bufsz;
            ASSERT(IS_SCTP(desc));
            bufsz = udesc->i_ptr - udesc->i_buf->orig_bytes;
            if (udesc->i_bufsz - bufsz < desc->bufsz) {
                int new_bufsz;
                new_bufsz = INT_MAX - desc->bufsz;
                if (new_bufsz < udesc->i_bufsz)
                    if (new_bufsz < bufsz)
                        goto bufsz_overflow;
                    else
                        new_bufsz = desc->bufsz + bufsz;
                else
                    new_bufsz = desc->bufsz + udesc->i_bufsz;
                if ((tmp = realloc_buffer(udesc->i_buf, new_bufsz)) == NULL) {
                bufsz_overflow:
                    release_buffer(udesc->i_buf);
                    udesc->i_buf = NULL;
                    return packet_error(udesc, ENOMEM);
                }
                udesc->i_ptr =
                    tmp->orig_bytes +
                    (udesc->i_ptr - udesc->i_buf->orig_bytes);
                udesc->i_buf = tmp;
                udesc->i_bufsz = new_bufsz;
            }
            have_fragment = TRUE;
	} else
#endif
        {
            ASSERT(udesc->i_buf == NULL);
	    udesc->i_bufsz = desc->bufsz + len;
	    if ((udesc->i_buf = alloc_buffer(udesc->i_bufsz)) == NULL)
		return packet_error(udesc, ENOMEM);
	    udesc->i_ptr = udesc->i_buf->orig_bytes + len;
	}
#ifdef HAVE_SCTP
	if (IS_SCTP(desc)) {
	    iov->iov_base = udesc->i_ptr;
	    iov->iov_len = desc->bufsz;
	    mhdr.msg_name	= &other;
	    mhdr.msg_namelen	= len;
	    mhdr.msg_iov	= iov;
	    mhdr.msg_iovlen	= 1;
	    mhdr.msg_control	= ancd;
	    mhdr.msg_controllen	= ANC_BUFF_SIZE;
	    mhdr.msg_flags	= 0;
	    n = sock_recvmsg(desc->s, &mhdr, 0);
	    len = mhdr.msg_namelen;
	    goto check_result;
	}
#endif
	if ((desc->state & INET_F_ACTIVE)) {
	    n = sock_recv(desc->s, udesc->i_ptr, desc->bufsz, 0);
	    other = desc->remote;
	    goto check_result;
	}
#ifndef __WIN32__
        if (desc->recv_cmsgflags) {
            iov->iov_base = udesc->i_ptr;
            iov->iov_len = desc->bufsz;
            mhdr.msg_name = &other;
            mhdr.msg_namelen = len;
            mhdr.msg_iov = iov;
            mhdr.msg_iovlen = 1;
	    mhdr.msg_control = ancd;
            mhdr.msg_controllen = ANC_BUFF_SIZE;
            mhdr.msg_flags = 0;
            n = sock_recvmsg(desc->s, &mhdr, 0);
            len = mhdr.msg_namelen;
            goto check_result;
        }
#endif
	n = sock_recvfrom(desc->s, udesc->i_ptr, desc->bufsz,
			  0, &other.sa, &len);
    check_result:
	if (IS_SOCKET_ERROR(n)) {
	    int err = sock_errno();
	    if (err != ERRNO_BLOCK) {
		release_buffer(udesc->i_buf);
		udesc->i_buf = NULL;
		if (! desc->active) {
		    async_error(desc, err);
                    driver_cancel_timer(desc->port);
		    sock_select(desc,FD_READ,0);
		}
		else {
		    packet_error_message(udesc, err);
		}
		return count;
	    }
	    if ((! desc->active)
#ifdef HAVE_SCTP
		|| have_fragment
#endif
		) {
		sock_select(desc,FD_READ,1);
	    }
#ifdef HAVE_SCTP
            if (! have_fragment) {
#endif
                release_buffer(udesc->i_buf);
                udesc->i_buf = NULL;
#ifdef HAVE_SCTP
            }
#endif
	    return count;
	}
#ifdef HAVE_SCTP
	if (IS_SCTP(desc) &&
            (have_fragment = (! (mhdr.msg_flags & MSG_EOR)))) {
	    inet_input_count(desc, n);
	    udesc->i_ptr += n;
	    continue;
	}
#endif
	{
	    int code;
            void *mp;
	    char * ptr;
	    int nsz;
	    inet_input_count(desc, n);
	    udesc->i_ptr += n;
	    inet_get_address(abuf, &other, &len);
	    ASSERT (len <= sizeof(other));
	    ptr = udesc->i_buf->orig_bytes + sizeof(other) - len;
	    sys_memcpy(ptr, abuf, len);
	    nsz = udesc->i_ptr - ptr;
	    if ((desc->mode == INET_MODE_BINARY)
		&& (desc->hsz < (nsz - len))
		&& (nsz + BIN_REALLOC_MARGIN(desc->bufsz) < udesc->i_bufsz)) {
		ErlDrvBinary* tmp;
		int bufsz;
		bufsz = udesc->i_ptr - udesc->i_buf->orig_bytes;
		if ((tmp = realloc_buffer(udesc->i_buf, bufsz)) != NULL) {
		    udesc->i_buf = tmp;
		    udesc->i_bufsz = bufsz;
		    udesc->i_ptr = NULL;
		}
	    }
            mp = NULL;
#ifdef HAVE_SCTP
	    if (IS_SCTP(desc)) mp = &mhdr;
#endif
#ifndef __WIN32__
            if (desc->recv_cmsgflags) mp = &mhdr;
#endif
	    code = packet_reply_binary_data(desc, len, udesc->i_buf,
					    (sizeof(other) - len),
					    nsz,
					    mp);
	    free_buffer(udesc->i_buf);
	    udesc->i_buf = NULL;
	    if (code < 0)
		return count;
	    count++;
	    if (! desc->active) {
                driver_cancel_timer(desc->port);
		sock_select(desc,FD_READ,0);
		return count;
	    }
	}
    }
#ifdef HAVE_SCTP
    if ((! desc->active) && have_fragment) {
	sock_select(desc, FD_READ, 1);
    }
#endif
    return count;
}
#endif
#ifdef __WIN32__
static void find_dynamic_functions(void)
{
    char kernel_dll_name[] = "kernel32";
    HMODULE module;
    module = GetModuleHandle(kernel_dll_name);
    fpSetHandleInformation = (module != NULL) ?
	(BOOL (WINAPI *)(HANDLE,DWORD,DWORD))
	    GetProcAddress(module,"SetHandleInformation") :
	NULL;
}
static SOCKET
make_noninheritable_handle(SOCKET s)
{
    if (s != INVALID_SOCKET) {
	if (fpSetHandleInformation != NULL) {
	    (*fpSetHandleInformation)((HANDLE) s, HANDLE_FLAG_INHERIT, 0);
	} else {
	    HANDLE non_inherited;
	    HANDLE this_process = GetCurrentProcess();
	    if (DuplicateHandle(this_process, (HANDLE) s,
				this_process, &non_inherited, 0,
				FALSE, DUPLICATE_SAME_ACCESS)) {
		sock_close(s);
		s = (SOCKET) non_inherited;
	    }
	}
    }
    return s;
}
#endif
static void fire_multi_timers(tcp_descriptor *desc, ErlDrvPort port,
			      ErlDrvData data)
{
    ErlDrvTime next_timeout = 0;
    if (!desc->mtd) {
        ASSERT(0);
        return;
    }
#ifdef DEBUG
    {
	ASSERT( erl_drv_monotonic_time(ERL_DRV_MSEC) >= desc->mtd->when );
    }
#endif
    do {
	MultiTimerData save = *desc->mtd;
        if (desc->mtd_cache == NULL) {
            desc->mtd_cache = desc->mtd;
        } else {
            FREE(desc->mtd);
        }
        desc->mtd = save.next;
        if (desc->mtd != NULL)
            desc->mtd->prev = NULL;
	if ((*(save.timeout_function))(data,save.caller) < 0)
            return;
        if (desc->mtd == NULL)
	    return;
	next_timeout = desc->mtd->when - erl_drv_monotonic_time(ERL_DRV_MSEC);
    } while (next_timeout <= 0);
    driver_set_timer(port, (unsigned long) next_timeout);
}
static void clean_multi_timers(tcp_descriptor *desc, ErlDrvPort port)
{
    if (desc->mtd) {
	driver_cancel_timer(port);
    }
    while (desc->mtd) {
	MultiTimerData *p = desc->mtd;
	desc->mtd = p->next;
        FREE(p);
    }
    desc->mtd = NULL;
    if (desc->mtd_cache) {
        FREE(desc->mtd_cache);
        desc->mtd_cache = NULL;
    }
}
static void remove_multi_timer(tcp_descriptor *desc, ErlDrvPort port, MultiTimerData *p)
{
    if (p->prev != NULL) {
	p->prev->next = p->next;
    } else {
	driver_cancel_timer(port);
	desc->mtd = p->next;
	if (desc->mtd) {
	    ErlDrvTime ntmo = desc->mtd->when - erl_drv_monotonic_time(ERL_DRV_MSEC);
	    if (ntmo < 0)
		ntmo = 0;
	    driver_set_timer(port, (unsigned long) ntmo);
	}
    }
    if (p->next != NULL) {
	p->next->prev = p->prev;
    }
    if (desc->mtd_cache == NULL)
        desc->mtd_cache = p;
    else
        FREE(p);
}
static void cancel_multi_timer(tcp_descriptor *desc, ErlDrvPort port,
                               MultiTimerFunction *timeout_fun)
{
    MultiTimerData *timer = desc->mtd;
    while(timer && timer->timeout_function != timeout_fun) {
        timer = timer->next;
    }
    if (timer) {
        remove_multi_timer(desc, port, timer);
    }
}
static MultiTimerData *add_multi_timer(tcp_descriptor *desc, ErlDrvPort port,
				       ErlDrvTermData caller, unsigned timeout,
				       MultiTimerFunction *timeout_fun)
{
    MultiTimerData *mtd, *p, *s;
    if (desc->mtd_cache != NULL) {
        mtd = desc->mtd_cache;
        desc->mtd_cache = NULL;
    } else
        mtd = ALLOC(sizeof(MultiTimerData));
    if (timeout)
        mtd->when = erl_drv_monotonic_time(ERL_DRV_MSEC) + ((ErlDrvTime) timeout);
    else
        mtd->when = INT64_MIN;
    mtd->timeout_function = timeout_fun;
    mtd->caller = caller;
    mtd->next = mtd->prev = NULL;
    for(p = desc->mtd,s = NULL; p != NULL; s = p, p = p->next) {
	if (p->when >= mtd->when) {
	    break;
	}
    }
    if (!p) {
	if (!s) {
	    desc->mtd = mtd;
	} else {
	    s->next = mtd;
	    mtd->prev = s;
	}
    } else {
	if (!s) {
	    desc->mtd = mtd;
	} else {
	    s->next = mtd;
	    mtd->prev = s;
	}
	mtd->next = p;
	p->prev = mtd;
    }
    if (!s) {
	driver_set_timer(port,timeout);
    }
    return mtd;
}
static int
save_subscriber(subs_list *subs, ErlDrvTermData subs_pid)
{
  subs_list *tmp;
  if(NO_SUBSCRIBERS(subs)) {
    subs->subscriber = subs_pid;
    subs->next = NULL;
  }
  else {
    tmp = subs->next;
    subs->next = ALLOC(sizeof(subs_list));
    if(subs->next == NULL) {
      subs->next = tmp;
      return 0;
    }
    subs->next->subscriber = subs_pid;
    subs->next->next = tmp;
  }
  return 1;
}
static void
free_subscribers(subs_list *subs)
{
  subs_list *this;
  subs_list *next;
  this = subs->next;
  while(this) {
    next = this->next;
    FREE(this);
    this = next;
  }
  subs->subscriber = NO_PROCESS;
  subs->next = NULL;
}
static void send_to_subscribers
(
    ErlDrvTermData port,
    subs_list	   *subs,
    int		   free_subs,
    ErlDrvTermData msg[],
    int msg_len
)
{
  subs_list *this;
  subs_list *next;
  int first = 1;
  if(NO_SUBSCRIBERS(subs))
    return;
  this = subs;
  while(this) {
    (void) erl_drv_send_term(port, this->subscriber, msg, msg_len);
    if(free_subs && !first) {
      next = this->next;
      FREE(this);
      this = next;
    }
    else
      this = this->next;
    first = 0;
  }
  if(free_subs) {
    subs->subscriber = NO_PROCESS;
    subs->next = NULL;
  }
}