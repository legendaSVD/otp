#ifndef PRIM_SOCKET_INT_H__
#define PRIM_SOCKET_INT_H__
#include <erl_nif.h>
#include <sys.h>
#include "socket_int.h"
#include "socket_dbg.h"
#define ESOCK_STATE_BOUND        0x0001
#define ESOCK_STATE_LISTENING    0x0002
#define ESOCK_STATE_ACCEPTING    0x0004
#define ESOCK_STATE_CONNECTING   0x0010
#define ESOCK_STATE_CONNECTED    0x0020
#define ESOCK_STATE_SELECTED     0x0100
#define ESOCK_STATE_CLOSING      0x0200
#define ESOCK_STATE_CLOSED       0x0400
#define ESOCK_STATE_DTOR         0x8000
#define IS_BOUND(st)                           \
    (((st) & ESOCK_STATE_BOUND) != 0)
#define IS_CLOSED(st)                           \
    (((st) & ESOCK_STATE_CLOSED) != 0)
#define IS_CLOSING(st)                          \
    (((st) & ESOCK_STATE_CLOSING) != 0)
#define IS_ACCEPTING(st)                        \
    (((st) & ESOCK_STATE_ACCEPTING) != 0)
#define IS_OPEN(st)                                             \
    (((st) & (ESOCK_STATE_CLOSED | ESOCK_STATE_CLOSING)) == 0)
#define IS_SELECTED(d)                                                  \
    ((((d)->readState | (d)->writeState) & ESOCK_STATE_SELECTED) != 0)
#define ESOCK_DESC_PATTERN_CREATED 0x03030303
#define ESOCK_DESC_PATTERN_DTOR    0xC0C0C0C0
#ifdef __WIN32__
#define ESOCK_IS_ERROR(val) ((val) == INVALID_SOCKET)
#else
#define ESOCK_IS_ERROR(val) ((val) < 0)
#endif
#define ESOCK_IDENTITY(c)    c
#define ESOCK_STRINGIFY_1(b) ESOCK_IDENTITY(#b)
#define ESOCK_STRINGIFY(a)   ESOCK_STRINGIFY_1(a)
#define ESOCK_GET_RESOURCE(ENV, REF, RES) \
    enif_get_resource((ENV), (REF), esocks, (RES))
#define ESOCK_MON2TERM(E, M) \
    esock_make_monitor_term((E), (M))
#if ESOCK_COUNTER_SIZE == 16
typedef Uint16                   ESockCounter;
#define ESOCK_COUNTER_MAX        ((ESockCounter) 0xFFFF)
#define MKCNT(ENV, CNT)          MKUI((ENV), (CNT))
#define MKCT(ENV, TAG, CNT)      MKT2((ENV), (TAG), MKCNT((CNT)))
#define ESOCK_COUNTER_FORMAT_STR "%u"
#elif ESOCK_COUNTER_SIZE == 24
typedef Uint32                   ESockCounter;
#define ESOCK_COUNTER_MAX        ((ESockCounter) 0xFFFFFF)
#define MKCNT(ENV, CNT)          MKUI((ENV), (CNT))
#define MKCT(ENV, TAG, CNT)      MKT2((ENV), (TAG), MKCNT((ENV), (CNT)))
#define ESOCK_COUNTER_FORMAT_STR "%lu"
#elif ESOCK_COUNTER_SIZE == 32
typedef Uint32 ESockCounter;
#define ESOCK_COUNTER_MAX        (~((ESockCounter) 0))
#define MKCNT(ENV, CNT)          MKUI((ENV), (CNT))
#define MKCT(ENV, TAG, CNT)      MKT2((ENV), (TAG), MKCNT((ENV), (CNT)))
#define ESOCK_COUNTER_FORMAT_STR "%lu"
#elif ESOCK_COUNTER_SIZE == 48
typedef Uint64                   ESockCounter;
#define ESOCK_COUNTER_MAX        ((ESockCounter) 0xFFFFFFFFFFFF)
#define MKCNT(ENV, CNT)          MKUI64((ENV), (CNT))
#define MKCT(ENV, TAG, CNT)      MKT2((ENV), (TAG), MKCNT((ENV), (CNT)))
#define ESOCK_COUNTER_FORMAT_STR "%llu"
#elif ESOCK_COUNTER_SIZE == 64
typedef Uint64                   ESockCounter;
#define ESOCK_COUNTER_MAX        (~((ESockCounter) 0))
#define MKCNT(ENV, CNT)          MKUI64((ENV), (CNT))
#define MKCT(ENV, TAG, CNT)      MKT2((ENV), (TAG), MKCNT((ENV), (CNT)))
#define ESOCK_COUNTER_FORMAT_STR "%llu"
#else
#error "Invalid counter size"
#endif
#define ESOCK_CNT_INC( __E__, __D__, SF, ACNT, CNT, INC)                \
    do {                                                                \
        if (esock_cnt_inc((CNT), (INC))) {                              \
	  esock_send_wrap_msg((__E__), (__D__), (SF), (ACNT));		\
	}								\
    } while (0)
#define SSDBG( __D__ , proto )    ESOCK_DBG_PRINTF( (__D__)->dbg , proto )
#define SSDBG2( __DBG__ , proto ) ESOCK_DBG_PRINTF( (__DBG__) , proto )
#if defined(HAVE_SENDFILE)
typedef struct {
    ESockCounter cnt;
    ESockCounter byteCnt;
    ESockCounter fails;
    ESockCounter max;
    ESockCounter maxCnt;
    ESockCounter pkg;
    ESockCounter pkgMax;
    ESockCounter tries;
    ESockCounter waits;
} ESockSendfileCounters;
#endif
typedef struct {
    ErlNifMonitor mon;
    BOOLEAN_T     isActive;
} ESockMonitor;
typedef struct {
    ErlNifPid    pid;
    ESockMonitor mon;
    ErlNifEnv*   env;
    ERL_NIF_TERM ref;
    SOCKET       sock;
    void*        dataP;
} ESockRequestor;
typedef struct esock_request_queue_element {
    struct esock_request_queue_element* nextP;
    ESockRequestor                      data;
} ESockRequestQueueElement;
typedef struct {
    ESockRequestQueueElement* first;
    ESockRequestQueueElement* last;
} ESockRequestQueue;
typedef struct{
    ErlNifEnv*   env;
    ERL_NIF_TERM ref;
} ESockMeta;
typedef struct {
    int type;
    BOOLEAN_T (* encode)(ErlNifEnv*     env,
                         unsigned char* data,
                         size_t         dataLen,
                         ERL_NIF_TERM*  eResult);
    BOOLEAN_T (* decode)(ErlNifEnv*      env,
                         ERL_NIF_TERM    eValue,
                         struct cmsghdr* cmsgP,
                         size_t          rem,
                         size_t*         usedP);
    ERL_NIF_TERM *nameP;
} ESockCmsgSpec;
typedef struct {
    int           flag;
    ERL_NIF_TERM* name;
} ESockFlag;
extern const ESockFlag esock_msg_flags[];
extern const int       esock_msg_flags_length;
extern const ESockFlag esock_ioctl_flags[];
extern const int       esock_ioctl_flags_length;
#if defined(HAVE_SCTP)
typedef sctp_assoc_t ESockAssocId;
#else
typedef int ESockAssocId;
#endif
typedef struct {
    BOOLEAN_T    dbg;
    BOOLEAN_T    useReg;
    BOOLEAN_T    eei;
    ErlNifPid    regPid;
    int          iov_max;
    BOOLEAN_T    iow;
    ErlNifMutex* protocolsMtx;
    ErlNifMutex* cntMtx;
    ESockCounter numSockets;
    ESockCounter numTypeStreams;
    ESockCounter numTypeDGrams;
    ESockCounter numTypeSeqPkgs;
    ESockCounter numDomainInet;
    ESockCounter numDomainInet6;
    ESockCounter numDomainLocal;
    ESockCounter numProtoIP;
    ESockCounter numProtoTCP;
    ESockCounter numProtoUDP;
    ESockCounter numProtoSCTP;
    BOOLEAN_T    sockDbg;
} ESockData;
typedef struct {
    Uint32             pattern;
    int                domain;
    int                type;
    int                protocol;
    ErlNifMutex*       writeMtx;
    unsigned int       writeState;
#ifndef __WIN32__
    ESockRequestor     currentWriter;
    ESockRequestor*    currentWriterP;
#endif
    ESockRequestQueue  writersQ;
    ESockCounter       writePkgCnt;
    ESockCounter       writePkgMax;
    ESockCounter       writePkgMaxCnt;
    ESockCounter       writeByteCnt;
    ESockCounter       writeTries;
    ESockCounter       writeWaits;
    ESockCounter       writeFails;
#ifdef HAVE_SENDFILE
    HANDLE                 sendfileHandle;
    ESockSendfileCounters* sendfileCountersP;
#endif
    ESockRequestor     connector;
    ESockRequestor*    connectorP;
    size_t             wCtrlSz;
    ESockMeta          meta;
    ErlNifMutex*       readMtx;
    unsigned int       readState;
#ifndef __WIN32__
    ESockRequestor     currentReader;
    ESockRequestor*    currentReaderP;
    ErlNifBinary       buf;
#endif
    ESockRequestQueue  readersQ;
    ESockCounter       readPkgCnt;
    ESockCounter       readPkgMax;
    ESockCounter       readPkgMaxCnt;
    ESockCounter       readByteCnt;
    ESockCounter       readTries;
    ESockCounter       readWaits;
    ESockCounter       readFails;
#ifndef __WIN32__
    ESockRequestor     currentAcceptor;
    ESockRequestor*    currentAcceptorP;
#endif
    ESockRequestQueue  acceptorsQ;
    ESockCounter       accSuccess;
    ESockCounter       accTries;
    ESockCounter       accWaits;
    ESockCounter       accFails;
    size_t             rBufSz;
#ifndef __WIN32__
    unsigned int       rNum;
    unsigned int       rNumCnt;
#endif
    size_t             rCtrlSz;
    ErlNifPid          closerPid;
    ESockMonitor       closerMon;
    ErlNifEnv*         closeEnv;
    ERL_NIF_TERM       closeRef;
    BOOLEAN_T          iow;
    ErlNifPid          ctrlPid;
    ESockMonitor       ctrlMon;
    SOCKET             sock;
    SOCKET             origFD;
    BOOLEAN_T          closeOnClose;
    BOOLEAN_T          selectRead;
    BOOLEAN_T          dbg;
    BOOLEAN_T          useReg;
#if defined(ESOCK_DESCRIPTOR_FILLER)
    char               filler[1024];
#endif
} ESockDescriptor;
extern char* erl_errno_id(int error);
extern ESockDescriptor* esock_alloc_descriptor(SOCKET sock);
extern void esock_dealloc_descriptor(ErlNifEnv*       env,
                                     ESockDescriptor* descP);
extern BOOLEAN_T esock_open_is_debug(ErlNifEnv*   env,
                                     ERL_NIF_TERM eopts,
                                     BOOLEAN_T    def);
extern BOOLEAN_T esock_open_use_registry(ErlNifEnv*   env,
                                         ERL_NIF_TERM eopts,
                                         BOOLEAN_T    def);
extern BOOLEAN_T esock_open_which_protocol(SOCKET sock, int* proto);
extern BOOLEAN_T esock_getopt_int(SOCKET sock,
                                  int    level,
                                  int    opt,
                                  int*   valP);
extern BOOLEAN_T esock_getopt_uint(SOCKET        sock,
                                   int           level,
                                   int           opt,
                                   unsigned int *valP);
extern void esock_send_reg_add_msg(ErlNifEnv*   env,
                                   ESockDescriptor* descP,
                                   ERL_NIF_TERM sockRef);
extern void esock_send_reg_del_msg(ErlNifEnv*   env,
                                   ESockDescriptor* descP,
                                   ERL_NIF_TERM sockRef);
extern void esock_send_simple_abort_msg(ErlNifEnv*       env,
                                        ESockDescriptor* descP,
                                        ErlNifPid*       pid,
                                        ERL_NIF_TERM     sockRef,
                                        ERL_NIF_TERM     reason);
extern void esock_send_abort_msg(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ERL_NIF_TERM     sockRef,
                                 ESockRequestor*  reqP,
                                 ERL_NIF_TERM     reason);
extern void esock_send_close_msg(ErlNifEnv*       env,
                                 ESockDescriptor* descP,
                                 ErlNifPid*       pid);
extern void esock_send_wrap_msg(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     sockRef,
                                ERL_NIF_TERM     cnt);
extern int esock_monitor(const char*      slogan,
                         ErlNifEnv*       env,
                         ESockDescriptor* descP,
                         const ErlNifPid* pid,
                         ESockMonitor*    mon);
extern int esock_demonitor(const char*      slogan,
                           ErlNifEnv*       env,
                           ESockDescriptor* descP,
                           ESockMonitor*    monP);
extern void esock_monitor_init(ESockMonitor* mon);
extern ERL_NIF_TERM esock_make_monitor_term(ErlNifEnv*          env,
                                            const ESockMonitor* monP);
extern BOOLEAN_T esock_monitor_eq(const ESockMonitor* monP,
                                  const ErlNifMonitor* mon);
#if defined(HAVE_SCTP)
#if defined(SCTP_SNDRCV)
extern BOOLEAN_T esock_cmsg_encode_sctp_sndrcv(ErlNifEnv     *env,
                                               unsigned char *data,
                                               size_t         dataLen,
                                               ERL_NIF_TERM  *eResult);
#endif
#endif
extern BOOLEAN_T esock_cnt_inc(ESockCounter* cnt, ESockCounter inc);
extern void      esock_cnt_dec(ESockCounter* cnt, ESockCounter dec);
extern void      esock_inc_socket(int domain, int type, int protocol);
extern void      esock_dec_socket(int domain, int type, int protocol);
extern int esock_select_read(ErlNifEnv*       env,
                             ErlNifEvent      event,
                             void*            obj,
                             const ErlNifPid* pidP,
                             ERL_NIF_TERM     sockRef,
                             ERL_NIF_TERM     selectRef);
extern int esock_select_write(ErlNifEnv*       env,
                              ErlNifEvent      event,
                              void*            obj,
                              const ErlNifPid* pidP,
                              ERL_NIF_TERM     sockRef,
                              ERL_NIF_TERM     selectRef);
extern int esock_select_stop(ErlNifEnv*  env,
                             ErlNifEvent event,
                             void*       obj);
extern int esock_select_cancel(ErlNifEnv*             env,
                               ErlNifEvent            event,
                               enum ErlNifSelectFlags mode,
                               void*                  obj);
extern ERL_NIF_TERM esock_cancel_write_select(ErlNifEnv*       env,
                                              ESockDescriptor* descP,
                                              ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esock_cancel_read_select(ErlNifEnv*       env,
                                             ESockDescriptor* descP,
                                             ERL_NIF_TERM     opRef);
extern ERL_NIF_TERM esock_cancel_mode_select(ErlNifEnv*       env,
                                             ESockDescriptor* descP,
                                             ERL_NIF_TERM     opRef,
                                             int              smode,
                                             int              rmode);
extern void esock_free_request_queue(ESockRequestQueue* q);
extern BOOLEAN_T esock_requestor_pop(ESockRequestQueue* q,
                                     ESockRequestor*    reqP);
extern void esock_requestor_init(ESockRequestor* reqP);
extern void esock_requestor_release(const char*      slogan,
                                    ErlNifEnv*       env,
                                    ESockDescriptor* descP,
                                    ESockRequestor*  reqP);
#define ACTIVATE_NEXT_FUNCS_DEFS     \
    ACTIVATE_NEXT_FUNC_DEF(acceptor) \
    ACTIVATE_NEXT_FUNC_DEF(writer)   \
    ACTIVATE_NEXT_FUNC_DEF(reader)
#define ACTIVATE_NEXT_FUNC_DEF(F)                                       \
    extern BOOLEAN_T esock_activate_next_##F(ErlNifEnv*       env,      \
                                             ESockDescriptor* descP,    \
                                             ERL_NIF_TERM     sockRef);
ACTIVATE_NEXT_FUNCS_DEFS
#undef ACTIVATE_NEXT_FUNC_DEF
#define ESOCK_OPERATOR_FUNCS_DEFS      \
    ESOCK_OPERATOR_FUNCS_DEF(acceptor) \
    ESOCK_OPERATOR_FUNCS_DEF(writer)   \
    ESOCK_OPERATOR_FUNCS_DEF(reader)
#define ESOCK_OPERATOR_FUNCS_DEF(O)                                    \
    extern BOOLEAN_T esock_##O##_search4pid(ErlNifEnv*       env,      \
                                            ESockDescriptor* descP,    \
                                            ErlNifPid*       pid);     \
    extern void esock_##O##_push(ErlNifEnv*       env,                 \
                                 ESockDescriptor* descP,               \
                                 ErlNifPid        pid,                 \
                                 ERL_NIF_TERM     ref,                 \
                                 void*            dataP);              \
    extern BOOLEAN_T esock_##O##_pop(ErlNifEnv*       env,     \
                                     ESockDescriptor* descP,   \
                                     ESockRequestor*  reqP);   \
    extern BOOLEAN_T esock_##O##_unqueue(ErlNifEnv*       env,          \
                                         ESockDescriptor* descP,        \
                                         ERL_NIF_TERM*    refP,         \
                                         const ErlNifPid* pidP);
ESOCK_OPERATOR_FUNCS_DEFS
#undef ESOCK_OPERATOR_FUNCS_DEF
extern void       esock_clear_env(const char* slogan, ErlNifEnv* env);
extern void       esock_free_env(const char* slogan, ErlNifEnv* env);
extern ErlNifEnv* esock_alloc_env(const char* slogan);
#ifndef __WIN32__
extern void* esock_init_cmsghdr(struct cmsghdr* cmsgP,
                                size_t          rem,
                                size_t          size,
                                size_t*         usedP);
#if defined(IP_TTL) || \
    defined(IPV6_HOPLIMIT) || \
    defined(IPV6_TCLASS) || defined(IPV6_RECVTCLASS)
extern BOOLEAN_T esock_cmsg_decode_int(ErlNifEnv*      env,
                                       ERL_NIF_TERM    eValue,
                                       struct cmsghdr* cmsgP,
                                       size_t          rem,
                                       size_t*         usedP);
#endif
extern BOOLEAN_T esock_cmsg_decode_bool(ErlNifEnv*      env,
                                        ERL_NIF_TERM    eValue,
                                        struct cmsghdr* cmsgP,
                                        size_t          rem,
                                        size_t*         usedP);
extern ESockCmsgSpec* esock_lookup_cmsg_table(int level, size_t *num);
extern ESockCmsgSpec* esock_lookup_cmsg_spec(ESockCmsgSpec* table,
                                             size_t         num,
                                             ERL_NIF_TERM   eType);
extern BOOLEAN_T esock_encode_cmsg(ErlNifEnv*     env,
                                   int            level,
                                   int            type,
                                   unsigned char* dataP,
                                   size_t         dataLen,
                                   ERL_NIF_TERM*  eType,
                                   ERL_NIF_TERM*  eData);
extern void esock_encode_msg_flags(ErlNifEnv*       env,
                                   ESockDescriptor* descP,
                                   int              msgFlags,
                                   ERL_NIF_TERM*    flags);
#endif
extern void esock_stop_handle_current(ErlNifEnv*       env,
                                      const char*      role,
                                      ESockDescriptor* descP,
                                      ERL_NIF_TERM     sockRef,
                                      ESockRequestor*  reqP);
extern void esock_inform_waiting_procs(ErlNifEnv*         env,
                                       const char*        role,
                                       ESockDescriptor*   descP,
                                       ERL_NIF_TERM       sockRef,
                                       ESockRequestQueue* q,
                                       ERL_NIF_TERM       reason);
extern void* esock_init_cmsghdr(struct cmsghdr* cmsgP,
                                size_t          rem,
                                size_t          size,
                                size_t*         usedP);
extern ESockCmsgSpec* esock_lookup_cmsg_table(int level, size_t *num);
extern ESockCmsgSpec* esock_lookup_cmsg_spec(ESockCmsgSpec* table,
                                             size_t         num,
                                             ERL_NIF_TERM   eType);
#ifdef HAVE_SENDFILE
extern ESockSendfileCounters initESockSendfileCounters;
#endif
extern void esock_send_wrap_msg(ErlNifEnv*       env,
                                ESockDescriptor* descP,
                                ERL_NIF_TERM     sockRef,
                                ERL_NIF_TERM     cnt);
extern BOOLEAN_T esock_send_msg(ErlNifEnv*   env,
                                ErlNifPid*   pid,
                                ERL_NIF_TERM msg,
                                ErlNifEnv*   msgEnv);
extern ERL_NIF_TERM esock_mk_socket_msg(ErlNifEnv*   env,
                                        ERL_NIF_TERM sockRef,
                                        ERL_NIF_TERM tag,
                                        ERL_NIF_TERM info);
extern ERL_NIF_TERM esock_mk_socket(ErlNifEnv*   env,
                                    ERL_NIF_TERM sockRef);
#ifdef HAVE_SENDFILE
extern void esock_send_sendfile_deferred_close_msg(ErlNifEnv*       env,
                                                   ESockDescriptor* descP);
#endif
extern int esock_close_socket(ErlNifEnv*       env,
                              ESockDescriptor* descP,
                              BOOLEAN_T        unlock);
extern ERL_NIF_TERM esock_encode_ioctl_ivalue(ErlNifEnv*       env,
                                              ESockDescriptor* descP,
                                              int              ivalue);
extern ERL_NIF_TERM esock_encode_ioctl_bvalue(ErlNifEnv*       env,
                                              ESockDescriptor* descP,
                                              int              bvalue);
#endif