#ifndef _ERL_UNIX_FORKER_H
#define _ERL_UNIX_FORKER_H
#include "sys.h"
#ifdef __FreeBSD__
#define FORKER_PROTO_START_ACK 1
#endif
#define FORKER_ARGV_NO_OF_ARGS  3
#define FORKER_ARGV_PROGNAME_IX	0
#define FORKER_ARGV_MAX_FILES	1
#define FORKER_FLAG_USE_STDIO   (1 << 0)
#define FORKER_FLAG_EXIT_STATUS (1 << 1)
#define FORKER_FLAG_DO_READ     (1 << 2)
#define FORKER_FLAG_DO_WRITE    (1 << 3)
#if SIZEOF_VOID_P == SIZEOF_LONG
typedef unsigned long ErtsSysPortId;
#elif SIZEOF_VOID_P == SIZEOF_INT
typedef unsigned int ErtsSysPortId;
#elif SIZEOF_VOID_P == SIZEOF_LONG_LONG
typedef unsigned long long ErtsSysPortId;
#endif
typedef struct ErtsSysForkerProto_ {
    enum {
        ErtsSysForkerProtoAction_Start,
        ErtsSysForkerProtoAction_StartAck,
        ErtsSysForkerProtoAction_Go,
        ErtsSysForkerProtoAction_SigChld,
        ErtsSysForkerProtoAction_Ack
    } action;
    union {
        struct {
            ErtsSysPortId port_id;
            int fds[3];
        } start;
        struct {
            pid_t os_pid;
            int error_number;
        } go;
        struct {
            ErtsSysPortId port_id;
            int error_number;
        } sigchld;
    } u;
} ErtsSysForkerProto;
#endif