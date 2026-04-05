#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#ifdef ESOCK_ENABLE
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <sys.h>
#include "socket_int.h"
#include "socket_io.h"
#include "socket_asyncio.h"
#include "socket_util.h"
#include "socket_tarray.h"
#include "socket_dbg.h"
#define ESAIO_OK                     ESOCK_IO_OK
#define ESAIO_ERR_WINSOCK_INIT       0x0001
#define ESAIO_ERR_IOCPORT_CREATE     0x0002
#define ESAIO_ERR_FSOCK_CREATE       0x0003
#define ESAIO_ERR_IOCTL_ACCEPT_GET   0x0004
#define ESAIO_ERR_IOCTL_CONNECT_GET  0x0005
#define ESAIO_ERR_IOCTL_SENDMSG_GET  0x0006
#define ESAIO_ERR_IOCTL_RECVMSG_GET  0x0007
#define ESAIO_ERR_THREAD_OPTS_CREATE 0x0011
#define ESAIO_ERR_THREAD_CREATE      0x0012
#define ERRNO_BLOCK                  WSAEWOULDBLOCK
#define ESAIO_RECVFROM_MIN_BUFSZ     0x8000
#define sock_accept_O(s, as, b, al, rb, o)              \
    ctrl.accept((s), (as), (b), 0, (al), (al), (rb), (o))
#define sock_bind(s, addr, len)         bind((s), (addr), (len))
#define sock_close(s)                   closesocket((s))
#define sock_connect(s, a, al)          connect((s), (a), (al))
#define sock_connect_O(s, a, al, sent, o)                               \
    ctrl.connect((s), (struct sockaddr*) (a), (al), NULL, 0, (sent), (o))
#define sock_errno()                    WSAGetLastError()
#define sock_ioctl1(s, cc, b)                    \
    ioctlsocket((s), (cc), (b))
#define sock_ioctl2(s, cc, ib, ibs, ob, obs, br) \
    WSAIoctl((s), (cc), (ib), (ibs), (ob), (obs), (br), NULL, NULL)
#define sock_open(domain, type, proto)  socket((domain), (type), (proto))
#define sock_open_O(domain, type, proto) \
    WSASocket((domain), (type), (proto), NULL, 0, WSA_FLAG_OVERLAPPED)
#define sock_recv_O(s,buf,flag,ol)                      \
    WSARecv((s), (buf), 1, NULL, (flag), (ol), NULL)
#define sock_recvfrom_O(s,buf,flag,fa,fal,ol)                           \
    WSARecvFrom((s), (buf), 1, NULL, (flag), (fa), (fal), (ol), NULL)
#define sock_recvmsg_O(s,msg,o)     \
    ctrl.recvmsg((s), (msg), NULL, (o), NULL)
#define sock_send_O(s,buf,flag,o)                       \
    WSASend((s), (buf), 1, NULL, (flag), (o), NULL)
#define sock_sendmsg_O(s,buf,flag,ol)                   \
    ctrl.sendmsg((s), (buf), (flag), NULL, (ol), NULL)
#define sock_sendto_O(s,buf,flag,ta,tal,o)              \
    WSASendTo((s), (buf), 1, NULL, (flag), (ta), (tal), (o), NULL)
#define sock_sendv_O(s,iov,iovcnt,o)                  \
    WSASend((s), (iov), iovcnt, NULL, 0, (o), NULL)
#define sock_setopt(s,l,o,v,ln)        setsockopt((s),(l),(o),(v),(ln))
#define ESAIO_UPDATE_ACCEPT_CONTEXT(AS, LS)                 \
    sock_setopt( (AS), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, \
                 (char*) &(LS), sizeof( (LS) ))
#define ESAIO_UPDATE_CONNECT_CONTEXT(S)                                 \
    sock_setopt((S), SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, NULL, 0)
#define ESOCK_CMSG_FIRSTHDR(M) WSA_CMSG_FIRSTHDR((M))
#define ESOCK_CMSG_NXTHDR(M,C) WSA_CMSG_NXTHDR((M), (C))
#define ESOCK_CMSG_DATA(C)     WSA_CMSG_DATA((C))
typedef struct {
    Uint16     id;
#define ESAIO_THREAD_STATE_UNDEF        0xFFFF
#define ESAIO_THREAD_STATE_INITIATING   0x0000
#define ESAIO_THREAD_STATE_OPERATIONAL  0x0001
#define ESAIO_THREAD_STATE_TERMINATING  0x0002
#define ESAIO_THREAD_STATE_TERMINATED   0x0003
    Uint16     state;
#define ESAIO_THREAD_ERROR_UNDEF        0xFFFF
#define ESAIO_THREAD_ERROR_OK           0x0000
#define ESAIO_THREAD_ERROR_TOCREATE     0x0001
#define ESAIO_THREAD_ERROR_TCREATE      0x0002
#define ESAIO_THREAD_ERROR_GET          0x0003
#define ESAIO_THREAD_ERROR_CMD          0x0004
    Uint32       error;
    ErlNifEnv*   env;
#define ESAIO_THREAD_CNT_MAX 0xFFFFFFFF
    Uint32       cnt;
    unsigned int latest;
} ESAIOThreadData;
typedef struct {
    ErlNifThreadOpts* optsP;
    ErlNifTid         tid;
    ESAIOThreadData   data;
} ESAIOThread;
typedef struct {
    WSADATA         wsaData;
    HANDLE          cport;
    SOCKET          srvInit;
    LPFN_ACCEPTEX   accept;
    LPFN_CONNECTEX  connect;
    LPFN_WSASENDMSG sendmsg;
    LPFN_WSARECVMSG recvmsg;
    DWORD           numThreads;
    ESAIOThread*    threads;
    BOOLEAN_T       dbg;
    BOOLEAN_T       sockDbg;
    ErlNifMutex*    cntMtx;
    ESockCounter    unexpectedConnects;
    ESockCounter    unexpectedAccepts;
    ESockCounter    unexpectedWrites;
    ESockCounter    unexpectedReads;
    ESockCounter    genErrs;
    ESockCounter    unknownCmds;
} ESAIOControl;
typedef struct __ESAIOOpDataAccept {
    SOCKET       lsock;
    SOCKET       asock;
    char*        buf;
    ERL_NIF_TERM lSockRef;
    ERL_NIF_TERM accRef;
} ESAIOOpDataAccept;
typedef struct __ESAIOOpDataConnect {
    ERL_NIF_TERM sockRef;
    ERL_NIF_TERM connRef;
} ESAIOOpDataConnect;
typedef struct __ESAIOOpDataSend {
    WSABUF        wbuf;
    ERL_NIF_TERM  sockRef;
    ERL_NIF_TERM  sendRef;
} ESAIOOpDataSend;
typedef struct __ESAIOOpDataSendTo {
    WSABUF       wbuf;
    ESockAddress remoteAddr;
    SOCKLEN_T    remoteAddrLen;
    ERL_NIF_TERM sockRef;
    ERL_NIF_TERM sendRef;
} ESAIOOpDataSendTo;
typedef struct __ESAIOOpDataSendMsg {
    WSAMSG       msg;
    ErlNifIOVec* iovec;
    char*        ctrlBuf;
    ESockAddress addr;
    ERL_NIF_TERM sockRef;
    ERL_NIF_TERM sendRef;
} ESAIOOpDataSendMsg;
typedef struct __ESAIOOpDataSendv {
    ErlNifIOVec* iovec;
    WSABUF*      lpBuffers;
    DWORD        dwBufferCount;
    ERL_NIF_TERM sockRef;
    ERL_NIF_TERM sendRef;
    DWORD        toWrite;
    BOOLEAN_T    dataInTail;
 } ESAIOOpDataSendv;
typedef struct __ESAIOOpDataRecv {
    DWORD        toRead;
    ErlNifBinary buf;
    ERL_NIF_TERM sockRef;
    ERL_NIF_TERM recvRef;
} ESAIOOpDataRecv;
typedef struct __ESAIOOpDataRecvFrom {
    DWORD         toRead;
    ErlNifBinary  buf;
    ESockAddress  fromAddr;
    INT           addrLen;
    ERL_NIF_TERM  sockRef;
    ERL_NIF_TERM  recvRef;
} ESAIOOpDataRecvFrom;
typedef struct __ESAIOOpDataRecvMsg {
    WSAMSG        msg;
    WSABUF        wbufs[1];
    ErlNifBinary  data[1];
    ErlNifBinary  ctrl;
    ESockAddress  addr;
    ERL_NIF_TERM  sockRef;
    ERL_NIF_TERM  recvRef;
} ESAIOOpDataRecvMsg;
typedef struct __ESAIOOperation {
    WSAOVERLAPPED        ol;
#define ESAIO_OP_NONE         0x0000
#define ESAIO_OP_TERMINATE    0x0001
#define ESAIO_OP_DEBUG        0x0002
#define ESAIO_OP_CONNECT      0x0011
#define ESAIO_OP_ACCEPT       0x0012
#define ESAIO_OP_SEND         0x0021
#define ESAIO_OP_SENDTO       0x0022
#define ESAIO_OP_SENDMSG      0x0023
#define ESAIO_OP_SENDV        0x0024
#define ESAIO_OP_RECV         0x0031
#define ESAIO_OP_RECVFROM     0x0032
#define ESAIO_OP_RECVMSG      0x0033
    unsigned int          tag;
    ErlNifPid             caller;
    ErlNifEnv*            env;
    union {
        ESAIOOpDataAccept accept;
        ESAIOOpDataConnect connect;
        ESAIOOpDataSend send;
        ESAIOOpDataSendTo sendto;
        ESAIOOpDataSendMsg sendmsg;
        ESAIOOpDataSendv sendv;
        ESAIOOpDataRecv recv;
        ESAIOOpDataRecvFrom recvfrom;
        ESAIOOpDataRecvMsg recvmsg;
    } data;
} ESAIOOperation;
static
BOOLEAN_T init_srv_init_socket(int* savedErrno);
static ERL_NIF_TERM esaio_connect_stream(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ERL_NIF_TERM     sockRef,
                                         ERL_NIF_TERM     connRef,
                                         ESockAddress*    addrP,
                                         SOCKLEN_T        addrLen);
static ERL_NIF_TERM connect_stream_check_result(ErlNifEnv*       env,
                                                ESockDescriptor* descP,
                                                ESAIOOperation*  opP,
                                                BOOL             cres);
static ERL_NIF_TERM esaio_connect_dgram(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ERL_NIF_TERM     sockRef,
                                        ERL_NIF_TERM     connRef,
                                        ESockAddress*    addrP,
                                        SOCKLEN_T        addrLen);
static ERL_NIF_TERM accept_check_result(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ESAIOOperation*  opP,
                                        BOOL             ares,
                                        ERL_NIF_TERM     sockRef,
                                        ERL_NIF_TERM     accRef,
                                        SOCKET           accSock,
                                        ErlNifPid        caller);
static ERL_NIF_TERM accept_check_pending(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ESAIOOperation*  opP,
                                         ErlNifPid        caller,
                                         ERL_NIF_TERM     sockRef,
                                         ERL_NIF_TERM     accRef);
static ERL_NIF_TERM accept_check_fail(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ESAIOOperation*  opP,
                                      int              saveErrno,
                                      SOCKET           accSock,
                                      ERL_NIF_TERM     sockRef);
static ERL_NIF_TERM esaio_accept_accepted(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ErlNifPid        pid,
                                          ERL_NIF_TERM     sockRef,
                                          SOCKET           accSock);
static ERL_NIF_TERM send_check_result(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ESAIOOperation*  opP,
                                      ErlNifPid        caller,
                                      int              send_result,
                                      size_t           dataSize,
                                      BOOLEAN_T        dataInTail,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM*    sendRef,
                                      BOOLEAN_T*       cleanup);
static ERL_NIF_TERM send_check_ok(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  DWORD            written,
                                  ERL_NIF_TERM     sockRef);
static ERL_NIF_TERM send_check_pending(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ESAIOOperation*  opP,
                                       ErlNifPid        caller,
                                       ERL_NIF_TERM     sockRef,
                                       ERL_NIF_TERM     sendRef);
static ERL_NIF_TERM send_check_fail(ErlNifEnv*       env,
                                    ESockDescriptor* descP,
                                    int              saveErrno,
                                    ERL_NIF_TERM     sockRef);
static BOOLEAN_T init_sendmsg_sockaddr(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ERL_NIF_TERM     eMsg,
                                       WSAMSG*          msgP,
                                       ESockAddress*    addrP);
static BOOLEAN_T verify_sendmsg_iovec_size(const ESockData* dataP,
                                           ESockDescriptor* descP,
                                           ErlNifIOVec*     iovec);
static BOOLEAN_T verify_sendmsg_iovec_tail(ErlNifEnv*       env,
                                           ESockDescriptor* descP,
                                           ERL_NIF_TERM*    tail);
static BOOLEAN_T check_sendmsg_iovec_overflow(ESockDescriptor* descP,
                                              ErlNifIOVec*     iovec,
                                              size_t*          dataSize);
static BOOLEAN_T decode_cmsghdrs(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     eCMsg,
                                 char*            cmsgHdrBufP,
                                 size_t           cmsgHdrBufLen,
                                 size_t*          cmsgHdrBufUsed);
static BOOLEAN_T decode_cmsghdr(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     eCMsg,
                                char*            bufP,
                                size_t           rem,
                                size_t*          used);
static BOOLEAN_T decode_cmsghdr_value(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      int              level,
                                      ERL_NIF_TERM     eType,
                                      ERL_NIF_TERM     eValue,
                                      char*            dataP,
                                      size_t           dataLen,
                                      size_t*          dataUsedP);
static BOOLEAN_T decode_cmsghdr_data(ErlNifEnv*       env,
                                     ESockDescriptor* descP,
                                     int              level,
                                     ERL_NIF_TERM     eType,
                                     ERL_NIF_TERM     eData,
                                     char*            dataP,
                                     size_t           dataLen,
                                     size_t*          dataUsedP);
static void encode_msg(ErlNifEnv*       env,
                       ESockDescriptor* descP,
                       ssize_t          read,
                       WSAMSG*          msgP,
                       ErlNifBinary*    dataBufP,
                       ErlNifBinary*    ctrlBufP,
                       ERL_NIF_TERM*    eMsg);
static void encode_cmsgs(ErlNifEnv*       env,
                         ESockDescriptor* descP,
                         ErlNifBinary*    cmsgBinP,
                         WSAMSG*          msgP,
                         ERL_NIF_TERM*    eCMsg);
static ERL_NIF_TERM recv_check_ok(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ESAIOOperation*  opP,
                                  ssize_t          toRead,
                                  ErlNifPid        caller,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recv_check_result(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ssize_t          toRead,
                                      ESAIOOperation*  opP,
                                      ErlNifPid        caller,
                                      int              recv_result,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recv_check_pending(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ESAIOOperation*  opP,
                                       ErlNifPid        caller,
                                       ERL_NIF_TERM     sockRef,
                                       ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recv_check_fail(ErlNifEnv*       env,
                                    ESockDescriptor* descP,
                                    ESAIOOperation*  opP,
                                    int              saveErrno,
                                    ERL_NIF_TERM     sockRef);
static ERL_NIF_TERM recv_check_failure(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ESAIOOperation*  opP,
                                       int              saveErrno,
                                       ERL_NIF_TERM     sockRef);
static ERL_NIF_TERM recvfrom_check_result(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ESAIOOperation*  opP,
                                          ErlNifPid        caller,
                                          int              recv_result,
                                          ERL_NIF_TERM     sockRef,
                                          ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recvfrom_check_ok(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ESAIOOperation*  opP,
                                      ErlNifPid        caller,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recvfrom_check_fail(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ESAIOOperation*  opP,
                                        int              saveErrno,
                                        ERL_NIF_TERM     sockRef);
static ERL_NIF_TERM recvmsg_check_result(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ESAIOOperation*  opP,
                                         ErlNifPid        caller,
                                         int              recv_result,
                                         ERL_NIF_TERM     sockRef,
                                         ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recvmsg_check_ok(ErlNifEnv*       env,
                                     ESockDescriptor* descP,
                                     ESAIOOperation*  opP,
                                     ErlNifPid        caller,
                                     ERL_NIF_TERM     sockRef,
                                     ERL_NIF_TERM     recvRef);
static ERL_NIF_TERM recvmsg_check_fail(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ESAIOOperation*  opP,
                                       int              saveErrno,
                                       ERL_NIF_TERM     sockRef);
#if defined(FIONREAD)
static ERL_NIF_TERM esaio_ioctl_fionread(ErlNifEnv*       env,
                                         ESockDescriptor* descP);
#endif
#if defined(SIOCATMARK)
static ERL_NIF_TERM esaio_ioctl_siocatmark(ErlNifEnv*       env,
                                           ESockDescriptor* descP);
#endif
#if defined(SIO_TCP_INFO)
static ERL_NIF_TERM esaio_ioctl_tcp_info(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ERL_NIF_TERM     eversion);
static ERL_NIF_TERM encode_tcp_info_v0(ErlNifEnv*   env,
                                       TCP_INFO_v0* infoP);
#if defined(HAVE_TCP_INFO_V1)
static ERL_NIF_TERM encode_tcp_info_v1(ErlNifEnv*   env,
                                       TCP_INFO_v1* infoP);
#endif
static ERL_NIF_TERM encode_tcp_state(ErlNifEnv* env,
                                     TCPSTATE   state);
#endif
#if defined(SIO_RCVALL)
static ERL_NIF_TERM esaio_ioctl_rcvall(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       ERL_NIF_TERM     evalue);
#endif
#if defined(SIO_RCVALL_IGMPMCAST)
static ERL_NIF_TERM esaio_ioctl_rcvall_igmpmcast(ErlNifEnv*       env,
                                                 ESockDescriptor* descP,
                                                 ERL_NIF_TERM     evalue);
#endif
#if defined(SIO_RCVALL_MCAST)
static ERL_NIF_TERM esaio_ioctl_rcvall_mcast(ErlNifEnv*       env,
                                             ESockDescriptor* descP,
                                             ERL_NIF_TERM     evalue);
#endif
static void* esaio_completion_main(void* threadDataP);
static BOOLEAN_T esaio_completion_terminate(ESAIOThreadData* dataP,
                                            OVERLAPPED*      ovl);
static BOOLEAN_T esaio_completion_unknown(ESAIOThreadData* dataP,
                                          ESockDescriptor* descP,
                                          OVERLAPPED*      ovl,
                                          DWORD            numBytes,
                                          int              error);
static void esaio_completion_fail(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  const char*      opStr,
                                  int              error,
                                  BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_connect(ESAIOThreadData*    dataP,
                                          ESockDescriptor*    descP,
                                          OVERLAPPED*         ovl,
                                          ErlNifEnv*          opEnv,
                                          ErlNifPid*          opCaller,
                                          ESAIOOpDataConnect* opDataP,
                                          int                 error);
static void esaio_completion_connect_success(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ESAIOOpDataConnect* opDataP);
static void esaio_completion_connect_aborted(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ESAIOOpDataConnect* opDataP);
static void esaio_completion_connect_failure(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ESAIOOpDataConnect* opDataP,
                                             int                 error);
static void esaio_completion_connect_completed(ErlNifEnv*          env,
                                               ESockDescriptor*    descP,
                                               ESAIOOpDataConnect* opDataPP);
static void esaio_completion_connect_not_active(ESockDescriptor* descP);
static void esaio_completion_connect_fail(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          int              error,
                                          BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_accept(ESAIOThreadData*   dataP,
                                         ESockDescriptor*   descP,
                                         OVERLAPPED*        ovl,
                                         ErlNifEnv*         opEnv,
                                         ErlNifPid*         opCaller,
                                         ESAIOOpDataAccept* opDataP,
                                         int                error);
static void esaio_completion_accept_success(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            ErlNifEnv*         opEnv,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataAccept* opDataP);
static void esaio_completion_accept_aborted(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataAccept* opDataP);
static void esaio_completion_accept_failure(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataAccept* opDataP,
                                            int                error);
static void esaio_completion_accept_completed(ErlNifEnv*         env,
                                              ESockDescriptor*   descP,
                                              ErlNifEnv*         opEnv,
                                              ErlNifPid*         opCaller,
                                              ESAIOOpDataAccept* opDataP,
                                              ESockRequestor*    reqP);
static void esaio_completion_accept_not_active(ESockDescriptor* descP);
static void esaio_completion_accept_fail(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         int              error,
                                         BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_send(ESAIOThreadData* dataP,
                                       ESockDescriptor* descP,
                                       OVERLAPPED*      ovl,
                                       ErlNifEnv*       opEnv,
                                       ErlNifPid*       opCaller,
                                       ESAIOOpDataSend* opDataP,
                                       int              error);
static void esaio_completion_send_success(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          OVERLAPPED*      ovl,
                                          ErlNifEnv*       opEnv,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataSend* opDataP);
static void esaio_completion_send_aborted(ErlNifEnv*         env,
                                          ESockDescriptor* descP,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataSend* opDataP);
static void esaio_completion_send_failure(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataSend* opDataP,
                                          int              error);
static void esaio_completion_send_completed(ErlNifEnv*       env,
                                            ESockDescriptor* descP,
                                            OVERLAPPED*      ovl,
                                            ErlNifEnv*       opEnv,
                                            ErlNifPid*       sender,
                                            ERL_NIF_TERM     sockRef,
                                            ERL_NIF_TERM     sendRef,
                                            DWORD            toWrite,
                                            BOOLEAN_T        dataInTail,
                                            ESockRequestor*  reqP);
static ERL_NIF_TERM esaio_completion_send_done(ErlNifEnv*       env,
                                               ESockDescriptor* descP,
                                               ERL_NIF_TERM     sockRef,
                                               DWORD            written);
static ERL_NIF_TERM esaio_completion_send_partial(ErlNifEnv*       env,
                                                  ESockDescriptor* descP,
                                                  ERL_NIF_TERM     sockRef,
                                                  DWORD            written);
static void esaio_completion_send_fail(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       int              error,
                                       BOOLEAN_T        inform);
static void esaio_completion_send_not_active(ESockDescriptor* descP);
static BOOLEAN_T esaio_completion_sendv(ESAIOThreadData*  dataP,
                                        ESockDescriptor*  descP,
                                        OVERLAPPED*       ovl,
                                        ErlNifEnv*        opEnv,
                                        ErlNifPid*        opCaller,
                                        ESAIOOpDataSendv* opDataP,
                                        int               error);
static void esaio_completion_sendv_success(ErlNifEnv*        env,
                                           ESockDescriptor*  descP,
                                           OVERLAPPED*       ovl,
                                           ErlNifEnv*        opEnv,
                                           ErlNifPid*        opCaller,
                                           ESAIOOpDataSendv* opDataP);
static void esaio_completion_sendv_aborted(ErlNifEnv*        env,
                                           ESockDescriptor*  descP,
                                           ErlNifPid*        opCaller,
                                           ESAIOOpDataSendv* opDataP);
static void esaio_completion_sendv_failure(ErlNifEnv*        env,
                                           ESockDescriptor*  descP,
                                           ErlNifPid*        opCaller,
                                           ESAIOOpDataSendv* opDataP,
                                           int               error);
static void esaio_completion_sendv_fail(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        int              error,
                                        BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_sendto(ESAIOThreadData*   dataP,
                                         ESockDescriptor*   descP,
                                         OVERLAPPED*        ovl,
                                         ErlNifEnv*         opEnv,
                                         ErlNifPid*         opCaller,
                                         ESAIOOpDataSendTo* opDataP,
                                         int                error);
static void esaio_completion_sendto_success(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            OVERLAPPED*        ovl,
                                            ErlNifEnv*         opEnv,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataSendTo* opDataP);
static void esaio_completion_sendto_aborted(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataSendTo* opDataP);
static void esaio_completion_sendto_failure(ErlNifEnv*         env,
                                            ESockDescriptor*   descP,
                                            ErlNifPid*         opCaller,
                                            ESAIOOpDataSendTo* opDataP,
                                            int                error);
static void esaio_completion_sendto_fail(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         int              error,
                                         BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_sendmsg(ESAIOThreadData*    dataP,
                                          ESockDescriptor*    descP,
                                          OVERLAPPED*         ovl,
                                          ErlNifEnv*          opEnv,
                                          ErlNifPid*          opCaller,
                                          ESAIOOpDataSendMsg* opDataP,
                                          int                 error);
static void esaio_completion_sendmsg_success(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             OVERLAPPED*         ovl,
                                             ErlNifEnv*          opEnv,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataSendMsg* opDataP);
static void esaio_completion_sendmsg_aborted(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataSendMsg* opDataP);
static void esaio_completion_sendmsg_failure(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataSendMsg* opDataP,
                                             int                 error);
static void esaio_completion_sendmsg_fail(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          int              error,
                                          BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_recv(ESAIOThreadData* dataP,
                                       ESockDescriptor* descP,
                                       OVERLAPPED*      ovl,
                                       ErlNifEnv*       opEnv,
                                       ErlNifPid*       opCaller,
                                       ESAIOOpDataRecv* opDataP,
                                       int              error);
static void esaio_completion_recv_success(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          OVERLAPPED*      ovl,
                                          ErlNifEnv*       opEnv,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataRecv* opDataP);
static void esaio_completion_recv_aborted(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataRecv* opDataP);
static void esaio_completion_recv_failure(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ErlNifPid*       opCaller,
                                          ESAIOOpDataRecv* opDataP,
                                          int              error);
static void esaio_completion_recv_completed(ErlNifEnv*       env,
                                            ESockDescriptor* descP,
                                            OVERLAPPED*      ovl,
                                            ErlNifEnv*       opEnv,
                                            ErlNifPid*       opCaller,
                                            ESAIOOpDataRecv* opDataP,
                                            ESockRequestor*  reqP);
static ERL_NIF_TERM esaio_completion_recv_done(ErlNifEnv*       env,
                                               ESockDescriptor* descP,
                                               ErlNifEnv*       opEnv,
                                               ESAIOOpDataRecv* opDataP,
                                               DWORD            flags);
static ERL_NIF_TERM esaio_completion_recv_partial(ErlNifEnv*       env,
                                                  ESockDescriptor* descP,
                                                  ErlNifEnv*       opEnv,
                                                  ESAIOOpDataRecv* opDataP,
                                                  ESockRequestor*  reqP,
                                                  DWORD            read,
                                                  DWORD            flags);
static ERL_NIF_TERM esaio_completion_recv_partial_done(ErlNifEnv*       env,
                                                       ESockDescriptor* descP,
                                                       ErlNifEnv*       opEnv,
                                                       ESAIOOpDataRecv* opDataP,
                                                       ssize_t          read,
                                                       DWORD            flags);
static ERL_NIF_TERM esaio_completion_recv_partial_part(ErlNifEnv*       env,
                                                       ESockDescriptor* descP,
                                                       ErlNifEnv*       opEnv,
                                                       ESAIOOpDataRecv* opDataP,
                                                       ssize_t          read,
                                                       DWORD            flags);
static void esaio_completion_recv_not_active(ESockDescriptor* descP);
static void esaio_completion_recv_closed(ESockDescriptor* descP,
                                         int              error);
static void esaio_completion_recv_fail(ErlNifEnv*       env,
                                       ESockDescriptor* descP,
                                       int              error,
                                       BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_recvfrom(ESAIOThreadData*     dataP,
                                           ESockDescriptor*     descP,
                                           OVERLAPPED*          ovl,
                                           ErlNifEnv*           opEnv,
                                           ErlNifPid*           opCaller,
                                           ESAIOOpDataRecvFrom* opDataP,
                                           int                  error);
static void esaio_completion_recvfrom_success(ErlNifEnv*           env,
                                              ESockDescriptor*     descP,
                                              OVERLAPPED*          ovl,
                                              ErlNifEnv*           opEnv,
                                              ErlNifPid*           opCaller,
                                              ESAIOOpDataRecvFrom* opDataP);
static void esaio_completion_recvfrom_more_data(ErlNifEnv*           env,
                                                ESockDescriptor*     descP,
                                                ErlNifEnv*           opEnv,
                                                ErlNifPid*           opCaller,
                                                ESAIOOpDataRecvFrom* opDataP,
                                                int                  error);
static void esaio_completion_recvfrom_aborted(ErlNifEnv*           env,
                                              ESockDescriptor*     descP,
                                              ErlNifPid*           opCaller,
                                              ESAIOOpDataRecvFrom* opDataP);
static void esaio_completion_recvfrom_failure(ErlNifEnv*           env,
                                              ESockDescriptor*     descP,
                                              ErlNifPid*           opCaller,
                                              ESAIOOpDataRecvFrom* opDataP,
                                              int                  error);
static void esaio_completion_recvfrom_completed(ErlNifEnv*           env,
                                                ESockDescriptor*     descP,
                                                OVERLAPPED*          ovl,
                                                ErlNifEnv*           opEnv,
                                                ErlNifPid*           opCaller,
                                                ESAIOOpDataRecvFrom* opDataP,
                                                ESockRequestor*      reqP);
static ERL_NIF_TERM esaio_completion_recvfrom_done(ErlNifEnv*           env,
                                                   ESockDescriptor*     descP,
                                                   ErlNifEnv*           opEnv,
                                                   ESAIOOpDataRecvFrom* opDataP,
                                                   DWORD                flags);
static ERL_NIF_TERM esaio_completion_recvfrom_partial(ErlNifEnv*           env,
                                                      ESockDescriptor*     descP,
                                                      ErlNifEnv*           opEnv,
                                                      ESAIOOpDataRecvFrom* opDataP,
                                                      ESockRequestor*      reqP,
                                                      DWORD                read,
                                                      DWORD                flags);
static void esaio_completion_recvfrom_fail(ErlNifEnv*       env,
                                           ESockDescriptor* descP,
                                           int              error,
                                           BOOLEAN_T        inform);
static BOOLEAN_T esaio_completion_recvmsg(ESAIOThreadData*    dataP,
                                          ESockDescriptor*    descP,
                                          OVERLAPPED*         ovl,
                                          ErlNifEnv*          opEnv,
                                          ErlNifPid*          opCaller,
                                          ESAIOOpDataRecvMsg* opDataP,
                                          int                error);
static void esaio_completion_recvmsg_success(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             OVERLAPPED*         ovl,
                                             ErlNifEnv*          opEnv,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataRecvMsg* opDataP);
static void esaio_completion_recvmsg_aborted(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataRecvMsg* opDataP);
static void esaio_completion_recvmsg_failure(ErlNifEnv*          env,
                                             ESockDescriptor*    descP,
                                             ErlNifPid*          opCaller,
                                             ESAIOOpDataRecvMsg* opDataP,
                                             int                 error);
static void esaio_completion_recvmsg_completed(ErlNifEnv*          env,
                                               ESockDescriptor*    descP,
                                               OVERLAPPED*         ovl,
                                               ErlNifEnv*          opEnv,
                                               ErlNifPid*          opCaller,
                                               ESAIOOpDataRecvMsg* opDataP,
                                               ESockRequestor*     reqP);
static ERL_NIF_TERM esaio_completion_recvmsg_done(ErlNifEnv*          env,
                                                  ESockDescriptor*    descP,
                                                  ErlNifEnv*          opEnv,
                                                  ESAIOOpDataRecvMsg* opDataP,
                                                  DWORD               flags);
static ERL_NIF_TERM esaio_completion_recvmsg_partial(ErlNifEnv*          env,
                                                     ESockDescriptor*    descP,
                                                     ErlNifEnv*          opEnv,
                                                     ESAIOOpDataRecvMsg* opDataP,
                                                     ESockRequestor*     reqP,
                                                     DWORD               read,
                                                     DWORD               flags);
static void esaio_completion_recvmsg_fail(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          int              error,
                                          BOOLEAN_T        inform);
static ERL_NIF_TERM esaio_completion_get_ovl_result_fail(ErlNifEnv*       env,
                                                         ESockDescriptor* descP,
                                                         int              error);
static BOOL get_send_ovl_result(SOCKET      sock,
                                OVERLAPPED* ovl,
                                DWORD*      written);
static BOOL get_recv_ovl_result(SOCKET      sock,
                                OVERLAPPED* ovl,
                                DWORD*      read,
                                DWORD*      flags);
static BOOL get_recvmsg_ovl_result(SOCKET      sock,
                                   OVERLAPPED* ovl,
                                   DWORD*      read);
static BOOL get_ovl_result(SOCKET      sock,
                           OVERLAPPED* ovl,
                           DWORD*      transfer,
                           DWORD*      flags);
static void esaio_completion_inc(ESAIOThreadData* dataP);
static int esaio_add_socket(ESockDescriptor* descP);
static void esaio_send_completion_msg(ErlNifEnv*       sendEnv,
                                      ESockDescriptor* descP,
                                      ErlNifPid*       pid,
                                      ErlNifEnv*       msgEnv,
                                      ERL_NIF_TERM     sockRef,
                                      ERL_NIF_TERM     connRef);
static ERL_NIF_TERM mk_completion_msg(ErlNifEnv*   env,
                                      ERL_NIF_TERM sockRef,
                                      ERL_NIF_TERM completionRef);
static void esaio_down_acceptor(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     sockRef,
                                const ErlNifPid* pidP,
                                const ErlNifMonitor* monP);
static void esaio_down_writer(ErlNifEnv*       env,
                              ESockDescriptor* descP,
                              ERL_NIF_TERM     sockRef,
                              const ErlNifPid* pidP,
                              const ErlNifMonitor* monP);
static void esaio_down_reader(ErlNifEnv*       env,
                              ESockDescriptor* descP,
                              ERL_NIF_TERM     sockRef,
                              const ErlNifPid* pidP,
                              const ErlNifMonitor* monP);
static BOOLEAN_T do_stop(ErlNifEnv*       env,
                         ESockDescriptor* descP);
static ESAIOControl ctrl = {0};
#define SGDBG( proto )            ESOCK_DBG_PRINTF( ctrl.dbg , proto )
#define ESAIO_IOCP_CREATE(NT)                           \
    CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (u_long) 0, (NT))
#define ESAIO_IOCP_ADD(SOCK, DP)                                \
    CreateIoCompletionPort((SOCK), ctrl.cport, (ULONG*) (DP), 0)
#define ESAIO_IOCQ_POP(NB, DP, OLP)                                     \
    GetQueuedCompletionStatus(ctrl.cport, (DP), (DP), (OLP), INFINITE)
#define ESAIO_IOCQ_PUSH(OP)                                             \
    PostQueuedCompletionStatus(ctrl.cport, 0, 0, (OVERLAPPED*) (OP))
#define ESAIO_IOCQ_CANCEL(H, OP)                                        \
    CancelIoEx((H), (OVERLAPPED*) (OP))
extern
int esaio_init(unsigned int     numThreads,
               const ESockData* dataP)
{
    int          ires, save_errno;
    unsigned int i;
    DWORD        dummy;
    GUID         guidAcceptEx  = WSAID_ACCEPTEX;
    GUID         guidConnectEx = WSAID_CONNECTEX;
    GUID         guidSendMsg   = WSAID_WSASENDMSG;
    GUID         guidRecvMsg   = WSAID_WSARECVMSG;
    ctrl.dbg               = dataP->dbg;
    ctrl.sockDbg           = dataP->sockDbg;
    SGDBG( ("WIN-ESAIO", "esaio_init -> entry\r\n") );
    ctrl.cntMtx             = MCREATE("win-esaio.cnt");
    ctrl.unexpectedConnects = 0;
    ctrl.unexpectedAccepts  = 0;
    ctrl.unexpectedWrites   = 0;
    ctrl.unexpectedReads    = 0;
    ctrl.genErrs            = 0;
    ctrl.unknownCmds        = 0;
    ctrl.numThreads = (DWORD) numThreads;
    SGDBG( ("WIN-ESAIO", "esaio_init -> try initialize winsock\r\n") );
    ires = WSAStartup(MAKEWORD(2, 2), &ctrl.wsaData);
    if (ires != NO_ERROR) {
        save_errno = sock_errno();
        esock_error_msg("Failed initialize winsock: %d"
                        "\r\n   %s (%d)"
                        "\r\n",
                        ires, erl_errno_id(save_errno), save_errno);
        return ESAIO_ERR_WINSOCK_INIT;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try create I/O completion port\r\n") );
    ctrl.cport = CreateIoCompletionPort(INVALID_HANDLE_VALUE,
                                        NULL, (u_long) 0, ctrl.numThreads);
    if (ctrl.cport == NULL) {
        save_errno = sock_errno();
        esock_error_msg("Failed create I/O Completion Port:"
                        "\r\n   %s (%d)"
                        "\r\n", erl_errno_id(save_errno), save_errno);
        WSACleanup();
        return ESAIO_ERR_IOCPORT_CREATE;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try create 'service init' socket\r\n") );
    if ( !init_srv_init_socket(&save_errno) ) {
        esock_error_msg("Failed create 'service init' socket: "
                        "\r\n   %s (%d)"
                        "\r\n",
                        erl_errno_id(save_errno), save_errno);
        WSACleanup();
        return ESAIO_ERR_FSOCK_CREATE;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try extract 'accept' function\r\n") );
    ires = WSAIoctl(ctrl.srvInit, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guidAcceptEx, sizeof (guidAcceptEx),
                    &ctrl.accept, sizeof (ctrl.accept),
                    &dummy, NULL, NULL);
    if (ires == SOCKET_ERROR) {
        save_errno = sock_errno();
        esock_error_msg("Failed extracting 'accept' function: %d"
                        "\r\n   %s (%d)"
                        "\r\n",
                        ires, erl_errno_id(save_errno), save_errno);
        (void) sock_close(ctrl.srvInit);
        ctrl.srvInit  = INVALID_SOCKET;
        ctrl.accept = NULL;
        WSACleanup();
        return ESAIO_ERR_IOCTL_ACCEPT_GET;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try extract 'connect' function\r\n") );
    ires = WSAIoctl(ctrl.srvInit, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guidConnectEx, sizeof (guidConnectEx),
                    &ctrl.connect, sizeof (ctrl.connect),
                    &dummy, NULL, NULL);
    if (ires == SOCKET_ERROR) {
        save_errno = sock_errno();
        esock_error_msg("Failed extracting 'connect' function: %d"
                        "\r\n   %s (%d)"
                        "\r\n",
                        ires, erl_errno_id(save_errno), save_errno);
        (void) sock_close(ctrl.srvInit);
        ctrl.srvInit  = INVALID_SOCKET;
        ctrl.accept = NULL;
        WSACleanup();
        return ESAIO_ERR_IOCTL_CONNECT_GET;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try extract 'sendmsg' function\r\n") );
    ires = WSAIoctl(ctrl.srvInit, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guidSendMsg, sizeof (guidSendMsg),
                    &ctrl.sendmsg, sizeof (ctrl.sendmsg),
                    &dummy, NULL, NULL);
    if (ires == SOCKET_ERROR) {
        save_errno = sock_errno();
        esock_error_msg("Failed extracting 'sendmsg' function: %d"
                        "\r\n   %s (%d)"
                        "\r\n",
                        ires, erl_errno_id(save_errno), save_errno);
        (void) sock_close(ctrl.srvInit);
        ctrl.srvInit   = INVALID_SOCKET;
        ctrl.accept  = NULL;
        ctrl.connect = NULL;
        WSACleanup();
        return ESAIO_ERR_IOCTL_SENDMSG_GET;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try extract 'recvmsg' function\r\n") );
    ires = WSAIoctl(ctrl.srvInit, SIO_GET_EXTENSION_FUNCTION_POINTER,
                    &guidRecvMsg, sizeof (guidRecvMsg),
                    &ctrl.recvmsg, sizeof (ctrl.recvmsg),
                    &dummy, NULL, NULL);
    if (ires == SOCKET_ERROR) {
        save_errno = sock_errno();
        esock_error_msg("Failed extracting 'recvmsg' function: %d"
                        "\r\n   %s (%d)"
                        "\r\n",
                        ires, erl_errno_id(save_errno), save_errno);
        (void) sock_close(ctrl.srvInit);
        ctrl.srvInit   = INVALID_SOCKET;
        ctrl.accept  = NULL;
        ctrl.connect = NULL;
        ctrl.sendmsg = NULL;
        WSACleanup();
        return ESAIO_ERR_IOCTL_RECVMSG_GET;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try alloc thread pool memory\r\n") );
    ctrl.threads = MALLOC(numThreads * sizeof(ESAIOThread));
    ESOCK_ASSERT( ctrl.threads != NULL );
    SGDBG( ("WIN-ESAIO", "esaio_init -> basic init of thread data\r\n") );
    for (i = 0; i < numThreads; i++) {
        ctrl.threads[i].data.id    = i;
        ctrl.threads[i].data.state = ESAIO_THREAD_STATE_UNDEF;
        ctrl.threads[i].data.error = ESAIO_THREAD_ERROR_UNDEF;
        ctrl.threads[i].data.env   = NULL;
        ctrl.threads[i].data.cnt   = 0;
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> try create thread(s)\r\n") );
    for (i = 0; i < numThreads; i++) {
        char buf[64];
        int  j;
        ctrl.threads[i].data.state = ESAIO_THREAD_STATE_INITIATING;
        ctrl.threads[i].data.error = ESAIO_THREAD_ERROR_OK;
        SGDBG( ("WIN-ESAIO",
                "esaio_init -> try create %d thread opts\r\n", i) );
        sprintf(buf, "esaio-opts[%d]", i);
        ctrl.threads[i].optsP      = TOCREATE(buf);
        if (ctrl.threads[i].optsP == NULL) {
            esock_error_msg("Failed create thread opts %d\r\n", i);
            ctrl.threads[i].data.error = ESAIO_THREAD_ERROR_TOCREATE;
            for (j = 0; j < i; j++) {
                SGDBG( ("WIN-ESAIO",
                        "esaio_init -> destroy thread opts %d\r\n", j) );
                TODESTROY(ctrl.threads[j].optsP);
            }
            WSACleanup();
            return ESAIO_ERR_THREAD_OPTS_CREATE;
        }
        SGDBG( ("WIN-ESAIO",
                "esaio_init -> try create thread %d\r\n", i) );
        sprintf(buf, "esaio[%d]", i);
        if (0 != TCREATE(buf,
                         &ctrl.threads[i].tid,
                         esaio_completion_main,
                         (void*) &ctrl.threads[i].data,
                         ctrl.threads[i].optsP)) {
            esock_error_msg("Failed create thread %d\r\n", i);
            ctrl.threads[i].data.error = ESAIO_THREAD_ERROR_TCREATE;
            for (j = 0; j <= i; j++) {
                SGDBG( ("WIN-ESAIO",
                        "esaio_init -> destroy thread opts %d\r\n", j) );
                TODESTROY(ctrl.threads[j].optsP);
            }
            WSACleanup();
            return ESAIO_ERR_THREAD_CREATE;
        }
    }
    SGDBG( ("WIN-ESAIO", "esaio_init -> done\r\n") );
    return ESAIO_OK;
}
static
BOOLEAN_T init_srv_init_socket(int* savedErrno)
{
    SOCKET sock;
    int    save_errno = 0;
    sock = sock_open(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        save_errno = sock_errno();
        if (save_errno == WSAEAFNOSUPPORT) {
            sock = sock_open(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
            if (sock == INVALID_SOCKET) {
                ctrl.srvInit = INVALID_SOCKET;
                *savedErrno  = save_errno;
                return FALSE;
            } else {
                ctrl.srvInit  = sock;
                *savedErrno = 0;
                return TRUE;
            }
        } else {
            ctrl.srvInit = INVALID_SOCKET;
            *savedErrno  = save_errno;
            return FALSE;
        }
    } else {
        ctrl.srvInit = sock;
        *savedErrno  = 0;
        return TRUE;
    }
}
extern
void esaio_finish()
{
    int t, lastThread;
    SGDBG( ("WIN-ESAIO", "esaio_finish -> entry\r\n") );
    if (ctrl.srvInit != INVALID_SOCKET) {
        SGDBG( ("WIN-ESAIO", "esaio_finish -> close 'dummy' socket\r\n") );
        (void) sock_close(ctrl.srvInit);
        ctrl.srvInit = INVALID_SOCKET;
    }
    SGDBG( ("WIN-ESAIO",
            "esaio_finish -> try terminate %d worker threads\r\n",
            ctrl.numThreads) );
    for (t = 0, lastThread = -1, lastThread; t < ctrl.numThreads; t++) {
        ESAIOOperation* opP;
        BOOL            qres;
        SGDBG( ("WIN-ESAIO",
                "esaio_finish -> "
                "[%d] try allocate (terminate-) operation\r\n", t) );
        opP = MALLOC( sizeof(ESAIOOperation) );
        if (opP != NULL) {
            sys_memzero((char *) opP, sizeof(ESAIOOperation));
            opP->tag = ESAIO_OP_TERMINATE;
            enif_set_pid_undefined(&opP->caller);
            opP->env = NULL;
            SGDBG( ("WIN-ESAIO",
                    "esaio_finish -> "
                    "try post (terminate-) package %d\r\n", t) );
            qres = PostQueuedCompletionStatus(ctrl.cport,
                                              0, 0, (OVERLAPPED*) opP);
            if (!qres) {
                int save_errno = sock_errno();
                esock_error_msg("Failed posting 'terminate' command: "
                                "\r\n   %s (%d)"
                                "\r\n", erl_errno_id(save_errno), save_errno);
                break;
            } else {
                lastThread = t;
            }
        } else {
            SGDBG( ("WIN-ESAIO",
                    "esaio_finish -> "
                    "failed allocate (terminate-) operation %d\r\n", t) );
        }
    }
    if (lastThread >= 0) {
        SGDBG( ("WIN-ESAIO",
                "esaio_finish -> await (worker) thread(s) termination\r\n") );
        for (t = 0; t < (lastThread+1); t++) {
            SGDBG( ("WIN-ESAIO",
                    "esaio_finish -> try join with thread %d\r\n", t) );
            (void) TJOIN(ctrl.threads[t].tid, NULL);
            SGDBG( ("WIN-ESAIO", "esaio_finish -> joined with %d\r\n", t) );
        }
    }
    SGDBG( ("WIN-ESAIO", "esaio_finish -> cleanup\r\n") );
    WSACleanup();
    SGDBG( ("WIN-ESAIO", "esaio_finish -> free the thread pool data\r\n") );
    FREE( ctrl.threads );
    SGDBG( ("WIN-ESAIO",
            "esaio_finish -> invalidate (extension) functions\r\n") );
    ctrl.accept  = NULL;
    ctrl.connect = NULL;
    ctrl.sendmsg = NULL;
    ctrl.recvmsg = NULL;
    SGDBG( ("WIN-ESAIO", "esaio_finish -> done\r\n") );
    return;
}
extern
ERL_NIF_TERM esaio_info(ErlNifEnv* env)
{
    ERL_NIF_TERM info, numThreads,
        numUnexpAccs, numUnexpConns, numUnexpWs, numUnexpRs,
        numGenErrs,
        numUnknownCmds;
    numThreads     = MKUI(env, ctrl.numThreads);
    MLOCK(ctrl.cntMtx);
    numUnexpConns  = MKUI(env, ctrl.unexpectedConnects);
    numUnexpAccs   = MKUI(env, ctrl.unexpectedAccepts);
    numUnexpWs     = MKUI(env, ctrl.unexpectedWrites);
    numUnexpRs     = MKUI(env, ctrl.unexpectedReads);
    numGenErrs     = MKUI(env, ctrl.genErrs);
    numUnknownCmds = MKUI(env, ctrl.unknownCmds);
    MUNLOCK(ctrl.cntMtx);
    {
        ERL_NIF_TERM cntKeys[] = {esock_atom_num_unexpected_connects,
                                  esock_atom_num_unexpected_accepts,
                                  esock_atom_num_unexpected_writes,
                                  esock_atom_num_unexpected_reads,
                                  esock_atom_num_general_errors,
                                  esock_atom_num_unknown_cmds};
        ERL_NIF_TERM cntVals[] = {numUnexpConns, numUnexpAccs,
                                  numUnexpWs, numUnexpRs,
                                  numGenErrs,
                                  numUnknownCmds};
        unsigned int numCntKeys = NUM(cntKeys);
        unsigned int numCntVals = NUM(cntVals);
        ERL_NIF_TERM counters;
        ESOCK_ASSERT( numCntKeys == numCntVals );
        ESOCK_ASSERT( MKMA(env, cntKeys, cntVals, numCntKeys, &counters) );
        {
            ERL_NIF_TERM keys[]  = {esock_atom_name,
                                    esock_atom_num_threads,
                                    esock_atom_counters};
            ERL_NIF_TERM vals[]  = {MKA(env, "win_esaio"),
                                    numThreads,
                                    counters};
            unsigned int numKeys = NUM(keys);
            unsigned int numVals = NUM(vals);
            ESOCK_ASSERT( numKeys == numVals );
            ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &info) );
        }
    }
    return info;
}
extern
ERL_NIF_TERM esaio_command(ErlNifEnv*   env,
                           ERL_NIF_TERM command,
                           ERL_NIF_TERM cdata)
{
    ERL_NIF_TERM res;
    SGDBG( ("WIN-ESAIO", "esaio_command -> entry with %T\r\n", command) );
    if (COMPARE(command, esock_atom_socket_debug) == 0) {
        BOOLEAN_T dbg;
        if (! esock_decode_bool(cdata, &dbg)) {
            res = esock_atom_invalid;
        } else {
            ctrl.sockDbg = dbg;
            res = esock_atom_ok;
        }
    } else if (COMPARE(command, esock_atom_debug) == 0) {
        BOOLEAN_T dbg;
        if (! esock_decode_bool(cdata, &dbg)) {
            res = esock_atom_invalid;
        } else {
            ctrl.dbg = dbg;
            res = esock_atom_ok;
        }
    } else {
        res = esock_atom_invalid;
    }
    SGDBG( ("WIN-ESAIO", "esaio_command -> done when res: %T\r\n", res) );
    return esock_atom_ok;
}
extern
ERL_NIF_TERM esaio_open_plain(ErlNifEnv*       env,
                              int              domain,
                              int              type,
                              int              protocol,
                              ERL_NIF_TERM     eopts,
                              const ESockData* dataP)
{
    BOOLEAN_T        dbg    = esock_open_is_debug(env, eopts, dataP->sockDbg);
    BOOLEAN_T        useReg = esock_open_use_registry(env, eopts, dataP->useReg);
    ESockDescriptor* descP;
    ERL_NIF_TERM     sockRef;
    int              proto   = protocol;
    DWORD            dwFlags = WSA_FLAG_OVERLAPPED;
    SOCKET           sock    = INVALID_SOCKET;
    ErlNifPid        self;
    int              res, save_errno;
    ESOCK_ASSERT( enif_self(env, &self) != NULL );
    SSDBG2( dbg,
            ("WIN-ESAIO", "esaio_open_plain -> entry with"
             "\r\n   domain:   %d"
             "\r\n   type:     %d"
             "\r\n   protocol: %d"
             "\r\n   eopts:    %T"
             "\r\n", domain, type, protocol, eopts) );
    sock = sock_open_O(domain, type, proto);
    if (sock == INVALID_SOCKET) {
        save_errno = sock_errno();
        return esock_make_error_errno(env, save_errno);
    }
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> open success: %d\r\n", sock) );
    if (proto == 0)
        (void) esock_open_which_protocol(sock, &proto);
    descP           = esock_alloc_descriptor(sock);
    descP->ctrlPid  = self;
    descP->domain   = domain;
    descP->type     = type;
    descP->protocol = proto;
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> add to completion port\r\n") );
    if (ESAIO_OK != (save_errno = esaio_add_socket(descP))) {
        ERL_NIF_TERM tag    = esock_atom_add_socket;
        ERL_NIF_TERM reason = MKA(env, erl_errno_id(save_errno));
        SSDBG2( dbg, ("WIN-ESAIO",
                      "esaio_open_plain -> "
                      "failed adding socket to completion port: "
                      "%T (%d)\r\n", reason, save_errno) );
        esock_dealloc_descriptor(env, descP);
        sock_close(sock);
        return esock_make_error_t2r(env, tag, reason);
    }
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> create socket ref\r\n") );
    sockRef = enif_make_resource(env, descP);
    enif_release_resource(descP);
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> monitor owner %T\r\n", descP->ctrlPid) );
    ESOCK_ASSERT( MONP("esaio_open -> ctrl",
                       env, descP,
                       &descP->ctrlPid,
                       &descP->ctrlMon) == 0 );
    descP->dbg    = dbg;
    descP->useReg = useReg;
    esock_inc_socket(domain, type, proto);
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> maybe update registry\r\n") );
    if (descP->useReg) esock_send_reg_add_msg(env, descP, sockRef);
    SSDBG2( dbg, ("WIN-ESAIO",
                  "esaio_open_plain -> done\r\n") );
    return esock_make_ok2(env, sockRef);
}
extern
ERL_NIF_TERM esaio_bind(ErlNifEnv*       env,
                        ESockDescriptor* descP,
                        ESockAddress*    sockAddrP,
                        SOCKLEN_T        addrLen)
{
    if (! IS_OPEN(descP->readState))
        return esock_make_error_closed(env);
    if (sock_bind(descP->sock, &sockAddrP->sa, addrLen) < 0) {
        return esock_make_error_errno(env, sock_errno());
    }
    descP->writeState |= ESOCK_STATE_BOUND;
    return esock_atom_ok;
}
extern
ERL_NIF_TERM esaio_connect(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           ERL_NIF_TERM     sockRef,
                           ERL_NIF_TERM     connRef,
                           ESockAddress*    addrP,
                           SOCKLEN_T        addrLen)
{
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_connect(%T, %d) -> verify open\r\n",
            sockRef, descP->sock) );
    if (! IS_OPEN(descP->writeState))
        return esock_make_error(env, esock_atom_closed);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_connect(%T, %d) -> verify type: %s\r\n",
            sockRef, descP->sock, TYPE2STR(descP->type)) );
    switch (descP->type) {
    case SOCK_STREAM:
        return esaio_connect_stream(env,
                                    descP, sockRef, connRef,
                                    addrP, addrLen);
        break;
    case SOCK_DGRAM:
        return esaio_connect_dgram(env,
                                   descP, sockRef, connRef,
                                   addrP, addrLen);
        break;
    default:
        return enif_make_badarg(env);
    }
}
static
ERL_NIF_TERM esaio_connect_stream(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     connRef,
                                  ESockAddress*    addrP,
                                  SOCKLEN_T        addrLen)
{
    int             save_errno;
    BOOL            cres;
    ESAIOOperation* opP;
    ERL_NIF_TERM    eres;
    ErlNifPid       self;
    ESOCK_ASSERT( enif_self(env, &self) != NULL );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_connect_stream(%T) -> verify bound\r\n", sockRef) );
    if (! IS_BOUND(descP->writeState))
        return esock_make_error(env, esock_atom_not_bound);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_connect_stream(%T) -> check if ongoing\r\n",
            sockRef) );
    if (descP->connectorP != NULL) {
        if (COMPARE_PIDS(&self, &descP->connector.pid) != 0) {
            if (addrP != NULL) {
                eres = esock_make_error(env, esock_atom_already);
            } else {
                eres = esock_raise_invalid(env, esock_atom_state);
            }
        } else {
            eres = esock_raise_invalid(env, esock_atom_state);
        }
    } else if (addrP == NULL) {
        eres = esock_raise_invalid(env, esock_atom_state);
    } else {
        DWORD sentDummy = 0;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_connect_stream(%T) -> allocate (connect) operation\r\n",
                sockRef) );
        opP = MALLOC( sizeof(ESAIOOperation) );
        ESOCK_ASSERT( opP != NULL);
        sys_memzero((char*) opP, sizeof(ESAIOOperation));
        opP->tag = ESAIO_OP_CONNECT;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_connect_stream(%T) -> initiate connector\r\n",
                sockRef) );
        descP->connector.pid   = self;
        ESOCK_ASSERT( MONP("esaio_connect_stream -> conn",
                           env, descP,
                           &self, &descP->connector.mon) == 0 );
        descP->connector.dataP = (void*) opP;
        descP->connector.env   = esock_alloc_env("connector");
        descP->connector.ref   = CP_TERM(descP->connector.env, connRef);
        descP->connectorP      = &descP->connector;
        descP->writeState |=
            (ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
        opP->env                  = esock_alloc_env("esaio-connect-stream");
        opP->caller               = self;
        opP->data.connect.sockRef = CP_TERM(opP->env, sockRef);
        opP->data.connect.connRef = CP_TERM(opP->env, connRef);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_connect_stream {%d} -> try connect\r\n",
                descP->sock) );
        cres = sock_connect_O(descP->sock,
                              addrP, addrLen,
                              &sentDummy, (OVERLAPPED*) opP);
        eres = connect_stream_check_result(env, descP, opP, cres);
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_connect {%d} -> done with"
                   "\r\n   eres: %T"
                   "\r\n",
                   descP->sock, eres) );
    return eres;
}
static
ERL_NIF_TERM connect_stream_check_result(ErlNifEnv*       env,
                                         ESockDescriptor* descP,
                                         ESAIOOperation*  opP,
                                         BOOL             cres)
{
    ERL_NIF_TERM eres, tag, reason;
    int          save_errno;
    if (cres) {
        int err;
        SSDBG( descP,
               ("WIN-ESAIO",
                "connect_stream_check_result(%d) -> connected\r\n",
                descP->sock) );
        esock_requestor_release("connect_stream_check_result -> success",
                                env, descP, &descP->connector);
        descP->connectorP = NULL;
        SSDBG( descP,
               ("WIN-ESAIO",
                "connect_stream_check_result {%d} -> "
                "update connect context\r\n",
                descP->sock) );
        err = ESAIO_UPDATE_CONNECT_CONTEXT( descP->sock );
        if (err == 0) {
            descP->writeState &=
                ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
            descP->writeState |= ESOCK_STATE_CONNECTED;
            eres = esock_atom_ok;
        } else {
            save_errno = sock_errno();
            tag        = esock_atom_update_connect_context;
            reason     = ENO2T(env, save_errno);
            SSDBG( descP, ("WIN-ESAIO",
                           "connect_stream_check_result(%d) -> "
                           "connect context update failed: %T\r\n",
                           descP->sock, reason) );
            sock_close(descP->sock);
            descP->writeState = ESOCK_STATE_CLOSED;
            eres = esock_make_error_t2r(env, tag, reason);
        }
    } else {
        save_errno = sock_errno();
        if (save_errno == WSA_IO_PENDING) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "connect_stream_check_result(%d) -> connect scheduled\r\n",
                    descP->sock) );
            eres = esock_atom_completion;
        } else {
            ERL_NIF_TERM ereason = ENO2T(env, save_errno);
            SSDBG( descP, ("WIN-ESAIO",
                           "connect_stream_check_result(%d) -> "
                           "connect attempt failed: %T\r\n",
                           descP->sock, ereason) );
            esock_requestor_release("connect_stream_check_result -> failure",
                                    env, descP, &descP->connector);
            descP->connectorP = NULL;
            esock_clear_env("connect_stream_check_result", opP->env);
            esock_free_env("connect_stream_check_result", opP->env);
            FREE( opP );
            sock_close(descP->sock);
            descP->writeState = ESOCK_STATE_CLOSED;
            eres = esock_make_error(env, ereason);
        }
    }
    return eres;
}
static
ERL_NIF_TERM esaio_connect_dgram(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     connRef,
                                 ESockAddress*    addrP,
                                 SOCKLEN_T        addrLen)
{
    int       save_errno;
    ErlNifPid self;
    ESOCK_ASSERT( enif_self(env, &self) != NULL );
    if (! IS_OPEN(descP->writeState))
        return esock_make_error_closed(env);
    if (descP->connectorP != NULL) {
        return esock_make_error(env, esock_atom_already);
    }
    if (addrP == NULL) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    if (sock_connect(descP->sock, (struct sockaddr*) addrP, addrLen) == 0) {
        SSDBG( descP, ("WIN-ESAIO",
                       "essio_connect_dgram {%d} -> connected\r\n",
                       descP->sock) );
        descP->writeState |= ESOCK_STATE_CONNECTED;
        return esock_atom_ok;
    }
    save_errno = sock_errno();
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_connect_dgram {%d} -> error: %d\r\n",
            descP->sock, save_errno) );
    return esock_make_error_errno(env, save_errno);
}
extern
ERL_NIF_TERM esaio_accept(ErlNifEnv*       env,
                          ESockDescriptor* descP,
                          ERL_NIF_TERM     sockRef,
                          ERL_NIF_TERM     accRef)
{
    ErlNifPid       caller;
    ERL_NIF_TERM    eres;
    SOCKET          accSock;
    ESAIOOperation* opP;
    BOOLEAN_T       ares;
    unsigned int    addrSz, bufSz;
    DWORD           recvBytes;
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (esock_acceptor_search4pid(env, descP, &caller)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_accept {%d} -> verify not reading\r\n",
                   descP->sock) );
    if (descP->readersQ.first != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    SSDBG( descP, ("WIN-ESAIO",
                   "esaio_accept {%d} -> allocate 'operation'\r\n",
                   descP->sock) );
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_ACCEPT;
    SSDBG( descP, ("WIN-ESAIO",
                   "esaio_accept {%d} -> initiate 'operation'\r\n",
                   descP->sock) );
    opP->env                  = esock_alloc_env("esaio_accept - operation");
    opP->data.accept.lSockRef = CP_TERM(opP->env, sockRef);
    opP->data.accept.accRef   = CP_TERM(opP->env, accRef);
    opP->data.accept.lsock    = descP->sock;
    opP->caller               = caller;
    SSDBG( descP, ("WIN-ESAIO",
                   "esaio_accept {%d} -> try create 'accepting' socket\r\n",
                   descP->sock) );
    accSock = sock_open(descP->domain, descP->type, descP->protocol);
    if (accSock == INVALID_SOCKET) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = MKA(env, erl_errno_id(save_errno));
        ERL_NIF_TERM tag        = esock_atom_create_accept_socket;
        esock_clear_env("esaio_accept - invalid accept socket", opP->env);
        esock_free_env("esaio_accept - invalid accept socket", opP->env);
        FREE( opP );
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_accept {%d} -> failed create 'accepting' socket:"
                "\r\n   %T (%d)"
                "\r\n",
                descP->sock, reason, save_errno) );
        return esock_make_error_t2r(env, tag, reason);
    }
    opP->data.accept.asock = accSock;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_accept {%d} -> "
            "try calculate address and address buffer size(s)\r\n",
            descP->sock) );
    switch (descP->domain) {
    case AF_INET:
        addrSz = sizeof(struct sockaddr_in) + 16;
        break;
    case AF_INET6:
        addrSz = sizeof(struct sockaddr_in6) + 16;
        break;
    case AF_LOCAL:
        addrSz = sizeof(struct sockaddr_un) + 16;
        break;
    default:
        return esock_make_error_invalid(env, esock_atom_domain);
        break;
    }
    bufSz  = 2 * addrSz;
    opP->data.accept.buf = MALLOC( bufSz );
    ESOCK_ASSERT( opP->data.accept.buf != NULL);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_accept(%T, %d) -> try accept\r\n",
            sockRef, descP->sock) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_acc_tries, &descP->accTries, 1);
    ares = sock_accept_O(descP->sock, accSock,
                         opP->data.accept.buf,
                         addrSz,
                         &recvBytes,
                         (OVERLAPPED*) opP);
    eres = accept_check_result(env, descP, opP, ares,
                               sockRef, accRef, accSock, caller);
    SSDBG( descP, ("WIN-ESAIO", "esaio_accept(%T, %d) -> done when"
                   "\r\n   eres: %T"
                   "\r\n", sockRef, descP->sock, eres) );
    return eres;
}
static
ERL_NIF_TERM accept_check_result(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ESAIOOperation*  opP,
                                 BOOL             ares,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     accRef,
                                 SOCKET           accSock,
                                 ErlNifPid        caller)
{
    ERL_NIF_TERM eres;
   if (ares) {
       eres = esaio_accept_accepted(env, descP, caller, sockRef, accSock);
    } else {
        int save_errno = sock_errno();
        if (save_errno == WSA_IO_PENDING) {
            eres = accept_check_pending(env, descP, opP, caller,
                                        sockRef, accRef);
        } else {
            eres = accept_check_fail(env, descP, opP, save_errno,
                                     accSock, sockRef);
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "accept_check_result(%T, %d) -> done with"
            "\r\n  result: %T"
            "\r\n", sockRef, descP->sock, eres) );
    return eres;
}
static
ERL_NIF_TERM accept_check_pending(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ESAIOOperation*  opP,
                                  ErlNifPid        caller,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     accRef)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "accept_check_pending(%T, %d) -> entry with"
            "\r\n   accRef: %T"
            "\r\n", sockRef, descP->sock, accRef) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_acc_waits, &descP->accWaits, 1);
    if (descP->acceptorsQ.first == NULL)
        descP->readState |= (ESOCK_STATE_ACCEPTING | ESOCK_STATE_SELECTED);
    esock_acceptor_push(env, descP, caller, accRef, opP);
    return esock_atom_completion;
}
static
ERL_NIF_TERM accept_check_fail(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ESAIOOperation*  opP,
                               int              saveErrno,
                               SOCKET           accSock,
                               ERL_NIF_TERM     sockRef)
{
    ERL_NIF_TERM reason;
    SSDBG( descP,
           ("WIN-ESAIO",
            "accept_check_fail(%T, %d) -> entry with"
            "\r\n   errno:        %d"
            "\r\n   (acc) socket: %d"
            "\r\n", sockRef, descP->sock, saveErrno, accSock) );
    reason = MKA(env, erl_errno_id(saveErrno));
    esock_clear_env("esaio_accept", opP->env);
    esock_free_env("esaio_accept", opP->env);
    FREE( opP->data.accept.buf );
    FREE( opP );
    sock_close(accSock);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_acc_fails, &descP->accFails, 1);
    return esock_make_error(env, reason);
}
static
ERL_NIF_TERM esaio_accept_accepted(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ErlNifPid        pid,
                                   ERL_NIF_TERM     sockRef,
                                   SOCKET           accSock)
{
    ESockDescriptor* accDescP;
    ERL_NIF_TERM     accRef;
    int              save_errno;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_accept_accepted(%T, %d) -> entry with"
            "\r\n   (accept) socket: %T"
            "\r\n", sockRef, descP->sock, accSock) );
    accDescP = esock_alloc_descriptor(accSock);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_accept_accepted(%T, %d) -> add to completion port\r\n",
            sockRef, descP->sock) );
    if (ESAIO_OK != (save_errno = esaio_add_socket(accDescP))) {
        ERL_NIF_TERM tag    = esock_atom_add_socket;
        ERL_NIF_TERM reason = MKA(env, erl_errno_id(save_errno));
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_acc_fails, &descP->accFails, 1);
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_accept_accepted(%T, %d) -> "
                "failed adding (accepted) socket to completion port: "
                "%T (%d)\r\n", sockRef, descP->sock, reason, save_errno) );
        esock_dealloc_descriptor(env, accDescP);
        sock_close(accSock);
        return esock_make_error_t2r(env, tag, reason);
    }
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_acc_success, &descP->accSuccess, 1);
    accDescP->domain   = descP->domain;
    accDescP->type     = descP->type;
    accDescP->protocol = descP->protocol;
    MLOCK(descP->writeMtx);
    accDescP->rBufSz   = descP->rBufSz;
    accDescP->rCtrlSz  = descP->rCtrlSz;
    accDescP->wCtrlSz  = descP->wCtrlSz;
    accDescP->iow      = descP->iow;
    accDescP->dbg      = descP->dbg;
    accDescP->useReg   = descP->useReg;
    esock_inc_socket(accDescP->domain, accDescP->type, accDescP->protocol);
    accRef = enif_make_resource(env, accDescP);
    enif_release_resource(accDescP);
    accDescP->ctrlPid = pid;
    ESOCK_ASSERT( MONP("esaio_accept_accepted -> ctrl",
                       env, accDescP,
                       &accDescP->ctrlPid,
                       &accDescP->ctrlMon) == 0 );
    accDescP->writeState |= ESOCK_STATE_CONNECTED;
    MUNLOCK(descP->writeMtx);
    if (descP->useReg) esock_send_reg_add_msg(env, descP, accRef);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_accept_accepted(%T, %d) -> done\r\n",
            sockRef, descP->sock) );
    return esock_make_ok2(env, accRef);
}
extern
ERL_NIF_TERM esaio_send(ErlNifEnv*       env,
                        ESockDescriptor* descP,
                        ERL_NIF_TERM     sockRef,
                        ERL_NIF_TERM     sendRef,
                        ErlNifBinary*    sndDataP,
                        int              flags)
{
    ErlNifPid       caller;
    ERL_NIF_TERM    eres;
    BOOLEAN_T       cleanup = FALSE;
    int             wres;
    DWORD           toWrite;
    char*           buf;
    DWORD           f = (DWORD) flags;
    ESAIOOperation* opP;
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->writeState)) {
        ESOCK_EPRINTF("esaio_send(%T, %d) -> NOT OPEN\r\n",
                      sockRef, descP->sock);
        return esock_make_error_closed(env);
    }
    if (descP->connectorP != NULL) {
        ESOCK_EPRINTF("esaio_send(%T, %d) -> CONNECTING\r\n",
                      sockRef, descP->sock);
        return esock_make_error_invalid(env, esock_atom_state);
    }
    if (esock_writer_search4pid(env, descP, &caller)) {
        ESOCK_EPRINTF("esaio_send(%T, %d) -> ALREADY SENDING\r\n",
                      sockRef, descP->sock);
        return esock_raise_invalid(env, esock_atom_state);
    }
    toWrite = (DWORD) sndDataP->size;
    if ((size_t) toWrite != sndDataP->size)
        return esock_make_error_invalid(env, esock_atom_data_size);
    buf = MALLOC( toWrite );
    ESOCK_ASSERT( buf != NULL );
    sys_memcpy(buf, sndDataP->data, toWrite);
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_SEND;
    opP->env                = esock_alloc_env("esaio-send - operation");
    opP->data.send.sendRef  = CP_TERM(opP->env, sendRef);
    opP->data.send.sockRef  = CP_TERM(opP->env, sockRef);
    opP->data.send.wbuf.buf = buf;
    opP->data.send.wbuf.len = toWrite;
    opP->caller             = caller;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_tries, &descP->writeTries, 1);
    wres = sock_send_O(descP->sock, &opP->data.send.wbuf, f, (OVERLAPPED*) opP);
    eres = send_check_result(env, descP, opP, caller,
                             wres, toWrite, FALSE,
                             sockRef, &opP->data.send.sendRef, &cleanup);
    if (cleanup) {
        FREE( opP->data.send.wbuf.buf );
        esock_clear_env("esaio_send - cleanup", opP->env);
        esock_free_env("esaio_send - cleanup", opP->env);
        FREE( opP );
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_send {%d} -> done (%s)"
            "\r\n   %T"
            "\r\n", descP->sock, B2S(cleanup), eres) );
    return eres;
}
extern
ERL_NIF_TERM esaio_sendto(ErlNifEnv*       env,
                          ESockDescriptor* descP,
                          ERL_NIF_TERM     sockRef,
                          ERL_NIF_TERM     sendRef,
                          ErlNifBinary*    sndDataP,
                          int              flags,
                          ESockAddress*    toAddrP,
                          SOCKLEN_T        toAddrLen)
{
    ErlNifPid       caller;
    ERL_NIF_TERM    eres;
    BOOLEAN_T       cleanup = FALSE;
    int             wres;
    DWORD           toWrite;
    char*           buf;
    DWORD           f = (DWORD) flags;
    ESAIOOperation* opP;
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->writeState))
        return esock_make_error_closed(env);
    if (descP->connectorP != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    if (toAddrP == NULL)
        return esock_make_invalid(env, esock_atom_sockaddr);
    if (esock_writer_search4pid(env, descP, &caller)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    toWrite = (DWORD) sndDataP->size;
    if ((size_t) toWrite != sndDataP->size)
        return esock_make_error_invalid(env, esock_atom_data_size);
    buf = MALLOC( toWrite );
    ESOCK_ASSERT( buf != NULL );
    sys_memcpy(buf, sndDataP->data, toWrite);
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_SENDTO;
    opP->env = esock_alloc_env("esaio-sendto - operation");
    opP->data.sendto.sendRef       = CP_TERM(opP->env, sendRef);
    opP->data.sendto.sockRef       = CP_TERM(opP->env, sockRef);
    opP->data.sendto.wbuf.buf      = buf;
    opP->data.sendto.wbuf.len      = toWrite;
    opP->data.sendto.remoteAddr    = *toAddrP;
    opP->data.sendto.remoteAddrLen = toAddrLen;
    opP->caller                    = caller;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_tries, &descP->writeTries, 1);
    wres = sock_sendto_O(descP->sock, &opP->data.sendto.wbuf, f,
                         (struct sockaddr*) toAddrP, toAddrLen,
                         (OVERLAPPED*) opP);
    eres = send_check_result(env, descP, opP, caller,
                             wres, toWrite, FALSE,
                             sockRef, &opP->data.sendto.sendRef, &cleanup);
    if (cleanup) {
        FREE( opP->data.sendto.wbuf.buf );
        esock_clear_env("esaio_sendto - cleanup", opP->env);
        esock_free_env("esaio_sendto - cleanup", opP->env);
        FREE( opP );
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendto {%d} -> done (%s)"
            "\r\n   %T"
            "\r\n", descP->sock, B2S(cleanup), eres) );
    return eres;
}
extern
ERL_NIF_TERM esaio_sendmsg(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           ERL_NIF_TERM     sockRef,
                           ERL_NIF_TERM     sendRef,
                           ERL_NIF_TERM     eMsg,
                           int              flags,
                           ERL_NIF_TERM     eIOV,
                           const ESockData* dataP)
{
    ErlNifPid       caller;
    ERL_NIF_TERM    eres;
    BOOLEAN_T       cleanup = FALSE;
    int             wres;
    ERL_NIF_TERM    tail;
    ERL_NIF_TERM    eAddr, eCtrl;
    size_t          dataSize;
    size_t          ctrlBufLen,  ctrlBufUsed;
    WSABUF*         wbufs = NULL;
    ESAIOOperation* opP   = NULL;
    SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> entry"
                   "\r\n", sockRef, descP->sock) );
    if (! ((descP->type == SOCK_DGRAM) || (descP->type == SOCK_RAW))) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "NOT SUPPORTED FOR TYPE %d (%s)"
                       "\r\n", sockRef, descP->sock, TYPE2STR(descP->type)) );
        return enif_raise_exception(env, MKA(env, "notsup"));
    }
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->writeState))
        return esock_make_error_closed(env);
    if (descP->connectorP != NULL) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "simultaneous connect and send not allowed"
                       "\r\n", sockRef, descP->sock) );
        return esock_make_error_invalid(env, esock_atom_state);
    }
    if (esock_writer_search4pid(env, descP, &caller)) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "already sending"
                       "\r\n", sockRef, descP->sock) );
        return esock_raise_invalid(env, esock_atom_state);
    }
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_SENDMSG;
    if (! init_sendmsg_sockaddr(env, descP, eMsg,
                                &opP->data.sendmsg.msg,
                                &opP->data.sendmsg.addr)) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "address"
                       "\r\n", sockRef, descP->sock) );
        FREE( opP );
        return esock_make_invalid(env, esock_atom_addr);
    }
    opP->env = esock_alloc_env("esaio_sendmsg - operation");
    if ((! enif_inspect_iovec(opP->env,
                              dataP->iov_max, eIOV, &tail,
                              &opP->data.sendmsg.iovec))) {
        SSDBG( descP, ("WIN-ESAIO",
                       "essaio_sendmsg {%d} -> not an iov\r\n",
                       descP->sock) );
        esock_free_env("esaio-sendmsg - iovec failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg {%d} ->"
                   "\r\n   iovcnt: %lu"
                   "\r\n   tail:   %s"
                   "\r\n", descP->sock,
                   (unsigned long) opP->data.sendmsg.iovec->iovcnt,
                   B2S(! enif_is_empty_list(opP->env, tail))) );
    if (! verify_sendmsg_iovec_size(dataP, descP, opP->data.sendmsg.iovec)) {
        SSDBG( descP, ("WIN-ESAIO",
                       "esaio_sendmsg {%d} -> iovcnt > iov_max\r\n",
                       descP->sock) );
        esock_free_env("esaio-sendmsg - iovec failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    if (! verify_sendmsg_iovec_tail(opP->env, descP, &tail)) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "tail not empty"
                       "\r\n", sockRef, descP->sock) );
        esock_free_env("esaio-sendmsg - iovec tail failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    if (! check_sendmsg_iovec_overflow(descP,
                                       opP->data.sendmsg.iovec, &dataSize)) {
        SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg(%T, %d) -> "
                       "iovec size failure"
                       "\r\n", sockRef, descP->sock) );
        esock_free_env("esaio-sendmsg - iovec size failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_sendmsg {%d} -> iovec size verified"
            "\r\n   iov length: %lu"
            "\r\n   data size:  %lu"
            "\r\n",
            descP->sock,
            (unsigned long) opP->data.sendmsg.iovec->iovcnt,
            (unsigned long) dataSize) );
    wbufs = MALLOC(opP->data.sendmsg.iovec->iovcnt * sizeof(WSABUF));
    ESOCK_ASSERT( wbufs != NULL );
    for (int i = 0; i < opP->data.sendmsg.iovec->iovcnt; i++) {
        wbufs[i].len = opP->data.sendmsg.iovec->iov[i].iov_len;
        wbufs[i].buf = opP->data.sendmsg.iovec->iov[i].iov_base;
    }
    opP->data.sendmsg.msg.lpBuffers     = wbufs;
    opP->data.sendmsg.msg.dwBufferCount = opP->data.sendmsg.iovec->iovcnt;
    eCtrl                     = esock_atom_undefined;
    ctrlBufLen                = 0;
    opP->data.sendmsg.ctrlBuf = NULL;
    if (GET_MAP_VAL(env, eMsg, esock_atom_ctrl, &eCtrl)) {
        ctrlBufLen                = descP->wCtrlSz;
        opP->data.sendmsg.ctrlBuf = (char*) MALLOC(ctrlBufLen);
        ESOCK_ASSERT( opP->data.sendmsg.ctrlBuf != NULL );
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_sendmsg {%d} -> optional ctrl: "
                   "\r\n   ctrlBuf:    %p"
                   "\r\n   ctrlBufLen: %lu"
                   "\r\n   eCtrl:      %T"
                   "\r\n", descP->sock,
                   opP->data.sendmsg.ctrlBuf,
                   (unsigned long) ctrlBufLen, eCtrl) );
    if (opP->data.sendmsg.ctrlBuf != NULL) {
        if (! decode_cmsghdrs(env, descP,
                              eCtrl,
                              opP->data.sendmsg.ctrlBuf, ctrlBufLen,
                              &ctrlBufUsed)) {
            FREE( opP->data.sendmsg.ctrlBuf );
            FREE( opP->data.sendmsg.msg.lpBuffers );
            esock_free_env("esaio-sendmsg - iovec size failure", opP->env);
            FREE( opP );
            return esock_make_invalid(env, esock_atom_ctrl);
        }
    } else {
         ctrlBufUsed = 0;
    }
    opP->data.sendmsg.msg.Control.len = ctrlBufUsed;
    opP->data.sendmsg.msg.Control.buf = opP->data.sendmsg.ctrlBuf;
    opP->data.sendmsg.msg.dwFlags = 0;
    opP->tag                  = ESAIO_OP_SENDMSG;
    opP->caller               = caller;
    opP->data.sendmsg.sockRef = CP_TERM(opP->env, sockRef);
    opP->data.sendmsg.sendRef = CP_TERM(opP->env, sendRef);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_tries, &descP->writeTries, 1);
    wres = sock_sendmsg_O(descP->sock, &opP->data.sendmsg.msg, flags,
                          (OVERLAPPED*) opP);
    eres = send_check_result(env, descP, opP, caller,
                             wres, dataSize,
                             (! enif_is_empty_list(opP->env, tail)),
                             sockRef, &opP->data.sendmsg.sendRef, &cleanup);
    if (cleanup) {
        FREE( opP->data.sendmsg.msg.lpBuffers );
        if (opP->data.sendmsg.ctrlBuf != NULL)
            FREE( opP->data.sendmsg.ctrlBuf );
        esock_clear_env("esaio_sendmsg - cleanup", opP->env);
        esock_free_env("esaio_sendmsg - cleanup", opP->env);
        FREE( opP );
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendmsg {%d} -> done (%s)"
            "\r\n   %T"
            "\r\n", descP->sock, B2S(cleanup), eres) );
    return eres;
}
static
BOOLEAN_T init_sendmsg_sockaddr(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     eMsg,
                                WSAMSG*          msgP,
                                ESockAddress*    addrP)
{
    ERL_NIF_TERM eAddr;
    if (! GET_MAP_VAL(env, eMsg, esock_atom_addr, &eAddr)) {
        SSDBG( descP, ("WIN-ESAIO",
                       "init_sendmsg_sockaddr {%d} -> no address\r\n",
                       descP->sock) );
        msgP->name    = NULL;
        msgP->namelen = 0;
    } else {
        SSDBG( descP, ("WIN-ESAIO", "init_sendmsg_sockaddr {%d} ->"
                       "\r\n   address: %T"
                       "\r\n", descP->sock, eAddr) );
        msgP->name    = (void*) addrP;
        msgP->namelen = sizeof(ESockAddress);
        sys_memzero((char *) msgP->name, msgP->namelen);
        if (! esock_decode_sockaddr(env, eAddr,
                                    (ESockAddress*) msgP->name,
                                    (SOCKLEN_T*) &msgP->namelen)) {
            SSDBG( descP, ("WIN-ESAIO",
                           "init_sendmsg_sockaddr {%d} -> invalid address\r\n",
                           descP->sock) );
            return FALSE;
        }
    }
    return TRUE;
}
static
BOOLEAN_T verify_sendmsg_iovec_size(const ESockData* dataP,
                                    ESockDescriptor* descP,
                                    ErlNifIOVec*     iovec)
{
    if (iovec->iovcnt > dataP->iov_max) {
        if (descP->type == SOCK_STREAM) {
            iovec->iovcnt = dataP->iov_max;
        } else {
            return FALSE;
        }
    }
    return TRUE;
}
static
BOOLEAN_T verify_sendmsg_iovec_tail(ErlNifEnv*       env,
                                    ESockDescriptor* descP,
                                    ERL_NIF_TERM*    tail)
{
#if defined(ESOCK_SENDV_COUNT_DATA_IN_TAIL)
    DWORD        dataInTail = 0;
    DWORD        binCount = 0;
#endif
    ERL_NIF_TERM h, t, tmp = *tail;
    ErlNifBinary bin;
#if defined(ESOCK_SENDV_COUNT_DATA_IN_TAIL)
    for (;;) {
        if (enif_get_list_cell(env, tmp, &h, &t) &&
            enif_inspect_binary(env, h, &bin)) {
            binCount   += 1;
            dataInTail += bin.size;
            tmp = t;
            continue;
        } else
            break;
    }
    esock_debug_msg("verify_sendmsg_iovec_tail(%d) -> "
                    "\r\n   Number of bins in tail: %lu"
                    "\r\n   data in tail:           %lu"
                    "\r\n",
                    descP->sock, binCount, dataInTail);
#else
    for (;;) {
        if (enif_get_list_cell(env, tmp, &h, &t) &&
            enif_inspect_binary(env, h, &bin) &&
            (bin.size == 0)) {
            tmp = t;
            continue;
        } else
            break;
    }
#endif
    *tail = tmp;
#if defined(ESOCK_SENDV_COUNT_DATA_IN_TAIL)
    if ((dataInTail > 0) && (descP->type != SOCK_STREAM)) {
#else
    if ((! enif_is_empty_list(env, tmp)) && (descP->type != SOCK_STREAM)) {
#endif
        SSDBG( descP, ("WIN-ESAIO",
                       "verify_sendmsg_iovec_tail {%d} -> invalid tail\r\n",
                       descP->sock) );
        return FALSE;
    }
    return TRUE;
}
static
BOOLEAN_T check_sendmsg_iovec_overflow(ESockDescriptor* descP,
                                       ErlNifIOVec*     iovec,
                                       size_t*          dataSize)
{
    size_t dsz = 0;
    size_t i;
    for (i = 0;  i < iovec->iovcnt;  i++) {
        size_t len = iovec->iov[i].iov_len;
        dsz += len;
        if (dsz < len) {
            SSDBG( descP, ("WIN-ESAIO",
                           "check_sendmsg_iovec_overflow {%d} -> Overflow"
                           "\r\n   i:         %lu"
                           "\r\n   len:       %lu"
                           "\r\n   dataSize:  %ld"
                           "\r\n", descP->sock, (unsigned long) i,
                           (unsigned long) len, (unsigned long) dsz) );
            *dataSize = dsz;
            return FALSE;
        }
    }
    *dataSize = dsz;
    return TRUE;
}
extern
ERL_NIF_TERM esaio_sendv(ErlNifEnv*       env,
                         ESockDescriptor* descP,
                         ERL_NIF_TERM     sockRef,
                         ERL_NIF_TERM     sendRef,
                         ERL_NIF_TERM     eIOV,
                         const ESockData* dataP)
{
    ErlNifPid       caller;
    ERL_NIF_TERM    eres;
    ESockAddress    addr;
    ERL_NIF_TERM    tail;
    size_t          dataSize;
    ssize_t         sendv_result;
    BOOLEAN_T       dataInTail, cleanup;
    WSABUF*         wbufs = NULL;
    ESAIOOperation* opP   = NULL;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> get caller\r\n",
            descP->sock) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> check state (0x%lX)\r\n",
            descP->sock, descP->writeState) );
    if (! IS_OPEN(descP->writeState))
        return esock_make_error_closed(env);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> check if connecting\r\n",
            descP->sock) );
    if (descP->connectorP != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> check if already writing\r\n",
            descP->sock) );
    if (esock_writer_search4pid(env, descP, &caller)) {
        ESOCK_EPRINTF("esaio_send(%T, %d) -> ALREADY SENDING\r\n",
                      sockRef, descP->sock);
        return esock_raise_invalid(env, esock_atom_state);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> allocate operation\r\n",
            descP->sock) );
    opP = MALLOC( sizeof(ESAIOOperation) );
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> "
            "\r\n   operation:  0x%lX"
            "\r\n",
            descP->sock, opP) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_SENDV;
    opP->env                = esock_alloc_env("esaio_sendv - operation");
    opP->data.sendv.sendRef = CP_TERM(opP->env, sendRef);
    opP->data.sendv.sockRef = CP_TERM(opP->env, sockRef);
    opP->caller             = caller;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> extract I/O vector\r\n",
            descP->sock) );
    if (! enif_inspect_iovec(opP->env,
                             dataP->iov_max, eIOV, &tail,
                             &opP->data.sendv.iovec)) {
        SSDBG( descP, ("WIN-ESAIO",
                       "esaio_sendv(%d) -> iov inspection failed\r\n",
                       descP->sock) );
        esock_free_env("esaio-sendv - iovec inspection failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> I/O vector: "
            "\r\n   iovec:           0x%lX"
            "\r\n   (vector) length: %lu elements"
            "\r\n",
            descP->sock,
            opP->data.sendv.iovec,
            opP->data.sendv.iovec->iovcnt) );
    if (opP->data.sendv.iovec == NULL) {
        SSDBG( descP, ("WIN-ESAIO",
                       "esaio_sendv(%d) -> not an iov\r\n",
                       descP->sock) );
        esock_free_env("esaio-sendv - iovec failure", opP->env);
        FREE( opP );
        return esock_make_invalid(env, esock_atom_iov);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> check if data in tail\r\n",
            descP->sock) );
    dataInTail = (! enif_is_empty_list(opP->env, tail));
    SSDBG( descP, ("WIN-ESAIO", "esaio_sendv(%d) -> verify iovec tail"
                   "\r\n   data in tail: %s"
                   "\r\n", descP->sock, B2S(dataInTail)) );
    if (! verify_sendmsg_iovec_tail(opP->env, descP, &tail)) {
        esock_free_env("esaio-sendv - iovec tail failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> check iovec overflow\r\n",
            descP->sock) );
    if (! check_sendmsg_iovec_overflow(descP,
                                       opP->data.sendv.iovec, &dataSize)) {
        esock_free_env("esaio-sendv - iovec size failure", opP->env);
        FREE( opP );
        return esock_make_error_invalid(env, esock_atom_iov);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_sendv(%d) -> iovec size verified"
            "\r\n   data in tail: %s"
            "\r\n   data size:    %u"
            "\r\n",
            descP->sock,
            B2S(dataInTail),
            (long) dataSize) );
    wbufs = MALLOC(opP->data.sendv.iovec->iovcnt * sizeof(WSABUF));
    ESOCK_ASSERT( wbufs != NULL );
    for (int i = 0; i < opP->data.sendv.iovec->iovcnt; i++) {
        wbufs[i].len = opP->data.sendv.iovec->iov[i].iov_len;
        wbufs[i].buf = opP->data.sendv.iovec->iov[i].iov_base;
    }
    opP->data.sendv.lpBuffers     = wbufs;
    opP->data.sendv.dwBufferCount = opP->data.sendv.iovec->iovcnt;
    opP->data.sendv.toWrite       = dataSize;
    opP->data.sendv.dataInTail    = dataInTail;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_tries, &descP->writeTries, 1);
    sendv_result = sock_sendv_O(descP->sock,
                                (LPWSABUF) opP->data.sendv.lpBuffers,
                                opP->data.sendv.dwBufferCount,
                                (OVERLAPPED*) opP);
    eres = send_check_result(env, descP, opP, caller,
                             sendv_result, dataSize, dataInTail,
                             sockRef, &opP->data.sendv.sendRef, &cleanup);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> sent and analyzed: %d\r\n",
            descP->sock, sendv_result) );
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_sendv(%d) -> done"
            "\r\n   result: %T"
            "\r\n", descP->sock, eres) );
    return eres;
}
static
BOOLEAN_T decode_cmsghdrs(ErlNifEnv*       env,
                          ESockDescriptor* descP,
                          ERL_NIF_TERM     eCMsg,
                          char*            cmsgHdrBufP,
                          size_t           cmsgHdrBufLen,
                          size_t*          cmsgHdrBufUsed)
{
    ERL_NIF_TERM elem, tail, list;
    char*        bufP;
    size_t       rem, used, totUsed = 0;
    unsigned int len;
    int          i;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdrs {%d} -> entry with"
                   "\r\n   eCMsg:         %T"
                   "\r\n   cmsgHdrBufP:   0x%lX"
                   "\r\n   cmsgHdrBufLen: %d"
                   "\r\n", descP->sock,
                   eCMsg, cmsgHdrBufP, cmsgHdrBufLen) );
    if (! GET_LIST_LEN(env, eCMsg, &len))
        return FALSE;
    SSDBG( descP,
           ("WIN-ESAIO",
            "decode_cmsghdrs {%d} -> list length: %d\r\n",
            descP->sock, len) );
    for (i = 0, list = eCMsg, rem  = cmsgHdrBufLen, bufP = cmsgHdrBufP;
         i < len; i++) {
        SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdrs {%d} -> process elem %d:"
                       "\r\n   (buffer) rem:     %u"
                       "\r\n   (buffer) totUsed: %u"
                       "\r\n", descP->sock, i, rem, totUsed) );
        if (! GET_LIST_ELEM(env, list, &elem, &tail))
            return FALSE;
        used = 0;
        if (! decode_cmsghdr(env, descP, elem, bufP, rem, &used))
            return FALSE;
#ifdef __WIN32__
        bufP     = CHARP( bufP + used );
#else
        bufP     = CHARP( ULONG(bufP) + used );
#endif
        rem      = SZT( rem - used );
        list     = tail;
        totUsed += used;
    }
    *cmsgHdrBufUsed = totUsed;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdrs {%d} -> done"
                   "\r\n   all %u ctrl headers processed"
                   "\r\n   totUsed = %lu\r\n",
                   descP->sock, len, (unsigned long) totUsed) );
    return TRUE;
}
static
BOOLEAN_T decode_cmsghdr(ErlNifEnv*       env,
                         ESockDescriptor* descP,
                         ERL_NIF_TERM     eCMsg,
                         char*            bufP,
                         size_t           rem,
                         size_t*          used)
{
    ERL_NIF_TERM eLevel, eType, eData, eValue;
    int          level;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d} -> entry with"
                   "\r\n   eCMsg: %T"
                   "\r\n", descP->sock, eCMsg) );
    if (! GET_MAP_VAL(env, eCMsg, esock_atom_level, &eLevel))
        return FALSE;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d} -> eLevel: %T"
                   "\r\n", descP->sock, eLevel) );
    if (! GET_MAP_VAL(env, eCMsg, esock_atom_type, &eType))
        return FALSE;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d} -> eType:  %T"
                   "\r\n", descP->sock, eType) );
    if (! esock_decode_level(env, eLevel, &level))
        return FALSE;
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d}-> level:  %d\r\n",
                   descP->sock, level) );
    if (! GET_MAP_VAL(env, eCMsg, esock_atom_data, &eData)) {
        if (! GET_MAP_VAL(env, eCMsg, esock_atom_value, &eValue))
            return FALSE;
        SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d} -> eValue:  %T"
                   "\r\n", descP->sock, eValue) );
        if (! decode_cmsghdr_value(env, descP, level, eType, eValue,
                                   bufP, rem, used))
            return FALSE;
    } else {
        if (GET_MAP_VAL(env, eCMsg, esock_atom_value, &eValue))
            return FALSE;
        SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d} -> eData:  %T"
                   "\r\n", descP->sock, eData) );
        if (! decode_cmsghdr_data(env, descP, level, eType, eData,
                                  bufP, rem, used))
            return FALSE;
    }
    SSDBG( descP, ("WIN-ESAIO", "decode_cmsghdr {%d}-> used:  %lu\r\n",
                   descP->sock, (unsigned long) *used) );
    return TRUE;
}
static
BOOLEAN_T decode_cmsghdr_value(ErlNifEnv*   env,
                               ESockDescriptor* descP,
                               int          level,
                               ERL_NIF_TERM eType,
                               ERL_NIF_TERM eValue,
                               char*        bufP,
                               size_t       rem,
                               size_t*      usedP)
{
    int type;
    struct cmsghdr* cmsgP     = (struct cmsghdr *) bufP;
    ESockCmsgSpec*  cmsgTable;
    ESockCmsgSpec*  cmsgSpecP = NULL;
    size_t          num       = 0;
    SSDBG( descP,
           ("WIN-ESAIO",
            "decode_cmsghdr_value {%d} -> entry  \r\n"
            "   eType:  %T\r\n"
            "   eValue: %T\r\n",
            descP->sock, eType, eValue) );
    if (! IS_ATOM(env, eType)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "decode_cmsghdr_value {%d} -> FALSE:\r\n"
                "   eType not an atom\r\n",
                descP->sock) );
        return FALSE;
    }
    if (((cmsgTable = esock_lookup_cmsg_table(level, &num)) == NULL) ||
        ((cmsgSpecP = esock_lookup_cmsg_spec(cmsgTable, num, eType)) == NULL) ||
        (cmsgSpecP->decode == NULL)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "decode_cmsghdr_value {%d} -> FALSE:\r\n"
                "   cmsgTable:  %p\r\n"
                "   cmsgSpecP:  %p\r\n",
                descP->sock, cmsgTable, cmsgSpecP) );
        return FALSE;
    }
    if (! cmsgSpecP->decode(env, eValue, cmsgP, rem, usedP)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "decode_cmsghdr_value {%d} -> FALSE:\r\n"
                "   decode function failed\r\n",
                descP->sock) );
        return FALSE;
    }
    type = cmsgSpecP->type;
    SSDBG( descP,
           ("WIN-ESAIO",
            "decode_cmsghdr_value {%d} -> TRUE:\r\n"
            "   level:   %d\r\n"
            "   type:    %d\r\n",
            "   *usedP:  %lu\r\n",
            descP->sock, level, type, (unsigned long) *usedP) );
    cmsgP->cmsg_level = level;
    cmsgP->cmsg_type = type;
    return TRUE;
}
static
BOOLEAN_T decode_cmsghdr_data(ErlNifEnv*       env,
                              ESockDescriptor* descP,
                              int              level,
                              ERL_NIF_TERM     eType,
                              ERL_NIF_TERM     eData,
                              char*            bufP,
                              size_t           rem,
                              size_t*          usedP)
{
    int             type;
    ErlNifBinary    bin;
    struct cmsghdr* cmsgP     = (struct cmsghdr *) bufP;
    ESockCmsgSpec*  cmsgSpecP = NULL;
    SSDBG( descP,
           ("WIN-ESAIO",
            "decode_cmsghdr_data {%d} -> entry  \r\n"
            "   eType: %T\r\n"
            "   eData: %T\r\n",
            descP->sock, eType, eData) );
    if (! GET_INT(env, eType, &type)) {
        ESockCmsgSpec* cmsgTable = NULL;
        size_t         num       = 0;
        if ((! IS_ATOM(env, eType)) ||
            ((cmsgTable = esock_lookup_cmsg_table(level, &num)) == NULL) ||
            ((cmsgSpecP = esock_lookup_cmsg_spec(cmsgTable, num, eType)) == NULL)) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "decode_cmsghdr_data {%d} -> FALSE:\r\n"
                    "   cmsgTable:  %p\r\n"
                    "   cmsgSpecP:  %p\r\n",
                    descP->sock, cmsgTable, cmsgSpecP) );
            return FALSE;
        }
        type = cmsgSpecP->type;
    }
    if (GET_BIN(env, eData, &bin)) {
        void *p;
        p = esock_init_cmsghdr(cmsgP, rem, bin.size, usedP);
        if (p == NULL) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "decode_cmsghdr_data {%d} -> FALSE:\r\n"
                    "   rem:      %lu\r\n"
                    "   bin.size: %lu\r\n",
                    descP->sock,
                    (unsigned long) rem,
                    (unsigned long) bin.size) );
            return FALSE;
        }
        sys_memcpy(p, bin.data, bin.size);
    } else if ((! esock_cmsg_decode_int(env, eData, cmsgP, rem, usedP)) &&
               (! esock_cmsg_decode_bool(env, eData, cmsgP, rem, usedP))) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "decode_cmsghdr_data {%d} -> FALSE\r\n",
                descP->sock) );
        return FALSE;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "decode_cmsghdr_data {%d} -> TRUE:\r\n"
            "   level:   %d\r\n"
            "   type:    %d\r\n"
            "   *usedP:  %lu\r\n",
            descP->sock, level, type, (unsigned long) *usedP) );
    cmsgP->cmsg_level = level;
    cmsgP->cmsg_type = type;
    return TRUE;
}
static
void encode_msg(ErlNifEnv*       env,
                ESockDescriptor* descP,
                ssize_t          read,
                WSAMSG*          msgP,
                ErlNifBinary*    dataBufP,
                ErlNifBinary*    ctrlBufP,
                ERL_NIF_TERM*    eMsg)
{
    ERL_NIF_TERM addr, iov, ctrl, flags;
    SSDBG( descP,
           ("WIN-ESAIO", "encode_msg {%d} -> entry with"
            "\r\n   read: %ld"
            "\r\n", descP->sock, (long) read) );
    if (msgP->namelen != 0) {
        esock_encode_sockaddr(env,
                              (ESockAddress*) msgP->name,
                              msgP->namelen,
                              &addr);
    } else {
        addr = esock_atom_undefined;
    }
    SSDBG( descP,
           ("WIN-ESAIO", "encode_msg {%d} -> encode iov"
            "\r\n   num vectors: %lu"
            "\r\n", descP->sock, (unsigned long) msgP->dwBufferCount) );
    esock_encode_iov(env, read,
                     (SysIOVec*) msgP->lpBuffers, msgP->dwBufferCount, dataBufP,
                     &iov);
    SSDBG( descP,
           ("WIN-ESAIO",
            "encode_msg {%d} -> try encode cmsgs\r\n",
            descP->sock) );
    encode_cmsgs(env, descP, ctrlBufP, msgP, &ctrl);
    SSDBG( descP,
           ("WIN-ESAIO",
            "encode_msg {%d} -> try encode flags\r\n",
            descP->sock) );
    esock_encode_msg_flags(env, descP, msgP->dwFlags, &flags);
    SSDBG( descP,
           ("WIN-ESAIO", "encode_msg {%d} -> components encoded:"
            "\r\n   addr:  %T"
            "\r\n   ctrl:  %T"
            "\r\n   flags: %T"
            "\r\n", descP->sock, addr, ctrl, flags) );
    {
        ERL_NIF_TERM keys[]  = {esock_atom_iov,
                                esock_atom_ctrl,
                                esock_atom_flags,
                                esock_atom_addr};
        ERL_NIF_TERM vals[]  = {iov, ctrl, flags, addr};
        size_t       numKeys = NUM(keys);
        ESOCK_ASSERT( numKeys == NUM(vals) );
        SSDBG( descP,
               ("WIN-ESAIO",
                "encode_msg {%d} -> create map\r\n",
                descP->sock) );
        if (msgP->namelen == 0)
            numKeys--;
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, eMsg) );
        SSDBG( descP,
               ("WIN-ESAIO",
                "encode_msg {%d}-> map encoded\r\n",
                descP->sock) );
    }
    SSDBG( descP,
           ("WIN-ESAIO", "encode_msg {%d} -> done\r\n", descP->sock) );
}
static
void encode_cmsgs(ErlNifEnv*       env,
                  ESockDescriptor* descP,
                  ErlNifBinary*    cmsgBinP,
                  WSAMSG*          msgP,
                  ERL_NIF_TERM*    eCMsg)
{
    ERL_NIF_TERM ctrlBuf  = MKBIN(env, cmsgBinP);
    SocketTArray cmsghdrs = TARRAY_CREATE(128);
    WSACMSGHDR*  firstP   = ESOCK_CMSG_FIRSTHDR(msgP);
    WSACMSGHDR*  currentP;
    SSDBG( descP, ("WIN-ESAIO", "encode_cmsgs {%d} -> entry when"
                   "\r\n   msg ctrl len:  %d"
                   "\r\n   (ctrl) firstP: 0x%lX"
                   "\r\n", descP->sock, msgP->Control.len, firstP) );
    for (currentP = firstP;
         (currentP != NULL);
#pragma warning(disable:4116)
         currentP = ESOCK_CMSG_NXTHDR(msgP, currentP)) {
#pragma warning(default:4116)
        SSDBG( descP,
               ("WIN-ESAIO", "encode_cmsgs {%d} -> process cmsg header when"
                "\r\n   TArray Size: %d"
                "\r\n", descP->sock, TARRAY_SZ(cmsghdrs)) );
        if (((CHARP(currentP) + currentP->cmsg_len) - CHARP(firstP)) >
            msgP->Control.len) {
            SSDBG( descP,
                   ("WIN-ESAIO", "encode_cmsgs {%d} -> check failed when: "
                    "\r\n   currentP:           0x%lX"
                    "\r\n   (current) cmsg_len: %d"
                    "\r\n   firstP:             0x%lX"
                    "\r\n   =>                  %d"
                    "\r\n   msg ctrl len:       %d"
                    "\r\n", descP->sock,
                    CHARP(currentP), currentP->cmsg_len, CHARP(firstP),
                    (CHARP(currentP) + currentP->cmsg_len) - CHARP(firstP),
                    msgP->Control.len) );
            TARRAY_ADD(cmsghdrs, esock_atom_bad_data);
            break;
        } else {
            unsigned char* dataP   = UCHARP(ESOCK_CMSG_DATA(currentP));
            size_t         dataPos = dataP - cmsgBinP->data;
            size_t         dataLen =
                (UCHARP(currentP) + currentP->cmsg_len) - dataP;
            ERL_NIF_TERM
                cmsgHdr,
                keys[]  =
                {esock_atom_level,
                 esock_atom_type,
                 esock_atom_data,
                 esock_atom_value},
                vals[NUM(keys)];
            size_t numKeys = NUM(keys);
            BOOLEAN_T have_value;
            SSDBG( descP,
                   ("WIN-ESAIO", "encode_cmsgs {%d} -> cmsg header data: "
                    "\r\n   dataPos: %d"
                    "\r\n   dataLen: %d"
                    "\r\n", descP->sock, dataPos, dataLen) );
            vals[0] = esock_encode_level(env, currentP->cmsg_level);
            vals[2] = MKSBIN(env, ctrlBuf, dataPos, dataLen);
            have_value = esock_encode_cmsg(env,
                                           currentP->cmsg_level,
                                           currentP->cmsg_type,
                                           dataP, dataLen, &vals[1], &vals[3]);
            SSDBG( descP,
                   ("WIN-ESAIO", "encode_cmsgs {%d} -> "
                    "\r\n   %T: %T"
                    "\r\n   %T: %T"
                    "\r\n   %T: %T"
                    "\r\n", descP->sock,
                    keys[0], vals[0], keys[1], vals[1], keys[2], vals[2]) );
            if (have_value)
                SSDBG( descP,
                       ("WIN-ESAIO", "encode_cmsgs {%d} -> "
                        "\r\n   %T: %T"
                        "\r\n", descP->sock, keys[3], vals[3]) );
            ESOCK_ASSERT( numKeys == NUM(vals) );
            ESOCK_ASSERT( MKMA(env, keys, vals,
                               numKeys - (have_value ? 0 : 1), &cmsgHdr) );
            TARRAY_ADD(cmsghdrs, cmsgHdr);
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "encode_cmsgs {%d} -> cmsg headers processed when"
            "\r\n   TArray Size: %d"
            "\r\n", descP->sock, TARRAY_SZ(cmsghdrs)) );
    TARRAY_TOLIST(cmsghdrs, env, eCMsg);
}
extern
ERL_NIF_TERM esaio_recv(ErlNifEnv*       env,
                        ESockDescriptor* descP,
                        ERL_NIF_TERM     sockRef,
                        ERL_NIF_TERM     recvRef,
                        ssize_t          len,
                        int              flags)
{
    ErlNifPid       caller;
    ESAIOOperation* opP;
    int             rres;
    WSABUF          wbuf;
    DWORD           f = flags;
    size_t          bufSz = (len != 0 ? len : descP->rBufSz);
    SSDBG( descP, ("WIN-ESAIO", "esaio_recv {%d} -> entry with"
                   "\r\n   length:        %ld"
                   "\r\n   (buffer) size: %lu"
                   "\r\n", descP->sock,
                   (long) len, (unsigned long) bufSz) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->readState))
        return esock_make_error_closed(env);
    if (descP->acceptorsQ.first != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    if (esock_reader_search4pid(env, descP, &caller)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_RECV;
    opP->env                = esock_alloc_env("esaio-recv - operation");
    opP->data.recv.recvRef  = CP_TERM(opP->env, recvRef);
    opP->data.recv.sockRef  = CP_TERM(opP->env, sockRef);
    opP->caller             = caller;
    ESOCK_ASSERT( ALLOC_BIN(bufSz, &opP->data.recv.buf) );
    opP->data.recv.toRead = len;
    wbuf.buf = opP->data.recv.buf.data;
    wbuf.len = opP->data.recv.buf.size;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_tries, &descP->readTries, 1);
    SSDBG( descP, ("WIN-ESAIO", "esaio_recv {%d} -> try read (%lu)\r\n",
                   descP->sock, (unsigned long) bufSz) );
    rres = sock_recv_O(descP->sock, &wbuf, &f, (OVERLAPPED*) opP);
    return recv_check_result(env, descP, len, opP, caller, rres,
                             sockRef, recvRef);
}
static
ERL_NIF_TERM recv_check_result(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ssize_t          toRead,
                               ESAIOOperation*  opP,
                               ErlNifPid        caller,
                               int              recv_result,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM eres;
    if (recv_result == 0) {
        eres = recv_check_ok(env, descP, opP, toRead, caller, sockRef, recvRef);
    } else {
        int err;
        err = sock_errno();
        if (err == WSA_IO_PENDING) {
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recv_check_result(%T, %d) -> "
                        "pending - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recv_check_result(%T, %d) -> "
                            "failed cancel pending operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
        } else {
            eres = recv_check_fail(env, descP, opP, err, sockRef);
        }
    }
    return eres;
}
static
ERL_NIF_TERM recv_check_ok(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           ESAIOOperation*  opP,
                           ssize_t          toRead,
                           ErlNifPid        caller,
                           ERL_NIF_TERM     sockRef,
                           ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM data, eres;
    DWORD        read = 0, flags = 0;
    SSDBG( descP,
           ("WIN-ESAIO",
            "recv_check_ok -> try get overlapped result\r\n") );
    if (get_recv_ovl_result(descP->sock, (OVERLAPPED*) opP, &read, &flags)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "recv_check_ok -> overlapped success result: "
                "\r\n   read:  %d"
                "\r\n   flags: 0x%X"
                "\r\n", read, flags) );
        (void) flags;
        if ((read == 0) && (descP->type == SOCK_STREAM)) {
            ESOCK_CNT_INC(env, descP, sockRef,
                          esock_atom_read_fails, &descP->readFails, 1);
            eres = esock_make_error(env, esock_atom_closed);
        } else {
            if (read == opP->data.recv.buf.size) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recv_check_ok(%T, %d) -> complete success (%d)"
                        "\r\n", sockRef, descP->sock, read) );
                data = MKBIN(env, &opP->data.recv.buf);
                eres = esock_make_ok2(env, data);
            } else if ((toRead == 0) ||
                       (descP->type != SOCK_STREAM)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recv_check_ok(%T, %d) -> complete success (%d, %d)"
                        "\r\n",
                        sockRef, descP->sock, read, opP->data.recv.buf.size) );
                ESOCK_ASSERT( REALLOC_BIN(&opP->data.recv.buf, read) );
                data = MKBIN(env, &opP->data.recv.buf);
                data = MKSBIN(env, data, 0, read);
                eres = esock_make_ok2(env, data);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recv_check_ok(%T, %d) -> partial (%d) success"
                        "\r\n", sockRef, descP->sock, read) );
                ESOCK_ASSERT( REALLOC_BIN(&opP->data.recv.buf, read) );
                data = MKBIN(env, &opP->data.recv.buf);
                data = MKSBIN(env, data, 0, read);
                eres = MKT2(env, esock_atom_more, data);
            }
            ESOCK_CNT_INC(env, descP, sockRef,
                          esock_atom_read_pkg, &descP->readPkgCnt, 1);
            ESOCK_CNT_INC(env, descP, sockRef,
                          esock_atom_read_byte, &descP->readByteCnt, read);
            if (read > descP->readPkgMax)
                descP->readPkgMax = read;
        }
    } else {
        int save_errno = sock_errno();
        switch (save_errno) {
        case WSA_IO_INCOMPLETE:
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recv_check_ok(%T, %d) -> "
                        "incomplete - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recv_check_ok(%T, %d) -> "
                            "failed cancel incomplete operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
            break;
        default:
            {
                ERL_NIF_TERM eerrno = ENO2T(env, save_errno);
                ERL_NIF_TERM reason = MKT2(env,
                                           esock_atom_get_overlapped_result,
                                           eerrno);
                ESOCK_CNT_INC(env, descP, sockRef,
                              esock_atom_read_fails, &descP->readFails, 1);
                MLOCK(ctrl.cntMtx);
                esock_cnt_inc(&ctrl.genErrs, 1);
                MUNLOCK(ctrl.cntMtx);
                eres = esock_make_error(env, reason);
            }
            break;
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "recv_check_ok(%T) {%d} -> done"
            "\r\n",
            sockRef, descP->sock) );
    return eres;
}
static
ERL_NIF_TERM recv_check_pending(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ESAIOOperation*  opP,
                                ErlNifPid        caller,
                                ERL_NIF_TERM     sockRef,
                                ERL_NIF_TERM     recvRef)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "recv_check_pending(%T, %d) -> entry with"
            "\r\n   recvRef: %T"
            "\r\n", sockRef, descP->sock, recvRef) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_waits, &descP->readWaits, 1);
    descP->readState |= ESOCK_STATE_SELECTED;
    esock_reader_push(env, descP, caller, recvRef, opP);
    return esock_atom_completion;
}
static
ERL_NIF_TERM recv_check_fail(ErlNifEnv*       env,
                             ESockDescriptor* descP,
                             ESAIOOperation*  opP,
                             int              saveErrno,
                             ERL_NIF_TERM     sockRef)
{
    SSDBG( descP,
           ("WIN-ESAIO", "recv_check_fail(%T) {%d} -> entry with"
            "\r\n   errno: %d"
            "\r\n",
            sockRef, descP->sock, saveErrno) );
    FREE_BIN( &opP->data.recv.buf );
    return recv_check_failure(env, descP, opP, saveErrno, sockRef);
}
static
ERL_NIF_TERM recv_check_failure(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ESAIOOperation*  opP,
                                int              saveErrno,
                                ERL_NIF_TERM     sockRef)
{
    ERL_NIF_TERM reason = MKA(env, erl_errno_id(saveErrno));
    SSDBG( descP,
           ("WIN-ESAIO", "recv_check_failure(%T) {%d} -> error: %d (%T)\r\n",
            sockRef, descP->sock, saveErrno, reason) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_fails, &descP->readFails, 1);
    esock_clear_env("recv_check_failure", opP->env);
    esock_free_env("recv_check_failure", opP->env);
    FREE( opP );
    return esock_make_error(env, reason);
}
extern
ERL_NIF_TERM esaio_recvfrom(ErlNifEnv*       env,
                            ESockDescriptor* descP,
                            ERL_NIF_TERM     sockRef,
                            ERL_NIF_TERM     recvRef,
                            ssize_t          len,
                            int              flags)
{
    ErlNifPid       caller;
    ESAIOOperation* opP;
    int             rres;
    WSABUF          wbuf;
    DWORD           f = flags;
    size_t          bufSz = (len != 0 ? len : descP->rBufSz);
    if (bufSz < ESAIO_RECVFROM_MIN_BUFSZ) bufSz = ESAIO_RECVFROM_MIN_BUFSZ;
    SSDBG( descP, ("WIN-ESAIO", "essio_recvfrom {%d} -> entry with"
                   "\r\n   bufSz: %d"
                   "\r\n", descP->sock, bufSz) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->readState))
        return esock_make_error_closed(env);
    if (descP->acceptorsQ.first != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    if (esock_reader_search4pid(env, descP, &caller)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_RECVFROM;
    opP->env                    = esock_alloc_env("esaio-recvfrom - operation");
    opP->data.recvfrom.recvRef  = CP_TERM(opP->env, recvRef);
    opP->data.recvfrom.sockRef  = CP_TERM(opP->env, sockRef);
    opP->caller                 = caller;
    ESOCK_ASSERT( ALLOC_BIN(bufSz, &opP->data.recv.buf) );
    opP->data.recvfrom.toRead = len;
    wbuf.buf = opP->data.recvfrom.buf.data;
    wbuf.len = opP->data.recvfrom.buf.size;
    opP->data.recvfrom.addrLen = sizeof(ESockAddress);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_tries, &descP->readTries, 1);
    SSDBG( descP, ("WIN-ESAIO", "esaio_recvfrom {%d} -> try read (%lu)\r\n",
                   descP->sock, (unsigned long) bufSz) );
    rres = sock_recvfrom_O(descP->sock, &wbuf, &f,
                           (struct sockaddr*) &opP->data.recvfrom.fromAddr,
                           &opP->data.recvfrom.addrLen, (OVERLAPPED*) opP);
    return recvfrom_check_result(env, descP, opP, caller, rres,
                                 sockRef, recvRef);
}
static
ERL_NIF_TERM recvfrom_check_result(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ESAIOOperation*  opP,
                                   ErlNifPid        caller,
                                   int              recv_result,
                                   ERL_NIF_TERM     sockRef,
                                   ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM eres;
    if (recv_result == 0) {
        eres = recvfrom_check_ok(env, descP, opP, caller, sockRef, recvRef);
    } else {
        int err;
        err = sock_errno();
        if (err == WSA_IO_PENDING) {
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recvfrom_check_result(%T, %d) -> "
                        "pending - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recvfrom_check_result(%T, %d) -> "
                            "failed cancel pending operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
        } else {
            eres = recvfrom_check_fail(env, descP, opP, err, sockRef);
        }
    }
    return eres;
}
static
ERL_NIF_TERM recvfrom_check_ok(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ESAIOOperation*  opP,
                               ErlNifPid        caller,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM data, eres;
    DWORD        read = 0, flags = 0;
    SSDBG( descP,
           ("WIN-ESAIO",
            "recvfrom_check_ok -> try get overlapped result\r\n") );
    if (get_recv_ovl_result(descP->sock, (OVERLAPPED*) opP, &read, &flags)) {
        ERL_NIF_TERM eSockAddr;
        SSDBG( descP,
               ("WIN-ESAIO",
                "recvfrom_check_ok -> overlapped result: "
                "\r\n   read:  %d"
                "\r\n   flags: 0x%X"
                "\r\n", read, flags) );
        (void) flags;
        esock_encode_sockaddr(env,
                              &opP->data.recvfrom.fromAddr,
                              opP->data.recvfrom.addrLen,
                              &eSockAddr);
        if (read != opP->data.recvfrom.buf.size) {
            ESOCK_ASSERT( REALLOC_BIN(&opP->data.recvfrom.buf, read) );
        }
        data = MKBIN(env, &opP->data.recvfrom.buf);
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_read_pkg, &descP->readPkgCnt, 1);
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_read_byte, &descP->readByteCnt, read);
        if (read > descP->readPkgMax)
            descP->readPkgMax = read;
        eres = esock_make_ok2(env, MKT2(env, eSockAddr, data));
    } else {
        int save_errno = sock_errno();
        switch (save_errno) {
        case WSA_IO_INCOMPLETE:
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recvfrom_check_ok(%T, %d) -> "
                        "incomplete - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recvfrom_check_ok(%T, %d) -> "
                            "failed cancel incomplete operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
            break;
        default:
            {
                ERL_NIF_TERM eerrno = ENO2T(env, save_errno);
                ERL_NIF_TERM reason = MKT2(env,
                                           esock_atom_get_overlapped_result,
                                           eerrno);
                ESOCK_CNT_INC(env, descP, sockRef,
                              esock_atom_read_fails, &descP->readFails, 1);
                MLOCK(ctrl.cntMtx);
                esock_cnt_inc(&ctrl.genErrs, 1);
                MUNLOCK(ctrl.cntMtx);
                eres = esock_make_error(env, reason);
            }
            break;
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "recvfrom_check_ok(%T) {%d} -> done with"
            "\r\n   result: %T"
            "\r\n",
            sockRef, descP->sock, eres) );
    return eres;
}
static
ERL_NIF_TERM recvfrom_check_fail(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ESAIOOperation*  opP,
                                 int              saveErrno,
                                 ERL_NIF_TERM     sockRef)
{
    SSDBG( descP,
           ("WIN-ESAIO", "recfrom_check_fail(%T) {%d} -> entry with"
            "\r\n   errno: %d"
            "\r\n",
            sockRef, descP->sock, saveErrno) );
    FREE_BIN( &opP->data.recvfrom.buf );
    return recv_check_failure(env, descP, opP, saveErrno, sockRef);
}
extern
ERL_NIF_TERM esaio_recvmsg(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           ERL_NIF_TERM     sockRef,
                           ERL_NIF_TERM     recvRef,
                           ssize_t          bufLen,
                           ssize_t          ctrlLen,
                           int              flags)
{
    ErlNifPid       caller;
    ESAIOOperation* opP;
    SOCKLEN_T       addrLen;
    size_t          bufSz  = (bufLen  != 0 ? bufLen  : descP->rBufSz);
    size_t          ctrlSz = (ctrlLen != 0 ? ctrlLen : descP->rCtrlSz);
    int             rres;
    ERL_NIF_TERM    eres;
    (void) flags;
    SSDBG( descP, ("WIN-ESAIO", "esaio_recvmsg(%T) {%d} -> entry with"
                   "\r\n   bufSz:  %lu (%ld)"
                   "\r\n   ctrlSz: %ld (%ld)"
                   "\r\n", sockRef, descP->sock,
                   (unsigned long) bufSz, (long) bufLen,
                   (unsigned long) ctrlSz, (long) ctrlLen) );
    if (! ((descP->type == SOCK_DGRAM) || (descP->type == SOCK_RAW))) {
        return enif_raise_exception(env, MKA(env, "notsup"));
    }
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->readState))
        return esock_make_error_closed(env);
    if (descP->acceptorsQ.first != NULL)
        return esock_make_error_invalid(env, esock_atom_state);
    if (esock_reader_search4pid(env, descP, &caller)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    opP = MALLOC( sizeof(ESAIOOperation) );
    ESOCK_ASSERT( opP != NULL);
    sys_memzero((char*) opP, sizeof(ESAIOOperation));
    opP->tag = ESAIO_OP_RECVMSG;
    opP->env                   = esock_alloc_env("esaio-recvmsg - operation");
    opP->data.recvmsg.recvRef  = CP_TERM(opP->env, recvRef);
    opP->data.recvmsg.sockRef  = CP_TERM(opP->env, sockRef);
    opP->caller                = caller;
    ESOCK_ASSERT( ALLOC_BIN(bufSz, &opP->data.recvmsg.data[0]) );
    ESOCK_ASSERT( ALLOC_BIN(ctrlSz, &opP->data.recvmsg.ctrl) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_tries, &descP->readTries, 1);
    addrLen = sizeof(opP->data.recvmsg.addr);
    sys_memzero((char*) &opP->data.recvmsg.addr, addrLen);
    sys_memzero((char*) &opP->data.recvmsg.msg,  sizeof(opP->data.recvmsg.msg));
    opP->data.recvmsg.wbufs[0].buf = opP->data.recvmsg.data[0].data;
    opP->data.recvmsg.wbufs[0].len = opP->data.recvmsg.data[0].size;
    opP->data.recvmsg.msg.name          = (SOCKADDR*) &opP->data.recvmsg.addr;
    opP->data.recvmsg.msg.namelen       = addrLen;
    opP->data.recvmsg.msg.lpBuffers     = opP->data.recvmsg.wbufs;
    opP->data.recvmsg.msg.dwBufferCount = 1;
    opP->data.recvmsg.msg.Control.buf   = opP->data.recvmsg.ctrl.data;
    opP->data.recvmsg.msg.Control.len   = opP->data.recvmsg.ctrl.size;
    opP->data.recvmsg.msg.dwFlags       = 0;
    rres = sock_recvmsg_O(descP->sock,
                          &opP->data.recvmsg.msg,
                          (OVERLAPPED*) opP);
    eres = recvmsg_check_result(env, descP, opP, caller, rres,
                                sockRef, recvRef);
    SSDBG( descP, ("WIN-ESAIO", "esaio_recvmsg(%T) {%d} -> done\r\n",
                   sockRef, descP->sock) );
    return eres;
}
static
BOOLEAN_T recv_check_reader(ErlNifEnv*       env,
                            ESockDescriptor* descP,
                            ErlNifPid*       caller,
                            ERL_NIF_TERM     ref,
                            ERL_NIF_TERM*    checkResult)
{
    BOOLEAN_T result;
    if (! esock_reader_search4pid(env, descP, caller)) {
        if (COMPARE(ref, esock_atom_zero) == 0)
            return FALSE;
    } else {
        *checkResult = esock_raise_invalid(env, esock_atom_state);
    }
    return TRUE;
}
static
ERL_NIF_TERM recvmsg_check_result(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ESAIOOperation*  opP,
                                  ErlNifPid        caller,
                                  int              recv_result,
                                  ERL_NIF_TERM     sockRef,
                                  ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM eres;
    SSDBG( descP,
           ("WIN-ESAIO", "recvmsg_check_result(%T) {%d} -> entry with"
            "\r\n   recv_result: %d"
            "\r\n   recvRef:     %T"
            "\r\n", sockRef, descP->sock, recv_result, recvRef) );
    if (recv_result == 0) {
        eres = recvmsg_check_ok(env, descP, opP, caller, sockRef, recvRef);
    } else {
        int err;
        err = sock_errno();
        if (err == WSA_IO_PENDING) {
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recvmsg_check_result(%T, %d) -> "
                        "pending - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recvmsg_check_result(%T, %d) -> "
                            "failed cancel pending operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
        } else {
            eres = recvmsg_check_fail(env, descP, opP, err, sockRef);
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "recvmsg_check_result(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return eres;
}
static
ERL_NIF_TERM recvmsg_check_ok(ErlNifEnv*       env,
                              ESockDescriptor* descP,
                              ESAIOOperation*  opP,
                              ErlNifPid        caller,
                              ERL_NIF_TERM     sockRef,
                              ERL_NIF_TERM     recvRef)
{
    ERL_NIF_TERM eMsg, eres;
    DWORD        read = 0, flags = 0;
    SSDBG( descP,
           ("WIN-ESAIO",
            "recvmsg_check_ok(%T) {%d} -> try get overlapped result\r\n",
            sockRef, descP->sock) );
    if (get_recv_ovl_result(descP->sock, (OVERLAPPED*) opP, &read, &flags)) {
        ERL_NIF_TERM eSockAddr;
        SSDBG( descP,
               ("WIN-ESAIO",
                "recvmsg_check_ok(%T, %d) -> overlapped success result: "
                "\r\n   read:  %d"
                "\r\n   flags: 0x%X"
                "\r\n", sockRef, descP->sock, read, flags) );
        (void) flags;
        encode_msg(env, descP, read,
                   &opP->data.recvmsg.msg,
                   opP->data.recvmsg.data,
                   &opP->data.recvmsg.ctrl,
                   &eMsg);
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_read_pkg, &descP->readPkgCnt, 1);
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_read_byte, &descP->readByteCnt, read);
        if (read > descP->readPkgMax)
            descP->readPkgMax = read;
        eres = esock_make_ok2(env, eMsg);
    } else {
        int save_errno = sock_errno();
        switch (save_errno) {
        case WSA_IO_INCOMPLETE:
            if (! IS_ZERO(recvRef)) {
                eres = recv_check_pending(env, descP, opP, caller,
                                          sockRef, recvRef);
            } else {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "recvmsg_check_ok(%T, %d) -> "
                        "incomplete - but we are not allowed to wait => cancel"
                        "\r\n", sockRef, descP->sock) );
                if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) opP)) {
                    int          save_errno = sock_errno();
                    ERL_NIF_TERM tag        = esock_atom_cancel;
                    ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                    SSDBG( descP,
                           ("WIN-ESAIO",
                            "recvmsg_check_ok(%T, %d) -> "
                            "failed cancel incomplete operation"
                            "\r\n   %T"
                            "\r\n", sockRef, descP->sock, reason) );
                    eres = esock_make_error(env, MKT2(env, tag, reason));
                } else {
                    eres = esock_atom_timeout;
                }
            }
            break;
        default:
            {
                ERL_NIF_TERM eerrno = ENO2T(env, save_errno);
                ERL_NIF_TERM reason = MKT2(env,
                                           esock_atom_get_overlapped_result,
                                           eerrno);
                ESOCK_CNT_INC(env, descP, sockRef,
                              esock_atom_read_fails, &descP->readFails, 1);
                MLOCK(ctrl.cntMtx);
                esock_cnt_inc(&ctrl.genErrs, 1);
                MUNLOCK(ctrl.cntMtx);
                eres = esock_make_error(env, reason);
            }
            break;
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "recvmsg_check_ok(%T) {%d} -> done with"
            "\r\n   result: %T"
            "\r\n",
            sockRef, descP->sock, eres) );
    return eres;
}
static
ERL_NIF_TERM recvmsg_check_fail(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ESAIOOperation*  opP,
                                int              saveErrno,
                                ERL_NIF_TERM     sockRef)
{
    SSDBG( descP,
           ("WIN-ESAIO", "recvmsg_check_fail(%T) {%d} -> entry with"
            "\r\n   errno: %d"
            "\r\n",
            sockRef, descP->sock, saveErrno) );
    FREE_BIN( &opP->data.recvmsg.data[0] );
    FREE_BIN( &opP->data.recvmsg.ctrl );
    return recv_check_failure(env, descP, opP, saveErrno, sockRef);
}
extern
ERL_NIF_TERM esaio_close(ErlNifEnv*       env,
                         ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_close(%d) -> begin closing\r\n",
            descP->sock) );
     if (! IS_OPEN(descP->readState)) {
        return esock_make_error_closed(env);
    }
    ESOCK_ASSERT( enif_self(env, &descP->closerPid) != NULL );
    if (COMPARE_PIDS(&descP->closerPid, &descP->ctrlPid) != 0) {
        ESOCK_ASSERT( MONP("esaio_close-check -> closer",
                           env, descP,
                           &descP->closerPid,
                           &descP->closerMon) == 0 );
    }
    descP->readState  |= ESOCK_STATE_CLOSING;
    descP->writeState |= ESOCK_STATE_CLOSING;
    if (do_stop(env, descP)) {
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_close {%d} -> stop was scheduled\r\n",
                descP->sock) );
        descP->closeEnv = esock_alloc_env("esock_close_do - close-env");
        descP->closeRef = MKREF(descP->closeEnv);
        return esock_make_ok2(env, CP_TERM(env, descP->closeRef));
    } else {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_close {%d} -> stop was called\r\n",
                descP->sock) );
        return esock_atom_ok;
    }
}
static
BOOLEAN_T do_stop(ErlNifEnv*       env,
                  ESockDescriptor* descP)
{
    BOOLEAN_T    ret;
    ERL_NIF_TERM sockRef;
    sockRef = enif_make_resource(env, descP);
    if (IS_SELECTED(descP)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "do_stop {%d} -> cancel outstanding I/O operations\r\n",
                descP->sock) );
        if (! CancelIoEx((HANDLE) descP->sock, NULL) ) {
            int          save_errno = sock_errno();
            ERL_NIF_TERM ereason    = ENO2T(env, save_errno);
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "do_stop {%d} -> cancel I/O failed: "
                    "\r\n   %T\r\n",
                    descP->sock, ereason) );
            if (save_errno != ERROR_NOT_FOUND)
                esock_error_msg("Failed cancel outstanding I/O operations:"
                                "\r\n   Socket: " SOCKET_FORMAT_STR
                                "\r\n   Reason: %T"
                                "\r\n",
                                descP->sock, ereason);
            ret = FALSE;
        } else {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "do_stop {%d} -> successfully canceled\r\n", descP->sock) );
            ret = TRUE;
        }
    } else {
        SSDBG( descP,
               ("WIN-ESAIO",
                "do_stop {%d} -> no active I/O requests\r\n", descP->sock) );
        ret = FALSE;
    }
    if (descP->connectorP != NULL) {
        esock_stop_handle_current(env,
                                  "connector",
                                  descP, sockRef, &descP->connector);
        descP->connectorP = NULL;
    }
    return ret;
}
extern
ERL_NIF_TERM esaio_fin_close(ErlNifEnv*       env,
                             ESockDescriptor* descP)
{
    int       err;
    ErlNifPid self;
    ESOCK_ASSERT( enif_self(env, &self) != NULL );
    if (IS_CLOSED(descP->readState))
        return esock_make_error_closed(env);
    if (! IS_CLOSING(descP->readState)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    if (IS_SELECTED(descP) && (descP->closeEnv != NULL)) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    if (COMPARE_PIDS(&descP->closerPid, &self) != 0) {
        return esock_raise_invalid(env, esock_atom_state);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_fin_close {%d} -> demonitor closer process %T\r\n",
            descP->sock, descP->closerPid) );
    enif_set_pid_undefined(&descP->closerPid);
    if (descP->closerMon.isActive) {
        (void) DEMONP("esaio_fin_close -> closer",
                      env, descP, &descP->closerMon);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_fin_close {%d} -> demonitor owner process %T\r\n",
            descP->sock, descP->ctrlPid) );
    enif_set_pid_undefined(&descP->ctrlPid);
    (void) DEMONP("esaio_fin_close -> ctrl",
                  env, descP, &descP->ctrlMon);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_fin_close {%d} -> (try) close the socket\r\n",
            descP->sock, descP->ctrlPid) );
    err = esock_close_socket(env, descP, TRUE);
    if (err != 0) {
        if (err == ERRNO_BLOCK) {
            return esock_make_error(env, esock_atom_timeout);
        } else {
            return esock_make_error_errno(env, err);
        }
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_fin_close -> done\r\n") );
    return esock_atom_ok;
}
extern
ERL_NIF_TERM esaio_cancel_connect(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     opRef)
{
    ERL_NIF_TERM res;
    ErlNifPid    self;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_cancel_connect {%d} -> entry with"
            "\r\n   writeState: 0x%X"
            "\r\n   opRef:      %T"
            "\r\n",
            descP->sock, descP->writeState, opRef) );
    ESOCK_ASSERT( enif_self(env, &self) != NULL );
    if (! IS_OPEN(descP->writeState)) {
        res = esock_make_error_closed(env);
    } else if ((descP->connectorP == NULL) ||
               (COMPARE_PIDS(&self, &descP->connector.pid) != 0) ||
               (COMPARE(opRef, descP->connector.ref) != 0)) {
        res = esock_make_error(env, esock_atom_not_found);
    } else {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_cancel_connect {%d} -> "
                "try cancel connect I/O request\r\n",
                descP->sock) );
       if (! CancelIoEx((HANDLE) descP->sock,
                         (OVERLAPPED*) descP->connector.dataP)) {
            int save_errno = sock_errno();
            res = esock_make_error_errno(env, save_errno);
        } else {
            res = esock_atom_ok;
        }
        esock_requestor_release("esock_cancel_connect",
                                env, descP, &descP->connector);
        descP->connectorP = NULL;
        descP->writeState &= ~ESOCK_STATE_CONNECTING;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_cancel_connect {%d} -> done when"
            "\r\n   res: %T"
            "\r\n",
            descP->sock, descP->writeState,
            opRef, res) );
    return res;
}
extern
ERL_NIF_TERM esaio_cancel_accept(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     opRef)
{
    ERL_NIF_TERM   res;
    ESockRequestor req;
    ErlNifPid      caller;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_cancel_accept(%T), {%d,0x%X} ->"
            "\r\n   opRef: %T"
            "\r\n", sockRef, descP->sock, descP->readState, opRef) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->readState)) {
        res = esock_make_error_closed(env);
    } else if (esock_acceptor_get(env, descP, &opRef, &caller, &req)) {
        ESOCK_ASSERT( DEMONP("esaio_cancel_accept -> acceptor",
                             env, descP, &req.mon) == 0);
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_cancel_accept {%d} -> try cancel accept I/O request\r\n",
                descP->sock) );
        if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) req.dataP)) {
            int save_errno = sock_errno();
            res = esock_make_error_errno(env, save_errno);
        } else {
            res = esock_atom_ok;
        }
        if (descP->acceptorsQ.first == NULL) {
            descP->readState &= ~ESOCK_STATE_ACCEPTING;
        }
    } else {
        res = esock_make_error(env, esock_atom_not_found);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_cancel_accept(%T) -> done with result:"
            "\r\n   %T"
            "\r\n", sockRef, res) );
    return res;
}
extern
ERL_NIF_TERM esaio_cancel_send(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ERL_NIF_TERM     opRef)
{
    ERL_NIF_TERM   res;
    ESockRequestor req;
    ErlNifPid      caller;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_cancel_send(%T), {%d,0x%X} ->"
            "\r\n   opRef: %T"
            "\r\n", sockRef, descP->sock, descP->readState, opRef) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->writeState)) {
        res = esock_make_error_closed(env);
    } else if (esock_writer_get(env, descP, &opRef, &caller, &req)) {
        ESOCK_ASSERT( DEMONP("esaio_cancel_send -> sender",
                             env, descP, &req.mon) == 0);
         SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_cancel_send {%d} -> try cancel send I/O request\r\n",
                descP->sock) );
       if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) req.dataP)) {
            int save_errno = sock_errno();
            res = esock_make_error_errno(env, save_errno);
        } else {
            res = esock_atom_ok;
        }
        esock_clear_env("esaio_cancel_send -> req cleanup", req.env);
        esock_free_env("esaio_cancel_send -> req cleanup", req.env);
    } else {
        res = esock_make_error(env, esock_atom_not_found);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_cancel_send(%T) -> done with result:"
            "\r\n   %T"
            "\r\n", sockRef, res) );
    return res;
}
extern
ERL_NIF_TERM esaio_cancel_recv(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     opRef)
{
    ERL_NIF_TERM   res;
    ESockRequestor req;
    ErlNifPid      caller;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_cancel_recv(%T), {%d,0x%X} ->"
            "\r\n   opRef: %T"
            "\r\n", sockRef, descP->sock, descP->readState, opRef) );
    ESOCK_ASSERT( enif_self(env, &caller) != NULL );
    if (! IS_OPEN(descP->readState)) {
        res = esock_make_error_closed(env);
    } else if (esock_reader_get(env, descP, &opRef, &caller, &req)) {
        ESOCK_ASSERT( DEMONP("esaio_cancel_recv -> reader",
                             env, descP, &req.mon) == 0);
         SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_cancel_recv {%d} -> try cancel send I/O request\r\n",
                descP->sock) );
       if (! CancelIoEx((HANDLE) descP->sock, (OVERLAPPED*) req.dataP)) {
            int save_errno = sock_errno();
            res = esock_make_error_errno(env, save_errno);
        } else {
            res = esock_atom_ok;
        }
        esock_clear_env("esaio_cancel_recv -> req cleanup", req.env);
        esock_free_env("esaio_cancel_recv -> req cleanup", req.env);
    } else {
        res = esock_make_error(env, esock_atom_not_found);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_cancel_recv(%T) -> done with result:"
            "\r\n   %T"
            "\r\n", sockRef, res) );
    return res;
}
extern
ERL_NIF_TERM esaio_ioctl3(ErlNifEnv*       env,
			  ESockDescriptor* descP,
			  unsigned long    req,
			  ERL_NIF_TERM     arg)
{
  switch (req) {
#if defined(SIO_TCP_INFO)
  case SIO_TCP_INFO:
      return esaio_ioctl_tcp_info(env, descP, arg);
      break;
#endif
#if defined(SIO_RCVALL)
  case SIO_RCVALL:
      return esaio_ioctl_rcvall(env, descP, arg);
      break;
#endif
#if defined(SIO_RCVALL_IGMPMCAST)
  case SIO_RCVALL_IGMPMCAST:
      return esaio_ioctl_rcvall_igmpmcast(env, descP, arg);
      break;
#endif
#if defined(SIO_RCVALL_MCAST)
  case SIO_RCVALL_MCAST:
      return esaio_ioctl_rcvall_mcast(env, descP, arg);
      break;
#endif
  default:
      return esock_make_error(env, esock_atom_enotsup);
      break;
  }
}
#if defined(SIO_TCP_INFO)
static
ERL_NIF_TERM esaio_ioctl_tcp_info(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  ERL_NIF_TERM     eversion)
{
    DWORD        ndata = 0;
    ERL_NIF_TERM result;
    int          res;
    int          version;
    SSDBG( descP, ("WIN-ESAIO", "esaio_ioctl_tcp_info(%d) -> entry with"
                   "\r\n      (e)version: %T"
                   "\r\n", descP->sock, eversion) );
    if (!GET_INT(env, eversion, &version))
        return enif_make_badarg(env);
    switch (version) {
    case 0:
        {
            TCP_INFO_v0 info;
            sys_memzero((char *) &info, sizeof(info));
            res = sock_ioctl2(descP->sock, SIO_TCP_INFO,
                              &version, sizeof(version),
                              &info, sizeof(info), &ndata);
            (void) ndata;
            if (res != 0) {
                int          save_errno = sock_errno();
                ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                SSDBG( descP,
                       ("WIN-ESAIO", "esaio_ioctl_tcp_info(%d,v0) -> failure: "
                        "\r\n      reason: %T"
                        "\r\n", descP->sock, reason) );
                result = esock_make_error(env, reason);
            } else {
                ERL_NIF_TERM einfo = encode_tcp_info_v0(env, &info);
                result = esock_make_ok2(env, einfo);
            }
        }
        break;
#if defined(HAVE_TCP_INFO_V1)
    case 1:
        {
            TCP_INFO_v1 info;
            sys_memzero((char *) &info, sizeof(info));
            res = sock_ioctl2(descP->sock, SIO_TCP_INFO,
                              &version, sizeof(version),
                              &info, sizeof(info), &ndata);
            (void) ndata;
            if (res != 0) {
                int          save_errno = sock_errno();
                ERL_NIF_TERM reason     = ENO2T(env, save_errno);
                SSDBG( descP,
                       ("WIN-ESAIO", "esaio_ioctl_tcp_info(%d,v1) -> failure: "
                        "\r\n      reason: %T"
                        "\r\n", descP->sock, reason) );
                result = esock_make_error(env, reason);
            } else {
                ERL_NIF_TERM einfo = encode_tcp_info_v1(env, &info);
                result = esock_make_ok2(env, einfo);
            }
        }
        break;
#endif
    default:
        return enif_make_badarg(env);
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_tcp_info(%d) -> done with"
            "\r\n      result: %T"
            "\r\n",
            descP->sock, result) );
    return result;
}
#endif
#if defined(SIO_TCP_INFO)
static
ERL_NIF_TERM encode_tcp_info_v0(ErlNifEnv* env, TCP_INFO_v0* infoP)
{
    ERL_NIF_TERM einfo;
    ERL_NIF_TERM keys[] = {esock_atom_state,
        esock_atom_mss,
        esock_atom_connection_time,
        esock_atom_timestamp_enabled,
        esock_atom_rtt,
        esock_atom_min_rtt,
        esock_atom_bytes_in_flight,
        esock_atom_cwnd,
        esock_atom_snd_wnd,
        esock_atom_rcv_wnd,
        esock_atom_rcv_buf,
        esock_atom_bytes_out,
        esock_atom_bytes_in,
        esock_atom_bytes_reordered,
        esock_atom_bytes_retrans,
        esock_atom_fast_retrans,
        esock_atom_dup_acks_in,
        esock_atom_timeout_episodes,
        esock_atom_syn_retrans};
    ERL_NIF_TERM vals[]  = {encode_tcp_state(env, infoP->State),
        MKUL(env, infoP->Mss),
        MKUI64(env, infoP->ConnectionTimeMs),
        infoP->TimestampsEnabled ? esock_atom_true : esock_atom_false,
        MKUL(env,   infoP->RttUs),
        MKUL(env,   infoP->MinRttUs),
        MKUL(env,   infoP->BytesInFlight),
        MKUL(env,   infoP->Cwnd),
        MKUL(env,   infoP->SndWnd),
        MKUL(env,   infoP->RcvWnd),
        MKUL(env,   infoP->RcvBuf),
        MKUI64(env, infoP->BytesOut),
        MKUI64(env, infoP->BytesIn),
        MKUL(env,   infoP->BytesReordered),
        MKUL(env,   infoP->BytesRetrans),
        MKUL(env,   infoP->FastRetrans),
        MKUL(env,   infoP->DupAcksIn),
        MKUL(env,   infoP->TimeoutEpisodes),
        MKUI(env,   infoP->SynRetrans)};
    unsigned int numKeys = NUM(keys);
    unsigned int numVals = NUM(vals);
    ESOCK_ASSERT( numKeys == numVals );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &einfo) );
    return einfo;
}
#endif
#if defined(SIO_TCP_INFO) && defined(HAVE_TCP_INFO_V1)
static
ERL_NIF_TERM encode_tcp_info_v1(ErlNifEnv* env, TCP_INFO_v1* infoP)
{
    ERL_NIF_TERM einfo;
    ERL_NIF_TERM keys[] = {esock_atom_state,
        esock_atom_mss,
        esock_atom_connection_time,
        esock_atom_timestamp_enabled,
        esock_atom_rtt,
        esock_atom_min_rtt,
        esock_atom_bytes_in_flight,
        esock_atom_cwnd,
        esock_atom_snd_wnd,
        esock_atom_rcv_wnd,
        esock_atom_rcv_buf,
        esock_atom_bytes_out,
        esock_atom_bytes_in,
        esock_atom_bytes_reordered,
        esock_atom_bytes_retrans,
        esock_atom_fast_retrans,
        esock_atom_dup_acks_in,
        esock_atom_timeout_episodes,
        esock_atom_syn_retrans,
        esock_atom_syn_lim_trans_rwin,
        esock_atom_syn_lim_time_rwin,
        esock_atom_syn_lim_bytes_rwin,
        esock_atom_syn_lim_trans_cwnd,
        esock_atom_syn_lim_time_cwnd,
        esock_atom_syn_lim_bytes_cwnd,
        esock_atom_syn_lim_trans_snd,
        esock_atom_syn_lim_time_snd,
        esock_atom_syn_lim_bytes_snd};
    ERL_NIF_TERM vals[]  = {encode_tcp_state(env, infoP->State),
        MKUL(env,   infoP->Mss),
        MKUI64(end, infoP->ConnectionTimeMs),
        infoP->TimestampsEnabled ? esock_atom_true : esock_atom_false,
        MKUL(env,   infoP->RttUs),
        MKUL(env,   infoP->MinRttUs),
        MKUL(env,   infoP->BytesInFlight),
        MKUL(env,   infoP->Cwnd),
        MKUL(env,   infoP->SndWnd),
        MKUL(env,   infoP->RcvWnd),
        MKUL(env,   infoP->RcvBuf),
        MKUI64(env, infoP->BytesOut),
        MKUI64(env, infoP->BytesIn),
        MKUL(env,   infoP->BytesReordered),
        MKUL(env,   infoP->BytesRetrans),
        MKUL(env,   infoP->FastRetrans),
        MKUL(env,   infoP->DupAcksIn),
        MKUL(env,   infoP->TimeoutEpisodes),
        MKUI(env,   infoP->SynRetrans),
        MKUL(env,   infoP->SndLimTransRwin),
        MKUL(env,   infoP->SndLimTimeRwin),
        MKUI64(env, infoP->SndLimBytesRwin),
        MKUL(env,   infoP->SndLimTransCwnd),
        MKUL(env,   infoP->SndLimTimeCwnd),
        MKUI64(env, infoP->SndLimBytesCwnd),
        MKUL(env,   infoP->SndLimTransSnd),
        MKUL(env,   infoP->SndLimTimeSnd),
        MKUI64(env, infoP->SndLimBytesSnd)};
    unsigned int numKeys = NUM(keys);
    unsigned int numVals = NUM(vals);
    ESOCK_ASSERT( numKeys == numVals );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &einfo) );
    return einfo;
}
#endif
#if defined(SIO_TCP_INFO)
static
ERL_NIF_TERM encode_tcp_state(ErlNifEnv* env, TCPSTATE state)
{
    ERL_NIF_TERM estate;
    switch (state) {
    case TCPSTATE_CLOSED:
        estate = esock_atom_closed;
        break;
    case TCPSTATE_LISTEN:
        estate = esock_atom_listen;
        break;
    case TCPSTATE_SYN_SENT:
        estate = esock_atom_syn_sent;
        break;
    case TCPSTATE_SYN_RCVD:
        estate = esock_atom_syn_rcvd;
        break;
    case TCPSTATE_ESTABLISHED:
        estate = esock_atom_established;
        break;
    case TCPSTATE_FIN_WAIT_1:
        estate = esock_atom_fin_wait_1;
        break;
    case TCPSTATE_FIN_WAIT_2:
        estate = esock_atom_fin_wait_2;
        break;
    case TCPSTATE_CLOSE_WAIT:
        estate = esock_atom_close_wait;
        break;
    case TCPSTATE_CLOSING:
        estate = esock_atom_closing;
        break;
    case TCPSTATE_LAST_ACK:
        estate = esock_atom_last_ack;
        break;
    case TCPSTATE_TIME_WAIT:
        estate = esock_atom_time_wait;
        break;
    case TCPSTATE_MAX:
        estate = esock_atom_max;
        break;
    default:
        estate = MKI(env, state);
        break;
    }
    return estate;
}
#endif
#if defined(SIO_RCVALL)
static
ERL_NIF_TERM esaio_ioctl_rcvall(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     evalue)
{
    DWORD        ndata = 0;
    ERL_NIF_TERM result;
    int          value, res;
    SSDBG( descP, ("WIN-ESAIO", "esaio_ioctl_rcvall(%d) -> entry with"
                   "\r\n      (e)value: %T"
                   "\r\n", descP->sock, evalue) );
    if (! IS_ATOM(env, evalue))
        return enif_make_badarg(env);
    if (COMPARE(evalue, esock_atom_off) == 0) {
        value = RCVALL_OFF;
    } else if (COMPARE(evalue, esock_atom_on) == 0) {
        value = RCVALL_ON;
    } else if (COMPARE(evalue, esock_atom_iplevel) == 0) {
        value = RCVALL_IPLEVEL;
    } else {
        return enif_make_badarg(env);
    }
    res = sock_ioctl2(descP->sock, SIO_RCVALL,
                      &value, sizeof(value),
                      NULL, 0, &ndata);
    (void) ndata;
    if (res != 0) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_ioctl_rcvall(%d) -> failure: "
                "\r\n      reason: %T"
                "\r\n", descP->sock, reason) );
        result = esock_make_error(env, reason);
    } else {
        result = esock_atom_ok;
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_rcvall(%d) -> done with"
            "\r\n      result: %T"
            "\r\n",
            descP->sock, result) );
    return result;
}
#endif
#if defined(SIO_RCVALL_IGMPMCAST)
static
ERL_NIF_TERM esaio_ioctl_rcvall_igmpmcast(ErlNifEnv*       env,
                                          ESockDescriptor* descP,
                                          ERL_NIF_TERM     evalue)
{
    DWORD        ndata = 0;
    ERL_NIF_TERM result;
    int          value, res;
    SSDBG( descP, ("WIN-ESAIO", "esaio_ioctl_rcvall_igmpmcast(%d) -> entry with"
                   "\r\n      (e)value: %T"
                   "\r\n", descP->sock, evalue) );
    if (! IS_ATOM(env, evalue))
        return enif_make_badarg(env);
    if (COMPARE(evalue, esock_atom_off) == 0) {
        value = RCVALL_OFF;
    } else if (COMPARE(evalue, esock_atom_on) == 0) {
        value = RCVALL_ON;
    } else {
        return enif_make_badarg(env);
    }
    res = sock_ioctl2(descP->sock, SIO_RCVALL_IGMPMCAST,
                      &value, sizeof(value),
                      NULL, 0, &ndata);
    (void) ndata;
    if (res != 0) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_ioctl_rcvall_igmpmcast(%d) -> failure: "
                "\r\n      reason: %T"
                "\r\n", descP->sock, reason) );
        result = esock_make_error(env, reason);
    } else {
        result = esock_atom_ok;
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_rcvall_igmpmcast(%d) -> done with"
            "\r\n      result: %T"
            "\r\n",
            descP->sock, result) );
    return result;
}
#endif
#if defined(SIO_RCVALL_MCAST)
static
ERL_NIF_TERM esaio_ioctl_rcvall_mcast(ErlNifEnv*       env,
                                      ESockDescriptor* descP,
                                      ERL_NIF_TERM     evalue)
{
    DWORD        ndata = 0;
    ERL_NIF_TERM result;
    int          value, res;
    SSDBG( descP, ("WIN-ESAIO", "esaio_ioctl_rcvall_mcast(%d) -> entry with"
                   "\r\n      (e)value: %T"
                   "\r\n", descP->sock, evalue) );
    if (! IS_ATOM(env, evalue))
        return enif_make_badarg(env);
    if (COMPARE(evalue, esock_atom_off) == 0) {
        value = RCVALL_OFF;
    } else if (COMPARE(evalue, esock_atom_on) == 0) {
        value = RCVALL_ON;
    } else {
        return enif_make_badarg(env);
    }
    res = sock_ioctl2(descP->sock, SIO_RCVALL_MCAST,
                      &value, sizeof(value),
                      NULL, 0, &ndata);
    (void) ndata;
    if (res != 0) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_ioctl_rcvall_mcast(%d) -> failure: "
                "\r\n      reason: %T"
                "\r\n", descP->sock, reason) );
        result = esock_make_error(env, reason);
    } else {
        result = esock_atom_ok;
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_rcvall_mcast(%d) -> done with"
            "\r\n      result: %T"
            "\r\n",
            descP->sock, result) );
    return result;
}
#endif
extern
ERL_NIF_TERM esaio_ioctl2(ErlNifEnv*       env,
			  ESockDescriptor* descP,
			  unsigned long    req)
{
  switch (req) {
#if defined(FIONREAD)
  case FIONREAD:
      return esaio_ioctl_fionread(env, descP);
      break;
#endif
#if defined(SIOCATMARK)
  case SIOCATMARK:
      return esaio_ioctl_siocatmark(env, descP);
      break;
#endif
  default:
      return esock_make_error(env, esock_atom_enotsup);
      break;
  }
}
#if defined(FIONREAD)
static
ERL_NIF_TERM esaio_ioctl_fionread(ErlNifEnv*       env,
                                  ESockDescriptor* descP)
{
    u_long       n     = 0;
    DWORD        ndata = 0;
    int          res;
    ERL_NIF_TERM result;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_fionread(%d) -> entry\r\n", descP->sock) );
    res = sock_ioctl2(descP->sock, FIONREAD, NULL, 0, &n, sizeof(n), &ndata);
    (void) ndata;
    if (res != 0) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_ioctl_fionread(%d) -> failure: "
                "\r\n      reason: %T"
                "\r\n", descP->sock, reason) );
        result = esock_make_error(env, reason);
    } else {
        result = esock_encode_ioctl_ivalue(env, descP, n);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_ioctl_fionread(%d) -> done with: "
            "\r\n   result: %T"
            "\r\n", descP->sock, result) );
    return result;
}
#endif
#if defined(SIOCATMARK)
static
ERL_NIF_TERM esaio_ioctl_siocatmark(ErlNifEnv*       env,
                                    ESockDescriptor* descP)
{
    int          b     = 0;
    DWORD        ndata = 0;
    int          res;
    ERL_NIF_TERM result;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_ioctl_siocatmark(%d) -> entry\r\n",
            descP->sock) );
    res = sock_ioctl2(descP->sock, SIOCATMARK, NULL, 0, &b, sizeof(b), &ndata);
    (void) ndata;
    if (res != 0) {
        int          save_errno = sock_errno();
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_ioctl_siocatmark(%d) -> failure: "
                "\r\n      reason: %T"
                "\r\n", descP->sock, reason) );
        result = esock_make_error(env, reason);
    } else {
        result = esock_encode_ioctl_bvalue(env, descP, b);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_ioctl_siocatmark(%d) -> done with: "
            "\r\n   result: %T"
            "\r\n", descP->sock, result) );
    return result;
}
#endif
static
void* esaio_completion_main(void* threadDataP)
{
    char             envName[64];
    BOOLEAN_T        done  = FALSE;
    ESAIOThreadData* dataP = (ESAIOThreadData*) threadDataP;
    ESockDescriptor* descP = NULL;
    ESAIOOperation*  opP;
    OVERLAPPED*      olP;
    BOOL             res;
    DWORD            numBytes, flags = 0;
    int              save_errno;
    SGDBG( ("WIN-ESAIO", "esaio_completion_main -> entry\r\n") );
    dataP->state = ESAIO_THREAD_STATE_INITIATING;
    sprintf(envName, "esaio-completion-main[%d]", dataP->id);
    dataP->env = esock_alloc_env(envName);
    dataP->state = ESAIO_THREAD_STATE_OPERATIONAL;
    SGDBG( ("WIN-ESAIO", "esaio_completion_main -> initiated\r\n") );
    while (!done) {
        SGDBG( ("WIN-ESAIO",
                "esaio_completion_main -> [%d] try dequeue packet\r\n",
                dataP->cnt) );
        res = GetQueuedCompletionStatus(ctrl.cport,
                                        &numBytes,
                                        (PULONG_PTR) &descP,
                                        &olP,
                                        INFINITE);
        save_errno = NO_ERROR;
        if (!res) {
            save_errno = sock_errno();
            if (olP == NULL) {
                SGDBG( ("WIN-ESAIO",
                        "esaio_completion_main -> [failure 1]"
                        "\r\n   %s (%d)"
                        "\r\n", erl_errno_id(save_errno), save_errno) );
                dataP->state = ESAIO_THREAD_STATE_TERMINATING;
                dataP->error = ESAIO_THREAD_ERROR_GET;
                opP          = NULL;
                done         = TRUE;
                break;
            } else {
                SGDBG( ("WIN-ESAIO",
                        "esaio_completion_main -> [failure 2] "
                        "\r\n   %s (%d)"
                        "\r\n", erl_errno_id(save_errno), save_errno) );
                opP = CONTAINING_RECORD(olP, ESAIOOperation, ol);
                esaio_completion_inc(dataP);
            }
        } else {
            opP = CONTAINING_RECORD(olP, ESAIOOperation, ol);
            esaio_completion_inc(dataP);
            SGDBG( ("WIN-ESAIO", "esaio_completion_main -> success\r\n") );
        }
        dataP->latest = opP->tag;
        switch (opP->tag) {
        case ESAIO_OP_TERMINATE:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received terminate cmd\r\n") );
            done = esaio_completion_terminate(dataP, (OVERLAPPED*) opP);
            break;
        case ESAIO_OP_CONNECT:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received connect cmd\r\n") );
            done = esaio_completion_connect(dataP, descP, (OVERLAPPED*) opP,
                                            opP->env, &opP->caller,
                                            &opP->data.connect,
                                            save_errno);
            break;
        case ESAIO_OP_ACCEPT:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received accept cmd\r\n") );
            done = esaio_completion_accept(dataP, descP, (OVERLAPPED*) opP,
                                           opP->env, &opP->caller,
                                           &opP->data.accept,
                                           save_errno);
            break;
        case ESAIO_OP_SEND:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received send cmd\r\n") );
            done = esaio_completion_send(dataP, descP, (OVERLAPPED*) opP,
                                         opP->env, &opP->caller,
                                         &opP->data.send,
                                         save_errno);
            break;
        case ESAIO_OP_SENDV:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received sendv cmd\r\n") );
            done = esaio_completion_sendv(dataP, descP, (OVERLAPPED*) opP,
                                          opP->env, &opP->caller,
                                          &opP->data.sendv,
                                          save_errno);
            break;
        case ESAIO_OP_SENDTO:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received sendto cmd\r\n") );
            done = esaio_completion_sendto(dataP, descP, (OVERLAPPED*) opP,
                                           opP->env, &opP->caller,
                                           &opP->data.sendto,
                                           save_errno);
            break;
        case ESAIO_OP_SENDMSG:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received sendmsg cmd\r\n") );
            done = esaio_completion_sendmsg(dataP, descP, (OVERLAPPED*) opP,
                                            opP->env, &opP->caller,
                                            &opP->data.sendmsg,
                                            save_errno);
            break;
        case ESAIO_OP_RECV:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received recv cmd\r\n") );
            done = esaio_completion_recv(dataP, descP, (OVERLAPPED*) opP,
                                         opP->env, &opP->caller,
                                         &opP->data.recv,
                                         save_errno);
            break;
        case ESAIO_OP_RECVFROM:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received recvfrom cmd\r\n") );
            done = esaio_completion_recvfrom(dataP, descP, (OVERLAPPED*) opP,
                                             opP->env, &opP->caller,
                                             &opP->data.recvfrom,
                                             save_errno);
            break;
        case ESAIO_OP_RECVMSG:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received recvmsg cmd\r\n") );
            done = esaio_completion_recvmsg(dataP, descP, (OVERLAPPED*) opP,
                                            opP->env, &opP->caller,
                                            &opP->data.recvmsg,
                                            save_errno);
            break;
        default:
            SGDBG( ("WIN-ESAIO",
                    "esaio_completion_main -> received unknown cmd: "
                    "\r\n   %d"
                    "\r\n",
                    opP->tag) );
            done = esaio_completion_unknown(dataP, descP, (OVERLAPPED*) opP,
                                            numBytes, save_errno);
            break;
        }
        SGDBG( ("WIN-ESAIO", "esaio_completion_main -> free OVERLAPPED\r\n") );
        FREE(opP);
    }
    SGDBG( ("WIN-ESAIO", "esaio_completion_main -> terminating\r\n") );
    TEXIT(threadDataP);
    SGDBG( ("WIN-ESAIO", "esaio_completion_main -> terminated\r\n") );
    dataP->state = ESAIO_THREAD_STATE_TERMINATED;
    SGDBG( ("WIN-ESAIO", "esaio_completion_main -> done\r\n") );
    return threadDataP;
}
static
BOOLEAN_T  esaio_completion_terminate(ESAIOThreadData* dataP,
                                      OVERLAPPED*      ovl)
{
    (void) ovl;
    dataP->state = ESAIO_THREAD_STATE_TERMINATING;
    dataP->error = ESAIO_THREAD_ERROR_CMD;
    return TRUE;
}
static
BOOLEAN_T esaio_completion_connect(ESAIOThreadData*    dataP,
                                   ESockDescriptor*    descP,
                                   OVERLAPPED*         ovl,
                                   ErlNifEnv*          opEnv,
                                   ErlNifPid*          opCaller,
                                   ESAIOOpDataConnect* opDataP,
                                   int                 error)
{
    ErlNifEnv*   env = dataP->env;
    ERL_NIF_TERM reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_connect(%d) -> entry\r\n",
            descP->sock, error) );
    (void) opCaller;
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_connect(%d) -> success"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_connect_success(env, descP, opDataP);
        MUNLOCK(descP->writeMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_connect(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_connect_aborted(env, descP, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_connect(%d) -> unknown failure:"
                "\r\n   %T"
                "\r\n", descP->sock, ENO2T(env, error)) );
        MLOCK(descP->writeMtx);
        esaio_completion_connect_failure(env, descP, opDataP, error);
        MUNLOCK(descP->writeMtx);
        break;
    }
    SGDBG( ("WIN-ESAIO",
            "esaio_completion_connect -> clear and delete op env\r\n") );
    esock_clear_env("esaio_completion_connect", opEnv);
    esock_free_env("esaio_completion_connect", opEnv);
    SGDBG( ("WIN-ESAIO", "esaio_completion_connect -> done\r\n") );
    return FALSE;
}
static
void esaio_completion_connect_success(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ESAIOOpDataConnect* opDataP)
{
    if (descP->connectorP != NULL) {
        if (IS_OPEN(descP->writeState)) {
            esaio_completion_connect_completed(env, descP, opDataP);
        } else {
            esock_requestor_release("esaio_completion_connect_success -> "
                                    "not active",
                                    env, descP, &descP->connector);
            descP->connectorP = NULL;
            descP->writeState &=
                ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
            esaio_completion_connect_not_active(descP);
        }
    } else {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_connect_aborted(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ESAIOOpDataConnect* opDataP)
{
    if (descP->connectorP != NULL) {
        ERL_NIF_TERM reason = esock_atom_closed;
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             descP->connectorP, reason);
        esock_requestor_release("connect_stream_check_result -> abort",
                                env, descP, &descP->connector);
        descP->connectorP = NULL;
        descP->writeState &= ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
        if (! IS_OPEN(descP->writeState)) {
            esaio_stop(env, descP);
        }
    } else {
        descP->writeState &= ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
    }
}
static
void esaio_completion_connect_failure(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ESAIOOpDataConnect* opDataP,
                                      int                 error)
{
    if (descP->connectorP != NULL) {
        ERL_NIF_TERM reason = MKT2(env,
                                   esock_atom_completion_status,
                                   ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             descP->connectorP, reason);
        esaio_completion_connect_fail(env, descP, error, FALSE);
        esock_requestor_release("connect_stream_check_result -> failure",
                                env, descP, &descP->connector);
        descP->connectorP = NULL;
        descP->writeState &= ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
    } else {
        esaio_completion_connect_fail(env, descP, error, TRUE);
    }
}
static
void esaio_completion_connect_completed(ErlNifEnv*          env,
                                        ESockDescriptor*    descP,
                                        ESAIOOpDataConnect* opDataP)
{
    ERL_NIF_TERM completionStatus, completionInfo;
    int          ucres;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_connect_completed(%d) -> "
            "success - try update context\r\n", descP->sock) );
    ucres = ESAIO_UPDATE_CONNECT_CONTEXT( descP->sock );
    if (ucres == 0) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_connect_completed({%d) -> success\r\n",
                descP->sock) );
        descP->writeState &= ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
        descP->writeState |= ESOCK_STATE_CONNECTED;
        completionStatus   = esock_atom_ok;
    } else {
        int          save_errno = sock_errno();
        ERL_NIF_TERM tag        = esock_atom_update_connect_context;
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP, ("WIN-ESAIO",
                       "esaio_completion_connect_completed(%d) -> "
                       "failed update connect context: %T\r\n",
                       descP->sock, reason) );
        descP->writeState = ESOCK_STATE_CLOSED;
        sock_close(descP->sock);
        completionStatus = esock_make_error_t2r(descP->connector.env,
                                                tag, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_connect_completed {%d} -> "
            "completion status: %T\r\n",
            descP->sock, completionStatus) );
    completionInfo = MKT2(descP->connector.env,
                          descP->connector.ref,
                          completionStatus);
    esaio_send_completion_msg(env,
                              descP,
                              &descP->connector.pid,
                              descP->connector.env,
                              CP_TERM(descP->connector.env, opDataP->sockRef),
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_connect_completed {%d} -> cleanup\r\n",
            descP->sock) );
    esock_requestor_release("esaio_completion_connect_completed",
                            env, descP, &descP->connector);
    descP->connectorP = NULL;
}
static
void esaio_completion_connect_not_active(ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_connect_not_active -> "
            "success for cancelled connect\r\n") );
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.unexpectedConnects, 1);
    MUNLOCK(ctrl.cntMtx);
}
static
void esaio_completion_connect_fail(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   int              error,
                                   BOOLEAN_T        inform)
{
    descP->writeState &= ~(ESOCK_STATE_CONNECTING | ESOCK_STATE_SELECTED);
    esaio_completion_fail(env, descP, "connect", error, inform);
}
static
BOOLEAN_T esaio_completion_accept(ESAIOThreadData*   dataP,
                                  ESockDescriptor*   descP,
                                  OVERLAPPED*        ovl,
                                  ErlNifEnv*         opEnv,
                                  ErlNifPid*         opCaller,
                                  ESAIOOpDataAccept* opDataP,
                                  int                error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_accept(%d) -> entry with"
            "\r\n   error: %s (%d)"
            "\r\n", descP->sock, erl_errno_id(error), error) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_accept(%d) -> success"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_accept_success(env, descP, opEnv, opCaller, opDataP);
        MUNLOCK(descP->readMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_accept(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_accept_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_accept(%d) -> unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_accept_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->readMtx);
        break;
    }
    SGDBG( ("WIN-ESAIO",
            "esaio_completion_accept -> clear and delete op env\r\n") );
    FREE( opDataP->buf );
    esock_clear_env("esaio_completion_accept - op cleanup", opEnv);
    esock_free_env("esaio_completion_accept - op cleanup", opEnv);
    SGDBG( ("WIN-ESAIO", "esaio_completion_accept -> done\r\n") );
    return FALSE;
}
static
void esaio_completion_accept_success(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     ErlNifEnv*         opEnv,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataAccept* opDataP)
{
    ESockRequestor req;
    if (esock_acceptor_get(env, descP,
                           &opDataP->accRef,
                           opCaller,
                           &req)) {
        if (IS_OPEN(descP->readState)) {
            esaio_completion_accept_completed(env, descP,
                                              opEnv, opCaller, opDataP,
                                              &req);
        } else {
            esaio_completion_accept_not_active(descP);
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_success(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->acceptorsQ.first == NULL)), descP->readState) );
    if (descP->acceptorsQ.first == NULL) {
        descP->readState &= ~(ESOCK_STATE_ACCEPTING | ESOCK_STATE_SELECTED);
    }
}
static
void esaio_completion_accept_aborted(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataAccept* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "%s(%d) -> "
            "try get request"
            "\r\n", __FUNCTION__, descP->sock) );
    if (esock_acceptor_get(env, descP,
                           &opDataP->accRef,
                           opCaller,
                           &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "%s(%d) -> "
                "send abort message to %T"
                "\r\n", __FUNCTION__, descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->lSockRef,
                             &req, reason);
        esock_clear_env("esaio_cancel_accept -> req cleanup", req.env);
        esock_free_env("esaio_cancel_accept -> req cleanup", req.env);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "%s(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (read) open: %s"
            "\r\n",
            __FUNCTION__, descP->sock, B2S((IS_OPEN(descP->readState)))) );
    if (! IS_OPEN(descP->readState)) {
        if (descP->acceptorsQ.first == NULL) {
            if ((descP->readersQ.first == NULL) &&
                (descP->writersQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "%s(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        __FUNCTION__, descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "%s(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            __FUNCTION__, descP->sock,
            B2S((descP->acceptorsQ.first == NULL)), descP->readState) );
    if (descP->acceptorsQ.first == NULL) {
        descP->readState &= ~(ESOCK_STATE_ACCEPTING | ESOCK_STATE_SELECTED);
    }
}
static
void esaio_completion_accept_failure(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataAccept* opDataP,
                                     int                error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_acceptor_get(env, descP,
                           &opDataP->accRef,
                           opCaller,
                           &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->lSockRef,
                             &req, reason);
        esaio_completion_accept_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_accept_fail(env, descP, error, TRUE);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_failure(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->acceptorsQ.first == NULL)), descP->readState) );
    if (descP->acceptorsQ.first == NULL) {
        descP->readState &= ~(ESOCK_STATE_ACCEPTING | ESOCK_STATE_SELECTED);
    }
}
static
void esaio_completion_accept_completed(ErlNifEnv*         env,
                                       ESockDescriptor*   descP,
                                       ErlNifEnv*         opEnv,
                                       ErlNifPid*         opCaller,
                                       ESAIOOpDataAccept* opDataP,
                                       ESockRequestor*    reqP)
{
    ERL_NIF_TERM     completionStatus, completionInfo;
    int              ucres;
    ESockDescriptor* accDescP;
    ERL_NIF_TERM     accRef, accSocket;
    ESOCK_ASSERT( DEMONP("esaio_completion_accept_completed - acceptor",
                         env, descP, &reqP->mon) == 0);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_completed -> "
            "success - try update context\r\n") );
    ucres = ESAIO_UPDATE_ACCEPT_CONTEXT( opDataP->asock, opDataP->lsock );
    if (ucres == 0) {
        int save_errno;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_accept_completed -> "
                "create (accepted) descriptor\r\n") );
        accDescP = esock_alloc_descriptor(opDataP->asock);
        if (ESAIO_OK != (save_errno = esaio_add_socket(accDescP))) {
            ERL_NIF_TERM tag    = esock_atom_add_socket;
            ERL_NIF_TERM reason = ENO2T(opEnv, save_errno);
            ESOCK_CNT_INC(env, descP, CP_TERM(env, opDataP->lSockRef),
                          esock_atom_acc_fails, &descP->accFails, 1);
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_completion_accept_completed -> "
                    "failed adding (accepted) socket to completion port: "
                    "%T\r\n", reason) );
            esock_dealloc_descriptor(env, accDescP);
            sock_close(opDataP->asock);
            completionStatus = esock_make_error_t2r(opEnv, tag, reason);
        } else {
            ESOCK_CNT_INC(env, descP, CP_TERM(env, opDataP->lSockRef),
                          esock_atom_acc_success, &descP->accSuccess, 1);
            accDescP->domain   = descP->domain;
            accDescP->type     = descP->type;
            accDescP->protocol = descP->protocol;
            MLOCK(descP->writeMtx);
            accDescP->rBufSz   = descP->rBufSz;
            accDescP->rCtrlSz  = descP->rCtrlSz;
            accDescP->wCtrlSz  = descP->wCtrlSz;
            accDescP->iow      = descP->iow;
            accDescP->dbg      = descP->dbg;
            accDescP->useReg   = descP->useReg;
            esock_inc_socket(accDescP->domain, accDescP->type,
                             accDescP->protocol);
            accRef = enif_make_resource(env, accDescP);
            enif_release_resource(accDescP);
            accSocket = esock_mk_socket(opEnv, CP_TERM(opEnv, accRef));
            accDescP->ctrlPid = *opCaller;
            ESOCK_ASSERT( MONP("esaio_completion_accept_completed -> ctrl",
                               env, accDescP,
                               &accDescP->ctrlPid,
                               &accDescP->ctrlMon) == 0 );
            accDescP->writeState |= ESOCK_STATE_CONNECTED;
            MUNLOCK(descP->writeMtx);
            if (descP->useReg)
                esock_send_reg_add_msg(env, descP, accRef);
            completionStatus = esock_make_ok2(opEnv, accSocket);
        }
    } else {
        int          save_errno = sock_errno();
        ERL_NIF_TERM tag        = esock_atom_update_accept_context;
        ERL_NIF_TERM reason     = ENO2T(env, save_errno);
        SSDBG( descP, ("WIN-ESAIO",
                       "esaio_completion_accept_completed(%d) -> "
                       "accept context update failed: %T (%d)\r\n",
                       descP->sock, reason, save_errno) );
        sock_close(descP->sock);
        descP->writeState = ESOCK_STATE_CLOSED;
        completionStatus = esock_make_error_t2r(opEnv, tag, reason);
    }
    completionInfo = MKT2(opEnv, opDataP->accRef, completionStatus);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_completed -> "
            "send completion message to %T with"
            "\r\n   CompletionInfo: %T"
            "\r\n", MKPID(env, opCaller), completionInfo) );
    esaio_send_completion_msg(env,
                              descP,
                              opCaller,
                              opEnv,
                              opDataP->lSockRef,
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_accept_completed -> finalize\r\n") );
    esock_clear_env("esaio_completion_accept_completed -> req cleanup",
                    reqP->env);
    esock_free_env("esaio_completion_accept_completed -> req cleanup",
                   reqP->env);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_accept_completed -> done\r\n") );
}
static
void esaio_completion_accept_not_active(ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_not_active(%d) -> "
            "success for not active accept request\r\n", descP->sock) );
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.unexpectedAccepts, 1);
    MUNLOCK(ctrl.cntMtx);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_accept_not_active(%d) -> done\r\n",
            descP->sock) );
}
static
void esaio_completion_accept_fail(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  int              error,
                                  BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "accept", error, inform);
}
static
BOOLEAN_T esaio_completion_send(ESAIOThreadData* dataP,
                                ESockDescriptor* descP,
                                OVERLAPPED*      ovl,
                                ErlNifEnv*       opEnv,
                                ErlNifPid*       opCaller,
                                ESAIOOpDataSend* opDataP,
                                int              error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_send(%d) -> entry with"
            "\r\n   error: %T"
            "\r\n", descP->sock, ENO2T(env, error)) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_send(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_send_success(env, descP, ovl, opEnv,
                                      opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_send(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_send_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_send(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_send_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->writeMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send(%d) -> cleanup\r\n", descP->sock) );
    FREE( opDataP->wbuf.buf );
    esock_clear_env("esaio_completion_send - op cleanup", opEnv);
    esock_free_env("esaio_completion_send - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_send(%d) -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_send_success(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   OVERLAPPED*      ovl,
                                   ErlNifEnv*       opEnv,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataSend* opDataP)
{
    ESockRequestor req;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->writeState)) {
            esaio_completion_send_completed(env, descP, ovl, opEnv,
                                            opCaller,
                                            opDataP->sockRef,
                                            opDataP->sendRef,
                                            opDataP->wbuf.len,
                                            FALSE,
                                            &req);
        } else {
            esaio_completion_send_not_active(descP);
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_success(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_send_aborted(ErlNifEnv*         env,
                                   ESockDescriptor* descP,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataSend* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_send_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (write) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->writeState)))) );
    if (! IS_OPEN(descP->writeState)) {
        if (descP->writersQ.first == NULL) {
            if ((descP->readersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_send_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_aborted(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_send_failure(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataSend* opDataP,
                                   int              error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_send_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_send_fail(env, descP, error, TRUE);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_failure(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_send_completed(ErlNifEnv*       env,
                                     ESockDescriptor* descP,
                                     OVERLAPPED*      ovl,
                                     ErlNifEnv*       opEnv,
                                     ErlNifPid*       sender,
                                     ERL_NIF_TERM     sockRef,
                                     ERL_NIF_TERM     sendRef,
                                     DWORD            toWrite,
                                     BOOLEAN_T        dataInTail,
                                     ESockRequestor*  reqP)
{
    ERL_NIF_TERM completionStatus, completionInfo;
    DWORD        written;
    ESOCK_ASSERT( DEMONP("esaio_completion_send_completed - sender",
                         env, descP, &reqP->mon) == 0);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_completed ->"
            "success - try get overlapped result\r\n") );
    if (get_send_ovl_result(descP->sock, ovl, &written)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_send_completed -> overlapped result: "
                "\r\n   written:     %d"
                "\r\n   buffer size: %d"
                "\r\n", written, toWrite) );
        if (written == toWrite) {
            if (dataInTail) {
                completionStatus = esaio_completion_send_partial(env,
                                                                 descP,
                                                                 sockRef,
                                                                 written);
            } else {
                completionStatus = esaio_completion_send_done(env,
                                                              descP, sockRef,
                                                              written);
            }
        } else {
            completionStatus = esaio_completion_send_partial(env,
                                                             descP,
                                                             sockRef,
                                                             written);
        }
    } else {
        int save_errno = sock_errno();
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_send_completed -> "
                "overlapped result failure: %d\r\n", save_errno) );
        completionStatus =
            esaio_completion_get_ovl_result_fail(env, descP, save_errno);
    }
    completionInfo = MKT2(env, CP_TERM(env, sendRef), completionStatus);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_completed -> "
            "send completion message to %T with"
            "\r\n   CompletionInfo: %T"
            "\r\n", MKPID(env, sender), completionInfo) );
    esaio_send_completion_msg(env,
                              descP,
                              sender,
                              opEnv,
                              sockRef,
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_send_completed -> done\r\n") );
}
static
ERL_NIF_TERM esaio_completion_send_done(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ERL_NIF_TERM     sockRef,
                                        DWORD            written)
{
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_pkg, &descP->writePkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_byte, &descP->writeByteCnt, written);
    if (written > descP->writePkgMax)
        descP->writePkgMax = written;
    return esock_atom_ok;
}
static
ERL_NIF_TERM esaio_completion_send_partial(ErlNifEnv*       env,
                                           ESockDescriptor* descP,
                                           ERL_NIF_TERM     sockRef,
                                           DWORD            written)
{
    if (written > 0) {
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_write_pkg, &descP->writePkgCnt, 1);
        ESOCK_CNT_INC(env, descP, sockRef,
                      esock_atom_write_byte, &descP->writeByteCnt, written);
        if (written > descP->writePkgMax)
            descP->writePkgMax = written;
    }
    return esock_make_ok2(env, MKUI64(env, written));
}
static
void esaio_completion_send_not_active(ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_not_active(%d) -> "
            "success for not active send request\r\n",
            descP->sock) );
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.unexpectedWrites, 1);
    MUNLOCK(ctrl.cntMtx);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_send_not_active(%d) -> done\r\n",
            descP->sock) );
}
static
void esaio_completion_send_fail(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                int              error,
                                BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "send", error, inform);
}
static
BOOLEAN_T esaio_completion_sendv(ESAIOThreadData*  dataP,
                                 ESockDescriptor*  descP,
                                 OVERLAPPED*       ovl,
                                 ErlNifEnv*        opEnv,
                                 ErlNifPid*        opCaller,
                                 ESAIOOpDataSendv* opDataP,
                                 int               error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendv(%d) -> entry with"
            "\r\n   opDataP: 0x%lX"
            "\r\n   error:   %d"
            "\r\n", descP->sock, opDataP, error) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_sendv(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendv_success(env, descP, ovl, opEnv,
                                       opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendv(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_sendv_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendv(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendv_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->writeMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendv(%d) -> cleanup\r\n", descP->sock) );
    FREE( opDataP->lpBuffers );
    esock_clear_env("esaio_completion_sendv - op cleanup", opEnv);
    esock_free_env("esaio_completion_sendv - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendv(%d) -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_sendv_success(ErlNifEnv*        env,
                                    ESockDescriptor*  descP,
                                    OVERLAPPED*       ovl,
                                    ErlNifEnv*        opEnv,
                                    ErlNifPid*        opCaller,
                                    ESAIOOpDataSendv* opDataP)
{
    ESockRequestor req;
    BOOLEAN_T      cleanup;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->writeState)) {
            esaio_completion_send_completed(env, descP, ovl, opEnv,
                                            opCaller,
                                            opDataP->sockRef,
                                            opDataP->sendRef,
                                            opDataP->toWrite,
                                            opDataP->dataInTail,
                                            &req);
        } else {
            esaio_completion_send_not_active(descP);
        }
        cleanup = TRUE;
    } else {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendv_success(%d) -> "
                "ghost activation (no cleanup)"
                "\r\n", descP->sock) );
        cleanup = FALSE;
    }
    if (cleanup) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendv_success(%d) -> "
                "maybe update (write) state (0x%X)\r\n",
                descP->sock,
                B2S((descP->writersQ.first == NULL)), descP->writeState) );
        if (descP->writersQ.first == NULL) {
            descP->writeState &= ~ESOCK_STATE_SELECTED;
        }
    }
}
static
void esaio_completion_sendv_aborted(ErlNifEnv*        env,
                                    ESockDescriptor*  descP,
                                    ErlNifPid*        opCaller,
                                    ESAIOOpDataSendv* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendv_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendv_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendv_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (write) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->writeState)))) );
    if (! IS_OPEN(descP->writeState)) {
        if (descP->writersQ.first == NULL) {
            if ((descP->readersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_sendv_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendv_aborted(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendv_failure(ErlNifEnv*        env,
                                    ESockDescriptor*  descP,
                                    ErlNifPid*        opCaller,
                                    ESAIOOpDataSendv* opDataP,
                                    int               error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_sendv_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_sendv_fail(env, descP, error, TRUE);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendv_failure(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendv_fail(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 int              error,
                                 BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "sendv", error, inform);
}
static
BOOLEAN_T esaio_completion_sendto(ESAIOThreadData*   dataP,
                                  ESockDescriptor*   descP,
                                  OVERLAPPED*        ovl,
                                  ErlNifEnv*         opEnv,
                                  ErlNifPid*         opCaller,
                                  ESAIOOpDataSendTo* opDataP,
                                  int                error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendto(%d) -> entry"
            "\r\n   error: %T"
            "\r\n", descP->sock, ENO2T(env, error)) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_sendto(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendto_success(env, descP, ovl, opEnv,
                                        opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendto(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_sendto_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendto(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendto_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->writeMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto(%d) -> cleanup\r\n",
            descP->sock) );
    FREE( opDataP->wbuf.buf );
    esock_clear_env("esaio_completion_sendto - op cleanup", opEnv);
    esock_free_env("esaio_completion_sendto - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendto(%d) -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_sendto_success(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     OVERLAPPED*        ovl,
                                     ErlNifEnv*         opEnv,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataSendTo* opDataP)
{
    ESockRequestor req;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->writeState)) {
            esaio_completion_send_completed(env, descP, ovl, opEnv,
                                            opCaller,
                                            opDataP->sockRef,
                                            opDataP->sendRef,
                                            opDataP->wbuf.len,
                                            FALSE,
                                            &req);
        } else {
            esaio_completion_send_not_active(descP);
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto_success(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendto_aborted(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataSendTo* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendto_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (write) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->writeState)))) );
    if (! IS_OPEN(descP->writeState)) {
        if (descP->writersQ.first == NULL) {
            if ((descP->readersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_sendto_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto_aborted(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendto_failure(ErlNifEnv*         env,
                                     ESockDescriptor*   descP,
                                     ErlNifPid*         opCaller,
                                     ESAIOOpDataSendTo* opDataP,
                                     int                error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_sendto_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_sendto_fail(env, descP, error, TRUE);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendto_failure(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendto_fail(ErlNifEnv*       env,
                                  ESockDescriptor* descP,
                                  int              error,
                                  BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "sendto", error, inform);
}
static
BOOLEAN_T esaio_completion_sendmsg(ESAIOThreadData*    dataP,
                                   ESockDescriptor*    descP,
                                   OVERLAPPED*         ovl,
                                   ErlNifEnv*          opEnv,
                                   ErlNifPid*          opCaller,
                                   ESAIOOpDataSendMsg* opDataP,
                                   int                 error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendmsg(%d) -> entry with"
            "\r\n   error: %T"
            "\r\n", descP->sock, ENO2T(env, error)) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_sendmsg(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendmsg_success(env, descP, ovl, opEnv,
                                         opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendmsg(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_sendmsg_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendmsg(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->writeMtx);
        esaio_completion_sendmsg_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->writeMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg(%d) -> cleanup\r\n", descP->sock) );
    FREE( opDataP->msg.lpBuffers );
    if (opDataP->ctrlBuf != NULL)
        FREE( opDataP->ctrlBuf );
    esock_clear_env("esaio_completion_sendmsg - op cleanup", opEnv);
    esock_free_env("esaio_completion_sendmsg - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_sendmsg(%d) -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_sendmsg_success(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      OVERLAPPED*         ovl,
                                      ErlNifEnv*          opEnv,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataSendMsg* opDataP)
{
    ESockRequestor req;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->writeState)) {
            DWORD toWrite = 0;
            for (int i = 0; i < opDataP->iovec->iovcnt; i++) {
                toWrite += opDataP->iovec->iov[i].iov_len;
            }
            esaio_completion_send_completed(env, descP, ovl, opEnv,
                                            opCaller,
                                            opDataP->sockRef,
                                            opDataP->sendRef,
                                            toWrite,
                                            FALSE,
                                            &req);
        } else {
            esaio_completion_send_not_active(descP);
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg_success(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendmsg_aborted(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataSendMsg* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_sendmsg_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (write) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->writeState)))) );
    if (! IS_OPEN(descP->writeState)) {
        if (descP->writersQ.first == NULL) {
            if ((descP->readersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_sendmsg_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg_aborted(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendmsg_failure(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataSendMsg* opDataP,
                                      int                 error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_writer_get(env, descP,
                         &opDataP->sendRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_sendmsg_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_sendmsg_fail(env, descP, error, TRUE);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_sendmsg_success(%d) -> "
            "maybe (%s) update (write) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->writersQ.first == NULL)), descP->writeState) );
    if (descP->writersQ.first == NULL) {
        descP->writeState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_sendmsg_fail(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   int              error,
                                   BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "sendmsg", error, inform);
}
static
BOOLEAN_T esaio_completion_recv(ESAIOThreadData* dataP,
                                ESockDescriptor* descP,
                                OVERLAPPED*      ovl,
                                ErlNifEnv*       opEnv,
                                ErlNifPid*       opCaller,
                                ESAIOOpDataRecv* opDataP,
                                int              error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recv(%d) -> entry with"
            "\r\n   error: %T"
            "\r\n", descP->sock, ENO2T(env, error)) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_recv(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recv_success(env, descP, ovl, opEnv,
                                      opCaller, opDataP);
        MUNLOCK(descP->readMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_recv_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recv_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->readMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv {%d} -> clear and delete op env\r\n",
            descP->sock) );
    esock_clear_env("esaio_completion_recv - op cleanup", opEnv);
    esock_free_env("esaio_completion_recv - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recv(%d) -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_recv_success(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   OVERLAPPED*      ovl,
                                   ErlNifEnv*       opEnv,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataRecv* opDataP)
{
    ESockRequestor req;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->readState)) {
            esaio_completion_recv_completed(env, descP, ovl, opEnv,
                                            opCaller, opDataP,
                                            &req);
        } else {
            esaio_completion_recv_not_active(descP);
            FREE_BIN( &opDataP->buf );
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_success(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recv_aborted(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataRecv* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (read) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->readState)))) );
    if (! IS_OPEN(descP->readState)) {
        if (descP->readersQ.first == NULL) {
            if ((descP->writersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_recv_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    FREE_BIN( &opDataP->buf );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_aborted(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recv_failure(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   ErlNifPid*       opCaller,
                                   ESAIOOpDataRecv* opDataP,
                                   int              error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_recv_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_recv_fail(env, descP, error, TRUE);
    }
    FREE_BIN( &opDataP->buf );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_failure(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recv_completed(ErlNifEnv*       env,
                                     ESockDescriptor* descP,
                                     OVERLAPPED*      ovl,
                                     ErlNifEnv*       opEnv,
                                     ErlNifPid*       opCaller,
                                     ESAIOOpDataRecv* opDataP,
                                     ESockRequestor*  reqP)
{
    ERL_NIF_TERM completionStatus, completionInfo;
    DWORD        read, flags;
    ESOCK_ASSERT( DEMONP("esaio_completion_recv_completed - sender",
                         env, descP, &reqP->mon) == 0);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_completed ->"
            "success - try get overlapped result\r\n") );
    if (get_recv_ovl_result(descP->sock, ovl, &read, &flags)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_completed -> overlapped result: "
                "\r\n   read:        %d"
                "\r\n   buffer size: %d"
                "\r\n   flags:       %d"
                "\r\n", read, opDataP->buf.size, flags) );
        if ((read == 0) && (descP->type == SOCK_STREAM)) {
            ESOCK_CNT_INC(env, descP, opDataP->sockRef,
                          esock_atom_read_fails, &descP->readFails, 1);
            completionStatus = esock_make_error(opEnv, esock_atom_closed);
        } else {
            if (read == opDataP->buf.size) {
                completionStatus =
                    esaio_completion_recv_done(env, descP,
                                               opEnv, opDataP,
                                               flags);
            } else {
                completionStatus =
                    esaio_completion_recv_partial(env, descP,
                                                  opEnv, opDataP,
                                                  reqP, read, flags);
            }
        }
    } else {
        int save_errno = sock_errno();
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_completed -> "
                "overlapped result failure: %d\r\n", save_errno) );
        completionStatus =
            esaio_completion_get_ovl_result_fail(opEnv, descP, save_errno);
    }
    completionInfo = MKT2(opEnv, opDataP->recvRef, completionStatus);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_completed -> "
            "send completion message to %T with"
            "\r\n   CompletionInfo: %T"
            "\r\n", MKPID(env, opCaller), completionInfo) );
    esaio_send_completion_msg(env,
                              descP,
                              opCaller,
                              opEnv,
                              opDataP->sockRef,
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recv_completed -> finalize\r\n") );
    esock_clear_env("esaio_completion_recv_completed -> req cleanup",
                    reqP->env);
    esock_free_env("esaio_completion_recv_completed -> req cleanup",
                   reqP->env);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recv_completed -> done\r\n") );
}
static
ERL_NIF_TERM esaio_completion_recv_done(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ErlNifEnv*       opEnv,
                                        ESAIOOpDataRecv* opDataP,
                                        DWORD            flags)
{
    ERL_NIF_TERM data;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    DWORD        read    = opDataP->buf.size;
    (void) flags;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_done(%T) {%d} -> entry with"
            "\r\n   recvRef: %T"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock, recvRef, flags) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    data = MKBIN(opEnv, &opDataP->buf);
    return esock_make_ok2(opEnv, data);
}
static
ERL_NIF_TERM esaio_completion_recv_partial(ErlNifEnv*       env,
                                           ESockDescriptor* descP,
                                           ErlNifEnv*       opEnv,
                                           ESAIOOpDataRecv* opDataP,
                                           ESockRequestor*  reqP,
                                           DWORD            read,
                                           DWORD            flags)
{
    ERL_NIF_TERM res;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    DWORD        toRead  = opDataP->toRead;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_partial(%T) {%d} -> entry with"
            "\r\n   toRead:  %ld"
            "\r\n   recvRef: %T"
            "\r\n   read:    %ld"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock,
            (long) toRead, recvRef, (long) read, flags) );
    if ((toRead == 0) ||
        (descP->type != SOCK_STREAM)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_partial(%T) {%d} -> done reading\r\n",
                sockRef, descP->sock) );
        res = esaio_completion_recv_partial_done(env, descP,
                                                 opEnv, opDataP,
                                                 read, flags);
    } else {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_partial(%T) {%d} ->"
                " only part of data - expected more"
                "\r\n", sockRef, descP->sock) );
        res = esaio_completion_recv_partial_part(env, descP,
                                                 opEnv, opDataP,
                                                 read, flags);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_partial(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return res;
}
static
ERL_NIF_TERM esaio_completion_recv_partial_done(ErlNifEnv*       env,
                                                ESockDescriptor* descP,
                                                ErlNifEnv*       opEnv,
                                                ESAIOOpDataRecv* opDataP,
                                                ssize_t          read,
                                                DWORD            flags)
{
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM data;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    ESOCK_ASSERT( REALLOC_BIN(&opDataP->buf, read) );
    data = MKBIN(opEnv, &opDataP->buf);
    (void) flags;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_partial_done(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return esock_make_ok2(opEnv, data);
}
static
ERL_NIF_TERM esaio_completion_recv_partial_part(ErlNifEnv*       env,
                                                ESockDescriptor* descP,
                                                ErlNifEnv*       opEnv,
                                                ESAIOOpDataRecv* opDataP,
                                                ssize_t          read,
                                                DWORD            flags)
{
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM data;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    data = MKBIN(opEnv, &opDataP->buf);
    data = MKSBIN(opEnv, data, 0, read);
    (void) flags;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_partial_part(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return MKT2(env, esock_atom_more, data);
}
static
void esaio_completion_recv_not_active(ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_not_active {%d} -> "
            "success for not active read request\r\n", descP->sock) );
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.unexpectedReads, 1);
    MUNLOCK(ctrl.cntMtx);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recv_not_active {%d} -> done\r\n",
            descP->sock) );
}
static
void esaio_completion_recv_closed(ESockDescriptor* descP,
                                  int              error)
{
    if (error == NO_ERROR) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recv_closed -> "
                "success for closed socket (%d)\r\n",
                descP->sock) );
        MLOCK(ctrl.cntMtx);
        esock_cnt_inc(&ctrl.unexpectedReads, 1);
        MUNLOCK(ctrl.cntMtx);
    }
}
static
void esaio_completion_recv_fail(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                int              error,
                                BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "recv", error, inform);
}
static
BOOLEAN_T esaio_completion_recvfrom(ESAIOThreadData*     dataP,
                                    ESockDescriptor*     descP,
                                    OVERLAPPED*          ovl,
                                    ErlNifEnv*           opEnv,
                                    ErlNifPid*           opCaller,
                                    ESAIOOpDataRecvFrom* opDataP,
                                    int                  error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvfrom(%d) -> entry with"
            "\r\n   error: %T, %s (%d)"
            "\r\n",
            descP->sock, ENO2T(env, error),
            erl_errno_id(error), error) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_recvfrom(%d) -> no error"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recvfrom_success(env, descP, ovl, opEnv,
                                          opCaller, opDataP);
        MUNLOCK(descP->readMtx);
        break;
    case ERROR_MORE_DATA:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom(%d) -> more data"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recvfrom_more_data(env, descP,
                                            opEnv, opCaller, opDataP,
                                            error);
        MUNLOCK(descP->readMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_recvfrom_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom(%d) -> operation unknown failure"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recvfrom_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->readMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom {%d} -> clear and delete op env\r\n") );
    esock_clear_env("esaio_completion_recvfrom - op cleanup", opEnv);
    esock_free_env("esaio_completion_recvfrom - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvfrom {%d} -> done\r\n") );
    return FALSE;
}
static
void esaio_completion_recvfrom_success(ErlNifEnv*           env,
                                       ESockDescriptor*     descP,
                                       OVERLAPPED*          ovl,
                                       ErlNifEnv*           opEnv,
                                       ErlNifPid*           opCaller,
                                       ESAIOOpDataRecvFrom* opDataP)
{
    ESockRequestor req;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->readState)) {
            esaio_completion_recvfrom_completed(env, descP,
                                                ovl, opEnv, opCaller,
                                                opDataP, &req);
        } else {
            esaio_completion_recv_not_active(descP);
            FREE_BIN( &opDataP->buf );
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_success(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvfrom_more_data(ErlNifEnv*           env,
                                         ESockDescriptor*     descP,
                                         ErlNifEnv*           opEnv,
                                         ErlNifPid*           opCaller,
                                         ESAIOOpDataRecvFrom* opDataP,
                                         int                  error)
{
    ESockRequestor req;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->readState)) {
            ERL_NIF_TERM reason           = MKT2(env,
                                                 esock_atom_completion_status,
                                                 ENO2T(env, error));
            ERL_NIF_TERM completionStatus = esock_make_error(env, reason);
            ERL_NIF_TERM completionInfo   = MKT2(opEnv,
                                                 opDataP->recvRef,
                                                 completionStatus);
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_completion_recvfrom_more_data(%d) -> "
                    "send completion message: "
                    "\r\n   Completion Status: %T"
                    "\r\n", descP->sock, completionStatus) );
            esaio_send_completion_msg(env,
                                      descP,
                                      opCaller,
                                      opEnv,
                                      opDataP->sockRef,
                                      completionInfo);
        }
        FREE_BIN( &opDataP->buf );
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_more_data(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvfrom_aborted(ErlNifEnv*           env,
                                       ESockDescriptor*     descP,
                                       ErlNifPid*           opCaller,
                                       ESAIOOpDataRecvFrom* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (read) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->readState)))) );
    if (! IS_OPEN(descP->readState)) {
        if (descP->readersQ.first == NULL) {
            if ((descP->writersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_recvfrom_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    FREE_BIN( &opDataP->buf );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_aborted(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvfrom_failure(ErlNifEnv*           env,
                                       ESockDescriptor*     descP,
                                       ErlNifPid*           opCaller,
                                       ESAIOOpDataRecvFrom* opDataP,
                                       int                  error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_recvfrom_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_recvfrom_fail(env, descP, error, TRUE);
    }
    FREE_BIN( &opDataP->buf );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_failure(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvfrom_completed(ErlNifEnv*           env,
                                         ESockDescriptor*     descP,
                                         OVERLAPPED*          ovl,
                                         ErlNifEnv*           opEnv,
                                         ErlNifPid*           opCaller,
                                         ESAIOOpDataRecvFrom* opDataP,
                                         ESockRequestor*      reqP)
{
    ERL_NIF_TERM completionStatus, completionInfo;
    DWORD        read, flags;
    ESOCK_ASSERT( DEMONP("esaio_completion_recvfrom_completed - sender",
                         env, descP, &reqP->mon) == 0);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_completed ->"
            "success - try get overlapped result\r\n") );
    if (get_recv_ovl_result(descP->sock, ovl, &read, &flags)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom_completed -> overlapped result: "
                "\r\n   read:        %d"
                "\r\n   buffer size: %d"
                "\r\n   flags:       %d"
                "\r\n", read, opDataP->buf.size, flags) );
        if (read == opDataP->buf.size) {
            completionStatus =
                esaio_completion_recvfrom_done(env, descP,
                                               opEnv, opDataP,
                                               flags);
        } else {
            completionStatus =
                esaio_completion_recvfrom_partial(env, descP,
                                                  opEnv, opDataP,
                                                  reqP, read, flags);
        }
    } else {
        int save_errno = sock_errno();
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvfrom_completed -> "
                "overlapped result failure: %d\r\n", save_errno) );
        completionStatus =
            esaio_completion_get_ovl_result_fail(opEnv, descP, save_errno);
        FREE_BIN( &opDataP->buf );
    }
    completionInfo = MKT2(opEnv, opDataP->recvRef, completionStatus);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_completed -> "
            "send completion message to %T with"
            "\r\n   CompletionInfo: %T"
            "\r\n", MKPID(env, opCaller), completionInfo) );
    esaio_send_completion_msg(env,
                              descP,
                              opCaller,
                              opEnv,
                              opDataP->sockRef,
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_completed -> finalize\r\n") );
    esock_clear_env("esaio_completion_recvfrom_completed -> req cleanup",
                    reqP->env);
    esock_free_env("esaio_completion_recvfrom_completed -> req cleanup",
                   reqP->env);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvfrom_completed -> done\r\n") );
}
static
ERL_NIF_TERM esaio_completion_recvfrom_done(ErlNifEnv*           env,
                                            ESockDescriptor*     descP,
                                            ErlNifEnv*           opEnv,
                                            ESAIOOpDataRecvFrom* opDataP,
                                            DWORD                flags)
{
    ERL_NIF_TERM res, data, eSockAddr;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    DWORD        read    = opDataP->buf.size;
    (void) flags;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_done(%T) {%d} -> entry with"
            "\r\n   recvRef: %T"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock, recvRef, flags) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    esock_encode_sockaddr(opEnv,
                          &opDataP->fromAddr,
                          opDataP->addrLen,
                          &eSockAddr);
    data = MKBIN(opEnv, &opDataP->buf);
    res = esock_make_ok2(opEnv, MKT2(opEnv, eSockAddr, data));
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_done(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return res;
}
static
ERL_NIF_TERM esaio_completion_recvfrom_partial(ErlNifEnv*           env,
                                               ESockDescriptor*     descP,
                                               ErlNifEnv*           opEnv,
                                               ESAIOOpDataRecvFrom* opDataP,
                                               ESockRequestor*      reqP,
                                               DWORD                read,
                                               DWORD                flags)
{
    ERL_NIF_TERM res, data, eSockAddr;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    (void) flags;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_partial(%T) {%d} -> entry with"
            "\r\n   recvRef: %T"
            "\r\n   read:    %ld"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock, recvRef, (long) read, flags) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    esock_encode_sockaddr(opEnv,
                          &opDataP->fromAddr,
                          opDataP->addrLen,
                          &eSockAddr);
    ESOCK_ASSERT( REALLOC_BIN(&opDataP->buf, read) );
    data = MKBIN(opEnv, &opDataP->buf);
    res = esock_make_ok2(opEnv, MKT2(opEnv, eSockAddr, data));
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvfrom_partial(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return res;
}
static
void esaio_completion_recvfrom_fail(ErlNifEnv*       env,
                                    ESockDescriptor* descP,
                                    int              error,
                                    BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "recvfrom", error, inform);
}
static
BOOLEAN_T esaio_completion_recvmsg(ESAIOThreadData*    dataP,
                                   ESockDescriptor*    descP,
                                   OVERLAPPED*         ovl,
                                   ErlNifEnv*          opEnv,
                                   ErlNifPid*          opCaller,
                                   ESAIOOpDataRecvMsg* opDataP,
                                   int                 error)
{
    ErlNifEnv*     env = dataP->env;
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvmsg(%d) -> entry with"
            "\r\n   error: %T"
            "\r\n", descP->sock, ENO2T(env, error)) );
    switch (error) {
    case NO_ERROR:
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_completion_recvmsg(%d) -> no error:"
                "\r\n   try get request %T from %T"
                "\r\n",
                descP->sock,
                opDataP->recvRef, MKPID(env, opCaller)) );
        MLOCK(descP->readMtx);
        esaio_completion_recvmsg_success(env, descP, ovl, opEnv,
                                         opCaller, opDataP);
        MUNLOCK(descP->readMtx);
        break;
    case WSA_OPERATION_ABORTED:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvmsg(%d) -> operation aborted"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        MLOCK(descP->writeMtx);
        esaio_completion_recvmsg_aborted(env, descP, opCaller, opDataP);
        MUNLOCK(descP->writeMtx);
        MUNLOCK(descP->readMtx);
        break;
    default:
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvmsg(%d) -> unknown operation failure"
                "\r\n", descP->sock) );
        MLOCK(descP->readMtx);
        esaio_completion_recvmsg_failure(env, descP, opCaller, opDataP, error);
        MUNLOCK(descP->readMtx);
        break;
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg {%d} -> clear and delete op env\r\n",
            descP->sock) );
    esock_clear_env("esaio_completion_recvmsg - op cleanup", opEnv);
    esock_free_env("esaio_completion_recvmsg - op cleanup", opEnv);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvmsg {%d} -> done\r\n",
            descP->sock) );
    return FALSE;
}
static
void esaio_completion_recvmsg_success(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      OVERLAPPED*         ovl,
                                      ErlNifEnv*          opEnv,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataRecvMsg* opDataP)
{
    ESockRequestor req;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        if (IS_OPEN(descP->readState)) {
            esaio_completion_recvmsg_completed(env, descP, ovl, opEnv,
                                               opCaller, opDataP,
                                               &req);
        } else {
            esaio_completion_recv_not_active(descP);
            FREE_BIN( &opDataP->data[0] );
            FREE_BIN( &opDataP->ctrl );
        }
    } else {
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_success(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvmsg_aborted(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataRecvMsg* opDataP)
{
    ESockRequestor req;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_aborted(%d) -> "
            "try get request"
            "\r\n", descP->sock) );
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        ERL_NIF_TERM reason = esock_atom_closed;
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvmsg_aborted(%d) -> "
                "send abort message to %T"
                "\r\n", descP->sock, req.pid) );
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_aborted(%d) -> "
            "maybe send close message => "
            "\r\n   is socket (read) open: %s"
            "\r\n",
            descP->sock, B2S((IS_OPEN(descP->readState)))) );
    if (! IS_OPEN(descP->readState)) {
        if (descP->readersQ.first == NULL) {
            if ((descP->writersQ.first == NULL) &&
                (descP->acceptorsQ.first == NULL)) {
                SSDBG( descP,
                       ("WIN-ESAIO",
                        "esaio_completion_recvmsg_aborted(%d) -> "
                        "all queues are empty => "
                        "\r\n   send close message"
                        "\r\n",
                        descP->sock) );
                esaio_stop(env, descP);
            }
        }
    }
    FREE_BIN( &opDataP->data[0] );
    FREE_BIN( &opDataP->ctrl );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_aborted(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvmsg_failure(ErlNifEnv*          env,
                                      ESockDescriptor*    descP,
                                      ErlNifPid*          opCaller,
                                      ESAIOOpDataRecvMsg* opDataP,
                                      int                 error)
{
    ESockRequestor req;
    ERL_NIF_TERM   reason;
    if (esock_reader_get(env, descP,
                         &opDataP->recvRef,
                         opCaller,
                         &req)) {
        reason = MKT2(env,
                      esock_atom_completion_status,
                      ENO2T(env, error));
        esock_send_abort_msg(env, descP, opDataP->sockRef,
                             &req, reason);
        esaio_completion_recvmsg_fail(env, descP, error, FALSE);
    } else {
        esaio_completion_recvmsg_fail(env, descP, error, TRUE);
    }
    FREE_BIN( &opDataP->data[0] );
    FREE_BIN( &opDataP->ctrl );
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_failure(%d) -> "
            "maybe (%s) update (read) state (0x%X)\r\n",
            descP->sock,
            B2S((descP->readersQ.first == NULL)), descP->readState) );
    if (descP->readersQ.first == NULL) {
        descP->readState &= ~ESOCK_STATE_SELECTED;
    }
}
static
void esaio_completion_recvmsg_completed(ErlNifEnv*          env,
                                        ESockDescriptor*    descP,
                                        OVERLAPPED*         ovl,
                                        ErlNifEnv*          opEnv,
                                        ErlNifPid*          opCaller,
                                        ESAIOOpDataRecvMsg* opDataP,
                                        ESockRequestor*     reqP)
{
    ERL_NIF_TERM completionStatus, completionInfo;
    DWORD        read, flags;
    ESOCK_ASSERT( DEMONP("esaio_completion_recvmsg_completed - sender",
                         env, descP, &reqP->mon) == 0);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_completed ->"
            "success - try get overlapped result\r\n") );
    if (get_recv_ovl_result(descP->sock, ovl, &read, &flags)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvmsg_completed -> overlapped result: "
                "\r\n   read:        %d"
                "\r\n   buffer size: %d"
                "\r\n   flags:       %d"
                "\r\n", read, opDataP->data[0].size, flags) );
        if (read == opDataP->data[0].size) {
            completionStatus =
                esaio_completion_recvmsg_done(env, descP,
                                              opEnv, opDataP,
                                              flags);
        } else {
            completionStatus =
                esaio_completion_recvmsg_partial(env, descP,
                                                 opEnv, opDataP,
                                                 reqP, read, flags);
        }
    } else {
        int save_errno = sock_errno();
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_completion_recvmsg_completed -> "
                "overlapped result failure: %d\r\n", save_errno) );
        completionStatus =
            esaio_completion_get_ovl_result_fail(opEnv, descP, save_errno);
    }
    completionInfo = MKT2(opEnv, opDataP->recvRef, completionStatus);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_completed -> "
            "send completion message to %T with"
            "\r\n   CompletionInfo: %T"
            "\r\n", MKPID(env, opCaller), completionInfo) );
    esaio_send_completion_msg(env,
                              descP,
                              opCaller,
                              opEnv,
                              opDataP->sockRef,
                              completionInfo);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_completed -> finalize\r\n") );
    esock_clear_env("esaio_completion_recvmsg_completed -> req cleanup",
                    reqP->env);
    esock_free_env("esaio_completion_recvmsg_completed -> req cleanup",
                   reqP->env);
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_completion_recvmsg_completed -> done\r\n") );
}
static
ERL_NIF_TERM esaio_completion_recvmsg_done(ErlNifEnv*          env,
                                           ESockDescriptor*    descP,
                                           ErlNifEnv*          opEnv,
                                           ESAIOOpDataRecvMsg* opDataP,
                                           DWORD               flags)
{
    ERL_NIF_TERM res, eMsg;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    DWORD        read    = opDataP->data[0].size;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_done(%T) {%d} -> entry with"
            "\r\n   recvRef: %T"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock, recvRef, flags) );
    (void) flags;
    encode_msg(opEnv, descP, read,
               &opDataP->msg,
               opDataP->data,
               &opDataP->ctrl,
               &eMsg);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    res = esock_make_ok2(opEnv, eMsg);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_done(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return res;
}
static
ERL_NIF_TERM esaio_completion_recvmsg_partial(ErlNifEnv*          env,
                                              ESockDescriptor*    descP,
                                              ErlNifEnv*          opEnv,
                                              ESAIOOpDataRecvMsg* opDataP,
                                              ESockRequestor*     reqP,
                                              DWORD               read,
                                              DWORD               flags)
{
    ERL_NIF_TERM res, eMsg;
    ERL_NIF_TERM sockRef = opDataP->sockRef;
    ERL_NIF_TERM recvRef = opDataP->recvRef;
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_partial(%T) {%d} -> entry with"
            "\r\n   recvRef: %T"
            "\r\n   read:    %ld"
            "\r\n   flags:   0x%X"
            "\r\n", sockRef, descP->sock, recvRef, (long) read, flags) );
    (void) flags;
    encode_msg(opEnv, descP, read,
               &opDataP->msg,
               opDataP->data,
               &opDataP->ctrl,
               &eMsg);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_pkg, &descP->readPkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_read_byte, &descP->readByteCnt, read);
    if (read > descP->readPkgMax)
        descP->readPkgMax = read;
    res = esock_make_ok2(opEnv, eMsg);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_recvmsg_partial(%T) {%d} -> done\r\n",
            sockRef, descP->sock) );
    return res;
}
static
void esaio_completion_recvmsg_fail(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   int              error,
                                   BOOLEAN_T        inform)
{
    esaio_completion_fail(env, descP, "recvmsg", error, inform);
}
static
ERL_NIF_TERM esaio_completion_get_ovl_result_fail(ErlNifEnv*       env,
                                                  ESockDescriptor* descP,
                                                  int              error)
{
    ERL_NIF_TERM eerrno = ENO2T(env, error);
    ERL_NIF_TERM reason = MKT2(env, esock_atom_get_overlapped_result, eerrno);
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_completion_get_ovl_result_fail{%d} -> entry with"
            "\r\n   Errno: %d (%T)"
            "\r\n", descP->sock, error, eerrno) );
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.genErrs, 1);
    MUNLOCK(ctrl.cntMtx);
    return esock_make_error(env, reason);
}
static
BOOLEAN_T esaio_completion_unknown(ESAIOThreadData* dataP,
                                   ESockDescriptor* descP,
                                   OVERLAPPED*      ovl,
                                   DWORD            numBytes,
                                   int              error)
{
    (void) dataP;
    (void) descP;
    (void) ovl;
    (void) numBytes;
    (void) error;
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.unknownCmds, 1);
    MUNLOCK(ctrl.cntMtx);
    return FALSE;
}
static
void esaio_completion_fail(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           const char*      opStr,
                           int              error,
                           BOOLEAN_T        inform)
{
    if (inform)
        esock_warning_msg("[WIN-ESAIO] Unknown (%s) operation failure: "
                          "\r\n   Descriptor: %d"
                          "\r\n   Error:      %T"
                          "\r\n",
                          opStr, descP->sock, ENO2T(env, error));
    MLOCK(ctrl.cntMtx);
    esock_cnt_inc(&ctrl.genErrs, 1);
    MUNLOCK(ctrl.cntMtx);
}
static
void esaio_completion_inc(ESAIOThreadData* dataP)
{
    if (dataP->cnt == ESAIO_THREAD_CNT_MAX) {
        dataP->cnt = 0;
    } else {
        dataP->cnt++;
    }
}
extern
void esaio_dtor(ErlNifEnv*       env,
                ESockDescriptor* descP)
{
    ERL_NIF_TERM sockRef;
    SGDBG( ("WIN-ESAIO", "esaio_dtor -> entry\r\n") );
    if (IS_SELECTED(descP)) {
        if (! IS_CLOSED(descP->readState) )
            esock_warning_msg("Socket Read State not CLOSED (0x%X) "
                              "at dtor\r\n", descP->readState);
        if (! IS_CLOSED(descP->writeState) )
            esock_warning_msg("Socket Write State not CLOSED (0x%X) "
                              "at dtor\r\n", descP->writeState);
        if ( descP->sock != INVALID_SOCKET )
            esock_warning_msg("Socket %d still valid\r\n", descP->sock);
        ESOCK_ASSERT( IS_CLOSED(descP->readState) );
        ESOCK_ASSERT( IS_CLOSED(descP->writeState) );
        ESOCK_ASSERT( descP->sock == INVALID_SOCKET );
    } else {
        (void) sock_close(descP->sock);
        descP->sock = INVALID_SOCKET;
    }
    SGDBG( ("WIN-ESAIO", "esaio_dtor -> set state and pattern\r\n") );
    descP->readState  |= (ESOCK_STATE_DTOR | ESOCK_STATE_CLOSED);
    descP->writeState |= (ESOCK_STATE_DTOR | ESOCK_STATE_CLOSED);
    descP->pattern     = (ESOCK_DESC_PATTERN_DTOR | ESOCK_STATE_CLOSED);
    SGDBG( ("WIN-ESAIO",
            "esaio_dtor -> try free readers request queue\r\n") );
    esock_free_request_queue(&descP->readersQ);
    SGDBG( ("WIN-ESAIO",
            "esaio_dtor -> try free writers request queue\r\n") );
    esock_free_request_queue(&descP->writersQ);
    SGDBG( ("WIN-ESAIO",
            "esaio_dtor -> try free acceptors request queue\r\n") );
    esock_free_request_queue(&descP->acceptorsQ);
    esock_free_env("esaio_dtor close env", descP->closeEnv);
    descP->closeEnv = NULL;
    esock_free_env("esaio_dtor meta env", descP->meta.env);
    descP->meta.env = NULL;
    SGDBG( ("WIN-ESAIO", "esaio_dtor -> done\r\n") );
}
extern
void esaio_stop(ErlNifEnv*       env,
                ESockDescriptor* descP)
{
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_stop(%d) -> entry\r\n", descP->sock) );
    if ( !IS_PID_UNDEF(&descP->closerPid) &&
        (descP->closeEnv != NULL) ) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_stop(%d) -> send close msg to %T\r\n",
                descP->sock, MKPID(env, &descP->closerPid)) );
        esock_send_close_msg(env, descP, &descP->closerPid);
        descP->closeEnv = NULL;
        descP->closeRef = esock_atom_undefined;
    } else {
        int err;
        err = esock_close_socket(env, descP, FALSE);
        switch (err) {
        case NO_ERROR:
            break;
        case WSAENOTSOCK:
            if (descP->sock != INVALID_SOCKET)
                esock_warning_msg("[WIN-ESAIO] Attempt to close an "
                                  "already closed socket"
                                  "\r\n(without a closer process): "
                                  "\r\n   Controlling Process: %T"
                                  "\r\n   socket fd:           %d"
                                  "\r\n",
                                  descP->ctrlPid, descP->sock);
            break;
        default:
            esock_warning_msg("[WIN-ESAIO] Failed closing socket without "
                              "closer process: "
                              "\r\n   Controlling Process: %T"
                              "\r\n   socket fd:           %d"
                              "\r\n   Errno:               %T"
                              "\r\n",
                              descP->ctrlPid, descP->sock, ENO2T(env, err));
            break;
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_stop(%d) -> done\r\n", descP->sock) );
}
extern
void esaio_down(ErlNifEnv*           env,
                ESockDescriptor*     descP,
                const ErlNifPid*     pidP,
                const ErlNifMonitor* monP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_down {%d} -> entry with:"
            "\r\n   Pid: %T"
            "\r\n   Mon: %T"
            "\r\n", descP->sock, MKPID(env, pidP), MON2T(env, monP)) );
    if (COMPARE_PIDS(&descP->closerPid, pidP) == 0) {
        enif_set_pid_undefined(&descP->closerPid);
        if (MON_EQ(&descP->closerMon, monP)) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down {%d} -> closer process exit\r\n",
                    descP->sock) );
            MON_INIT(&descP->closerMon);
        } else {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down {%d} -> closer controlling process exit\r\n",
                    descP->sock) );
            ESOCK_ASSERT( MON_EQ(&descP->ctrlMon, monP) );
            MON_INIT(&descP->ctrlMon);
            enif_set_pid_undefined(&descP->ctrlPid);
        }
        if (descP->closeEnv == NULL) {
            int err;
            err = esock_close_socket(env, descP, FALSE);
            if (err != 0)
                esock_warning_msg("[WIN-ESAIO] "
                                  "Failed closing socket for terminating "
                                  "closer process: "
                                  "\r\n   Closer Process: %T"
                                  "\r\n   Descriptor:     %d"
                                  "\r\n   Errno:          %d (%T)"
                                  "\r\n",
                                  MKPID(env, pidP), descP->sock,
                                  err, ENO2T(env, err));
        } else {
            esock_clear_env("esaio_down - close-env", descP->closeEnv);
            esock_free_env("esaio_down - close-env", descP->closeEnv);
            descP->closeEnv = NULL;
            descP->closeRef = esock_atom_undefined;
        }
    } else if (MON_EQ(&descP->ctrlMon, monP)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_down {%d} -> controller process exit\r\n",
                descP->sock) );
        MON_INIT(&descP->ctrlMon);
        enif_set_pid_undefined(&descP->ctrlPid);
        if (IS_OPEN(descP->readState)) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down {%d} -> OPEN => initiate close\r\n",
                    descP->sock) );
            esaio_down_ctrl(env, descP, pidP);
            descP->readState  |= ESOCK_STATE_CLOSING;
            descP->writeState |= ESOCK_STATE_CLOSING;
        } else {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down {%d} -> already closed or closing\r\n",
                    descP->sock) );
        }
    } else if (descP->connectorP != NULL &&
               MON_EQ(&descP->connector.mon, monP)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_down {%d} -> connector process exit\r\n",
                descP->sock) );
        MON_INIT(&descP->connector.mon);
        esock_requestor_release("esaio_down->connector",
                                env, descP, &descP->connector);
        descP->connectorP = NULL;
        descP->writeState &= ~ESOCK_STATE_CONNECTING;
    } else {
        ERL_NIF_TERM sockRef = enif_make_resource(env, descP);
        if (IS_CLOSED(descP->readState)) {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down(%T) {%d} -> stray down: %T\r\n",
                    sockRef, descP->sock, pidP) );
        } else {
            SSDBG( descP,
                   ("WIN-ESAIO",
                    "esaio_down(%T) {%d} -> "
                    "other process - check readers, writers and acceptors\r\n",
                    sockRef, descP->sock) );
            if (descP->readersQ.first != NULL)
                esaio_down_reader(env, descP, sockRef, pidP, monP);
            if (descP->acceptorsQ.first != NULL)
                esaio_down_acceptor(env, descP, sockRef, pidP, monP);
            if (descP->writersQ.first != NULL)
                esaio_down_writer(env, descP, sockRef, pidP, monP);
        }
    }
    SSDBG( descP, ("WIN-ESAIO", "esaio_down {%d} -> done\r\n", descP->sock) );
}
extern
void esaio_down_ctrl(ErlNifEnv*           env,
                     ESockDescriptor*     descP,
                     const ErlNifPid*     pidP)
{
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_down_ctrl {%d} -> entry with"
            "\r\n   Pid: %T"
            "\r\n", descP->sock, MKPID(env, pidP)) );
    if (do_stop(env, descP)) {
        SSDBG( descP,
               ("WIN-ESAIO", "esaio_down_ctrl {%d} -> stop was scheduled\r\n",
                descP->sock) );
    } else {
        int err;
        err = esock_close_socket(env, descP, FALSE);
        if (err != 0)
            esock_warning_msg("[WIN-ESAIO] "
                              "Failed closing socket for terminating "
                              "owner process: "
                              "\r\n   Owner Process:  %T"
                              "\r\n   Descriptor:     %d"
                              "\r\n   Errno:          %d (%T)"
                              "\r\n",
                              MKPID(env, pidP), descP->sock,
                              err, ENO2T(env, err));
    }
    SSDBG( descP,
           ("WIN-ESAIO", "esaio_down_ctrl {%d} -> done\r\n", descP->sock) );
}
static
void esaio_down_acceptor(ErlNifEnv*           env,
                         ESockDescriptor*     descP,
                         ERL_NIF_TERM         sockRef,
                         const ErlNifPid*     pidP,
                         const ErlNifMonitor* monP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_down_acceptor(%T) {%d} -> "
            "maybe unqueue a waiting acceptor\r\n",
            sockRef, descP->sock) );
    esock_acceptor_unqueue(env, descP, NULL, pidP);
}
static
void esaio_down_writer(ErlNifEnv*           env,
                       ESockDescriptor*     descP,
                       ERL_NIF_TERM         sockRef,
                       const ErlNifPid*     pidP,
                       const ErlNifMonitor* monP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_down_writer(%T) {%d} -> maybe unqueue a waiting writer\r\n",
            sockRef, descP->sock) );
    esock_writer_unqueue(env, descP, NULL, pidP);
}
static
void esaio_down_reader(ErlNifEnv*           env,
                       ESockDescriptor*     descP,
                       ERL_NIF_TERM         sockRef,
                       const ErlNifPid*     pidP,
                       const ErlNifMonitor* monP)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "esaio_down_reader(%T) {%d} -> maybe unqueue a waiting reader\r\n",
            sockRef, descP->sock) );
    esock_reader_unqueue(env, descP, NULL, pidP);
}
static
ERL_NIF_TERM send_check_result(ErlNifEnv*       env,
                               ESockDescriptor* descP,
                               ESAIOOperation*  opP,
                               ErlNifPid        caller,
                               int              send_result,
                               size_t           dataSize,
                               BOOLEAN_T        dataInTail,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM*    sendRef,
                               BOOLEAN_T*       cleanup)
{
    ERL_NIF_TERM res;
    BOOLEAN_T    send_error;
    int          err;
    if (send_result == 0) {
        *sendRef = esock_atom_undefined;
        if (!dataInTail) {
            *cleanup = FALSE;
            res = send_check_ok(env, descP, dataSize, sockRef);
        } else {
            descP->writePkgMaxCnt += dataSize;
            ESOCK_CNT_INC(env, descP, sockRef,
                          esock_atom_write_byte, &descP->writeByteCnt,
                          dataSize);
            res = MKT2(env, esock_atom_iov, MKUI64(env, dataSize));
        }
    } else {
        int save_errno = sock_errno();
        if (save_errno == WSA_IO_PENDING) {
            *cleanup = FALSE;
            res = send_check_pending(env, descP, opP, caller,
                                     sockRef, *sendRef);
        } else {
            *cleanup = TRUE;
            res = send_check_fail(env, descP, save_errno, sockRef);
        }
    }
    SSDBG( descP,
           ("WIN-ESAIO",
            "send_check_result(%T) {%d} -> done:"
            "\r\n   res: %T"
            "\r\n", sockRef, descP->sock, res) );
    return res;
}
static
ERL_NIF_TERM send_check_ok(ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           DWORD            written,
                           ERL_NIF_TERM     sockRef)
{
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_pkg, &descP->writePkgCnt, 1);
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_byte, &descP->writeByteCnt, written);
    descP->writePkgMaxCnt += written;
    if (descP->writePkgMaxCnt > descP->writePkgMax)
        descP->writePkgMax = descP->writePkgMaxCnt;
    descP->writePkgMaxCnt = 0;
    SSDBG( descP,
           ("WIN-ESAIO", "send_check_ok(%T) {%d} -> %ld written - done\r\n",
            sockRef, descP->sock, written) );
    return esock_atom_ok;
}
static
ERL_NIF_TERM send_check_pending(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ESAIOOperation*  opP,
                                ErlNifPid        caller,
                                ERL_NIF_TERM     sockRef,
                                ERL_NIF_TERM     sendRef)
{
    SSDBG( descP,
           ("WIN-ESAIO",
            "send_check_pending(%T, %d) -> entry with"
            "\r\n   sendRef: %T"
            "\r\n", sockRef, descP->sock, sendRef) );
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_waits, &descP->writeWaits, 1);
    descP->writeState |= ESOCK_STATE_SELECTED;
    esock_writer_push(env, descP, caller, sendRef, opP);
    return esock_atom_completion;
}
static
ERL_NIF_TERM send_check_fail(ErlNifEnv*       env,
                             ESockDescriptor* descP,
                             int              saveErrno,
                             ERL_NIF_TERM     sockRef)
{
    ERL_NIF_TERM reason;
    ESOCK_CNT_INC(env, descP, sockRef,
                  esock_atom_write_fails, &descP->writeFails, 1);
    reason = ENO2T(env, saveErrno);
    SSDBG( descP,
           ("WIN-ESAIO",
            "send_check_fail(%T, %d) -> error: "
            "\r\n   %d (%T)\r\n",
            sockRef, descP->sock, saveErrno, reason) );
    return esock_make_error(env, reason);
}
static
BOOL get_send_ovl_result(SOCKET      sock,
                         OVERLAPPED* ovl,
                         DWORD*      written)
{
    DWORD flags  = 0;
    BOOL  result = get_ovl_result(sock, ovl, written, &flags);
    (void) flags;
    return result;
}
static
BOOL get_recv_ovl_result(SOCKET      sock,
                         OVERLAPPED* ovl,
                         DWORD*      read,
                         DWORD*      flags)
{
    return get_ovl_result(sock, ovl, read, flags);
}
static
BOOL get_recvmsg_ovl_result(SOCKET      sock,
                            OVERLAPPED* ovl,
                            DWORD*      read)
{
    DWORD flags  = 0;
    BOOL  result = get_ovl_result(sock, ovl, read, &flags);
    (void) flags;
    return result;
}
static
BOOL get_ovl_result(SOCKET      sock,
                    OVERLAPPED* ovl,
                    DWORD*      transfer,
                    DWORD*      flags)
{
    return WSAGetOverlappedResult(sock, ovl, transfer, FALSE, flags);
}
static
int esaio_add_socket(ESockDescriptor* descP)
{
    int    res;
    HANDLE tmp = CreateIoCompletionPort((HANDLE) descP->sock, ctrl.cport,
                                        (ULONG_PTR) descP, 0);
    if (tmp != NULL) {
        res = ESAIO_OK;
    } else {
        res = sock_errno();
    }
    return res;
}
static
void esaio_send_completion_msg(ErlNifEnv*       sendEnv,
                               ESockDescriptor* descP,
                               ErlNifPid*       pid,
                               ErlNifEnv*       msgEnv,
                               ERL_NIF_TERM     sockRef,
                               ERL_NIF_TERM     completionInfo)
{
    ERL_NIF_TERM msg = mk_completion_msg(msgEnv, sockRef, completionInfo);
    if (! esock_send_msg(sendEnv, pid, msg, NULL)) {
        SSDBG( descP,
               ("WIN-ESAIO",
                "esaio_send_completion_msg(%T) {%d} failed ->"
                "\r\n   pid: %T"
                "\r\n",
                sockRef, descP->sock, MKPID(sendEnv, pid)) );
    }
}
static
ERL_NIF_TERM mk_completion_msg(ErlNifEnv*   env,
                               ERL_NIF_TERM sockRef,
                               ERL_NIF_TERM info)
{
    return esock_mk_socket_msg(env, sockRef,
                               esock_atom_completion, info);
}
#endif