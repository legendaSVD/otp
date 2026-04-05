#ifndef SOCKET_DBG_H__
#define SOCKET_DBG_H__
#include "socket_int.h"
#ifdef __WIN32__
#define LLU "%I64u"
#else
#define LLU "%llu"
#endif
typedef unsigned long long llu_t;
extern FILE* esock_dbgout;
#define ESOCK_DBG_PRINTF( ___COND___ , proto ) \
    do                                         \
        if ( ___COND___ ) {                    \
            esock_dbg_printf proto;            \
        }                                      \
    while (0)
extern BOOLEAN_T esock_dbg_init(char* filename);
extern void esock_dbg_printf( const char* prefix, const char* format, ... );
#endif