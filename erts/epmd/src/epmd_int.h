#ifdef __WIN32__
#define NO_SYSCONF
#define NO_DAEMON
#define FD_SETSIZE 1024
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __WIN32__
#  ifndef WINDOWS_H_INCLUDES_WINSOCK2_H
#    include <winsock2.h>
#  endif
#  include <ws2tcpip.h>
#  include <windows.h>
#  include <process.h>
#endif
#include <sys/types.h>
#include <fcntl.h>
#ifndef __WIN32__
#  include <time.h>
#  ifdef HAVE_SYS_TIME_H
#     include <sys/time.h>
#  endif
#endif
#if !defined(__WIN32__)
#  include <netinet/in.h>
#  include <sys/socket.h>
#  include <sys/stat.h>
#  ifdef DEF_INADDR_LOOPBACK_IN_RPC_TYPES_H
#    include <rpc/types.h>
#  endif
#  include <arpa/inet.h>
#  include <netinet/tcp.h>
#endif
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#ifdef HAVE_SYSLOG_H
#  include <syslog.h>
#endif
#ifdef SYS_SELECT_H
#  include <sys/select.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <stdarg.h>
#ifdef HAVE_SYSTEMD_DAEMON
#  include <systemd/sd-daemon.h>
#endif
#ifdef DEBUG
#define ASSERT(Cnd) do { if (!(Cnd)) { abort(); } } while(0)
#else
#define ASSERT(Cnd)
#endif
#if __GNUC__
#  define __decl_noreturn
#  ifndef __noreturn
#     define __noreturn __attribute__((noreturn))
#  endif
#else
#  if defined(__WIN32__) && defined(_MSC_VER)
#    define __noreturn
#    define __decl_noreturn __declspec(noreturn)
#  else
#    define __noreturn
#    define __decl_noreturn
#  endif
#endif
#if defined(HAVE_IN6) && defined(AF_INET6) && defined(HAVE_INET_PTON)
#  define EPMD6
#endif
#ifdef __WIN32__
#  define close(s) closesocket((s))
#  define write(a,b,c) send((a),(b),(c),0)
#  define read(a,b,c) recv((a),(char *)(b),(c),0)
#  define sleep(s) Sleep((s) * 1000)
#  define ioctl(s,r,o) ioctlsocket((s),(r),(o))
#endif
#ifdef USE_BCOPY
#  define memcpy(a, b, c) bcopy((b), (a), (c))
#  define memcmp(a, b, c) bcmp((a), (b), (c))
#  define memzero(buf, len) bzero((buf), (len))
#else
#  define memzero(buf, len) memset((buf), '\0', (len))
#endif
#if defined(__WIN32__) && !defined(EADDRINUSE)
#  define EADDRINUSE WSAEADDRINUSE
#endif
#if defined(__WIN32__) && !defined(ECONNABORTED)
#  define ECONNABORTED WSAECONNABORTED
#endif
#ifndef SOMAXCONN
#  define SOMAXCONN 128
#endif
#define MAX_FILES 2048
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
#define IS_ADDR_LOOPBACK(addr) ((addr).s_addr == htonl(INADDR_LOOPBACK))
#if defined(EPMD6)
#define EPMD_SOCKADDR_IN sockaddr_storage
#define FAMILY AF_INET6
#define SET_ADDR6(dst, addr, port) do { \
    struct sockaddr_in6 *sa = (struct sockaddr_in6 *)&(dst); \
    memset(sa, 0, sizeof(dst)); \
    sa->sin6_family = AF_INET6; \
    sa->sin6_addr = (addr); \
    sa->sin6_port = htons(port); \
 } while(0)
#define SET_ADDR(dst, addr, port) do { \
    struct sockaddr_in *sa = (struct sockaddr_in *)&(dst); \
    memset(sa, 0, sizeof(dst)); \
    sa->sin_family = AF_INET; \
    sa->sin_addr.s_addr = (addr); \
    sa->sin_port = htons(port); \
 } while(0)
#else
#define EPMD_SOCKADDR_IN sockaddr_in
#define FAMILY AF_INET
#define SET_ADDR(dst, addr, port) do { \
    memset((char*)&(dst), 0, sizeof(dst)); \
    (dst).sin_family = AF_INET; \
    (dst).sin_addr.s_addr = (addr); \
    (dst).sin_port = htons(port); \
 } while(0)
#endif
#define EPMD_FALSE 0
#define EPMD_TRUE 1
#define IDLE_TIMEOUT 5
#define CLOSE_TIMEOUT 60
#define MAX_UNREG_COUNT 1000
#define DEBUG_MAX_UNREG_COUNT 5
#define MAXSYMLEN (255*4)
#define MAX_LISTEN_SOCKETS 16
#define INBUF_SIZE (3*MAXSYMLEN)
#define OUTBUF_SIZE (3*MAXSYMLEN)
#define get_int16(s) ((((unsigned char*)  (s))[0] << 8) | \
                      (((unsigned char*)  (s))[1]))
#define put_int16(i, s) {((unsigned char*)(s))[0] = ((i) >> 8) & 0xff; \
                        ((unsigned char*)(s))[1] = (i)         & 0xff;}
#define put_int32(i, s) do {((char*)(s))[0] = (char)((i) >> 24) & 0xff;   \
                            ((char*)(s))[1] = (char)((i) >> 16) & 0xff;   \
                            ((char*)(s))[2] = (char)((i) >> 8)  & 0xff;   \
                            ((char*)(s))[3] = (char)(i)         & 0xff;} \
                        while (0)
#if defined(__GNUC__)
#  define EPMD_INLINE __inline__
#elif defined(__WIN32__)
#  define EPMD_INLINE __inline
#else
#  define EPMD_INLINE
#endif
typedef struct {
  int fd;
  unsigned char open;
  unsigned char keep;
  unsigned char local_peer;
  unsigned got;
  unsigned want;
  char *buf;
  time_t mod_time;
} Connection;
struct enode {
  struct enode *next;
  int fd;
  unsigned short port;
  char symname[MAXSYMLEN+1];
  unsigned int cr_counter;
  char nodetype;
  char protocol;
  unsigned short highvsn;
  unsigned short lowvsn;
  int extralen;
  char extra[MAXSYMLEN+1];
};
typedef struct enode Node;
typedef struct {
  Node *reg;
  Node *unreg;
  Node *unreg_tail;
  int unreg_count;
} Nodes;
typedef struct {
  int port;
  int debug;
  int silent;
  int is_daemon;
  int brutal_kill;
  unsigned packet_timeout;
  unsigned delay_accept;
  unsigned delay_write;
  int max_conn;
  int active_conn;
  int select_fd_top;
  char *progname;
  Connection *conn;
  Nodes nodes;
  fd_set orig_read_mask;
  int listenfd[MAX_LISTEN_SOCKETS];
  char *addresses;
  char **argv;
#ifdef HAVE_SYSTEMD_DAEMON
  int is_systemd;
#endif
} EpmdVars;
void dbg_printf(EpmdVars*,int,const char*,...);
void dbg_tty_printf(EpmdVars*,int,const char*,...);
void dbg_perror(EpmdVars*,const char*,...);
void kill_epmd(EpmdVars*);
void epmd_call(EpmdVars*,int);
void run(EpmdVars*);
__decl_noreturn void __noreturn epmd_cleanup_exit(EpmdVars*, int);
int epmd_conn_close(EpmdVars*,Connection*);
void stop_cli(EpmdVars *g, char *name);
#ifdef DONT_USE_MAIN
int  start_epmd(char *,char *,char *,char *,char *,char *,char *,char *,char *,char *);
int  epmd(int,char **);
int  epmd_dbg(int,int);
#endif