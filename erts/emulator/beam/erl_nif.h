#ifndef __ERL_NIF_H__
#define __ERL_NIF_H__
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "erl_drv_nif.h"
#define ERL_NIF_MAJOR_VERSION 2
#define ERL_NIF_MINOR_VERSION 18
#define ERL_NIF_MIN_ERTS_VERSION "erts-14.0"
#define ERL_NIF_MIN_REQUIRED_MAJOR_VERSION_ON_LOAD 2
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef ErlNapiUInt64 ErlNifUInt64;
typedef ErlNapiSInt64 ErlNifSInt64;
typedef ErlNapiUInt ErlNifUInt;
typedef ErlNapiSInt ErlNifSInt;
#define ERL_NIF_VM_VARIANT "beam.vanilla"
typedef ErlNifUInt ERL_NIF_TERM;
typedef ERL_NIF_TERM ERL_NIF_UINT;
typedef ErlNifSInt64 ErlNifTime;
#define ERL_NIF_TIME_ERROR ((ErlNifSInt64) ERTS_NAPI_TIME_ERROR__)
typedef enum {
    ERL_NIF_SEC    = ERTS_NAPI_SEC__,
    ERL_NIF_MSEC   = ERTS_NAPI_MSEC__,
    ERL_NIF_USEC   = ERTS_NAPI_USEC__,
    ERL_NIF_NSEC   = ERTS_NAPI_NSEC__
} ErlNifTimeUnit;
struct enif_environment_t;
typedef struct enif_environment_t ErlNifEnv;
typedef struct enif_func_t
{
    const char* name;
    unsigned arity;
    ERL_NIF_TERM (*fptr)(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]);
    unsigned flags;
}ErlNifFunc;
typedef struct enif_entry_t
{
    int major;
    int minor;
    const char* name;
    int num_of_funcs;
    ErlNifFunc* funcs;
    int  (*load)   (ErlNifEnv*, void** priv_data, ERL_NIF_TERM load_info);
    int  (*reload) (ErlNifEnv*, void** priv_data, ERL_NIF_TERM load_info);
    int  (*upgrade)(ErlNifEnv*, void** priv_data, void** old_priv_data, ERL_NIF_TERM load_info);
    void (*unload) (ErlNifEnv*, void* priv_data);
    const char* vm_variant;
    unsigned options;
    size_t sizeof_ErlNifResourceTypeInit;
    const char* min_erts;
}ErlNifEntry;
typedef struct
{
    size_t size;
    unsigned char* data;
    void* ref_bin;
    void* __spare__[2];
}ErlNifBinary;
#if (defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_))
typedef void* ErlNifEvent;
#else
typedef int ErlNifEvent;
#endif
#define ERL_NIF_SELECT_STOP_CALLED    (1 << 0)
#define ERL_NIF_SELECT_STOP_SCHEDULED (1 << 1)
#define ERL_NIF_SELECT_INVALID_EVENT  (1 << 2)
#define ERL_NIF_SELECT_FAILED         (1 << 3)
#define ERL_NIF_SELECT_READ_CANCELLED (1 << 4)
#define ERL_NIF_SELECT_WRITE_CANCELLED (1 << 5)
#define ERL_NIF_SELECT_ERROR_CANCELLED (1 << 6)
#define ERL_NIF_SELECT_NOTSUP          (1 << 7)
typedef enum
{
    ERL_NIF_RT_CREATE = 1,
    ERL_NIF_RT_TAKEOVER = 2
}ErlNifResourceFlags;
typedef enum
{
    ERL_NIF_LATIN1 = 1,
    ERL_NIF_UTF8 = 2,
}ErlNifCharEncoding;
typedef struct
{
    ERL_NIF_TERM pid;
} ErlNifPid;
typedef struct
{
    ERL_NIF_TERM port_id;
}ErlNifPort;
typedef ErlDrvMonitor ErlNifMonitor;
typedef void ErlNifOnHaltCallback(void *priv_data);
typedef void ErlNifOnUnloadThreadCallback(void *priv_data);
typedef struct enif_resource_type_t ErlNifResourceType;
typedef void ErlNifResourceDtor(ErlNifEnv*, void*);
typedef void ErlNifResourceStop(ErlNifEnv*, void*, ErlNifEvent, int is_direct_call);
typedef void ErlNifResourceDown(ErlNifEnv*, void*, ErlNifPid*, ErlNifMonitor*);
typedef void ErlNifResourceDynCall(ErlNifEnv*, void* obj, void* call_data);
typedef struct {
    ErlNifResourceDtor* dtor;
    ErlNifResourceStop* stop;
    ErlNifResourceDown* down;
    int members;
    ErlNifResourceDynCall* dyncall;
} ErlNifResourceTypeInit;
typedef ErlDrvSysInfo ErlNifSysInfo;
typedef struct ErlDrvTid_ *ErlNifTid;
typedef struct ErlDrvMutex_ ErlNifMutex;
typedef struct ErlDrvCond_ ErlNifCond;
typedef struct ErlDrvRWLock_ ErlNifRWLock;
typedef int ErlNifTSDKey;
typedef ErlDrvThreadOpts ErlNifThreadOpts;
typedef enum
{
    ERL_NIF_DIRTY_JOB_CPU_BOUND = ERL_DIRTY_JOB_CPU_BOUND,
    ERL_NIF_DIRTY_JOB_IO_BOUND  = ERL_DIRTY_JOB_IO_BOUND
}ErlNifDirtyTaskFlags;
typedef struct
{
    ERL_NIF_TERM map;
    ERL_NIF_UINT size;
    ERL_NIF_UINT idx;
    union {
        struct {
            ERL_NIF_TERM *ks;
            ERL_NIF_TERM *vs;
        }flat;
        struct {
            struct ErtsDynamicWStack_* wstack;
            ERL_NIF_TERM* kv;
        }hash;
    }u;
    void* __spare__[2];
} ErlNifMapIterator;
typedef enum {
    ERL_NIF_MAP_ITERATOR_FIRST = 1,
    ERL_NIF_MAP_ITERATOR_LAST = 2,
    ERL_NIF_MAP_ITERATOR_HEAD = ERL_NIF_MAP_ITERATOR_FIRST,
    ERL_NIF_MAP_ITERATOR_TAIL = ERL_NIF_MAP_ITERATOR_LAST
} ErlNifMapIteratorEntry;
typedef enum {
    ERL_NIF_UNIQUE_POSITIVE = (1 << 0),
    ERL_NIF_UNIQUE_MONOTONIC = (1 << 1)
} ErlNifUniqueInteger;
typedef enum {
    ERL_NIF_BIN2TERM_SAFE = 0x20000000
} ErlNifBinaryToTerm;
typedef enum {
    ERL_NIF_INTERNAL_HASH = 1,
    ERL_NIF_PHASH2 = 2
} ErlNifHash;
#define ERL_NIF_IOVEC_SIZE 16
typedef struct erl_nif_io_vec {
    int iovcnt;
    size_t size;
    SysIOVec *iov;
    void **ref_bins;
    int flags;
    SysIOVec small_iov[ERL_NIF_IOVEC_SIZE];
    void *small_ref_bin[ERL_NIF_IOVEC_SIZE];
} ErlNifIOVec;
typedef struct erts_io_queue ErlNifIOQueue;
typedef enum {
    ERL_NIF_IOQ_NORMAL = 1
} ErlNifIOQueueOpts;
typedef enum {
    ERL_NIF_TERM_TYPE_ATOM = 1,
    ERL_NIF_TERM_TYPE_BITSTRING = 2,
    ERL_NIF_TERM_TYPE_FLOAT = 3,
    ERL_NIF_TERM_TYPE_FUN = 4,
    ERL_NIF_TERM_TYPE_INTEGER = 5,
    ERL_NIF_TERM_TYPE_LIST = 6,
    ERL_NIF_TERM_TYPE_MAP = 7,
    ERL_NIF_TERM_TYPE_PID = 8,
    ERL_NIF_TERM_TYPE_PORT = 9,
    ERL_NIF_TERM_TYPE_REFERENCE = 10,
    ERL_NIF_TERM_TYPE_TUPLE = 11,
    ERL_NIF_TERM_TYPE__MISSING_DEFAULT_CASE__READ_THE_MANUAL = -1
} ErlNifTermType;
#define ERL_NIF_THR_UNDEFINED 0
#define ERL_NIF_THR_NORMAL_SCHEDULER 1
#define ERL_NIF_THR_DIRTY_CPU_SCHEDULER 2
#define ERL_NIF_THR_DIRTY_IO_SCHEDULER 3
typedef enum {
    ERL_NIF_OPT_DELAY_HALT = 1,
    ERL_NIF_OPT_ON_HALT = 2,
    ERL_NIF_OPT_ON_UNLOAD_THREAD = 3
} ErlNifOption;
#if (defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_))
#  define ERL_NIF_API_FUNC_DECL(RET_TYPE, NAME, ARGS) RET_TYPE (*NAME) ARGS
typedef struct {
#  include "erl_nif_api_funcs.h"
   void* erts_alc_test;
   const void* erts_internal_test_ptr;
} TWinDynNifCallbacks;
extern TWinDynNifCallbacks WinDynNifCallbacks;
#  undef ERL_NIF_API_FUNC_DECL
#endif
#ifdef STATIC_ERLANG_NIF_LIBNAME
#  define STATIC_ERLANG_NIF
#endif
#if (defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_)) && !defined(STATIC_ERLANG_DRIVER) && !defined(STATIC_ERLANG_NIF)
#  define ERL_NIF_API_FUNC_MACRO(NAME) (WinDynNifCallbacks.NAME)
#  include "erl_nif_api_funcs.h"
#else
extern void enif_free(void* ptr);
extern void enif_mutex_destroy(ErlNifMutex *mtx);
extern void enif_cond_destroy(ErlNifCond *cnd);
extern void enif_rwlock_destroy(ErlNifRWLock *rwlck);
extern void enif_thread_opts_destroy(ErlNifThreadOpts *opts);
extern void enif_ioq_destroy(ErlNifIOQueue *q);
#  if (defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_))
#    define ERL_NIF_API_FUNC_DECL(RET_TYPE, NAME, ARGS) extern RET_TYPE NAME ARGS
#  else
#    define ERL_NIF_API_FUNC_DECL(RET_TYPE, NAME, ARGS) ERL_NAPI_EXPORT extern RET_TYPE NAME ARGS
#  endif
#  include "erl_nif_api_funcs.h"
#endif
#if (defined(__WIN32__) || defined(_WIN32) || defined(_WIN32_))
#  define ERL_NIF_INIT_GLOB TWinDynNifCallbacks WinDynNifCallbacks;
#  define ERL_NIF_INIT_ARGS TWinDynNifCallbacks* callbacks
#  define ERL_NIF_INIT_BODY memcpy(&WinDynNifCallbacks,callbacks,sizeof(TWinDynNifCallbacks))
#else
#  define ERL_NIF_INIT_GLOB
#  define ERL_NIF_INIT_ARGS void
#  define ERL_NIF_INIT_BODY
#endif
#ifdef STATIC_ERLANG_NIF
#  ifdef STATIC_ERLANG_NIF_LIBNAME
#    define ERL_NIF_INIT_NAME(MODNAME) ERL_NIF_INIT_NAME2(STATIC_ERLANG_NIF_LIBNAME)
#    define ERL_NIF_INIT_NAME2(LIB) ERL_NIF_INIT_NAME3(LIB)
#    define ERL_NIF_INIT_NAME3(LIB) LIB ## _nif_init
#  else
#    define ERL_NIF_INIT_NAME(MODNAME) MODNAME ## _nif_init
#  endif
#  define ERL_NIF_INIT_DECL(MODNAME) \
          ErlNifEntry* ERL_NIF_INIT_NAME(MODNAME)(ERL_NIF_INIT_ARGS)
#else
#  define ERL_NIF_INIT_DECL(MODNAME) \
          ERL_NAPI_EXPORT ErlNifEntry* nif_init(ERL_NIF_INIT_ARGS)
#endif
#ifdef __cplusplus
}
#  define ERL_NIF_INIT_PROLOGUE extern "C" {
#  define ERL_NIF_INIT_EPILOGUE }
#else
#  define ERL_NIF_INIT_PROLOGUE
#  define ERL_NIF_INIT_EPILOGUE
#endif
#define ERL_NIF_INIT(NAME, FUNCS, LOAD, RELOAD, UPGRADE, UNLOAD) \
ERL_NIF_INIT_PROLOGUE                   \
ERL_NIF_INIT_GLOB                       \
ERL_NIF_INIT_DECL(NAME);		\
ERL_NIF_INIT_DECL(NAME)			\
{					\
    static ErlNifEntry entry = 		\
    {					\
	ERL_NIF_MAJOR_VERSION,		\
	ERL_NIF_MINOR_VERSION,		\
	#NAME,				\
	sizeof(FUNCS) / sizeof(*FUNCS),	\
	FUNCS,				\
	LOAD, RELOAD, UPGRADE, UNLOAD,	\
	ERL_NIF_VM_VARIANT,		\
        1,                              \
        sizeof(ErlNifResourceTypeInit), \
        ERL_NIF_MIN_ERTS_VERSION        \
    };                                  \
    ERL_NIF_INIT_BODY;                  \
    return &entry;			\
}                                       \
ERL_NIF_INIT_EPILOGUE
#if defined(USE_DYNAMIC_TRACE) && (defined(USE_DTRACE) || defined(USE_SYSTEMTAP))
#define HAVE_USE_DTRACE 1
#endif
#ifdef HAVE_USE_DTRACE
ERL_NIF_API_FUNC_DECL(ERL_NIF_TERM,erl_nif_user_trace_s1,(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]));
ERL_NIF_API_FUNC_DECL(ERL_NIF_TERM,erl_nif_user_trace_i4s4,(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]));
ERL_NIF_API_FUNC_DECL(ERL_NIF_TERM,erl_nif_user_trace_n,(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[]));
#endif
#undef ERL_NIF_API_FUNC_DECL
#endif