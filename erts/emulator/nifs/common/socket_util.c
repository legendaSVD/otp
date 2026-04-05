#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#ifdef ESOCK_ENABLE
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stddef.h>
#include <sys/types.h>
#if !defined(__WIN32__)
#include <sys/socket.h>
#endif
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#if !defined(__IOS__) && !defined(__WIN32__)
#include <net/if_arp.h>
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
#include <net/if_types.h>
#endif
#include "socket_int.h"
#include "sys.h"
#include "socket_util.h"
#include "socket_dbg.h"
#if defined(HAVE_SCTP_H)
#include <netinet/sctp.h>
#ifndef     HAVE_SCTP
#    define HAVE_SCTP
#endif
#endif
#if defined(COMPILE_DEBUG_FLAG_WE_NEED_TO_CHECK)
#define UTIL_DEBUG TRUE
#else
#define UTIL_DEBUG FALSE
#endif
#ifndef RTLD_NOW
#  define RTLD_NOW 1
#endif
#define DO_UDBG( dbg, proto ) ESOCK_DBG_PRINTF( dbg , proto )
#define UDBG( proto )         DO_UDBG( UTIL_DEBUG , proto )
#define UDBG2( dbg, proto )   DO_UDBG( dbg , proto )
#if defined(__WIN32__)
typedef u_short sa_family_t;
#endif
extern char* erl_errno_id(int error);
#if (defined(HAVE_LOCALTIME_R) && defined(HAVE_STRFTIME))
#define ESOCK_USE_PRETTY_TIMESTAMP 1
#endif
static
BOOLEAN_T esock_decode_sockaddr_native(ErlNifEnv*     env,
                                       ERL_NIF_TERM   eSockAddr,
                                       ESockAddress*  sockAddrP,
                                       int            family,
                                       SOCKLEN_T*     addrLen);
static void esock_encode_packet_addr_tuple(ErlNifEnv*     env,
                                           unsigned char  len,
                                           unsigned char* addr,
                                           ERL_NIF_TERM*  eAddr);
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
static void esock_encode_sockaddr_dl(ErlNifEnv*          env,
                                     struct sockaddr_dl* sockAddrP,
                                     SOCKLEN_T           addrLen,
                                     ERL_NIF_TERM*       eSockAddr);
#endif
static void esock_encode_sockaddr_broken(ErlNifEnv*       env,
                                         struct sockaddr* sa,
                                         SOCKLEN_T        len,
                                         ERL_NIF_TERM*    eSockAddr);
static void make_sockaddr_in(ErlNifEnv*    env,
                             ERL_NIF_TERM  port,
                             ERL_NIF_TERM  addr,
                             ERL_NIF_TERM* sa);
static void make_sockaddr_in6(ErlNifEnv*    env,
                              ERL_NIF_TERM  port,
                              ERL_NIF_TERM  addr,
                              ERL_NIF_TERM  flowInfo,
                              ERL_NIF_TERM  scopeId,
                              ERL_NIF_TERM* sa);
static void make_sockaddr_un(ErlNifEnv*    env,
                             ERL_NIF_TERM  path,
                             ERL_NIF_TERM* sa);
#if defined(HAVE_NETPACKET_PACKET_H)
static void make_sockaddr_ll(ErlNifEnv*    env,
                             ERL_NIF_TERM  proto,
                             ERL_NIF_TERM  ifindex,
                             ERL_NIF_TERM  hatype,
                             ERL_NIF_TERM  pkttype,
                             ERL_NIF_TERM  addr,
                             ERL_NIF_TERM* sa);
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
static void make_sockaddr_dl(ErlNifEnv*    env,
                             ERL_NIF_TERM  index,
                             ERL_NIF_TERM  type,
                             ERL_NIF_TERM  nlen,
                             ERL_NIF_TERM  alen,
                             ERL_NIF_TERM  slen,
                             ERL_NIF_TERM  data,
                             ERL_NIF_TERM* sa);
#endif
#ifdef HAS_AF_LOCAL
static SOCKLEN_T sa_local_length(int l, struct sockaddr_un* sa);
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
static ERL_NIF_TERM esock_encode_if_type(ErlNifEnv*   env,
                                         unsigned int ifType);
#endif
extern
ErlNifMutex* esock_mutex_create(const char* pre, char* buf, SOCKET sock)
{
#if defined(ESOCK_VERBOSE_MTX_NAMES)
    sprintf(buf, "%s[" SOCKET_FORMAT_STR "]", pre, sock);
#else
    VOID(sock);
    sprintf(buf, "%s", pre);
#endif
    return enif_mutex_create(buf);
}
extern
int esock_get_list_length(ErlNifEnv*   env,
                          ERL_NIF_TERM list,
                          int          def)
{
    unsigned int len;
    if (GET_LIST_LEN(env, list, &len))
        return (int) len;
    else
        return def;
}
extern
unsigned int esock_get_uint_from_map(ErlNifEnv*   env,
                                     ERL_NIF_TERM map,
                                     ERL_NIF_TERM key,
                                     unsigned int def)
{
    ERL_NIF_TERM eval;
    unsigned int val;
    if (!GET_MAP_VAL(env, map, key, &eval)) {
        return def;
    } else {
        if (GET_UINT(env, eval, &val))
            return val;
        else
            return def;
    }
}
extern
BOOLEAN_T esock_get_bool_from_map(ErlNifEnv*   env,
                                  ERL_NIF_TERM map,
                                  ERL_NIF_TERM key,
                                  BOOLEAN_T    def)
{
    ERL_NIF_TERM eval;
    if (!GET_MAP_VAL(env, map, key, &eval)) {
        return def;
    } else {
        if (COMPARE(eval, esock_atom_true) == 0)
            return TRUE;
        else if (COMPARE(eval, esock_atom_false) == 0)
            return FALSE;
        else
            return def;
    }
}
extern
BOOLEAN_T esock_get_string_from_map(ErlNifEnv*         env,
                                    ERL_NIF_TERM       map,
                                    ERL_NIF_TERM       key,
                                    ErlNifCharEncoding encoding,
                                    char**             str)
{
    ERL_NIF_TERM eval;
    unsigned int len;
    char*        buf;
    int          written;
    if (!GET_MAP_VAL(env, map, key, &eval)) {
        *str = NULL;
        return TRUE;
    }
    if (!enif_is_list(env, eval)) {
        *str = NULL;
        return FALSE;
    }
    if (!enif_get_string_length(env, eval, &len, encoding)) {
        *str = NULL;
        return FALSE;
    }
    if ((buf = MALLOC(len+1)) == NULL) {
        *str = NULL;
        return FALSE;
    }
    written = enif_get_string(env, eval, buf, len+1, encoding);
    if (written == (len+1)) {
        *str = buf;
        return TRUE;
    } else {
        FREE( buf );
        *str = NULL;
        return FALSE;
    }
}
extern
void esock_encode_iov(ErlNifEnv*    env,
                      ssize_t       read,
                      SysIOVec*     iov,
                      size_t        len,
                      ErlNifBinary* data,
                      ERL_NIF_TERM* eIOV)
{
    ssize_t       rem = read;
    size_t        i;
    ERL_NIF_TERM* a;
    UDBG( ("SUTIL", "esock_encode_iov -> entry with"
           "\r\n   read:      %ld"
           "\r\n   (IOV) len: %lu"
           "\r\n", (long) read, (unsigned long) len) );
    if (len == 0) {
        UDBG( ("SUTIL", "esock_encode_iov -> done when empty\r\n") );
        *eIOV = MKEL(env);
        return;
    } else {
        a = MALLOC(len * sizeof(ERL_NIF_TERM));
    }
    for (i = 0;  i < len;  i++) {
        UDBG( ("SUTIL", "esock_encode_iov -> process iov:"
               "\r\n   iov[%d].iov_len: %d"
               "\r\n   rem:            %d"
               "\r\n", i, iov[i].iov_len, rem) );
        if (iov[i].iov_len == rem) {
            UDBG( ("SUTIL", "esock_encode_iov -> exact => done\r\n") );
            a[i] = MKBIN(env, &data[i]);
            rem  = 0;
            i++;
            break;
        } else if (iov[i].iov_len < rem) {
            UDBG( ("SUTIL", "esock_encode_iov -> filled => continue\r\n") );
            a[i] = MKBIN(env, &data[i]);
            rem -= iov[i].iov_len;
        } else if (iov[i].iov_len > rem) {
            ERL_NIF_TERM tmp;
            UDBG( ("SUTIL", "esock_encode_iov -> split => done\r\n") );
            tmp  = MKBIN(env, &data[i]);
            a[i] = MKSBIN(env, tmp, 0, rem);
            rem  = 0;
            i++;
            break;
        }
    }
    UDBG( ("SUTIL", "esock_encode_iov -> create the IOV list (%d)\r\n", i) );
    *eIOV = MKLA(env, a, i);
    FREE(a);
    UDBG( ("SUTIL", "esock_encode_msghdr -> done\r\n") );
}
extern
BOOLEAN_T esock_decode_iov(ErlNifEnv*    env,
                           ERL_NIF_TERM  eIOV,
                           ErlNifBinary* bufs,
                           SysIOVec*     iov,
                           size_t        len,
                           ssize_t*      totSize)
{
    Uint16       i;
    ssize_t      sz;
    ERL_NIF_TERM elem, tail, list;
    UDBG( ("SUTIL", "esock_decode_iov -> entry with"
           "\r\n   (IOV) len: %d"
           "\r\n", read, len) );
    for (i = 0, list = eIOV, sz = 0; (i < len); i++) {
        UDBG( ("SUTIL", "esock_decode_iov -> "
               "\r\n   iov[%d].iov_len: %d"
               "\r\n   rem:            %d"
               "\r\n", i) );
        ESOCK_ASSERT( GET_LIST_ELEM(env, list, &elem, &tail) );
        if (IS_BIN(env, elem) && GET_BIN(env, elem, &bufs[i])) {
            ssize_t z;
            iov[i].iov_base  = (void*) bufs[i].data;
            iov[i].iov_len   = bufs[i].size;
            z = sz;
            sz += bufs[i].size;
            if (sz < z)
                return FALSE;
        } else {
            return FALSE;
        }
        list = tail;
    }
    *totSize = sz;
    UDBG( ("SUTIL", "esock_decode_iov -> done (%d)\r\n", sz) );
    return TRUE;
}
extern
BOOLEAN_T esock_decode_sockaddr(ErlNifEnv*    env,
                                ERL_NIF_TERM  eSockAddr,
                                ESockAddress* sockAddrP,
                                SOCKLEN_T*    addrLenP)
{
    ERL_NIF_TERM efam;
    int          decode;
    int          fam;
    UDBG( ("SUTIL", "esock_decode_sockaddr -> entry\r\n") );
    if (!IS_MAP(env, eSockAddr))
        return FALSE;
    if (!GET_MAP_VAL(env, eSockAddr, esock_atom_family, &efam))
        return FALSE;
    UDBG( ("SUTIL",
           "esock_decode_sockaddr -> try decode domain (%T)\r\n",
           efam) );
    decode = esock_decode_domain(env, efam, &fam);
    if (0 >= decode) {
        if (0 > decode) {
            UDBG( ("SUTIL", "esock_decode_sockaddr -> native (%d)\r\n",
                   decode) );
            return esock_decode_sockaddr_native(env, eSockAddr, sockAddrP,
                                                fam, addrLenP);
        }
        UDBG( ("SUTIL", "esock_decode_sockaddr -> domain fail\r\n") );
        return FALSE;
    }
    UDBG( ("SUTIL", "esock_decode_sockaddr -> fam: %d\r\n", fam) );
    switch (fam) {
    case AF_INET:
        UDBG( ("SUTIL", "esock_decode_sockaddr -> inet\r\n") );
        return esock_decode_sockaddr_in(env, eSockAddr,
                                        &sockAddrP->in4, addrLenP);
#if defined(HAVE_IN6) && defined(AF_INET6)
    case AF_INET6:
        UDBG( ("SUTIL", "esock_decode_sockaddr -> inet6\r\n") );
        return esock_decode_sockaddr_in6(env, eSockAddr,
                                         &sockAddrP->in6, addrLenP);
#endif
#ifdef HAS_AF_LOCAL
    case AF_LOCAL:
        UDBG( ("SUTIL", "esock_decode_sockaddr -> local\r\n") );
        return esock_decode_sockaddr_un(env, eSockAddr,
                                        &sockAddrP->un, addrLenP);
#endif
#ifdef AF_UNSPEC
    case AF_UNSPEC:
        UDBG( ("SUTIL", "esock_decode_sockaddr -> unspec\r\n") );
        return esock_decode_sockaddr_native(env, eSockAddr, sockAddrP,
                                            AF_UNSPEC, addrLenP);
#endif
    default:
        UDBG( ("SUTIL", "esock_decode_sockaddr -> UNKNOWN\r\n") );
        return FALSE;
    }
}
#if defined(HAVE_SCTP)
extern
BOOLEAN_T esock_decode_sockaddrs(ErlNifEnv*     env,
                                 BOOLEAN_T      dbg,
                                 int            family,
                                 ERL_NIF_TERM   eSockAddrs,
                                 ESockAddress** sockAddrs,
                                 int*           addrCnt)
{
    unsigned int  len;
    if (! GET_LIST_LEN(env, eSockAddrs, &len)) {
        UDBG2( dbg, ("SUTIL",
                     "esock_decode_sockaddrs -> "
                     "failed get (sockaddrs) list length\r\n") );
        *sockAddrs = NULL;
        *addrCnt   = 0;
        return FALSE;
    }
    if (len == 0) {
        UDBG2( dbg, ("SUTIL",
                     "esock_decode_sockaddrs -> "
                     "invalid (sockaddrs) list length (0)\r\n") );
        *sockAddrs = NULL;
        *addrCnt   = 0;
        return FALSE;
    }
    {
        ERL_NIF_TERM  taddrs[len];
        unsigned int  idx;
        ERL_NIF_TERM  esa, elem, tail, efam;
        ESockAddress* addr;
        ESockAddress* addrs;
        SOCKLEN_T     saLen;
        for (idx = 0, esa = eSockAddrs; idx < len; idx++) {
            ESOCK_ASSERT( GET_LIST_ELEM(env, esa, &elem, &tail) );
            if (! GET_MAP_VAL(env, elem, esock_atom_family, &efam)) {
                UDBG2( dbg, ("SUTIL",
                             "esock_decode_sockaddrs -> "
                             "failed get map element (family): "
                             "\r\n   element: %T"
                             "\r\n", elem) );
                *sockAddrs = NULL;
                *addrCnt   = 0;
                return FALSE;
            }
            if (!
                (
                 ((family == AF_INET) && IS_IDENTICAL(efam, esock_atom_inet))
                 ||
                 ((family == AF_INET6) && (IS_IDENTICAL(efam, esock_atom_inet) ||
                                           IS_IDENTICAL(efam, esock_atom_inet6)))
                 )
                ) {
                UDBG2( dbg, ("SUTIL",
                             "esock_decode_sockaddrs -> "
                             "invalid family: %T"
                             "\r\n", efam) );
                *sockAddrs = NULL;
                *addrCnt   = 0;
                return FALSE;
            }
            taddrs[idx] = elem;
            esa = tail;
        }
        addrs = MALLOC(len * sizeof(ESockAddress));
        ESOCK_ASSERT( addrs != NULL );
        for (idx = 0; idx < len; idx++) {
            elem = taddrs[idx];
            addr = &addrs[idx];
            if (!esock_decode_sockaddr(env, elem, addr, &saLen)) {
                UDBG2( dbg, ("SUTIL",
                             "esock_decode_sockaddrs -> "
                             "failed decode sockaddr %d:"
                             "\r\n   %T"
                             "\r\n", idx, elem) );
                FREE( addrs  );
                *sockAddrs = NULL;
                *addrCnt   = 0;
                return FALSE;
            }
            (void) saLen;
        }
        *sockAddrs = addrs;
        *addrCnt   = len;
    }
    UDBG2( dbg, ("SUTIL", "esock_decode_sockaddrs -> done\r\n") );
    return TRUE;
}
#endif
#define SALEN(L, SZ) (((L) > 0) ? (L) : (SZ))
extern
void esock_encode_sockaddr(ErlNifEnv*    env,
                           ESockAddress* sockAddrP,
                           int           addrLen,
                           ERL_NIF_TERM* eSockAddr)
{
  int       family;
  SOCKLEN_T len;
  if ((addrLen > 0) &&
      (addrLen < (char *)&sockAddrP->sa.sa_data - (char *)sockAddrP)) {
      esock_encode_sockaddr_broken(env, &sockAddrP->sa, addrLen, eSockAddr);
      return;
  }
  family = sockAddrP->ss.ss_family;
  UDBG( ("SUTIL", "esock_encode_sockaddr -> entry with"
	 "\r\n   family:  %d"
	 "\r\n   addrLen: %d"
	 "\r\n", family, addrLen) );
  switch (family) {
  case AF_INET:
      len = SALEN(addrLen, sizeof(struct sockaddr_in));
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'inet' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_in(env, &sockAddrP->in4, len, eSockAddr);
      break;
#if defined(HAVE_IN6) && defined(AF_INET6)
  case AF_INET6:
      len = SALEN(addrLen, sizeof(struct sockaddr_in6));
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'inet6' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_in6(env, &sockAddrP->in6, len, eSockAddr);
      break;
#endif
#ifdef HAS_AF_LOCAL
  case AF_LOCAL:
      len = sa_local_length(addrLen, &sockAddrP->un);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'local' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_un(env, &sockAddrP->un, len, eSockAddr);
      break;
#endif
#ifdef AF_UNSPEC
  case AF_UNSPEC:
      len = SALEN(addrLen, 0);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'unspec' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_native(env,
                                   &sockAddrP->sa, len,
                                   esock_atom_unspec,
                                   eSockAddr);
      break;
#endif
#if defined(HAVE_NETPACKET_PACKET_H)
  case AF_PACKET:
      len = SALEN(addrLen, sizeof(struct sockaddr_ll));
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'packet' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_ll(env, &sockAddrP->ll, len, eSockAddr);
      break;
#endif
#if defined(AF_IMPLINK)
  case AF_IMPLINK:
      len = SALEN(addrLen, 0);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'implink' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_native(env,
                                   &sockAddrP->sa, len,
                                   esock_atom_implink,
                                   eSockAddr);
    break;
#endif
#if defined(AF_PUP)
  case AF_PUP:
      len = SALEN(addrLen, 0);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'pup' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_native(env,
                                   &sockAddrP->sa, len,
                                   esock_atom_pup,
                                   eSockAddr);
      break;
#endif
#if defined(AF_CHAOS)
  case AF_CHAOS:
      len = SALEN(addrLen, 0);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'chaos' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_native(env,
                                   &sockAddrP->sa, len,
                                   esock_atom_chaos,
                                   eSockAddr);
      break;
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
  case AF_LINK:
#if defined(ESOCK_SDL_LEN)
      len = SALEN(addrLen, sockAddrP->dl.sdl_len);
#else
      len = SALEN(addrLen,
                  (CHARP(sockAddrP->dl.sdl_data) - CHARP(sockAddrP)) +
                  sockAddrP->dl.sdl_nlen + sockAddrP->dl.sdl_alen);
#endif
      UDBG( ("SUTIL", "esock_encode_sockaddr -> 'link' family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_dl(env, &sockAddrP->dl, len, eSockAddr);
    break;
#endif
  default:
      len = SALEN(addrLen, 0);
      UDBG( ("SUTIL", "esock_encode_sockaddr -> default (native) family addr: "
             "\r\n   len: %d"
             "\r\n", len) );
      esock_encode_sockaddr_native(env,
                                   &sockAddrP->sa, len,
                                   MKI(env, family),
                                   eSockAddr);
      break;
  }
}
#ifdef HAS_AF_LOCAL
static
SOCKLEN_T sa_local_length(int l, struct sockaddr_un* sa)
{
    if (l > 0) {
        return ((SOCKLEN_T) l);
    } else {
#if defined(SUN_LEN)
    return SUN_LEN(sa);
#else
    return (offsetof(struct sockaddr_un, sun_path) + strlen(sa->sun_path) + 1);
#endif
    }
}
#endif
extern
void esock_encode_hwsockaddr(ErlNifEnv*       env,
			     struct sockaddr* sockAddrP,
			     SOCKLEN_T        addrLen,
			     ERL_NIF_TERM*    eSockAddr)
{
    ERL_NIF_TERM efamily;
    int          family;
    if (addrLen < (char *)&sockAddrP->sa_data - (char *)sockAddrP) {
        esock_encode_sockaddr_broken(env, sockAddrP, addrLen, eSockAddr);
        return;
    }
    family = sockAddrP->sa_family;
    UDBG( ("SUTIL", "esock_encode_hwsockaddr -> entry with"
           "\r\n   family:  %d"
           "\r\n   addrLen: %d"
           "\r\n", family, addrLen) );
    switch (family) {
#if defined(ARPHRD_NETROM)
    case ARPHRD_NETROM:
        efamily = esock_atom_netrom;
        break;
#endif
#if defined(ARPHRD_ETHER)
    case ARPHRD_ETHER:
        efamily = esock_atom_ether;
        break;
#endif
#if defined(ARPHRD_IEEE802)
    case ARPHRD_IEEE802:
        efamily = esock_atom_ieee802;
        break;
#endif
#if defined(ARPHRD_DLCI)
    case ARPHRD_DLCI:
        efamily = esock_atom_dlci;
        break;
#endif
#if defined(ARPHRD_FRELAY)
    case ARPHRD_FRELAY:
        efamily = esock_atom_frelay;
        break;
#endif
#if defined(ARPHRD_IEEE1394)
    case ARPHRD_IEEE1394:
        efamily = esock_atom_ieee1394;
        break;
#endif
#if defined(ARPHRD_LOOPBACK)
    case ARPHRD_LOOPBACK:
        efamily = esock_atom_loopback;
        break;
#endif
#if defined(ARPHRD_RAWIP)
    case ARPHRD_RAWIP:
        efamily = esock_atom_rawip;
        break;
#endif
#if defined(ARPHRD_NONE)
    case ARPHRD_NONE:
        efamily = esock_atom_none;
        break;
#endif
    default:
        efamily = MKI(env, family);
        break;
    }
    esock_encode_sockaddr_native(env, sockAddrP, addrLen, efamily, eSockAddr);
}
extern
BOOLEAN_T esock_decode_hwsockaddr(ErlNifEnv*    env,
                                  ERL_NIF_TERM  eSockAddr,
                                  ESockAddress* sockAddrP,
                                  SOCKLEN_T*    addrLen)
{
    ERL_NIF_TERM efamily;
    int          family = -1;
    if (!IS_MAP(env, eSockAddr))
        return FALSE;
    if (!GET_MAP_VAL(env, eSockAddr, esock_atom_family, &efamily))
        return FALSE;
#if defined(ARPHRD_NETROM)
    if (IS_IDENTICAL(efamily, esock_atom_netrom)) {
        family = ARPHRD_NETROM;
    }
#endif
#if defined(ARPHRD_ETHER)
    if (IS_IDENTICAL(efamily, esock_atom_ether)) {
        family = ARPHRD_ETHER;
    }
#endif
#if defined(ARPHRD_IEEE802)
    if (IS_IDENTICAL(efamily, esock_atom_ieee802)) {
        family = ARPHRD_IEEE802;
    }
#endif
#if defined(ARPHRD_DLCI)
    if (IS_IDENTICAL(efamily, esock_atom_dlci)) {
        family = ARPHRD_DLCI;
    }
#endif
#if defined(ARPHRD_FRELAY)
    if (IS_IDENTICAL(efamily, esock_atom_frelay)) {
        family = ARPHRD_FRELAY;
    }
#endif
#if defined(ARPHRD_IEEE1394)
    if (IS_IDENTICAL(efamily, esock_atom_ieee1394)) {
        family = ARPHRD_IEEE1394;
    }
#endif
#if defined(ARPHRD_LOOPBACK)
    if (IS_IDENTICAL(efamily, esock_atom_loopback)) {
        family = ARPHRD_LOOPBACK;
    }
#endif
#if defined(ARPHRD_RAWIP)
    if (IS_IDENTICAL(efamily, esock_atom_rawip)) {
        family = ARPHRD_RAWIP;
    }
#endif
#if defined(ARPHRD_NONE)
    if (IS_IDENTICAL(efamily, esock_atom_none)) {
        family = ARPHRD_NONE;
    }
#endif
    if (family == -1)
	return FALSE;
    return esock_decode_sockaddr_native(env, eSockAddr,
                                        sockAddrP, family, addrLen);
}
extern
BOOLEAN_T esock_decode_sockaddr_in(ErlNifEnv*          env,
                                   ERL_NIF_TERM        eSockAddr,
                                   struct sockaddr_in* sockAddrP,
                                   SOCKLEN_T*          addrLen)
{
    ERL_NIF_TERM eport, eaddr;
    int          port;
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> entry\r\n") );
    sys_memzero((char*) sockAddrP, sizeof(struct sockaddr_in));
#ifndef NO_SA_LEN
    sockAddrP->sin_len = sizeof(struct sockaddr_in);
#endif
    sockAddrP->sin_family = AF_INET;
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> try get port number\r\n") );
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_port, &eport))
        return FALSE;
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> try decode port number\r\n") );
    if (! GET_INT(env, eport, &port))
        return FALSE;
    sockAddrP->sin_port = htons(port);
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> try get (ip) address\r\n") );
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_addr, &eaddr))
        return FALSE;
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> try decode (ip) address\r\n") );
    if (! esock_decode_in_addr(env,
                               eaddr,
                               &sockAddrP->sin_addr))
        return FALSE;
    *addrLen = sizeof(struct sockaddr_in);
    UDBG( ("SUTIL", "esock_decode_sockaddr_in -> done\r\n") );
    return TRUE;
}
extern
void esock_encode_sockaddr_in(ErlNifEnv*          env,
                              struct sockaddr_in* sockAddrP,
                              SOCKLEN_T           addrLen,
                              ERL_NIF_TERM*       eSockAddr)
{
    ERL_NIF_TERM ePort, eAddr;
    int          port;
    SOCKLEN_T    minSz = sizeof(struct sockaddr_in) -
        sizeof(sockAddrP->sin_zero);
    UDBG( ("SUTIL", "esock_encode_sockaddr_in -> entry with"
           "\r\n   addrLen:           %d"
           "\r\n   required min size: %d"
           "\r\n", addrLen, minSz) );
    if (addrLen >= minSz) {
        port  = ntohs(sockAddrP->sin_port);
        ePort = MKI(env, port);
        esock_encode_in_addr(env, &sockAddrP->sin_addr, &eAddr);
        make_sockaddr_in(env, ePort, eAddr, eSockAddr);
    } else {
        UDBG( ("SUTIL", "esock_encode_sockaddr_in -> wrong size: "
               "\r\n   addrLen:   %d"
               "\r\n   addr size: %d"
               "\r\n", addrLen, minSz) );
        esock_encode_sockaddr_native(env, (struct sockaddr *)sockAddrP,
                                     addrLen, esock_atom_inet, eSockAddr);
    }
}
#if defined(HAVE_IN6) && defined(AF_INET6)
extern
BOOLEAN_T esock_decode_sockaddr_in6(ErlNifEnv*           env,
                                    ERL_NIF_TERM         eSockAddr,
                                    struct sockaddr_in6* sockAddrP,
                                    SOCKLEN_T*           addrLen)
{
    ERL_NIF_TERM eport, eaddr, eflowInfo, escopeId;
    int          port;
    unsigned int flowInfo, scopeId;
    UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> entry\r\n") );
    sys_memzero((char*) sockAddrP, sizeof(struct sockaddr_in6));
#ifndef NO_SA_LEN
    sockAddrP->sin6_len = sizeof(struct sockaddr_in);
#endif
    sockAddrP->sin6_family = AF_INET6;
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_port, &eport)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed extract port number\r\n") );
        return FALSE;
    }
    if (! GET_INT(env, eport, &port)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed decode port number (%T)\r\n", eport) );
        return FALSE;
    }
    UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> port: %d\r\n", port) );
    sockAddrP->sin6_port = htons(port);
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_flowinfo, &eflowInfo)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed extract flowinfo\r\n") );
        return FALSE;
    }
    if (! GET_UINT(env, eflowInfo, &flowInfo)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed decode flowinfo (%T)\r\n", eport) );
        return FALSE;
    }
    UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> flowinfo: %d\r\n", flowInfo) );
    sockAddrP->sin6_flowinfo = flowInfo;
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_scope_id, &escopeId)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed extract scope_id\r\n") );
        return FALSE;
    }
    if (! GET_UINT(env, escopeId, &scopeId)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed decode scope_id (%T)\r\n", escopeId) );
        return FALSE;
    }
    UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> scopeId: %d\r\n", scopeId) );
    sockAddrP->sin6_scope_id = scopeId;
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_addr, &eaddr)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed extract address\r\n") );
        return FALSE;
    }
    if (!esock_decode_in6_addr(env,
                               eaddr,
                               &sockAddrP->sin6_addr)) {
        UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> failed decode address (%T))\r\n", eaddr) );
        return FALSE;
    }
    *addrLen = sizeof(struct sockaddr_in6);
    UDBG( ("SUTIL", "esock_decode_sockaddr_in6 -> done\r\n") );
    return TRUE;
}
#endif
#if defined(HAVE_IN6) && defined(AF_INET6)
extern
void esock_encode_sockaddr_in6(ErlNifEnv*            env,
                                struct sockaddr_in6* sockAddrP,
                                SOCKLEN_T            addrLen,
                                ERL_NIF_TERM*        eSockAddr)
{
    ERL_NIF_TERM ePort, eAddr, eFlowInfo, eScopeId;
    if (addrLen >= sizeof(struct sockaddr_in6)) {
        ePort = MKI(env, ntohs(sockAddrP->sin6_port));
        eFlowInfo = MKI(env, sockAddrP->sin6_flowinfo);
        eScopeId = MKI(env, sockAddrP->sin6_scope_id);
        esock_encode_in6_addr(env, &sockAddrP->sin6_addr, &eAddr);
        make_sockaddr_in6(env, ePort, eAddr,
                          eFlowInfo, eScopeId, eSockAddr);
    } else {
        esock_encode_sockaddr_native(env, (struct sockaddr *)sockAddrP,
                                     addrLen, esock_atom_inet6, eSockAddr);
    }
}
#endif
#ifdef HAS_AF_LOCAL
extern
BOOLEAN_T esock_decode_sockaddr_un(ErlNifEnv*          env,
                                   ERL_NIF_TERM        eSockAddr,
                                   struct sockaddr_un* sockAddrP,
                                   SOCKLEN_T*          addrLen)
{
    ErlNifBinary bin;
    ERL_NIF_TERM epath;
    SOCKLEN_T    len;
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_path, &epath))
        return FALSE;
    if (! GET_BIN(env, epath, &bin))
        return FALSE;
    if ((bin.size +
#ifdef __linux__
         (bin.data[0] == '\0' ? 0 : 1)
#else
         1
#endif
         ) > sizeof(sockAddrP->sun_path))
        return FALSE;
    sys_memzero((char*) sockAddrP, sizeof(struct sockaddr_un));
    sockAddrP->sun_family = AF_LOCAL;
    sys_memcpy(sockAddrP->sun_path, bin.data, bin.size);
    len = (sockAddrP->sun_path - (char *)sockAddrP) + bin.size;
#ifndef NO_SA_LEN
    sockAddrP->sun_len = len;
#endif
    *addrLen = len;
    return TRUE;
}
#endif
#ifdef HAS_AF_LOCAL
extern
void esock_encode_sockaddr_un(ErlNifEnv*          env,
                              struct sockaddr_un* sockAddrP,
                              SOCKLEN_T           addrLen,
                              ERL_NIF_TERM*       eSockAddr)
{
    ERL_NIF_TERM ePath;
    size_t       n, m;
    UDBG( ("SUTIL", "esock_encode_sockaddr_un -> entry with"
           "\r\n   addrLen: %d"
           "\r\n", addrLen) );
    n = sockAddrP->sun_path - (char *)sockAddrP;
    if (addrLen >= n) {
        n = addrLen - n;
        if (255 < n) {
            *eSockAddr = esock_atom_bad_data;
        } else {
            unsigned char *path;
            m = esock_strnlen(sockAddrP->sun_path, n);
#ifdef __linux__
            if (m == 0) {
                m = n;
            }
#endif
            UDBG( ("SUTIL", "esock_encode_sockaddr_un -> m: %d\r\n", m) );
            path = enif_make_new_binary(env, m, &ePath);
            ESOCK_ASSERT( path != NULL );
            sys_memcpy(path, sockAddrP->sun_path, m);
            make_sockaddr_un(env, ePath, eSockAddr);
        }
    } else {
        esock_encode_sockaddr_native(env, (struct sockaddr *)sockAddrP,
                                     addrLen, esock_atom_local, eSockAddr);
    }
}
#endif
#if defined(HAVE_NETPACKET_PACKET_H)
extern
void esock_encode_sockaddr_ll(ErlNifEnv*          env,
                              struct sockaddr_ll* sockAddrP,
                              SOCKLEN_T           addrLen,
                              ERL_NIF_TERM*       eSockAddr)
{
    ERL_NIF_TERM eProto, eIfIdx, eHaType, ePktType, eAddr;
    UDBG( ("SUTIL", "esock_encode_sockaddr_ll -> entry with"
           "\r\n.  addrLen: %d"
           "\r\n", addrLen) );
    if (addrLen >= sizeof(struct sockaddr_ll)) {
        esock_encode_packet_protocol(env, ntohs(sockAddrP->sll_protocol),
                                     &eProto);
        eIfIdx = MKI(env, sockAddrP->sll_ifindex);
        esock_encode_packet_hatype(env, sockAddrP->sll_hatype, &eHaType);
        esock_encode_packet_pkttype(env, sockAddrP->sll_pkttype, &ePktType);
        esock_encode_packet_addr(env,
                                 sockAddrP->sll_halen, sockAddrP->sll_addr,
                                 &eAddr);
        make_sockaddr_ll(env,
                         eProto, eIfIdx, eHaType, ePktType, eAddr,
                         eSockAddr);
    } else {
        esock_encode_sockaddr_native(env, (struct sockaddr *)sockAddrP,
                                     addrLen, esock_atom_packet, eSockAddr);
    }
}
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
extern
void esock_encode_sockaddr_dl(ErlNifEnv*          env,
                              struct sockaddr_dl* sockAddrP,
                              SOCKLEN_T           addrLen,
                              ERL_NIF_TERM*       eSockAddr)
{
    ERL_NIF_TERM eindex, etype, enlen, ealen, eslen, edata;
    SOCKLEN_T    dlen;
    SOCKLEN_T    ndsz = sizeof(struct sockaddr_dl)-sizeof(sockAddrP->sdl_data);
    UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> entry with"
           "\r\n   addrLen:              %d"
           "\r\n   non-data fields size: %d"
           "\r\n", addrLen, ndsz) );
    if (addrLen >= ndsz) {
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> index: %d"
               "\r\n", sockAddrP->sdl_index) );
        eindex = MKUI(env, sockAddrP->sdl_index);
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> type: %d"
               "\r\n", sockAddrP->sdl_type) );
        etype = esock_encode_if_type(env, sockAddrP->sdl_type);
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> nlen: %d"
               "\r\n", sockAddrP->sdl_nlen) );
        enlen = MKUI(env, sockAddrP->sdl_nlen);
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> alen: %d"
               "\r\n", sockAddrP->sdl_alen) );
        ealen = MKUI(env, sockAddrP->sdl_alen);
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> slen: %d"
               "\r\n", sockAddrP->sdl_slen) );
        eslen = MKUI(env, sockAddrP->sdl_slen);
        dlen  = addrLen - (CHARP(sockAddrP->sdl_data) - CHARP(sockAddrP));
        UDBG( ("SUTIL", "esock_encode_sockaddr_dl -> data len: %d"
               "\r\n", dlen) );
        edata = esock_make_new_binary(env, &sockAddrP->sdl_data, dlen);
        make_sockaddr_dl(env,
                         eindex, etype, enlen, ealen, eslen, edata,
                         eSockAddr);
    } else {
        esock_encode_sockaddr_native(env, (struct sockaddr *)sockAddrP,
                                     addrLen, esock_atom_link, eSockAddr);
    }
}
static
ERL_NIF_TERM esock_encode_if_type(ErlNifEnv*   env,
                                  unsigned int ifType)
{
    ERL_NIF_TERM eIfType;
    switch (ifType) {
#if defined(IFT_OTHER)
    case IFT_OTHER:
        eIfType = esock_atom_other;
        break;
#endif
#if defined(IFT_HDH1822)
    case IFT_HDH1822:
        eIfType = esock_atom_hdh1822;
        break;
#endif
#if defined(IFT_X25DDN)
    case IFT_X25DDN:
        eIfType = esock_atom_x25ddn;
        break;
#endif
#if defined(IFT_X25)
    case IFT_X25:
        eIfType = esock_atom_x25;
        break;
#endif
#if defined(IFT_ETHER)
    case IFT_ETHER:
        eIfType = esock_atom_ether;
        break;
#endif
#if defined(IFT_PPP)
    case IFT_PPP:
        eIfType = esock_atom_ppp;
        break;
#endif
#if defined(IFT_LOOP)
    case IFT_LOOP:
        eIfType = esock_atom_loop;
        break;
#endif
#if defined(IFT_IPV4)
    case IFT_IPV4:
        eIfType = esock_atom_ipv4;
        break;
#endif
#if defined(IFT_IPV6)
    case IFT_IPV6:
        eIfType = esock_atom_ipv6;
        break;
#endif
#if defined(IFT_6TO4)
    case IFT_6TO4:
        eIfType = esock_atom_6to4;
        break;
#endif
#if defined(IFT_GIF)
    case IFT_GIF:
        eIfType = esock_atom_gif;
        break;
#endif
#if defined(IFT_FAITH)
    case IFT_FAITH:
        eIfType = esock_atom_faith;
        break;
#endif
#if defined(IFT_STF)
    case IFT_STF:
        eIfType = esock_atom_stf;
        break;
#endif
#if defined(IFT_BRIDGE)
    case IFT_BRIDGE:
        eIfType = esock_atom_bridge;
        break;
#endif
#if defined(IFT_CELLULAR)
    case IFT_CELLULAR:
        eIfType = esock_atom_cellular;
        break;
#endif
    default:
        eIfType = MKUI(env, ifType);
        break;
    }
    return eIfType;
}
#endif
extern
BOOLEAN_T esock_decode_in_addr(ErlNifEnv*      env,
                               ERL_NIF_TERM    eAddr,
                               struct in_addr* inAddrP)
{
    struct in_addr addr;
    UDBG( ("SUTIL", "esock_decode_in_addr -> entry with"
           "\r\n   eAddr: %T"
           "\r\n", eAddr) );
    if (IS_ATOM(env, eAddr)) {
        if (COMPARE(esock_atom_loopback, eAddr) == 0) {
            UDBG( ("SUTIL",
		   "esock_decode_in_addr -> address: loopback\r\n") );
            addr.s_addr = htonl(INADDR_LOOPBACK);
        } else if (COMPARE(esock_atom_any, eAddr) == 0) {
            UDBG( ("SUTIL",
                   "esock_decode_in_addr -> address: any\r\n") );
            addr.s_addr = htonl(INADDR_ANY);
        } else if (COMPARE(esock_atom_broadcast, eAddr) == 0) {
            UDBG( ("SUTIL",
                   "esock_decode_in_addr -> address: broadcast\r\n") );
            addr.s_addr = htonl(INADDR_BROADCAST);
        } else {
            UDBG( ("SUTIL",
		   "esock_decode_in_addr -> address: unknown\r\n") );
            return FALSE;
        }
        inAddrP->s_addr = addr.s_addr;
    } else {
        const ERL_NIF_TERM* addrt;
        int                 addrtSz;
        int                 a, v;
        char                addr[4];
        if (! GET_TUPLE(env, eAddr, &addrtSz, &addrt))
            return FALSE;
        if (addrtSz != 4)
            return FALSE;
        for (a = 0; a < 4; a++) {
            if (! GET_INT(env, addrt[a], &v))
                return FALSE;
            if (v < 0 || 255 < v)
                return FALSE;
            addr[a] = v;
        }
        sys_memcpy(inAddrP, &addr, sizeof(addr));
    }
    return TRUE;
}
extern
void esock_encode_in_addr(ErlNifEnv*      env,
                          struct in_addr* addrP,
                          ERL_NIF_TERM*   eAddr)
{
    size_t         i;
    ERL_NIF_TERM   at[4];
    size_t         atLen = NUM(at);
    unsigned char* a     = (unsigned char*) addrP;
    ERL_NIF_TERM   addr;
    for (i = 0; i < atLen; i++) {
        at[i] = MKI(env, a[i]);
    }
    addr = MKTA(env, at, atLen);
    UDBG( ("SUTIL", "esock_encode_in_addr -> addr: %T\r\n", addr) );
    *eAddr = addr;
}
#if defined(HAVE_IN6) && defined(AF_INET6)
extern
BOOLEAN_T esock_decode_in6_addr(ErlNifEnv*       env,
                                ERL_NIF_TERM     eAddr,
                                struct in6_addr* inAddrP)
{
    UDBG( ("SUTIL", "esock_decode_in6_addr -> entry with"
           "\r\n   eAddr: %T"
           "\r\n", eAddr) );
    if (IS_ATOM(env, eAddr)) {
        UDBG( ("SUTIL", "esock_decode_in6_addr -> atom\r\n") );
        if (COMPARE(esock_atom_loopback, eAddr) == 0) {
            *inAddrP = in6addr_loopback;
        } else if (COMPARE(esock_atom_any, eAddr) == 0) {
            *inAddrP = in6addr_any;
        } else {
            return FALSE;
        }
    } else {
        const ERL_NIF_TERM* tuple;
        int                 arity;
        size_t              n;
        struct in6_addr     sa;
        #ifndef VALGRIND
        sys_memzero(&sa, sizeof(sa));
        #endif
        UDBG( ("SUTIL", "esock_decode_in6_addr -> tuple\r\n") );
        if (! GET_TUPLE(env, eAddr, &arity, &tuple))
            return FALSE;
        UDBG( ("SUTIL", "esock_decode_in6_addr -> arity: %d\r\n", arity) );
        n = arity << 1;
        if (n != sizeof(sa.s6_addr)) {
            UDBG( ("SUTIL",
                   "esock_decode_in6_addr -> invalid size (%d /= %d)\r\n",
                   n, sizeof(sa.s6_addr)) );
            return FALSE;
        }
        for (n = 0;  n < arity;  n++) {
            int v;
            UDBG( ("SUTIL", "esock_decode_in6_addr -> try get element %d\r\n",
                   n) );
            if (! GET_INT(env, tuple[n], &v) ||
                v < 0 || 65535 < v) {
                UDBG( ("SUTIL",
                       "esock_decode_in6_addr -> failed get part %d\r\n", n) );
                return FALSE;
            }
            put_int16(v, sa.s6_addr + (n << 1));
        }
        *inAddrP = sa;
    }
    UDBG( ("SUTIL", "esock_decode_in6_addr -> (success) done\r\n") );
    return TRUE;
}
#endif
#if defined(HAVE_IN6) && defined(AF_INET6)
extern
void esock_encode_in6_addr(ErlNifEnv*       env,
                           struct in6_addr* addrP,
                           ERL_NIF_TERM*    eAddr)
{
    size_t         i;
    ERL_NIF_TERM   at[8];
    size_t         atLen = NUM(at);
    unsigned char* a     = UCHARP(addrP->s6_addr);
    for (i = 0; i < atLen; i++) {
        at[i] = MKI(env, get_int16(a + i*2));
    }
    *eAddr = MKTA(env, at, atLen);
}
#endif
extern
void esock_encode_timeval(ErlNifEnv*      env,
                           struct timeval* timeP,
                           ERL_NIF_TERM*   eTime)
{
    ERL_NIF_TERM keys[]  = {esock_atom_sec, esock_atom_usec};
    ERL_NIF_TERM vals[]  = {MKL(env, timeP->tv_sec), MKL(env, timeP->tv_usec)};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, eTime) );
}
extern
BOOLEAN_T esock_decode_timeval(ErlNifEnv*      env,
                               ERL_NIF_TERM    eTime,
                               struct timeval* timeP)
{
    ERL_NIF_TERM eSec, eUSec;
    if (! GET_MAP_VAL(env, eTime, esock_atom_sec, &eSec))
        return FALSE;
    if (! GET_MAP_VAL(env, eTime, esock_atom_usec, &eUSec))
        return FALSE;
    {
#if (SIZEOF_TIME_T == 8)
        ErlNifSInt64 sec;
        if (! GET_INT64(env, eSec, &sec))
            return FALSE;
#elif (SIZEOF_TIME_T == SIZEOF_INT)
        int sec;
        if (! GET_INT(env, eSec, &sec))
            return FALSE;
#else
        long sec;
        if (! GET_LONG(env, eSec, &sec))
            return FALSE;
#endif
        timeP->tv_sec = sec;
    }
    {
#if (SIZEOF_SUSECONDS_T == 8)
        ErlNifSInt64 usec;
        if (! GET_INT64(env, eSec, &usec))
            return FALSE;
#elif (SIZEOF_SUSECONDS_T == SIZEOF_INT)
        int usec;
        if (! GET_INT(env, eSec, &usec))
            return FALSE;
#else
        long usec;
        if (! GET_LONG(env, eSec, &usec))
            return FALSE;
#endif
        timeP->tv_usec = usec;
    }
    return TRUE;
}
extern
int esock_decode_domain(ErlNifEnv*   env,
                    ERL_NIF_TERM eDomain,
                    int*         domain)
{
    if (COMPARE(esock_atom_inet, eDomain) == 0) {
        *domain = AF_INET;
#if defined(HAVE_IN6) && defined(AF_INET6)
    } else if (COMPARE(esock_atom_inet6, eDomain) == 0) {
        *domain = AF_INET6;
#endif
#ifdef HAS_AF_LOCAL
    } else if (COMPARE(esock_atom_local, eDomain) == 0) {
        *domain = AF_LOCAL;
#endif
#ifdef AF_UNSPEC
    } else if (COMPARE(esock_atom_unspec, eDomain) == 0) {
        *domain = AF_UNSPEC;
#endif
    } else {
        int d;
        d = 0;
        if (GET_INT(env, eDomain, &d)) {
            *domain = d;
            return -1;
        }
        return 0;
    }
    return 1;
}
extern
void esock_encode_domain(ErlNifEnv*    env,
                         int           domain,
                         ERL_NIF_TERM* eDomain)
{
    switch (domain) {
    case AF_INET:
        *eDomain = esock_atom_inet;
        break;
#if defined(HAVE_IN6) && defined(AF_INET6)
    case AF_INET6:
        *eDomain = esock_atom_inet6;
        break;
#endif
#ifdef HAS_AF_LOCAL
    case AF_LOCAL:
        *eDomain = esock_atom_local;
        break;
#endif
#ifdef AF_UNSPEC
    case AF_UNSPEC:
        *eDomain = esock_atom_unspec;
        break;
#endif
    default:
        *eDomain = MKI(env, domain);
    }
}
extern
char* esock_domain_to_string(int domain)
{
    switch (domain) {
    case AF_INET:
        return "inet";
        break;
#if defined(HAVE_IN6) && defined(AF_INET6)
    case AF_INET6:
        return "inet6";
        break;
#endif
#ifdef HAS_AF_LOCAL
    case AF_LOCAL:
        return "local";
        break;
#endif
#ifdef AF_UNSPEC
    case AF_UNSPEC:
        return "unspec";
        break;
#endif
    default:
        return "undefined";
    }
}
extern
char* esock_protocol_to_string(int protocol)
{
    switch (protocol) {
#if defined(IPPROTO_IP)
    case IPPROTO_IP:
        return "ip";
        break;
#endif
#if defined(IPPROTO_ICMP)
    case IPPROTO_ICMP:
        return "icmp";
        break;
#endif
#if defined(IPPROTO_IGMP)
    case IPPROTO_IGMP:
        return "igmp";
        break;
#endif
#if defined(IPPROTO_TCP)
    case IPPROTO_TCP:
        return "tcp";
        break;
#endif
#if defined(IPPROTO_UDP)
    case IPPROTO_UDP:
        return "udp";
        break;
#endif
#if defined(IPPROTO_SCTP)
    case IPPROTO_SCTP:
        return "sctp";
        break;
#endif
#if defined(IPPROTO_RAW)
    case IPPROTO_RAW:
        return "raw";
        break;
#endif
    default:
        return "undefined";
    }
}
extern
BOOLEAN_T esock_decode_type(ErlNifEnv*   env,
                            ERL_NIF_TERM eType,
                            int*         type)
{
    int cmp;
    cmp = COMPARE(esock_atom_raw, eType);
    if (cmp < 0) {
        if (COMPARE(esock_atom_stream, eType) == 0) {
            *type = SOCK_STREAM;
#ifdef SOCK_SEQPACKET
        } else if (COMPARE(esock_atom_seqpacket, eType) == 0) {
            *type = SOCK_SEQPACKET;
#endif
        } else
            goto integer;
    } else if (0 < cmp) {
        if (COMPARE(esock_atom_dgram, eType) == 0) {
            *type = SOCK_DGRAM;
        } else
            goto integer;
    } else
        *type = SOCK_RAW;
    return TRUE;
 integer:
    {
        int t = 0;
        if (GET_INT(env, eType, &t)) {
            *type = t;
            return TRUE;
        }
    }
    return FALSE;
}
extern
void esock_encode_type(ErlNifEnv*    env,
                       int           type,
                       ERL_NIF_TERM* eType)
{
    switch (type) {
    case SOCK_STREAM:
        *eType = esock_atom_stream;
        break;
    case SOCK_DGRAM:
        *eType = esock_atom_dgram;
        break;
    case SOCK_RAW:
        *eType = esock_atom_raw;
        break;
#ifdef SOCK_SEQPACKET
    case SOCK_SEQPACKET:
        *eType = esock_atom_seqpacket;
        break;
#endif
#ifdef SOCK_RDM
    case SOCK_RDM:
        *eType = esock_atom_rdm;
        break;
#endif
    default:
        *eType = MKI(env, type);
    }
}
extern
void esock_encode_packet_protocol(ErlNifEnv*     env,
                                  unsigned short protocol,
                                  ERL_NIF_TERM*  eProtocol)
{
    *eProtocol = MKUI(env, protocol);
}
extern
void esock_encode_packet_hatype(ErlNifEnv*     env,
                                unsigned short hatype,
                                ERL_NIF_TERM*  eHaType)
{
    ERL_NIF_TERM tmp;
    switch (hatype) {
#if defined(ARPHRD_NETROM)
    case ARPHRD_NETROM:
        tmp = esock_atom_netrom;
        break;
#endif
#if defined(ARPHRD_ETHER)
    case ARPHRD_ETHER:
        tmp = esock_atom_ether;
        break;
#endif
#if defined(ARPHRD_EETHER)
    case ARPHRD_EETHER:
        tmp = esock_atom_eether;
        break;
#endif
#if defined(ARPHRD_AX25)
    case ARPHRD_AX25:
        tmp = esock_atom_ax25;
        break;
#endif
#if defined(ARPHRD_PRONET)
    case ARPHRD_PRONET:
        tmp = esock_atom_pronet;
        break;
#endif
#if defined(ARPHRD_CHAOS)
    case ARPHRD_CHAOS:
        tmp = esock_atom_chaos;
        break;
#endif
#if defined(ARPHRD_IEEE802)
    case ARPHRD_IEEE802:
        tmp = esock_atom_ieee802;
        break;
#endif
#if defined(ARPHRD_ARCNET)
    case ARPHRD_ARCNET:
        tmp = esock_atom_arcnet;
        break;
#endif
#if defined(ARPHRD_APPLETLK)
    case ARPHRD_APPLETLK:
        tmp = esock_atom_appletlk;
        break;
#endif
#if defined(ARPHRD_DLCI)
    case ARPHRD_DLCI:
        tmp = esock_atom_dlci;
        break;
#endif
#if defined(ARPHRD_ATM)
    case ARPHRD_ATM:
        tmp = esock_atom_atm;
        break;
#endif
#if defined(ARPHRD_METRICOM)
    case ARPHRD_METRICOM:
        tmp = esock_atom_metricom;
        break;
#endif
#if defined(ARPHRD_IEEE1394)
    case ARPHRD_IEEE1394:
        tmp = esock_atom_ieee1394;
        break;
#endif
#if defined(ARPHRD_EUI64)
    case ARPHRD_EUI64:
        tmp = esock_atom_eui64;
        break;
#endif
#if defined(ARPHRD_INFINIBAND)
    case ARPHRD_INFINIBAND:
        tmp = esock_atom_infiniband;
        break;
#endif
#if defined(ARPHRD_TUNNEL)
    case ARPHRD_TUNNEL:
        tmp = esock_atom_tunnel;
        break;
#endif
#if defined(ARPHRD_TUNNEL6)
    case ARPHRD_TUNNEL6:
        tmp = esock_atom_tunnel6;
        break;
#endif
#if defined(ARPHRD_LOOPBACK)
    case ARPHRD_LOOPBACK:
        tmp = esock_atom_loopback;
        break;
#endif
#if defined(ARPHRD_LOCALTLK)
    case ARPHRD_LOCALTLK:
        tmp = esock_atom_localtlk;
        break;
#endif
#if defined(ARPHRD_NONE)
    case ARPHRD_NONE:
        tmp = esock_atom_none;
        break;
#endif
#if defined(ARPHRD_VOID)
    case ARPHRD_VOID:
        tmp = esock_atom_void;
        break;
#endif
    default:
        tmp = MKUI(env, hatype);
        break;
    }
    *eHaType = tmp;
}
extern
void esock_encode_packet_pkttype(ErlNifEnv*     env,
                                 unsigned short pkttype,
                                 ERL_NIF_TERM*  ePktType)
{
    switch (pkttype) {
#if defined(PACKET_HOST)
    case PACKET_HOST:
        *ePktType = esock_atom_host;
        break;
#endif
#if defined(PACKET_BROADCAST)
    case PACKET_BROADCAST:
        *ePktType = esock_atom_broadcast;
        break;
#endif
#if defined(PACKET_MULTICAST)
    case PACKET_MULTICAST:
        *ePktType = esock_atom_multicast;
        break;
#endif
#if defined(PACKET_OTHERHOST)
    case PACKET_OTHERHOST:
        *ePktType = esock_atom_otherhost;
        break;
#endif
#if defined(PACKET_OUTGOING)
    case PACKET_OUTGOING:
        *ePktType = esock_atom_outgoing;
        break;
#endif
#if defined(PACKET_LOOPBACK)
    case PACKET_LOOPBACK:
        *ePktType = esock_atom_loopback;
        break;
#endif
#if defined(PACKET_USER)
    case PACKET_USER:
        *ePktType = esock_atom_user;
        break;
#endif
#if defined(PACKET_KERNEL)
    case PACKET_KERNEL:
        *ePktType = esock_atom_kernel;
        break;
#endif
    default:
        *ePktType = MKUI(env, pkttype);
        break;
    }
}
extern
void esock_encode_packet_addr(ErlNifEnv*     env,
                              unsigned char  len,
                              unsigned char* addr,
                              ERL_NIF_TERM*  eAddr)
{
#if defined(ESOCK_PACKET_ADDRESS_AS_TUPLE)
    esock_encode_packet_addr_tuple(env, len, addr, eAddr);
#else
    SOCKOPTLEN_T vsz = len;
    ErlNifBinary val;
    if (ALLOC_BIN(vsz, &val)) {
        sys_memcpy(val.data, addr, len);
        *eAddr = MKBIN(env, &val);
    } else {
        esock_encode_packet_addr_tuple(env, len, addr, eAddr);
    }
#endif
}
static
void esock_encode_packet_addr_tuple(ErlNifEnv*     env,
                                    unsigned char  len,
                                    unsigned char* addr,
                                    ERL_NIF_TERM*  eAddr)
{
    ERL_NIF_TERM* array = MALLOC(len * sizeof(ERL_NIF_TERM));
    unsigned char i;
    for (i = 0; i < len; i++) {
        array[i] = MKUI(env, addr[i]);
    }
    *eAddr = MKTA(env, array, len);
    FREE(array);
}
static
BOOLEAN_T esock_decode_sockaddr_native(ErlNifEnv*     env,
                                       ERL_NIF_TERM   eSockAddr,
                                       ESockAddress*  sockAddrP,
                                       int            family,
                                       SOCKLEN_T*     addrLen)
{
    ErlNifBinary bin;
    ERL_NIF_TERM eAddr;
    SOCKLEN_T    len;
    if (! GET_MAP_VAL(env, eSockAddr, esock_atom_addr, &eAddr))
        return FALSE;
    if (! GET_BIN(env, eAddr, &bin))
        return FALSE;
    len = sizeof(*sockAddrP) -
        (CHARP(sockAddrP->sa.sa_data) - CHARP(sockAddrP));
    if ((size_t)len < bin.size)
        return FALSE;
    sys_memzero((char*) sockAddrP, sizeof(*sockAddrP));
    sockAddrP->sa.sa_family = (sa_family_t) family;
    sys_memcpy(sockAddrP->sa.sa_data, bin.data, bin.size);
    len = (sockAddrP->sa.sa_data - CHARP(sockAddrP)) + bin.size;
#ifndef NO_SA_LEN
    sockAddrP->sa.sa_len = len;
#endif
    *addrLen = len;
    return TRUE;
}
extern
void esock_encode_sockaddr_native(ErlNifEnv*       env,
                                  struct sockaddr* addr,
                                  SOCKLEN_T        len,
                                  ERL_NIF_TERM     eFamily,
                                  ERL_NIF_TERM*    eSockAddr)
{
    size_t size;
    ERL_NIF_TERM eData;
    UDBG( ("SUTIL", "esock_encode_sockaddr_native -> entry with"
           "\r\n.  len:     %d"
           "\r\n.  eFamily: %T"
           "\r\n", len, eFamily) );
    if (len > 0) {
        size = ((char*)addr + len) - (char*)&addr->sa_data;
        eData = esock_make_new_binary(env, &addr->sa_data, size);
    } else {
        eData = esock_make_new_binary(env, &addr->sa_data, 0);
    }
    {
        ERL_NIF_TERM keys[] = {esock_atom_family, esock_atom_addr};
        ERL_NIF_TERM vals[] = {eFamily, eData};
        size_t numKeys = NUM(keys);
        ESOCK_ASSERT( numKeys == NUM(vals) );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, eSockAddr) );
    }
}
static void esock_encode_sockaddr_broken(ErlNifEnv*       env,
                                         struct sockaddr* addr,
                                         SOCKLEN_T        len,
                                         ERL_NIF_TERM*    eSockAddr) {
    UDBG( ("SUTIL", "esock_encode_sockaddr_broken -> entry with"
           "\r\n.  len: %d"
           "\r\n", len) );
    *eSockAddr = esock_make_new_binary(env, addr, len);
}
extern
BOOLEAN_T esock_decode_bufsz(ErlNifEnv*   env,
                             ERL_NIF_TERM eVal,
                             size_t       defSz,
                             size_t*      szp)
{
    unsigned long val;
    if (GET_ULONG(env, eVal, &val)) {
        defSz = (size_t) val;
        if (val != (unsigned long) defSz || val == 0)
            return FALSE;
    } else {
        if (COMPARE(eVal, esock_atom_default) != 0)
            return FALSE;
    }
    *szp = defSz;
    return TRUE;
}
extern
BOOLEAN_T esock_decode_string(ErlNifEnv*         env,
                              const ERL_NIF_TERM eString,
                              char**             stringP)
{
    BOOLEAN_T    result;
    unsigned int len;
    char*        bufP;
    if (!GET_LIST_LEN(env, eString, &len) && (len != 0)) {
        *stringP = NULL;
        result   = FALSE;
    } else {
        UDBG( ("SUTIL", "esock_decode_string -> len: %d\r\n", len) );
        bufP = MALLOC(len + 1);
        if (GET_STR(env, eString, bufP, len+1)) {
            UDBG( ("SUTIL", "esock_decode_string -> buf: %s\r\n", bufP) );
            *stringP = bufP;
            result   = TRUE;
        } else {
            *stringP = NULL;
            result   = FALSE;
            FREE(bufP);
        }
    }
    return result;
}
extern
BOOLEAN_T esock_extract_pid_from_map(ErlNifEnv*   env,
                                     ERL_NIF_TERM map,
                                     ERL_NIF_TERM key,
                                     ErlNifPid*   pid)
{
    ERL_NIF_TERM val;
    BOOLEAN_T    res;
    if (! GET_MAP_VAL(env, map, key, &val))
        return FALSE;
    res = enif_get_local_pid(env, val, pid);
    return res;
}
extern
BOOLEAN_T esock_extract_int_from_map(ErlNifEnv*   env,
                                 ERL_NIF_TERM map,
                                 ERL_NIF_TERM key,
                                 int*         val)
{
    ERL_NIF_TERM eval;
    BOOLEAN_T    ret;
    if (! GET_MAP_VAL(env, map, key, &eval))
        return FALSE;
    ret = GET_INT(env, eval, val);
    return ret;
}
extern
BOOLEAN_T esock_decode_bool(ERL_NIF_TERM eVal, BOOLEAN_T* val)
{
    if (COMPARE(esock_atom_true, eVal) == 0)
        *val = TRUE;
    else if (COMPARE(esock_atom_false, eVal) == 0)
        *val = FALSE;
    else
        return FALSE;
    return TRUE;
}
extern
ERL_NIF_TERM esock_encode_bool(BOOLEAN_T val)
{
    if (val)
        return esock_atom_true;
    else
        return esock_atom_false;
}
extern
BOOLEAN_T esock_decode_level(ErlNifEnv* env, ERL_NIF_TERM elevel, int *level)
{
    if (COMPARE(esock_atom_socket, elevel) == 0)
        *level = SOL_SOCKET;
    else if (! GET_INT(env, elevel, level))
        return FALSE;
    return TRUE;
}
extern
ERL_NIF_TERM esock_encode_level(ErlNifEnv* env, int level)
{
    if (level == SOL_SOCKET)
        return esock_atom_socket;
    else
        return MKI(env, level);
}
extern
ERL_NIF_TERM esock_make_ok2(ErlNifEnv* env, ERL_NIF_TERM any)
{
    return MKT2(env, esock_atom_ok, any);
}
extern
ERL_NIF_TERM esock_errno_to_term(ErlNifEnv* env, int err)
{
    switch (err) {
#if defined(NO_ERROR)
    case NO_ERROR:
        return MKA(env, "no_error");
        break;
#endif
#if defined(WSA_IO_PENDING)
    case WSA_IO_PENDING:
        return MKA(env, "io_pending");
        break;
#endif
#if defined(WSA_IO_INCOMPLETE)
    case WSA_IO_INCOMPLETE:
        return MKA(env, "io_incomplete");
        break;
#endif
#if defined(WSA_OPERATION_ABORTED)
    case WSA_OPERATION_ABORTED:
        return MKA(env, "operation_aborted");
        break;
#endif
#if defined(WSA_INVALID_PARAMETER)
    case WSA_INVALID_PARAMETER:
        return MKA(env, "invalid_parameter");
        break;
#endif
#if defined(ERROR_INVALID_NETNAME)
    case ERROR_INVALID_NETNAME:
        return MKA(env, "invalid_netname");
        break;
#endif
#if defined(ERROR_NETNAME_DELETED)
    case ERROR_NETNAME_DELETED:
        return MKA(env, "netname_deleted");
        break;
#endif
#if defined(ERROR_TOO_MANY_CMDS)
    case ERROR_TOO_MANY_CMDS:
        return MKA(env, "too_many_cmds");
        break;
#endif
#if defined(ERROR_DUP_NAME)
    case ERROR_DUP_NAME:
        return MKA(env, "duplicate_name");
        break;
#endif
#if defined(ERROR_MORE_DATA)
    case ERROR_MORE_DATA:
        return MKA(env, "more_data");
        break;
#endif
#if defined(ERROR_NOT_FOUND)
    case ERROR_NOT_FOUND:
        return MKA(env, "not_found");
        break;
#endif
#if defined(ERROR_NETWORK_UNREACHABLE)
    case ERROR_NETWORK_UNREACHABLE:
        return MKA(env, "network_unreachable");
        break;
#endif
#if defined(ERROR_PORT_UNREACHABLE)
    case ERROR_PORT_UNREACHABLE:
        return MKA(env, "port_unreachable");
        break;
#endif
    default:
        {
            char* str = erl_errno_id(err);
            if ( strcmp(str, "unknown") == 0 )
                return MKI(env, err);
            else
                return MKA(env, str);
        }
        break;
    }
    return MKI(env, err);
}
extern
ERL_NIF_TERM esock_make_extra_error_info_term(ErlNifEnv*   env,
                                              const char*  file,
                                              const char*  function,
                                              const int    line,
                                              ERL_NIF_TERM rawinfo,
                                              ERL_NIF_TERM info)
{
    ERL_NIF_TERM keys[] = {MKA(env, "file"),
                           MKA(env, "function"),
                           MKA(env, "line"),
                           MKA(env, "raw_info"),
                           MKA(env, "info")};
    ERL_NIF_TERM vals[] = {MKS(env, file),
                           MKS(env, function),
                           MKI(env, line),
                           rawinfo,
                           info};
    unsigned int numKeys = NUM(keys);
    unsigned int numVals = NUM(vals);
    ERL_NIF_TERM map;
    ESOCK_ASSERT( numKeys == numVals );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &map) );
    return map;
}
extern
ERL_NIF_TERM esock_make_error(ErlNifEnv* env, ERL_NIF_TERM reason)
{
    return MKT2(env, esock_atom_error, reason);
}
extern
ERL_NIF_TERM esock_make_error_closed(ErlNifEnv* env)
{
    return esock_make_error(env, esock_atom_closed);
}
extern
ERL_NIF_TERM esock_make_error_str(ErlNifEnv* env, char* reason)
{
    return esock_make_error(env, MKA(env, reason));
}
extern
ERL_NIF_TERM esock_make_error_errno(ErlNifEnv* env, int err)
{
    return esock_make_error_str(env, erl_errno_id(err));
}
extern
ERL_NIF_TERM esock_make_error_t2r(ErlNifEnv*   env,
                                  ERL_NIF_TERM tag,
                                  ERL_NIF_TERM reason)
{
    return MKT2(env, esock_atom_error, MKT2(env, tag, reason));
}
extern
ERL_NIF_TERM esock_make_error_invalid(ErlNifEnv* env, ERL_NIF_TERM what)
{
    return MKT2(env,
                esock_atom_error,
                MKT2(env, esock_atom_invalid, what));
}
extern
ERL_NIF_TERM esock_make_error_integer_range(ErlNifEnv* env, ERL_NIF_TERM i)
{
    return
        esock_make_invalid(env, MKT2(env, esock_atom_integer_range, i));
}
extern
ERL_NIF_TERM esock_make_invalid(ErlNifEnv* env, ERL_NIF_TERM reason)
{
    return MKT2(env, esock_atom_invalid, reason);
}
extern
ERL_NIF_TERM esock_raise_invalid(ErlNifEnv* env, ERL_NIF_TERM what)
{
    return enif_raise_exception(env, MKT2(env, esock_atom_invalid, what));
}
extern
size_t esock_strnlen(const char *s, size_t maxlen)
{
    size_t i = 0;
    while (i < maxlen && s[i] != '\0')
        i++;
    return i;
}
extern
void __noreturn esock_abort(const char* expr,
                            const char* func,
                            const char* file,
                            int         line)
{
#if 0
    fflush(stdout);
    fprintf(stderr, "%s:%d:%s() Assertion failed: %s\n",
            file, line, func, expr);
    fflush(stderr);
    abort();
#else
    erts_exit(ERTS_DUMP_EXIT, "%s:%d:%s() Assertion failed: %s\n",
              file, line, func, expr);
#endif
}
extern
ERL_NIF_TERM esock_self(ErlNifEnv* env)
{
    ErlNifPid pid;
    if (env == NULL)
        return esock_atom_undefined;
    else if (enif_self(env, &pid) == NULL)
        return esock_atom_undefined;
    else
        return enif_make_pid(env, &pid);
}
#define MSG_FUNCS                        \
    MSG_FUNC_DECL(debug,   DEBUG)        \
    MSG_FUNC_DECL(info,    INFO)         \
    MSG_FUNC_DECL(warning, WARNING)      \
    MSG_FUNC_DECL(error,   ERROR)
#define MSG_FUNC_DECL(FN, MC)                                  \
    extern                                                     \
    void esock_##FN##_msg( const char* format, ... )           \
    {                                                          \
       va_list         args;                                   \
       char            f[512 + sizeof(format)];                \
       char            stamp[64];                              \
       int             res;                                    \
                                                               \
       if (esock_timestamp_str(stamp, sizeof(stamp))) {        \
          res = enif_snprintf(f, sizeof(f),                    \
                              "=ESOCK " #MC " MSG==== %s ===\r\n%s", \
                              stamp, format);                  \
       } else {                                                \
          res = enif_snprintf(f,                               \
                              sizeof(f),                       \
                              "=ESOCK " #MC " MSG==== %s", format);  \
       }                                                       \
                                                               \
       if (res > 0) {                                          \
           va_start (args, format);                            \
           enif_vfprintf (stdout, f, args);                    \
           va_end (args);                                      \
           fflush(stdout);                                     \
       }                                                       \
                                                               \
       return;                                                 \
    }                                                          \
MSG_FUNCS
#undef MSG_FUNC_DECL
#undef MSG_FUNCS
extern
ErlNifTime esock_timestamp(void)
{
    ErlNifTime monTime = enif_monotonic_time(ERL_NIF_USEC);
    ErlNifTime offTime = enif_time_offset(ERL_NIF_USEC);
    return (monTime + offTime);
}
extern
BOOLEAN_T esock_timestamp_str(char *buf, unsigned int len)
{
    return esock_format_timestamp(esock_timestamp(), buf, len);
}
extern
BOOLEAN_T esock_format_timestamp(ErlNifTime timestamp, char *buf, unsigned int len)
{
    unsigned  ret;
#if defined(ESOCK_USE_PRETTY_TIMESTAMP)
    time_t    sec     = timestamp / 1000000;
    time_t    usec    = timestamp % 1000000;
    struct tm t;
    if (localtime_r(&sec, &t) == NULL)
        return FALSE;
    ret = strftime(buf, len, "%d-%b-%Y::%T", &t);
    if (ret == 0)
        return FALSE;
    len -= ret;
    buf += ret;
    ret = enif_snprintf(buf, len, ".%06lu", (unsigned long) usec);
    if (ret >= len)
        return FALSE;
    return TRUE;
#else
    ret = enif_snprintf(buf, len, "%lu", (unsigned long) timestamp);
    if (ret >= len)
        return FALSE;
    return TRUE;
#endif
}
static
void make_sockaddr_in(ErlNifEnv*    env,
                           ERL_NIF_TERM  port,
                           ERL_NIF_TERM  addr,
                           ERL_NIF_TERM* sa)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family, esock_atom_port, esock_atom_addr};
    ERL_NIF_TERM vals[]  = {esock_atom_inet, port, addr};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, sa) );
}
static
void make_sockaddr_in6(ErlNifEnv*    env,
                       ERL_NIF_TERM  port,
                       ERL_NIF_TERM  addr,
                       ERL_NIF_TERM  flowInfo,
                       ERL_NIF_TERM  scopeId,
                       ERL_NIF_TERM* sa)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family,
                            esock_atom_port,
                            esock_atom_addr,
                            esock_atom_flowinfo,
                            esock_atom_scope_id};
    ERL_NIF_TERM vals[]  = {esock_atom_inet6,
                            port,
                            addr,
                            flowInfo,
                            scopeId};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, sa) );
}
static
void make_sockaddr_un(ErlNifEnv*    env,
                      ERL_NIF_TERM  path,
                      ERL_NIF_TERM* sa)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family, esock_atom_path};
    ERL_NIF_TERM vals[]  = {esock_atom_local,  path};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, sa) );
}
#ifdef HAVE_NETPACKET_PACKET_H
static
void make_sockaddr_ll(ErlNifEnv*    env,
                      ERL_NIF_TERM  proto,
                      ERL_NIF_TERM  ifindex,
                      ERL_NIF_TERM  hatype,
                      ERL_NIF_TERM  pkttype,
                      ERL_NIF_TERM  addr,
                      ERL_NIF_TERM* sa)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family,
        esock_atom_protocol,
        esock_atom_ifindex,
        esock_atom_hatype,
        esock_atom_pkttype,
        esock_atom_addr};
    ERL_NIF_TERM vals[]  = {esock_atom_packet,
        proto,
        ifindex,
        hatype,
        pkttype,
        addr};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, sa) );
}
#endif
#if defined(HAVE_NET_IF_DL_H) && defined(AF_LINK)
static
void make_sockaddr_dl(ErlNifEnv*    env,
                      ERL_NIF_TERM  index,
                      ERL_NIF_TERM  type,
                      ERL_NIF_TERM  nlen,
                      ERL_NIF_TERM  alen,
                      ERL_NIF_TERM  slen,
                      ERL_NIF_TERM  data,
                      ERL_NIF_TERM* sa)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family,
        esock_atom_index,
        esock_atom_type,
        esock_atom_nlen,
        esock_atom_alen,
        esock_atom_slen,
        esock_atom_data};
    ERL_NIF_TERM vals[]  = {esock_atom_link,
        index,
        type,
        nlen,
        alen,
        slen,
        data};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, sa) );
}
#endif
extern
ERL_NIF_TERM esock_make_new_binary(ErlNifEnv *env, void *buf, size_t size)
{
    ERL_NIF_TERM term;
    sys_memcpy(enif_make_new_binary(env, size, &term), buf, size);
    return term;
}
extern
BOOLEAN_T esock_is_integer(ErlNifEnv *env, ERL_NIF_TERM term)
{
    double d;
    if (enif_is_number(env, term))
        return (! enif_get_double(env, term, &d));
    else
        return FALSE;
}
extern
void* esock_dlopen(char* name)
{
#if defined(HAVE_DLOPEN)
    return dlopen(name, RTLD_NOW);
#else
    return NULL;
#endif
}
extern
void* esock_dlsym(void* handle, const char* symbolName)
{
#if defined(HAVE_DLOPEN)
    void* sym;
    char* e;
    dlerror();
    sym = dlsym(handle, symbolName);
    if ((e = dlerror()) != NULL) {
        return NULL;
    } else {
	return sym;
    }
#else
    return NULL;
#endif
}
#endif