#define STATIC_ERLANG_NIF 1
#ifdef HAVE_CONFIG_H
#    include "config.h"
#endif
#ifndef ESOCK_ENABLE
#    include <erl_nif.h>
static
ErlNifFunc net_funcs[] = {};
static
int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
    (void)env;
    (void)priv_data;
    (void)load_info;
    return 1;
}
ERL_NIF_INIT(prim_net, net_funcs, on_load, NULL, NULL, NULL)
#else
#if (defined(HAVE_SCTP_H) && defined(__sun) && defined(__SVR4))
#define SOLARIS10    1
#define _XPG4_2
#define __EXTENSIONS__
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
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
#elif defined(__PASE__)
#include <as400_protos.h>
#define ifaddrs ifaddrs_pase
#endif
#ifdef HAVE_NETPACKET_PACKET_H
#include <netpacket/packet.h>
#endif
#ifdef HAVE_SYS_UN_H
#include <sys/un.h>
#endif
#if defined(__APPLE__) && defined(__MACH__) && !defined(__DARWIN__)
#define __DARWIN__ 1
#endif
#ifdef __WIN32__
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
#  define NTDDI_VERSION NTDDI_WIN10_RS2
#endif
#include <iphlpapi.h>
#include <mstcpip.h>
#undef WANT_NONBLOCKING
#include "sys.h"
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
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
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
#ifndef WANT_NONBLOCKING
#define WANT_NONBLOCKING
#endif
#include "sys.h"
#endif
#include <erl_nif.h>
#include "socket_dbg.h"
#include "socket_int.h"
#include "socket_tarray.h"
#include "socket_util.h"
#define FATAL_MALLOC
#ifdef __WIN32__
#define net_gethostname(__buf__, __bufSz__)     \
    gethostname((__buf__), (__bufSz__))
#define net_getservbyname(__name__, __proto__)  \
    getservbyname((__name__), (__proto__))
#define net_getservbyport(__port__, __proto__)  \
    getservbyport((__port__), (__proto__))
#define net_ntohs(x)                            \
    ntohs((x))
#define net_htons(x)                            \
    htons((x))
#else
#define net_gethostname(__buf__, __bufSz__)     \
    gethostname((__buf__), (__bufSz__))
#define net_getservbyname(__name__, __proto__)  \
    getservbyname((__name__), (__proto__))
#define net_getservbyport(__port__, __proto__)  \
    getservbyport((__port__), (__proto__))
#define net_ntohs(x)                            \
    ntohs((x))
#define net_htons(x)                            \
    htons((x))
#endif
#ifdef __WIN32__
#define get_errno() WSAGetLastError()
#else
#define get_errno() errno
#endif
#define HOSTNAME_LEN 256
#define SERVICE_LEN  256
#define NET_MAXHOSTNAMELEN 255
#define NET_NIF_DEBUG_DEFAULT FALSE
#define NDBG( proto )       ESOCK_DBG_PRINTF( data.debug , proto )
#define NDBG2( dbg, proto ) ESOCK_DBG_PRINTF( (dbg || data.debug) , proto )
typedef struct {
    BOOLEAN_T debug;
} NetData;
static NetData data;
extern char* erl_errno_id(int error);
#define ENET_NIF_FUNCS                       \
    ENET_NIF_FUNC_DEF(info);                 \
    ENET_NIF_FUNC_DEF(command);              \
    ENET_NIF_FUNC_DEF(gethostname);          \
    ENET_NIF_FUNC_DEF(getnameinfo);          \
    ENET_NIF_FUNC_DEF(getaddrinfo);          \
    ENET_NIF_FUNC_DEF(getifaddrs);           \
    ENET_NIF_FUNC_DEF(get_adapters_addresses);  \
    ENET_NIF_FUNC_DEF(get_if_entry);         \
    ENET_NIF_FUNC_DEF(get_interface_info);   \
    ENET_NIF_FUNC_DEF(get_ip_address_table); \
    ENET_NIF_FUNC_DEF(getservbyname);        \
    ENET_NIF_FUNC_DEF(getservbyport);        \
    ENET_NIF_FUNC_DEF(if_name2index);        \
    ENET_NIF_FUNC_DEF(if_index2name);        \
    ENET_NIF_FUNC_DEF(if_names);
#define ENET_NIF_FUNC_DEF(F)                              \
    static ERL_NIF_TERM nif_##F(ErlNifEnv*         env,    \
                                int                argc,   \
                                const ERL_NIF_TERM argv[]);
ENET_NIF_FUNCS
#undef ENET_NIF_FUNC_DEF
static ERL_NIF_TERM enet_command(ErlNifEnv*   env,
                                 ERL_NIF_TERM cmd,
                                 ERL_NIF_TERM cdata);
static ERL_NIF_TERM enet_gethostname(ErlNifEnv* env);
#if defined(HAVE_GETNAMEINFO)
static ERL_NIF_TERM enet_getnameinfo(ErlNifEnv*          env,
                                     const ESockAddress* saP,
                                     SOCKLEN_T           saLen,
                                     int                 flags);
#endif
#if defined(HAVE_GETADDRINFO)
static ERL_NIF_TERM enet_getaddrinfo(ErlNifEnv* env,
                                     char*      host,
                                     char*      serv);
#endif
#if defined(HAVE_GETIFADDRS) || defined(__PASE__)
static ERL_NIF_TERM enet_getifaddrs(ErlNifEnv* env,
                                    BOOLEAN_T  dbg,
                                    char*      netns);
#endif
#if defined(__WIN32__)
static BOOLEAN_T enet_get_adapters_addresses_args_debug(ErlNifEnv*         env,
                                                        const ERL_NIF_TERM eargs);
static BOOLEAN_T enet_get_adapters_addresses_args_family(ErlNifEnv*         env,
                                                         const ERL_NIF_TERM eargs,
                                                         ULONG*             fam);
static BOOLEAN_T enet_get_adapters_addresses_args_flags(ErlNifEnv*         env,
                                                         const ERL_NIF_TERM eargs,
                                                         ULONG*             flags);
static ERL_NIF_TERM enet_get_adapters_addresses(ErlNifEnv* env,
                                                BOOLEAN_T  dbg,
                                                ULONG      fam,
                                                ULONG      flags);
static ERL_NIF_TERM enet_adapters_addresses_encode(ErlNifEnv*            env,
                                                   BOOLEAN_T             dbg,
                                                   IP_ADAPTER_ADDRESSES* ipAdAddrsP);
static ERL_NIF_TERM enet_adapter_addresses_encode(ErlNifEnv*            env,
                                                  BOOLEAN_T             dbg,
                                                  IP_ADAPTER_ADDRESSES* ipAdAddrsP);
static ERL_NIF_TERM enet_adapter_encode_name(ErlNifEnv* env,
                                             WCHAR*     name);
static ERL_NIF_TERM enet_adapter_encode_friendly_name(ErlNifEnv* env,
                                                      WCHAR*     fname);
static ERL_NIF_TERM encode_if_oper_status(ErlNifEnv* env,
                                          DWORD      status);
static ERL_NIF_TERM encode_adapter_flags(ErlNifEnv*            env,
                                         IP_ADAPTER_ADDRESSES* ipAdAddrsP);
static ERL_NIF_TERM encode_adapter_unicast_addrs(ErlNifEnv*                  env,
                                                 IP_ADAPTER_UNICAST_ADDRESS* firstP);
static ERL_NIF_TERM encode_adapter_unicast_addr(ErlNifEnv*                  env,
                                                IP_ADAPTER_UNICAST_ADDRESS* addrP);
static ERL_NIF_TERM encode_adapter_unicast_addr_flags(ErlNifEnv* env,
                                                      DWORD      flags);
static ERL_NIF_TERM encode_adapter_unicast_addr_sockaddr(ErlNifEnv*       env,
                                                         struct sockaddr* addrP);
static ERL_NIF_TERM encode_adapter_unicast_addr_porig(ErlNifEnv*       env,
                                                      IP_PREFIX_ORIGIN porig);
static ERL_NIF_TERM encode_adapter_unicast_addr_sorig(ErlNifEnv*       env,
                                                      IP_SUFFIX_ORIGIN sorig);
static ERL_NIF_TERM encode_adapter_unicast_addr_dad_state(ErlNifEnv*   env,
                                                          IP_DAD_STATE dstate);
static ERL_NIF_TERM encode_adapter_anycast_addrs(ErlNifEnv*                  env,
                                                 IP_ADAPTER_ANYCAST_ADDRESS* firstP);
static ERL_NIF_TERM encode_adapter_anycast_addr(ErlNifEnv*                  env,
                                                IP_ADAPTER_ANYCAST_ADDRESS* addrP);
static ERL_NIF_TERM encode_adapter_anycast_addr_flags(ErlNifEnv* env,
                                                      DWORD      flags);
static ERL_NIF_TERM encode_adapter_anycast_addr_sockaddr(ErlNifEnv*       env,
                                                         struct sockaddr* addrP);
static ERL_NIF_TERM encode_adapter_multicast_addrs(ErlNifEnv*                  env,
                                                   IP_ADAPTER_MULTICAST_ADDRESS* firstP);
static ERL_NIF_TERM encode_adapter_multicast_addr(ErlNifEnv*                    env,
                                                  IP_ADAPTER_MULTICAST_ADDRESS* addrP);
static ERL_NIF_TERM encode_adapter_multicast_addr_flags(ErlNifEnv* env,
                                                        DWORD      flags);
static ERL_NIF_TERM encode_adapter_multicast_addr_sockaddr(ErlNifEnv*       env,
                                                           struct sockaddr* addrP);
static ERL_NIF_TERM encode_adapter_dns_server_addrs(ErlNifEnv*                     env,
                                                    IP_ADAPTER_DNS_SERVER_ADDRESS* firstP);
static ERL_NIF_TERM encode_adapter_dns_server_addr(ErlNifEnv*                     env,
                                                   IP_ADAPTER_DNS_SERVER_ADDRESS* addrP);
static ERL_NIF_TERM encode_adapter_dns_server_addr_sockaddr(ErlNifEnv*       env,
                                                            struct sockaddr* addrP);
static ERL_NIF_TERM encode_adapter_zone_indices(ErlNifEnv* env,
                                                DWORD*     zoneIndices,
                                                DWORD      len);
static ERL_NIF_TERM encode_adapter_prefixes(ErlNifEnv*         env,
                                            IP_ADAPTER_PREFIX* firstP);
static ERL_NIF_TERM encode_adapter_prefix(ErlNifEnv*         env,
                                          IP_ADAPTER_PREFIX* prefP);
static ERL_NIF_TERM encode_adapter_prefix_sockaddr(ErlNifEnv*       env,
                                                   struct sockaddr* addrP);
static ERL_NIF_TERM enet_get_if_entry(ErlNifEnv* env,
                                      BOOLEAN_T  dbg,
                                      DWORD      index);
static BOOLEAN_T enet_get_if_entry_args_index(ErlNifEnv*         env,
                                              const ERL_NIF_TERM eargs,
                                              DWORD*             index);
static BOOLEAN_T enet_get_if_entry_args_debug(ErlNifEnv*         env,
                                              const ERL_NIF_TERM eargs);
static ERL_NIF_TERM enet_if_row_encode(ErlNifEnv* env,
                                       BOOLEAN_T  dbg,
                                       MIB_IFROW* rowP);
static ERL_NIF_TERM encode_if_type(ErlNifEnv* env,
                                   DWORD      type);
static ERL_NIF_TERM encode_if_row_description(ErlNifEnv* env,
                                              DWORD      len,
                                              UCHAR*     buf);
static ERL_NIF_TERM encode_if_admin_status(ErlNifEnv* env,
                                           DWORD      status);
static ERL_NIF_TERM encode_internal_if_oper_status(ErlNifEnv* env,
                                                   DWORD      status);
static ERL_NIF_TERM encode_if_row_phys_address(ErlNifEnv* env,
                                               DWORD      len,
                                               UCHAR*     buf);
static ERL_NIF_TERM enet_get_interface_info(ErlNifEnv* env,
                                            BOOLEAN_T  dbg);
static BOOLEAN_T enet_get_interface_info_args_debug(ErlNifEnv*         env,
                                                    const ERL_NIF_TERM eextra);
static ERL_NIF_TERM enet_interface_info_encode(ErlNifEnv*         env,
                                               BOOLEAN_T          dbg,
                                               IP_INTERFACE_INFO* infoP);
static void encode_adapter_index_map(ErlNifEnv*            env,
                                     BOOLEAN_T             dbg,
                                     IP_ADAPTER_INDEX_MAP* adapterP,
                                     ERL_NIF_TERM*         eadapter);
static ERL_NIF_TERM encode_adapter_index_map_name(ErlNifEnv* env, WCHAR* name);
static void make_adapter_index_map(ErlNifEnv*    env,
                                   ERL_NIF_TERM  eindex,
                                   ERL_NIF_TERM  ename,
                                   ERL_NIF_TERM* emap);
static ERL_NIF_TERM enet_get_ip_address_table(ErlNifEnv* env,
                                              BOOLEAN_T  sort,
                                              BOOLEAN_T  dbg);
static ERL_NIF_TERM enet_get_ip_address_table_encode(ErlNifEnv*       env,
                                                     BOOLEAN_T        dbg,
                                                     MIB_IPADDRTABLE* tabP);
static ERL_NIF_TERM encode_ip_address_row(ErlNifEnv*     env,
                                          BOOLEAN_T      dbg,
                                          MIB_IPADDRROW* rowP);
static ERL_NIF_TERM encode_ip_address_row_addr(ErlNifEnv*  env,
                                               BOOLEAN_T   dbg,
                                               const char* descr,
                                               DWORD       addr);
static void make_ip_address_row(ErlNifEnv*    env,
                                ERL_NIF_TERM  eaddr,
                                ERL_NIF_TERM  eindex,
                                ERL_NIF_TERM  emask,
                                ERL_NIF_TERM  eBCastAddr,
                                ERL_NIF_TERM  eReasmSize,
                                ERL_NIF_TERM* iar);
#endif
static ERL_NIF_TERM enet_getservbyname(ErlNifEnv*   env,
                                       ERL_NIF_TERM ename,
                                       ERL_NIF_TERM eproto);
static ERL_NIF_TERM enet_getservbyport(ErlNifEnv*   env,
                                       BOOLEAN_T      dbg,
                                       ERL_NIF_TERM eport,
                                       ERL_NIF_TERM eproto);
#if defined(HAVE_IF_NAMETOINDEX)
static ERL_NIF_TERM enet_if_name2index(ErlNifEnv* env,
                                       char*      ifn);
#endif
#if defined(HAVE_IF_INDEXTONAME)
static ERL_NIF_TERM enet_if_index2name(ErlNifEnv*   env,
                                       unsigned int id);
#endif
#if defined(HAVE_IF_NAMEINDEX) && defined(HAVE_IF_FREENAMEINDEX)
static ERL_NIF_TERM enet_if_names(ErlNifEnv* env);
static unsigned int enet_if_names_length(struct if_nameindex* p);
#endif
static ERL_NIF_TERM enet_getifaddrs(ErlNifEnv* env,
                                    BOOLEAN_T  dbg,
                                    char*      netns);
static ERL_NIF_TERM enet_getifaddrs_process(ErlNifEnv*      env,
                                            BOOLEAN_T       dbg,
                                            struct ifaddrs* ifap);
static unsigned int enet_getifaddrs_length(struct ifaddrs* ifap);
static void encode_ifaddrs(ErlNifEnv*      env,
                           struct ifaddrs* ifap,
                           ERL_NIF_TERM*   eifa);
static ERL_NIF_TERM encode_ifaddrs_name(ErlNifEnv* env,
                                        char*      name);
static ERL_NIF_TERM encode_ifaddrs_flags(ErlNifEnv*   env,
                                         unsigned int flags);
static ERL_NIF_TERM encode_ifaddrs_addr(ErlNifEnv*       env,
                                        struct sockaddr* sa);
static void make_ifaddrs(ErlNifEnv*    env,
                         ERL_NIF_TERM  name,
                         ERL_NIF_TERM  flags,
                         ERL_NIF_TERM  addr,
                         ERL_NIF_TERM  netmask,
                         ERL_NIF_TERM  ifu_key,
                         ERL_NIF_TERM  ifu_value,
                         ERL_NIF_TERM  data,
                         ERL_NIF_TERM* ifAddrs);
static BOOLEAN_T enet_getifaddrs_debug(ErlNifEnv*   env,
                                       ERL_NIF_TERM map);
#ifdef HAVE_SETNS
static BOOLEAN_T enet_getifaddrs_netns(ErlNifEnv*   env,
                                       ERL_NIF_TERM map,
                                       char**       netns);
static BOOLEAN_T change_network_namespace(char* netns, int* cns, int* err);
static BOOLEAN_T restore_network_namespace(int ns, int* err);
#endif
static ERL_NIF_TERM encode_sockaddr(ErlNifEnv*       env,
                                    struct sockaddr* sa);
static BOOLEAN_T decode_nameinfo_flags(ErlNifEnv*         env,
                                       const ERL_NIF_TERM eflags,
                                       int*               flags);
static BOOLEAN_T decode_nameinfo_flags_list(ErlNifEnv*         env,
                                            const ERL_NIF_TERM eflags,
                                            int*               flags);
static BOOLEAN_T decode_addrinfo_string(ErlNifEnv*         env,
                                        const ERL_NIF_TERM eString,
                                        char**             stringP);
static ERL_NIF_TERM encode_address_infos(ErlNifEnv*       env,
                                         struct addrinfo* addrInfo);
static ERL_NIF_TERM encode_address_info(ErlNifEnv*       env,
                                        struct addrinfo* addrInfoP);
static unsigned int address_info_length(struct addrinfo* addrInfoP);
static ERL_NIF_TERM encode_address_info_family(ErlNifEnv* env,
                                               int        family);
static ERL_NIF_TERM encode_address_info_type(ErlNifEnv* env,
                                             int        socktype);
static void make_address_info(ErlNifEnv*    env,
                              ERL_NIF_TERM  fam,
                              ERL_NIF_TERM  sockType,
                              ERL_NIF_TERM  proto,
                              ERL_NIF_TERM  addr,
                              ERL_NIF_TERM* ai);
#if defined(__WIN32__)
static ERL_NIF_TERM encode_uchar(ErlNifEnv* env,
                                 DWORD      len,
                                 UCHAR*     buf);
static ERL_NIF_TERM encode_wchar(ErlNifEnv* env,
                                 WCHAR* name);
#endif
static BOOLEAN_T get_debug(ErlNifEnv*   env,
                           ERL_NIF_TERM map);
static int on_load(ErlNifEnv*   env,
                   void**       priv_data,
                   ERL_NIF_TERM load_info);
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
#define LOCAL_ATOMS                             \
    LOCAL_ATOM_DECL(address_info);              \
    LOCAL_ATOM_DECL(admin_status);              \
    LOCAL_ATOM_DECL(anycast_addrs);             \
    LOCAL_ATOM_DECL(atm);                       \
    LOCAL_ATOM_DECL(automedia);                 \
    LOCAL_ATOM_DECL(bcast_addr);                \
    LOCAL_ATOM_DECL(broadaddr);                 \
    LOCAL_ATOM_DECL(broadcast);                 \
    LOCAL_ATOM_DECL(dad_state);                 \
    LOCAL_ATOM_DECL(debug);                     \
    LOCAL_ATOM_DECL(deprecated);                \
    LOCAL_ATOM_DECL(description);               \
    LOCAL_ATOM_DECL(dhcp);                      \
    LOCAL_ATOM_DECL(dhcp_v4_enabled);           \
    LOCAL_ATOM_DECL(ddns_enabled);              \
    LOCAL_ATOM_DECL(disconnected);              \
    LOCAL_ATOM_DECL(dns_eligible);              \
    LOCAL_ATOM_DECL(dns_server_addrs);          \
    LOCAL_ATOM_DECL(dns_suffix);                \
    LOCAL_ATOM_DECL(down);                      \
    LOCAL_ATOM_DECL(dstaddr);                   \
    LOCAL_ATOM_DECL(duplicate);                 \
    LOCAL_ATOM_DECL(dynamic);                   \
    LOCAL_ATOM_DECL(ethernet_csmacd);           \
    LOCAL_ATOM_DECL(fddi);                      \
    LOCAL_ATOM_DECL(friendly_name);             \
    LOCAL_ATOM_DECL(host);                      \
    LOCAL_ATOM_DECL(idn);                       \
    LOCAL_ATOM_DECL(ieee1394);                  \
    LOCAL_ATOM_DECL(ieee80211);                 \
    LOCAL_ATOM_DECL(ieee80216_wman);            \
    LOCAL_ATOM_DECL(include_prefix);              \
    LOCAL_ATOM_DECL(include_wins_info);           \
    LOCAL_ATOM_DECL(include_gateways);            \
    LOCAL_ATOM_DECL(include_all_interfaces);      \
    LOCAL_ATOM_DECL(include_all_compartments);    \
    LOCAL_ATOM_DECL(include_tunnel_bindingorder); \
    LOCAL_ATOM_DECL(index);                     \
    LOCAL_ATOM_DECL(internal_oper_status);      \
    LOCAL_ATOM_DECL(invalid);                   \
    LOCAL_ATOM_DECL(in_octets);                 \
    LOCAL_ATOM_DECL(in_ucast_pkts);             \
    LOCAL_ATOM_DECL(in_nucast_pkts);            \
    LOCAL_ATOM_DECL(in_discards);               \
    LOCAL_ATOM_DECL(in_errors);                 \
    LOCAL_ATOM_DECL(in_unknown_protos);         \
    LOCAL_ATOM_DECL(ipv4_enabled);                \
    LOCAL_ATOM_DECL(ipv6_enabled);                \
    LOCAL_ATOM_DECL(ipv6_index);                  \
    LOCAL_ATOM_DECL(ipv6_managed_address_config_supported);  \
    LOCAL_ATOM_DECL(ipv6_other_stateful_config);  \
    LOCAL_ATOM_DECL(iso88025_tokenring);        \
    LOCAL_ATOM_DECL(last_change);               \
    LOCAL_ATOM_DECL(lease_lifetime);            \
    LOCAL_ATOM_DECL(length);                    \
    LOCAL_ATOM_DECL(link_layer_address);        \
    LOCAL_ATOM_DECL(lower_layer_down);          \
    LOCAL_ATOM_DECL(manual);                    \
    LOCAL_ATOM_DECL(mask);                      \
    LOCAL_ATOM_DECL(master);                    \
    LOCAL_ATOM_DECL(multicast);                 \
    LOCAL_ATOM_DECL(multicast_addrs);           \
    LOCAL_ATOM_DECL(namereqd);                  \
    LOCAL_ATOM_DECL(name_info);                 \
    LOCAL_ATOM_DECL(netbios_over_tcpip_enabled);  \
    LOCAL_ATOM_DECL(netmask);                   \
    LOCAL_ATOM_DECL(noarp);                     \
    LOCAL_ATOM_DECL(nofqdn);                    \
    LOCAL_ATOM_DECL(non_operational);           \
    LOCAL_ATOM_DECL(notrailers);                \
    LOCAL_ATOM_DECL(not_present);               \
    LOCAL_ATOM_DECL(no_multicast);              \
    LOCAL_ATOM_DECL(numerichost);               \
    LOCAL_ATOM_DECL(numericserv);               \
    LOCAL_ATOM_DECL(on_link_prefix_length);     \
    LOCAL_ATOM_DECL(operational);               \
    LOCAL_ATOM_DECL(oper_status);               \
    LOCAL_ATOM_DECL(other);                     \
    LOCAL_ATOM_DECL(out_octets);                \
    LOCAL_ATOM_DECL(out_ucast_pkts);            \
    LOCAL_ATOM_DECL(out_nucast_pkts);           \
    LOCAL_ATOM_DECL(out_discards);              \
    LOCAL_ATOM_DECL(out_errors);                \
    LOCAL_ATOM_DECL(out_qlen);                  \
    LOCAL_ATOM_DECL(phys_addr);                 \
    LOCAL_ATOM_DECL(pointopoint);               \
    LOCAL_ATOM_DECL(portsel);                   \
    LOCAL_ATOM_DECL(ppp);                       \
    LOCAL_ATOM_DECL(preferred);                 \
    LOCAL_ATOM_DECL(preferred_lifetime);        \
    LOCAL_ATOM_DECL(prefixes);                  \
    LOCAL_ATOM_DECL(prefix_origin);             \
    LOCAL_ATOM_DECL(promisc);                   \
    LOCAL_ATOM_DECL(random);                    \
    LOCAL_ATOM_DECL(reasm_size);                \
    LOCAL_ATOM_DECL(receive_only);              \
    LOCAL_ATOM_DECL(register_adapter_suffix);   \
    LOCAL_ATOM_DECL(router_advertisement);      \
    LOCAL_ATOM_DECL(running);                   \
    LOCAL_ATOM_DECL(service);                   \
    LOCAL_ATOM_DECL(slave);                     \
    LOCAL_ATOM_DECL(skip_unicast);              \
    LOCAL_ATOM_DECL(skip_anycast);              \
    LOCAL_ATOM_DECL(skip_multicast);            \
    LOCAL_ATOM_DECL(skip_dns_server);           \
    LOCAL_ATOM_DECL(skip_friendly_name);        \
    LOCAL_ATOM_DECL(software_loopback);         \
    LOCAL_ATOM_DECL(speed);                     \
    LOCAL_ATOM_DECL(suffix_origin);             \
    LOCAL_ATOM_DECL(tentative);                 \
    LOCAL_ATOM_DECL(testing);                   \
    LOCAL_ATOM_DECL(transient);                 \
    LOCAL_ATOM_DECL(tunnel);                    \
    LOCAL_ATOM_DECL(unchanged);                 \
    LOCAL_ATOM_DECL(unknown);                   \
    LOCAL_ATOM_DECL(unicast_addrs);             \
    LOCAL_ATOM_DECL(unreachable);               \
    LOCAL_ATOM_DECL(up);                        \
    LOCAL_ATOM_DECL(valid_lifetime);            \
    LOCAL_ATOM_DECL(well_known);                \
    LOCAL_ATOM_DECL(wwanpp);                    \
    LOCAL_ATOM_DECL(wwanpp2);                   \
    LOCAL_ATOM_DECL(zone_indices);
#define LOCAL_ERROR_REASON_ATOMS               \
    LOCAL_ATOM_DECL(address_not_associated);   \
    LOCAL_ATOM_DECL(can_not_complete);         \
    LOCAL_ATOM_DECL(eaddrfamily);              \
    LOCAL_ATOM_DECL(ebadflags);                \
    LOCAL_ATOM_DECL(efail);                    \
    LOCAL_ATOM_DECL(efamily);                  \
    LOCAL_ATOM_DECL(efault);                   \
    LOCAL_ATOM_DECL(emem);                     \
    LOCAL_ATOM_DECL(enametoolong);             \
    LOCAL_ATOM_DECL(enodata);                  \
    LOCAL_ATOM_DECL(enoname);                  \
    LOCAL_ATOM_DECL(enxio);                    \
    LOCAL_ATOM_DECL(eoverflow);                \
    LOCAL_ATOM_DECL(eservice);                 \
    LOCAL_ATOM_DECL(esocktype);                \
    LOCAL_ATOM_DECL(esystem);                  \
    LOCAL_ATOM_DECL(insufficient_buffer);      \
    LOCAL_ATOM_DECL(invalid_data);             \
    LOCAL_ATOM_DECL(invalid_flags);            \
    LOCAL_ATOM_DECL(invalid_parameter);        \
    LOCAL_ATOM_DECL(not_found);                \
    LOCAL_ATOM_DECL(not_enough_memory);        \
    LOCAL_ATOM_DECL(not_supported);            \
    LOCAL_ATOM_DECL(no_data);                  \
    LOCAL_ATOM_DECL(no_function);              \
    LOCAL_ATOM_DECL(no_uniconde_traslation);
#define LOCAL_ATOM_DECL(A) static ERL_NIF_TERM atom_##A
LOCAL_ATOMS
LOCAL_ERROR_REASON_ATOMS
#undef LOCAL_ATOM_DECL
static ErlNifResourceType*    net;
static ErlNifResourceTypeInit netInit = {
    NULL,
    NULL,
    NULL
};
static
ERL_NIF_TERM nif_info(ErlNifEnv*         env,
                      int                argc,
                      const ERL_NIF_TERM argv[])
{
    BOOLEAN_T    dbg     = data.debug;
#ifdef HAVE_NETNS
    BOOLEAN_T    netns   = TRUE;
#else
    BOOLEAN_T    netns   = FALSE;
#endif
    ERL_NIF_TERM vals[]  = {BOOL2ATOM(dbg), BOOL2ATOM(netns)};
    ERL_NIF_TERM keys[]  = {atom_debug, enif_make_atom(env, "netns")};
    unsigned int numVals = NUM(vals);
    unsigned int numKeys = NUM(keys);
    ERL_NIF_TERM info;
    ESOCK_ASSERT( numKeys == numVals );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &info) );
    NDBG( ("NET", "info -> done\r\n") );
    return info;
}
static
ERL_NIF_TERM nif_command(ErlNifEnv*         env,
                         int                argc,
                         const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM command, cdata, result;
    NDBG( ("NET", "command -> entry (%d)\r\n", argc) );
    ESOCK_ASSERT( argc == 1 );
    if (! GET_MAP_VAL(env, argv[0], esock_atom_command, &command)) {
        NDBG( ("NET",
               "nif_command -> field not found: command\r\n") );
        return enif_make_badarg(env);
    }
    if (! GET_MAP_VAL(env, argv[0], esock_atom_data, &cdata)) {
        NDBG( ("NET",
               "nif_command -> field not found: data\r\n") );
        return enif_make_badarg(env);
    }
    NDBG( ("NET", "nif_command -> "
           "\r\n   command:  %T"
           "\r\n   cdata:    %T"
           "\r\n", command, cdata) );
    result = enet_command(env, command, cdata);
    NDBG( ("NET", "command -> result: %T\r\n", result) );
    return result;
}
static
ERL_NIF_TERM enet_command(ErlNifEnv*   env,
                          ERL_NIF_TERM cmd,
                          ERL_NIF_TERM cdata)
{
    NDBG( ("NET", "enet_command -> entry with %T\r\n", cmd) );
    if (COMPARE(cmd, esock_atom_debug) == 0) {
        BOOLEAN_T dbg;
        NDBG( ("NET", "enet_command -> debug command with"
               "\r\n   data: %T"
               "\r\n", cdata) );
        if (! esock_decode_bool(cdata, &dbg))
            return esock_raise_invalid(env, MKT2(env, esock_atom_data, cdata));
        NDBG( ("NET", "enet_command -> update debug (%T)\r\n", cdata) );
        data.debug = dbg;
        return esock_atom_ok;
    }
    NDBG( ("NET", "enet_command -> invalid command: %T\r\n", cmd) );
    return esock_raise_invalid(env, MKT2(env, esock_atom_command, cmd));
}
static
ERL_NIF_TERM nif_gethostname(ErlNifEnv*         env,
                             int                argc,
                             const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM result;
    NDBG( ("NET", "nif_gethostname -> entry (%d)\r\n", argc) );
    if (argc != 0)
        return enif_make_badarg(env);
    result = enet_gethostname(env);
    NDBG( ("NET", "nif_gethostname -> done when result: %T\r\n", result) );
    return result;
}
static
ERL_NIF_TERM enet_gethostname(ErlNifEnv* env)
{
    ERL_NIF_TERM result;
    char         buf[NET_MAXHOSTNAMELEN + 1];
    int          res;
    res = net_gethostname(buf, sizeof(buf));
    NDBG( ("NET", "enet_gethostname -> gethostname res: %d\r\n", res) );
    switch (res) {
    case 0:
        result = esock_make_ok2(env, MKS(env, buf));
        break;
    case EFAULT:
        result = esock_make_error(env, atom_efault);
        break;
    case EINVAL:
        result = esock_make_error(env, esock_atom_einval);
        break;
    case ENAMETOOLONG:
        result = esock_make_error(env, atom_enametoolong);
        break;
    default:
        result = esock_make_error(env, MKI(env, res));
        break;
    }
    return result;
}
static
ERL_NIF_TERM nif_getnameinfo(ErlNifEnv*         env,
                             int                argc,
                             const ERL_NIF_TERM argv[])
{
#if defined(HAVE_GETNAMEINFO)
    ERL_NIF_TERM result;
    ERL_NIF_TERM eSockAddr, eFlags;
    int          flags = 0;
    ESockAddress sa;
    SOCKLEN_T    saLen = 0;
    NDBG( ("NET", "nif_getnameinfo -> entry (%d)\r\n", argc) );
    if (argc != 2)
        return enif_make_badarg(env);
    eSockAddr = argv[0];
    eFlags    = argv[1];
    NDBG( ("NET",
           "nif_getnameinfo -> "
           "\r\n   SockAddr: %T"
           "\r\n   Flags:    %T"
           "\r\n", eSockAddr, eFlags) );
    if (! esock_decode_sockaddr(env, eSockAddr, &sa, &saLen)) {
        NDBG( ("NET", "nif_getnameinfo -> failed decode sockaddr\r\n") );
        return esock_make_error(env, esock_atom_einval);
    }
    NDBG( ("NET", "nif_getnameinfo -> (try) decode flags\r\n") );
    if (!decode_nameinfo_flags(env, eFlags, &flags))
        return enif_make_badarg(env);
    result = enet_getnameinfo(env, &sa, saLen, flags);
    NDBG( ("NET",
           "nif_getnameinfo -> done when result: "
           "\r\n   %T\r\n", result) );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
#if defined(HAVE_GETNAMEINFO)
static
ERL_NIF_TERM enet_getnameinfo(ErlNifEnv*          env,
                              const ESockAddress* saP,
                              SOCKLEN_T           saLen,
                              int                 flags)
{
    ERL_NIF_TERM result;
    char         host[HOSTNAME_LEN];
    SOCKLEN_T    hostLen = sizeof(host);
    char         serv[SERVICE_LEN];
    SOCKLEN_T    servLen = sizeof(serv);
    int res = getnameinfo((struct sockaddr*) saP, saLen,
                          host, hostLen,
                          serv, servLen,
                          flags);
    NDBG( ("NET", "enet_getnameinfo -> res: %d\r\n", res) );
    switch (res) {
    case 0:
        {
            ERL_NIF_TERM keys[]  = {atom_host,      atom_service};
            ERL_NIF_TERM vals[]  = {MKS(env, host), MKS(env, serv)};
            size_t       numKeys = NUM(keys);
            ERL_NIF_TERM info;
            ESOCK_ASSERT( numKeys == NUM(vals) );
            ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &info) );
            result = esock_make_ok2(env, info);
        }
        break;
#if defined(EAI_AGAIN)
    case EAI_AGAIN:
        result = esock_make_error(env, esock_atom_eagain);
        break;
#endif
#if defined(EAI_BADFLAGS)
    case EAI_BADFLAGS:
        result = esock_make_error(env, atom_ebadflags);
        break;
#endif
#if defined(EAI_FAIL)
    case EAI_FAIL:
        result = esock_make_error(env, atom_efail);
        break;
#endif
#if defined(EAI_FAMILY)
    case EAI_FAMILY:
        result = esock_make_error(env, atom_efamily);
        break;
#endif
#if defined(EAI_MEMORY)
    case EAI_MEMORY:
        result = esock_make_error(env, atom_emem);
        break;
#endif
#if !defined(__WIN32__) && defined(EAI_NONAME)
    case EAI_NONAME:
        result = esock_make_error(env, atom_enoname);
        break;
#endif
#if defined(EAI_OVERFLOW)
    case EAI_OVERFLOW:
        result = esock_make_error(env, atom_eoverflow);
        break;
#endif
#if defined(EAI_SYSTEM)
    case EAI_SYSTEM:
        result = esock_make_error_errno(env, get_errno());
        break;
#endif
    default:
        result = esock_make_error(env, esock_atom_einval);
        break;
    }
    return result;
}
#endif
static
ERL_NIF_TERM nif_getaddrinfo(ErlNifEnv*         env,
                             int                argc,
                             const ERL_NIF_TERM argv[])
{
#if defined(HAVE_GETADDRINFO)
    ERL_NIF_TERM     result, eHostName, eServName;
    char*            hostName;
    char*            servName;
    NDBG( ("NET", "nif_getaddrinfo -> entry (%d)\r\n", argc) );
    if (argc != 3) {
        return enif_make_badarg(env);
    }
    eHostName = argv[0];
    eServName = argv[1];
    NDBG( ("NET",
           "nif_getaddrinfo -> "
           "\r\n   ehost:    %T"
           "\r\n   eservice: %T"
           "\r\n   ehints:   %T"
           "\r\n", argv[0], argv[1], argv[2]) );
    if (!decode_addrinfo_string(env, eHostName, &hostName))
        return enif_make_badarg(env);
    if (!decode_addrinfo_string(env, eServName, &servName))
        return enif_make_badarg(env);
    if ((hostName == NULL) && (servName == NULL))
        return enif_make_badarg(env);
    result = enet_getaddrinfo(env, hostName, servName);
    if (hostName != NULL)
        FREE(hostName);
    if (servName != NULL)
        FREE(servName);
    NDBG( ("NET",
           "nif_getaddrinfo -> done when result: "
           "\r\n   %T\r\n", result) );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
#if defined(HAVE_GETADDRINFO)
static
ERL_NIF_TERM enet_getaddrinfo(ErlNifEnv* env,
                              char*      host,
                              char*      serv)
{
    ERL_NIF_TERM     result;
    struct addrinfo* addrInfoP;
    int              res;
    NDBG( ("NET", "enet_getaddrinfo -> entry with"
           "\r\n   host: %s"
           "\r\n   serv: %s"
           "\r\n",
           ((host == NULL) ? "NULL" : host),
           ((serv == NULL) ? "NULL" : serv)) );
    res = getaddrinfo(host, serv, NULL, &addrInfoP);
    NDBG( ("NET", "enet_getaddrinfo -> res: %d\r\n", res) );
    switch (res) {
    case 0:
        {
            ERL_NIF_TERM addrInfo = encode_address_infos(env, addrInfoP);
            freeaddrinfo(addrInfoP);
            result = esock_make_ok2(env, addrInfo);
        }
        break;
#if defined(EAI_ADDRFAMILY)
    case EAI_ADDRFAMILY:
        result = esock_make_error(env, atom_eaddrfamily);
        break;
#endif
#if defined(EAI_AGAIN)
    case EAI_AGAIN:
        result = esock_make_error(env, esock_atom_eagain);
        break;
#endif
#if defined(EAI_BADFLAGS)
    case EAI_BADFLAGS:
        result = esock_make_error(env, atom_ebadflags);
        break;
#endif
#if defined(EAI_FAIL)
    case EAI_FAIL:
        result = esock_make_error(env, atom_efail);
        break;
#endif
#if defined(EAI_FAMILY)
    case EAI_FAMILY:
        result = esock_make_error(env, atom_efamily);
        break;
#endif
#if defined(EAI_MEMORY)
    case EAI_MEMORY:
        result = esock_make_error(env, atom_emem);
        break;
#endif
#if defined(EAI_NODATA)
    case EAI_NODATA:
        result = esock_make_error(env, atom_enodata);
        break;
#endif
#if !defined(__WIN32__) && defined(EAI_NONAME)
    case EAI_NONAME:
        result = esock_make_error(env, atom_enoname);
        break;
#endif
#if defined(EAI_SERVICE)
    case EAI_SERVICE:
        result = esock_make_error(env, atom_eservice);
        break;
#endif
#if defined(EAI_SOCKTYPE)
    case EAI_SOCKTYPE:
        result = esock_make_error(env, atom_esocktype);
        break;
#endif
#if defined(EAI_SYSTEM)
    case EAI_SYSTEM:
        result = esock_make_error(env, atom_esystem);
        break;
#endif
    default:
        result = esock_make_error(env, esock_atom_einval);
        break;
    }
    return result;
}
#endif
static
ERL_NIF_TERM nif_getifaddrs(ErlNifEnv*         env,
                            int                argc,
                            const ERL_NIF_TERM argv[])
{
#if defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#elif defined(HAVE_GETIFADDRS) || defined(__PASE__)
    ERL_NIF_TERM extra;
    BOOLEAN_T    dbg;
    char*        netns;
    ERL_NIF_TERM result;
    NDBG( ("NET", "nif_getifaddrs -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !IS_MAP(env,  argv[0])) {
        return enif_make_badarg(env);
    }
    extra = argv[0];
    dbg = enet_getifaddrs_debug(env, extra);
#ifdef HAVE_SETNS
    if (!enet_getifaddrs_netns(env, extra, &netns)) {
        NDBG2( dbg, ("NET", "nif_getifaddrs -> namespace: %s\r\n", netns) );
        return enif_make_badarg(env);
    }
#else
    netns = NULL;
#endif
    result = enet_getifaddrs(env, dbg, netns);
    NDBG2( dbg, ("NET", "nif_getifaddrs -> done\r\n") );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
static
BOOLEAN_T enet_getifaddrs_debug(ErlNifEnv* env, ERL_NIF_TERM map)
{
    ERL_NIF_TERM key = enif_make_atom(env, "debug");
    return esock_get_bool_from_map(env, map, key, FALSE);
}
#if defined(HAVE_GETIFADDRS) || defined(__PASE__)
#ifdef HAVE_SETNS
static
BOOLEAN_T enet_getifaddrs_netns(ErlNifEnv* env, ERL_NIF_TERM map, char** netns)
{
    ERL_NIF_TERM       key      = enif_make_atom(env, "netns");
    ErlNifCharEncoding encoding = ERL_NIF_LATIN1;
    return esock_get_string_from_map(env, map, key, encoding, netns);
}
#endif
static
ERL_NIF_TERM enet_getifaddrs(ErlNifEnv* env, BOOLEAN_T dbg, char* netns)
{
    ERL_NIF_TERM    result;
    struct ifaddrs* ifap;
    int             save_errno;
#ifdef HAVE_SETNS
    int             current_ns = 0;
#endif
    NDBG2( dbg,
           ("NET", "enet_getifaddrs -> entry with"
            "\r\n   netns: %s"
            "\r\n", ((netns == NULL) ? "NULL" : netns)) );
#ifdef HAVE_SETNS
    if ((netns != NULL) &&
        !change_network_namespace(netns, &current_ns, &save_errno))
        return esock_make_error_errno(env, save_errno);
#endif
#ifdef __PASE__
    if (0 == Qp2getifaddrs(&ifap)) {
#else
    if (0 == getifaddrs(&ifap)) {
#endif
        result = enet_getifaddrs_process(env, dbg, ifap);
#ifdef __PASE__
        Qp2freeifaddrs(ifap);
#else
        freeifaddrs(ifap);
#endif
    } else {
        save_errno = get_errno();
        NDBG2( dbg,
               ("NET", "enet_getifaddrs -> failed get addrs: %d", save_errno) );
        result = esock_make_error_errno(env, save_errno);
    }
#ifdef HAVE_SETNS
    if ((netns != NULL) &&
        !restore_network_namespace(current_ns, &save_errno))
        return esock_make_error_errno(env, save_errno);
    if (netns != NULL)
        FREE(netns);
#endif
    NDBG2( dbg, ("NET", "enet_getifaddrs -> done\r\n") );
    return result;
}
static
ERL_NIF_TERM enet_getifaddrs_process(ErlNifEnv*      env,
                                     BOOLEAN_T       dbg,
                                     struct ifaddrs* ifap)
{
    ERL_NIF_TERM result;
    unsigned int len = ((ifap == NULL) ? 0 : enet_getifaddrs_length(ifap));
    NDBG2( dbg, ("NET", "enet_getifaddrs_process -> len: %d\r\n", len) );
    if (len > 0) {
      ERL_NIF_TERM*   array = MALLOC(len * sizeof(ERL_NIF_TERM));
      unsigned int    i     = 0;
      struct ifaddrs* p     = ifap;
        while (i < len) {
            ERL_NIF_TERM entry;
            NDBG2( dbg,
                   ("NET",
                    "enet_getifaddrs_process -> encode entry %d\r\n", i) );
            encode_ifaddrs(env, p, &entry);
            NDBG2( dbg,
                   ("NET", "enet_getifaddrs_process -> new entry (%d):"
                    "\r\n   %T"
                    "\r\n", i, entry) );
            array[i] = entry;
            p = p->ifa_next;
            i++;
        }
        NDBG2( dbg,
               ("NET",
                "enet_getifaddrs_process -> all (%d) entries processed\r\n",
                len) );
        result = esock_make_ok2(env, MKLA(env, array, len));
        FREE(array);
    } else {
        result = esock_make_ok2(env, MKEL(env));
    }
    NDBG2( dbg, ("NET", "enet_getifaddrs_process -> done\r\n") );
    return result;
}
static
unsigned int enet_getifaddrs_length(struct ifaddrs* ifap)
{
    unsigned int    len = 1;
    struct ifaddrs* tmp;
    BOOLEAN_T       done = FALSE;
    tmp = ifap;
    while (!done) {
        if (tmp->ifa_next != NULL) {
            len++;
            tmp = tmp->ifa_next;
        } else {
            done = TRUE;
        }
    }
    return len;
}
static
void encode_ifaddrs(ErlNifEnv*      env,
                    struct ifaddrs* ifap,
                    ERL_NIF_TERM*   eifa)
{
    ERL_NIF_TERM ename, eflags, eaddr, enetmask, eifu_key, eifu_value, edata;
    ERL_NIF_TERM eifAddrs;
    BOOLEAN_T    extraAddr;
    NDBG( ("NET", "encode_ifaddrs -> entry\r\n") );
    ename     = encode_ifaddrs_name(env,  ifap->ifa_name);
    NDBG( ("NET", "encode_ifaddrs -> name: %T\r\n", ename) );
    eflags    = encode_ifaddrs_flags(env, ifap->ifa_flags);
    NDBG( ("NET", "encode_ifaddrs -> flags: %T\r\n", eflags) );
    eaddr     = encode_ifaddrs_addr(env,  ifap->ifa_addr);
    NDBG( ("NET", "encode_ifaddrs -> addr: "
           "\r\n   %T"
           "\r\n", eaddr) );
    if ((ifap->ifa_addr != NULL) &&
        (((ESockAddress*)ifap->ifa_addr)->sa.sa_family == AF_INET)) {
        if ((ifap->ifa_netmask != NULL) &&
            (((ESockAddress*)ifap->ifa_netmask)->sa.sa_family == AF_UNSPEC)) {
            ((ESockAddress*)ifap->ifa_netmask)->sa.sa_family = AF_INET;
        }
    }
    enetmask  = encode_ifaddrs_addr(env,  ifap->ifa_netmask);
    NDBG( ("NET", "encode_ifaddrs -> netmask: "
           "\r\n   %T"
           "\r\n", enetmask) );
    if (ifap->ifa_dstaddr && (ifap->ifa_flags & IFF_POINTOPOINT)) {
        NDBG( ("NET", "encode_ifaddrs -> try encode dest addr\r\n") );
        if (((ESockAddress*)ifap->ifa_dstaddr)->sa.sa_family == AF_UNSPEC)
            ((ESockAddress*)ifap->ifa_dstaddr)->sa.sa_family = AF_INET;
        extraAddr  = TRUE;
        eifu_key   = atom_dstaddr;
        eifu_value = encode_ifaddrs_addr(env, ifap->ifa_dstaddr);
        NDBG( ("NET", "encode_ifaddrs -> dest addr: "
               "\r\n   %T"
               "\r\n", eifu_value) );
    } else if (ifap->ifa_broadaddr && (ifap->ifa_flags & IFF_BROADCAST)) {
        NDBG( ("NET", "encode_ifaddrs -> try encode broad addr\r\n") );
        extraAddr  = TRUE;
        eifu_key   = atom_broadaddr;
        eifu_value = encode_ifaddrs_addr(env, ifap->ifa_broadaddr);
        NDBG( ("NET", "encode_ifaddrs -> broad addr: "
               "\r\n   %T"
               "\r\n", eifu_value) );
    } else {
        extraAddr  = FALSE;
        eifu_key   = esock_atom_undefined;
        eifu_value = esock_atom_undefined;
    }
    if (extraAddr) {
        NDBG( ("NET", "encode_ifaddrs -> ifu: "
               "\r\n   key: %T"
               "\r\n   val: %T"
               "\r\n", eifu_key, eifu_value) );
    }
    edata = esock_atom_undefined;
    make_ifaddrs(env,
                 ename, eflags, eaddr, enetmask,
                 eifu_key, eifu_value, edata,
                 &eifAddrs);
    NDBG( ("NET", "encode_ifaddrs -> encoded ifAddrs: "
           "\r\n   %T"
           "\r\n", eifAddrs) );
    *eifa = eifAddrs;
}
static
ERL_NIF_TERM encode_ifaddrs_name(ErlNifEnv* env, char* name)
{
  return ((name == NULL) ? esock_atom_undefined : MKS(env, name));
}
static
ERL_NIF_TERM encode_ifaddrs_flags(ErlNifEnv* env, unsigned int flags)
{
    SocketTArray ta = TARRAY_CREATE(16);
    ERL_NIF_TERM eflags;
#if defined(IFF_UP)
    if (flags & IFF_UP)
        TARRAY_ADD(ta, atom_up);
#endif
#if defined(IFF_BROADCAST)
    if (flags & IFF_BROADCAST)
        TARRAY_ADD(ta, atom_broadcast);
#endif
#if defined(IFF_DEBUG)
    if (flags & IFF_DEBUG)
        TARRAY_ADD(ta, atom_debug);
#endif
#if defined(IFF_LOOPBACK)
    if (flags & IFF_LOOPBACK)
        TARRAY_ADD(ta, esock_atom_loopback);
#endif
#if defined(IFF_POINTOPOINT)
    if (flags & IFF_POINTOPOINT)
        TARRAY_ADD(ta, atom_pointopoint);
#endif
#if defined(IFF_NOTRAILERS)
    if (flags & IFF_NOTRAILERS)
        TARRAY_ADD(ta, atom_notrailers);
#endif
#if defined(IFF_RUNNING)
    if (flags & IFF_RUNNING)
        TARRAY_ADD(ta, atom_running);
#endif
#if defined(IFF_NOARP)
    if (flags & IFF_NOARP)
        TARRAY_ADD(ta, atom_noarp);
#endif
#if defined(IFF_PROMISC)
    if (flags & IFF_PROMISC)
        TARRAY_ADD(ta, atom_promisc);
#endif
#if defined(IFF_MASTER)
    if (flags & IFF_MASTER)
        TARRAY_ADD(ta, atom_master);
#endif
#if defined(IFF_SLAVE)
    if (flags & IFF_SLAVE)
        TARRAY_ADD(ta, atom_slave);
#endif
#if defined(IFF_MULTICAST)
    if (flags & IFF_MULTICAST)
        TARRAY_ADD(ta, atom_multicast);
#endif
#if defined(IFF_PORTSEL)
    if (flags & IFF_PORTSEL)
        TARRAY_ADD(ta, atom_portsel);
#endif
#if defined(IFF_AUTOMEDIA)
    if (flags & IFF_AUTOMEDIA)
        TARRAY_ADD(ta, atom_automedia);
#endif
#if defined(IFF_DYNAMIC)
    if (flags & IFF_DYNAMIC)
        TARRAY_ADD(ta, atom_dynamic);
#endif
    TARRAY_TOLIST(ta, env, &eflags);
    return eflags;
}
static
ERL_NIF_TERM encode_ifaddrs_addr(ErlNifEnv*       env,
                                 struct sockaddr* sa)
{
    return encode_sockaddr(env, sa);
}
static
void make_ifaddrs(ErlNifEnv*    env,
                  ERL_NIF_TERM  ename,
                  ERL_NIF_TERM  eflags,
                  ERL_NIF_TERM  eaddr,
                  ERL_NIF_TERM  enetmask,
                  ERL_NIF_TERM  eifu_key,
                  ERL_NIF_TERM  eifu_value,
                  ERL_NIF_TERM  edata,
                  ERL_NIF_TERM* eifAddrs)
{
    ERL_NIF_TERM keys[6];
    ERL_NIF_TERM vals[6];
    size_t       len = NUM(keys);
    size_t       idx = 0;
    NDBG( ("NET", "make_ifaddrs -> name: %T\r\n", ename) );
    keys[idx] = esock_atom_name;
    vals[idx] = ename;
    idx++;
    NDBG( ("NET", "make_ifaddrs -> flags: %T\r\n", eflags) );
    keys[idx] = esock_atom_flags;
    vals[idx] = eflags;
    idx++;
    NDBG( ("NET", "make_ifaddrs -> addr: "
           "\r\n   %T"
           "\r\n", eaddr) );
    if (COMPARE(eaddr, esock_atom_undefined) != 0) {
        keys[idx] = esock_atom_addr;
        vals[idx] = eaddr;
        idx++;
    } else {
        len--;
    }
    NDBG( ("NET", "make_ifaddrs -> netmask: "
           "\r\n   %T"
           "\r\n", enetmask) );
    if (COMPARE(enetmask, esock_atom_undefined) != 0) {
        keys[idx] = atom_netmask;
        vals[idx] = enetmask;
        idx++;
    } else {
        len--;
    }
    NDBG( ("NET", "make_ifaddrs -> ifu: %T, %T\r\n", eifu_key, eifu_value) );
    if ((COMPARE(eifu_key, esock_atom_undefined) != 0) &&
        (COMPARE(eifu_value, esock_atom_undefined) != 0)) {
        keys[idx] = eifu_key;
        vals[idx] = eifu_value;
        idx++;
    } else {
        len--;
    }
    NDBG( ("NET", "make_ifaddrs -> data: %T\r\n", edata) );
    if (COMPARE(edata, esock_atom_undefined) != 0) {
        keys[idx] = esock_atom_data;
        vals[idx] = edata;
        idx++;
    } else {
        len--;
    }
    NDBG( ("NET", "make_ifaddrs -> construct ifa with:"
           "\r\n   len: %d"
           "\r\n"
           ) );
    ESOCK_ASSERT( MKMA(env, keys, vals, len, eifAddrs) );
}
#endif
static
ERL_NIF_TERM nif_get_adapters_addresses(ErlNifEnv*         env,
                                        int                argc,
                                        const ERL_NIF_TERM argv[])
{
#if !defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#else
    ERL_NIF_TERM result, eargs;
    ULONG        fam, flags;
    BOOLEAN_T    dbg;
    NDBG( ("NET", "nif_get_adapters_addresses -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !IS_MAP(env, argv[0])) {
        return enif_make_badarg(env);
    }
    eargs = argv[0];
    if (!enet_get_adapters_addresses_args_family(env, eargs, &fam))
        return enif_make_badarg(env);
    if (!enet_get_adapters_addresses_args_flags(env, eargs, &flags))
        return enif_make_badarg(env);
    dbg    = enet_get_adapters_addresses_args_debug(env, eargs);
    result = enet_get_adapters_addresses(env, dbg, fam, flags);
    NDBG2( dbg,
           ("NET",
            "nif_get_adapters_addresses -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
#endif
}
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_adapters_addresses_args_debug(ErlNifEnv*         env,
                                                 const ERL_NIF_TERM eargs)
{
    return get_debug(env, eargs);
}
#endif
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_adapters_addresses_args_family(ErlNifEnv*         env,
                                                  const ERL_NIF_TERM eargs,
                                                  ULONG*             fam)
{
    ERL_NIF_TERM key = esock_atom_family;
    ERL_NIF_TERM eval;
    DWORD        val;
    if (!GET_MAP_VAL(env, eargs, key, &eval)) {
        *fam = AF_UNSPEC;
        return TRUE;
    } else {
        if (!IS_ATOM(env, eval))
            return FALSE;
        if (IS_IDENTICAL(eval, esock_atom_unspec))
            val = AF_UNSPEC;
        else if (IS_IDENTICAL(eval, esock_atom_inet))
            val = AF_INET;
        else if (IS_IDENTICAL(eval, esock_atom_inet6))
            val = AF_INET6;
        else
            return FALSE;
        *fam = val;
        return TRUE;
    }
}
#endif
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_adapters_addresses_args_flags(ErlNifEnv*         env,
                                                  const ERL_NIF_TERM eargs,
                                                  ULONG*             flags)
{
    ERL_NIF_TERM eflags;
    ULONG        val = 0;
    if (!GET_MAP_VAL(env, eargs, esock_atom_flags, &eflags)) {
        *flags =
            GAA_FLAG_INCLUDE_PREFIX |
            GAA_FLAG_SKIP_ANYCAST |
            GAA_FLAG_SKIP_DNS_SERVER |
            GAA_FLAG_SKIP_FRIENDLY_NAME |
            GAA_FLAG_SKIP_MULTICAST;
        return TRUE;
    } else {
        if (!IS_MAP(env, eflags))
            return FALSE;
        if (esock_get_bool_from_map(env, eflags, atom_skip_unicast, FALSE))
            val |= GAA_FLAG_SKIP_UNICAST;
        if (esock_get_bool_from_map(env, eflags, atom_skip_anycast, TRUE))
            val |= GAA_FLAG_SKIP_ANYCAST;
        if (esock_get_bool_from_map(env, eflags, atom_skip_multicast, TRUE))
            val |= GAA_FLAG_SKIP_MULTICAST;
        if (esock_get_bool_from_map(env, eflags, atom_skip_dns_server, TRUE))
            val |= GAA_FLAG_SKIP_DNS_SERVER;
        if (esock_get_bool_from_map(env, eflags, atom_skip_friendly_name, TRUE))
            val |= GAA_FLAG_SKIP_FRIENDLY_NAME;
        if (esock_get_bool_from_map(env, eflags, atom_include_prefix, TRUE))
            val |= GAA_FLAG_INCLUDE_PREFIX;
        if (esock_get_bool_from_map(env, eflags, atom_include_wins_info, FALSE))
            val |= GAA_FLAG_INCLUDE_WINS_INFO;
        if (esock_get_bool_from_map(env, eflags, atom_include_gateways, FALSE))
            val |= GAA_FLAG_INCLUDE_GATEWAYS;
        if (esock_get_bool_from_map(env, eflags,
                                    atom_include_all_interfaces, FALSE))
            val |= GAA_FLAG_INCLUDE_ALL_INTERFACES;
        if (esock_get_bool_from_map(env, eflags,
                                    atom_include_all_compartments, FALSE))
            val |= GAA_FLAG_INCLUDE_ALL_COMPARTMENTS;
        if (esock_get_bool_from_map(env, eflags,
                                    atom_include_tunnel_bindingorder, FALSE))
            val |= GAA_FLAG_INCLUDE_TUNNEL_BINDINGORDER;
        *flags = val;
        return TRUE;
    }
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_get_adapters_addresses(ErlNifEnv* env,
                                         BOOLEAN_T  dbg,
                                         ULONG      family,
                                         ULONG      flags)
{
    int                   i;
    DWORD                 ret;
    unsigned long         ipAdAddrsSz = 16 * 1024;
    IP_ADAPTER_ADDRESSES* ipAdAddrsP;
    ERL_NIF_TERM          eret, addrs, result;
    NDBG2( dbg,
           ("NET", "enet_get_adapters_addresses -> entry with"
            "\r\n   family: %d"
            "\r\n   flags:  %d"
            "\r\n", family, flags) );
    ipAdAddrsP = (IP_ADAPTER_ADDRESSES*) MALLOC(ipAdAddrsSz);
    for (i = 17;  i;  i--) {
        NDBG2( dbg,
               ("NET", "enet_get_adapters_addresses -> [%d] try get adapters"
                "\r\n", i) );
        ret = GetAdaptersAddresses(family, flags, NULL,
                                   ipAdAddrsP, &ipAdAddrsSz);
        if (ret == NO_ERROR) {
            NDBG2( dbg,
                   ("NET", "enet_get_adapters_addresses -> got adapters"
                    "\r\n") );
            break;
        } else if (ret == ERROR_BUFFER_OVERFLOW) {
            NDBG2( dbg,
                   ("NET", "enet_get_adapters_addresses -> overflow - realloc"
                    "\r\n") );
            ipAdAddrsP = REALLOC(ipAdAddrsP, ipAdAddrsSz);
            continue;
        } else {
            NDBG2( dbg,
                   ("NET", "enet_get_adapters_addresses -> error: %d"
                    "\r\n", ret) );
            i = 0;
        }
        if (ret == NO_ERROR) break;
        if (ret == ERROR_BUFFER_OVERFLOW) continue;
        i = 0;
    }
    if (! i) {
        NDBG2( dbg,
               ("NET", "enet_get_adapters_addresses -> "
                "try encode error (%d)\r\n", ret) );
        FREE(ipAdAddrsP);
        switch (ret) {
        case ERROR_ADDRESS_NOT_ASSOCIATED:
            eret = atom_address_not_associated;
            break;
        case ERROR_BUFFER_OVERFLOW:
            eret = atom_insufficient_buffer;
            break;
        case ERROR_INVALID_PARAMETER:
            eret = atom_invalid_parameter;
            break;
        case ERROR_NO_DATA:
            eret = atom_no_data;
            break;
        case ERROR_NOT_ENOUGH_MEMORY:
            eret = atom_not_enough_memory;
            break;
        default:
            eret = MKI(env, ret);
            break;
        }
        result = esock_make_error(env, eret);
    } else {
        NDBG2( dbg,
               ("NET", "enet_get_adapters_addresses -> "
                "try encode addresses\r\n") );
        addrs  = enet_adapters_addresses_encode(env, dbg, ipAdAddrsP);
        result = esock_make_ok2(env, addrs);
    }
    NDBG2( dbg,
           ("NET", "enet_get_adapters_addresses -> done with:"
            "\r\n   result: %T"
            "\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_adapters_addresses_encode(ErlNifEnv*            env,
                                            BOOLEAN_T             dbg,
                                            IP_ADAPTER_ADDRESSES* ipAdAddrsP)
{
    SocketTArray          adapterArray = TARRAY_CREATE(16);
    IP_ADAPTER_ADDRESSES* addrsP       = ipAdAddrsP;
    ERL_NIF_TERM          entry, result;
    NDBG2( dbg, ("NET", "enet_get_adapters_addresses -> entry\r\n") );
    while (addrsP != NULL) {
        entry  = enet_adapter_addresses_encode(env, dbg, addrsP);
        NDBG2( dbg, ("NET", "enet_get_adapters_addresses -> entry encoded:"
                     "\r\n   Adapter Entry: %T"
                     "\r\n", entry) );
        TARRAY_ADD(adapterArray, entry);
        addrsP = addrsP->Next;
     }
     TARRAY_TOLIST(adapterArray, env, &result);
     NDBG2( dbg, ("NET", "enet_get_adapters_addresses -> done:"
                  "\r\n   %T"
                  "\r\n", result) );
     return result;
    }
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_adapter_addresses_encode(ErlNifEnv*            env,
                                           BOOLEAN_T             dbg,
                                           IP_ADAPTER_ADDRESSES* ipAdAddrsP)
{
    ERL_NIF_TERM ifIdx, name;
    ERL_NIF_TERM unicastAddrs, anycastAddrs, multicastAddrs, dnsServerAddrs;
    ERL_NIF_TERM dnsSuffix, description, flags, physAddr, fName, mtu, ifType;
    ERL_NIF_TERM operStatus, zoneIndices, ipv6IfIdx, prefixes;
    ERL_NIF_TERM map;
    ifIdx          = MKI(env, ipAdAddrsP->IfIndex);
    name           = MKS(env, ipAdAddrsP->AdapterName);
    unicastAddrs   = encode_adapter_unicast_addrs(env, ipAdAddrsP->FirstUnicastAddress);
    anycastAddrs   = encode_adapter_anycast_addrs(env, ipAdAddrsP->FirstAnycastAddress);
    multicastAddrs = encode_adapter_multicast_addrs(env, ipAdAddrsP->FirstMulticastAddress);
    dnsServerAddrs = encode_adapter_dns_server_addrs(env, ipAdAddrsP->FirstDnsServerAddress);
    dnsSuffix      = encode_wchar(env, ipAdAddrsP->DnsSuffix);
    description    = encode_wchar(env, ipAdAddrsP->Description);
    fName          = encode_wchar(env, ipAdAddrsP->FriendlyName);
    physAddr       = encode_uchar(env,
                                  ipAdAddrsP->PhysicalAddressLength,
                                  ipAdAddrsP->PhysicalAddress);
    flags          = encode_adapter_flags(env, ipAdAddrsP);
    mtu            = MKUI(env, ipAdAddrsP->Mtu);
    ifType         = encode_if_type(env, ipAdAddrsP->IfType);
    operStatus     = encode_if_oper_status(env, ipAdAddrsP->OperStatus);
    zoneIndices    = encode_adapter_zone_indices(env,
                                                 ipAdAddrsP->ZoneIndices,
                                                 NUM(ipAdAddrsP->ZoneIndices));
    ipv6IfIdx      = MKI(env, ipAdAddrsP->Ipv6IfIndex);
    prefixes       = encode_adapter_prefixes(env, ipAdAddrsP->FirstPrefix);
    {
        ERL_NIF_TERM keys[] = {atom_index,
                               esock_atom_name,
                               atom_unicast_addrs,
                               atom_anycast_addrs,
                               atom_multicast_addrs,
                               atom_dns_server_addrs,
                               atom_dns_suffix,
                               atom_description,
                               atom_friendly_name,
                               atom_phys_addr,
                               esock_atom_flags,
                               esock_atom_mtu,
                               esock_atom_type,
                               atom_oper_status,
                               atom_zone_indices,
                               atom_ipv6_index,
                               atom_prefixes
        };
        ERL_NIF_TERM vals[] = {ifIdx,
                               name,
                               unicastAddrs,
                               anycastAddrs,
                               multicastAddrs,
                               dnsServerAddrs,
                               dnsSuffix,
                               description,
                               fName,
                               physAddr,
                               flags,
                               mtu,
                               ifType,
                               operStatus,
                               zoneIndices,
                               ipv6IfIdx,
                               prefixes
        };
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &map) );
    }
    return map;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_flags(ErlNifEnv*            env,
                                  IP_ADAPTER_ADDRESSES* ipAdAddrsP)
{
    ERL_NIF_TERM ddnsEnabled, regAdSuffix, dhcpv4Enabled, recvOnly;
    ERL_NIF_TERM noMulticast, ipv6OtherStatefulCfg, netbiosOverTcpipEnabled;
    ERL_NIF_TERM ipv4Enabled, ipv6Enabled, ipv6ManagedAddrCfgSup;
    ERL_NIF_TERM eflags;
#if defined(ESOCK_WIN_XP)
    ddnsEnabled             = BOOL2ATOM(ipAdAddrsP->DdnsEnabled);
    regAdSuffix             = BOOL2ATOM(ipAdAddrsP->RegisterAdapterSuffix);
    dhcpv4Enabled           = BOOL2ATOM(ipAdAddrsP->Dhcpv4Enabled);
    recvOnly                = BOOL2ATOM(ipAdAddrsP->ReceiveOnly);
    noMulticast             = BOOL2ATOM(ipAdAddrsP->NoMulticast);
    ipv6OtherStatefulCfg    = BOOL2ATOM(ipAdAddrsP->Ipv6OtherStatefulConfig);
    netbiosOverTcpipEnabled = BOOL2ATOM(ipAdAddrsP->NetbiosOverTcpipEnabled);
    ipv4Enabled             = BOOL2ATOM(ipAdAddrsP->Ipv4Enabled);
    ipv6Enabled             = BOOL2ATOM(ipAdAddrsP->Ipv6Enabled);
    ipv6ManagedAddrCfgSup   = BOOL2ATOM(ipAdAddrsP->Ipv6ManagedAddressConfigurationSupported);
#else
    ddnsEnabled             = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_DDNS_ENABLED);
    regAdSuffix             = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_REGISTER_ADAPTER_SUFFIX);
    dhcpv4Enabled           = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_DHCP_ENABLED);
    recvOnly                = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_RECEIVE_ONLY);
    noMulticast             = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_NO_MULTICAST);
    ipv6OtherStatefulCfg    = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_IPV6_OTHER_STATEFUL_CONFIG);
    netbiosOverTcpipEnabled = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_NETBIOS_OVER_TCPIP_ENABLED);
    ipv4Enabled             = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_IPV4_ENABLED);
    ipv6Enabled             = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_IPV6_ENABLED);
    ipv6ManagedAddrCfgSup   = BOOL2ATOM(ipAdAddrsP->Flags & IP_ADAPTER_IPV6_MANAGE_ADDRESS_CONFIG);
#endif
    {
        ERL_NIF_TERM keys[] = {atom_ddns_enabled,
                               atom_register_adapter_suffix,
                               atom_dhcp_v4_enabled,
                               atom_receive_only,
                               atom_no_multicast,
                               atom_ipv6_other_stateful_config,
                               atom_netbios_over_tcpip_enabled,
                               atom_ipv4_enabled,
                               atom_ipv6_enabled,
                               atom_ipv6_managed_address_config_supported};
        ERL_NIF_TERM vals[] = {ddnsEnabled,
                               regAdSuffix,
                               dhcpv4Enabled,
                               recvOnly,
                               noMulticast,
                               ipv6OtherStatefulCfg,
                               netbiosOverTcpipEnabled,
                               ipv4Enabled,
                               ipv6Enabled,
                               ipv6ManagedAddrCfgSup};
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &eflags) );
    }
    return eflags;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addrs(ErlNifEnv*                  env,
                                          IP_ADAPTER_UNICAST_ADDRESS* firstP)
{
    IP_ADAPTER_UNICAST_ADDRESS* tmp = firstP;
    SocketTArray                ta  = TARRAY_CREATE(16);
    ERL_NIF_TERM eaddrs;
    while (tmp != NULL) {
        TARRAY_ADD(ta, encode_adapter_unicast_addr(env, tmp));
        tmp = tmp->Next;
    }
    TARRAY_TOLIST(ta, env, &eaddrs);
    return eaddrs;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr(ErlNifEnv*                  env,
                                         IP_ADAPTER_UNICAST_ADDRESS* addrP)
{
    ERL_NIF_TERM eflags, esa, eporig, esorig, edstate, evlt, eplt, ellt;
    ERL_NIF_TERM eua;
    eflags  = encode_adapter_unicast_addr_flags(env, addrP->Flags);
    esa   = encode_adapter_unicast_addr_sockaddr(env,
                                                 addrP->Address.lpSockaddr);
    eporig  = encode_adapter_unicast_addr_porig(env, addrP->PrefixOrigin);
    esorig  = encode_adapter_unicast_addr_sorig(env, addrP->SuffixOrigin);
    edstate = encode_adapter_unicast_addr_dad_state(env, addrP->DadState);
    evlt    = MKUL(env, addrP->ValidLifetime);
    eplt    = MKUL(env, addrP->PreferredLifetime);
    ellt    = MKUL(env, addrP->LeaseLifetime);
    {
        ERL_NIF_TERM keys[] = {esock_atom_flags,
                               esock_atom_addr,
                               atom_prefix_origin,
                               atom_suffix_origin,
                               atom_dad_state,
                               atom_valid_lifetime,
                               atom_preferred_lifetime,
                               atom_lease_lifetime
        };
        ERL_NIF_TERM vals[] = {eflags,
                               esa,
                               eporig,
                               esorig,
                               edstate,
                               evlt,
                               eplt,
                               ellt
        };
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &eua) );
    }
    return eua;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr_flags(ErlNifEnv* env,
                                               DWORD      flags)
{
    ERL_NIF_TERM map;
    ERL_NIF_TERM dnsEl   = BOOL2ATOM(flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE);
    ERL_NIF_TERM trans   = BOOL2ATOM(flags & IP_ADAPTER_ADDRESS_TRANSIENT);
    ERL_NIF_TERM keys[]  = {atom_dns_eligible, atom_transient};
    ERL_NIF_TERM vals[]  = {dnsEl, trans};
    size_t       numKeys = NUM(keys);
    size_t       numVals = NUM(vals);
    ESOCK_ASSERT( numKeys == numVals );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &map) );
    return map;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr_sockaddr(ErlNifEnv*       env,
                                                  struct sockaddr* addrP)
{
    return encode_sockaddr(env, addrP);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr_porig(ErlNifEnv*       env,
                                               IP_PREFIX_ORIGIN porig)
{
    ERL_NIF_TERM eporig;
    switch (porig) {
    case IpPrefixOriginOther:
        eporig = atom_other;
        break;
    case IpPrefixOriginManual:
        eporig = atom_manual;
        break;
    case IpPrefixOriginWellKnown:
        eporig = atom_well_known;
        break;
    case IpPrefixOriginDhcp:
        eporig = atom_dhcp;
        break;
    case IpPrefixOriginRouterAdvertisement:
        eporig = atom_router_advertisement;
        break;
    case IpPrefixOriginUnchanged:
        eporig = atom_unchanged;
        break;
    default:
        eporig = MKI(env, (int) porig);
        break;
    }
    return eporig;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr_sorig(ErlNifEnv*       env,
                                               IP_SUFFIX_ORIGIN sorig)
{
    ERL_NIF_TERM esorig;
    switch (sorig) {
    case IpSuffixOriginOther:
        esorig = atom_other;
        break;
    case IpSuffixOriginManual:
        esorig = atom_manual;
        break;
    case IpSuffixOriginWellKnown:
        esorig = atom_well_known;
        break;
    case IpSuffixOriginDhcp:
        esorig = atom_dhcp;
        break;
    case IpSuffixOriginLinkLayerAddress:
        esorig = atom_link_layer_address;
        break;
    case IpSuffixOriginRandom:
        esorig = atom_random;
        break;
    case IpSuffixOriginUnchanged:
        esorig = atom_unchanged;
        break;
    default:
        esorig = MKI(env, (int) sorig);
        break;
    }
    return esorig;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_unicast_addr_dad_state(ErlNifEnv*   env,
                                                   IP_DAD_STATE dstate)
{
    ERL_NIF_TERM edstate;
    switch (dstate) {
    case IpDadStateInvalid:
        edstate = atom_invalid;
        break;
    case IpDadStateTentative:
        edstate = atom_tentative;
        break;
    case IpDadStateDuplicate:
        edstate = atom_duplicate;
        break;
    case IpDadStateDeprecated:
        edstate = atom_deprecated;
        break;
    case IpDadStatePreferred:
        edstate = atom_preferred;
        break;
    default:
        edstate = MKI(env, (int) dstate);
        break;
    }
    return edstate;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_anycast_addrs(ErlNifEnv*                  env,
                                          IP_ADAPTER_ANYCAST_ADDRESS* firstP)
{
    IP_ADAPTER_ANYCAST_ADDRESS* tmp = firstP;
    SocketTArray                ta  = TARRAY_CREATE(16);
    ERL_NIF_TERM eaddrs;
    while (tmp != NULL) {
        TARRAY_ADD(ta, encode_adapter_anycast_addr(env, tmp));
        tmp = tmp->Next;
    }
    TARRAY_TOLIST(ta, env, &eaddrs);
    return eaddrs;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_anycast_addr(ErlNifEnv*                  env,
                                         IP_ADAPTER_ANYCAST_ADDRESS* addrP)
{
    ERL_NIF_TERM eflags, esa;
    ERL_NIF_TERM eaa;
    eflags = encode_adapter_anycast_addr_flags(env, addrP->Flags);
    esa    = encode_adapter_anycast_addr_sockaddr(env,
                                                  addrP->Address.lpSockaddr);
    {
        ERL_NIF_TERM keys[] = {esock_atom_flags,
                               esock_atom_addr};
        ERL_NIF_TERM vals[] = {eflags,
                               esa};
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &eaa) );
    }
    return eaa;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_anycast_addr_flags(ErlNifEnv* env,
                                               DWORD      flags)
{
    return encode_adapter_unicast_addr_flags(env, flags);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_anycast_addr_sockaddr(ErlNifEnv*       env,
                                                  struct sockaddr* addrP)
{
    return encode_sockaddr(env, addrP);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_multicast_addrs(ErlNifEnv*                    env,
                                            IP_ADAPTER_MULTICAST_ADDRESS* firstP)
{
    IP_ADAPTER_MULTICAST_ADDRESS* tmp = firstP;
    SocketTArray                  ta  = TARRAY_CREATE(16);
    ERL_NIF_TERM eaddrs;
    while (tmp != NULL) {
        TARRAY_ADD(ta, encode_adapter_multicast_addr(env, tmp));
        tmp = tmp->Next;
    }
    TARRAY_TOLIST(ta, env, &eaddrs);
    return eaddrs;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_multicast_addr(ErlNifEnv*                    env,
                                           IP_ADAPTER_MULTICAST_ADDRESS* addrP)
{
    ERL_NIF_TERM eflags, esa;
    ERL_NIF_TERM ema;
    eflags = encode_adapter_multicast_addr_flags(env, addrP->Flags);
    esa    = encode_adapter_multicast_addr_sockaddr(env,
                                                    addrP->Address.lpSockaddr);
    {
        ERL_NIF_TERM keys[] = {esock_atom_flags,
                               esock_atom_addr};
        ERL_NIF_TERM vals[] = {eflags,
                               esa};
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &ema) );
    }
    return ema;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_multicast_addr_flags(ErlNifEnv* env,
                                                 DWORD      flags)
{
    return encode_adapter_unicast_addr_flags(env, flags);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_multicast_addr_sockaddr(ErlNifEnv*       env,
                                                    struct sockaddr* addrP)
{
    return encode_sockaddr(env, addrP);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_dns_server_addrs(ErlNifEnv*                     env,
                                             IP_ADAPTER_DNS_SERVER_ADDRESS* firstP)
{
    IP_ADAPTER_DNS_SERVER_ADDRESS* tmp = firstP;
    SocketTArray                   ta  = TARRAY_CREATE(16);
    ERL_NIF_TERM eaddrs;
    while (tmp != NULL) {
        TARRAY_ADD(ta, encode_adapter_dns_server_addr(env, tmp));
        tmp = tmp->Next;
    }
    TARRAY_TOLIST(ta, env, &eaddrs);
    return eaddrs;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_dns_server_addr(ErlNifEnv*                     env,
                                            IP_ADAPTER_DNS_SERVER_ADDRESS* addrP)
{
    ERL_NIF_TERM esa;
    ERL_NIF_TERM edsa;
    esa = encode_adapter_dns_server_addr_sockaddr(env,
                                                  addrP->Address.lpSockaddr);
    {
        ERL_NIF_TERM keys[] = {esock_atom_addr};
        ERL_NIF_TERM vals[] = {esa};
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &edsa) );
    }
    return edsa;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_dns_server_addr_sockaddr(ErlNifEnv*       env,
                                                     struct sockaddr* addrP)
{
    return encode_sockaddr(env, addrP);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_if_oper_status(ErlNifEnv* env,
                                   DWORD      status)
{
    ERL_NIF_TERM estatus;
    switch (status) {
    case IfOperStatusUp:
        estatus = esock_atom_up;
        break;
    case IfOperStatusDown:
        estatus = atom_down;
        break;
    case IfOperStatusTesting:
        estatus = atom_testing;
        break;
    case IfOperStatusUnknown:
        estatus = atom_unknown;
        break;
    case IfOperStatusDormant:
        estatus = esock_atom_dormant;
        break;
    case IfOperStatusNotPresent:
        estatus = atom_not_present;
        break;
    case IfOperStatusLowerLayerDown:
        estatus = atom_lower_layer_down;
        break;
    default:
        estatus = MKUI(env, status);
        break;
    }
    return estatus;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_zone_indices(ErlNifEnv* env,
                                         DWORD*     zoneIndices,
                                         DWORD      len)
{
    SocketTArray ta = TARRAY_CREATE(len);
    DWORD        i;
    ERL_NIF_TERM ezi;
    for (i = 0; i < len; i++) {
        TARRAY_ADD(ta, MKUI(env, zoneIndices[i]));
    }
    TARRAY_TOLIST(ta, env, &ezi);
    return ezi;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_prefixes(ErlNifEnv*         env,
                                     IP_ADAPTER_PREFIX* firstP)
{
    IP_ADAPTER_PREFIX* tmp = firstP;
    SocketTArray       ta  = TARRAY_CREATE(16);
    ERL_NIF_TERM       eprefs;
    while (tmp != NULL) {
        TARRAY_ADD(ta, encode_adapter_prefix(env, tmp));
        tmp = tmp->Next;
    }
    TARRAY_TOLIST(ta, env, &eprefs);
    return eprefs;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_prefix(ErlNifEnv*         env,
                                   IP_ADAPTER_PREFIX* prefP)
{
    ERL_NIF_TERM esa, eplen;
    ERL_NIF_TERM epref;
    esa   = encode_adapter_prefix_sockaddr(env, prefP->Address.lpSockaddr);
    eplen = MKUI(env, prefP->PrefixLength);
        {
            ERL_NIF_TERM keys[] = {esock_atom_addr, atom_length};
            ERL_NIF_TERM vals[] = {esa,             eplen};
            size_t       numKeys = NUM(keys);
            size_t       numVals = NUM(vals);
            ESOCK_ASSERT( numKeys == numVals );
            ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &epref) );
        }
    return epref;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_prefix_sockaddr(ErlNifEnv*       env,
                                            struct sockaddr* addrP)
{
    return encode_sockaddr(env, addrP);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_adapter_encode_name(ErlNifEnv* env, WCHAR* name)
{
    return encode_wchar(env, name);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_adapter_encode_friendly_name(ErlNifEnv* env, WCHAR* fname)
{
    return encode_wchar(env, fname);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_adapter_index_map_name(ErlNifEnv* env, WCHAR* name)
{
    return encode_wchar(env, name);
}
#endif
#if defined(__WIN32__)
static
void make_adapter_index_map(ErlNifEnv*    env,
                            ERL_NIF_TERM  eindex,
                            ERL_NIF_TERM  ename,
                            ERL_NIF_TERM* emap)
{
    ERL_NIF_TERM keys[2];
    ERL_NIF_TERM vals[2];
    size_t       len = NUM(keys);
    NDBG( ("NET", "make_adapter_index_map -> index: %T\r\n", eindex) );
    keys[0] = atom_index;
    vals[0] = eindex;
    NDBG( ("NET", "make_adapter_index_map -> name: %T\r\n", ename) );
    keys[1] = esock_atom_name;
    vals[1] = ename;
    ESOCK_ASSERT( MKMA(env, keys, vals, len, emap) );
}
#endif
static
ERL_NIF_TERM nif_get_if_entry(ErlNifEnv*         env,
                              int                argc,
                              const ERL_NIF_TERM argv[])
{
#if !defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#else
    ERL_NIF_TERM result, eargs;
    DWORD        index;
    BOOLEAN_T    dbg;
    NDBG( ("NET", "nif_get_if_entry -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !IS_MAP(env, argv[0])) {
        return enif_make_badarg(env);
    }
    eargs = argv[0];
    if (!enet_get_if_entry_args_index(env, eargs, &index))
        return enif_make_badarg(env);
    dbg    = enet_get_if_entry_args_debug(env, eargs);
    result = enet_get_if_entry(env, dbg, index);
    NDBG2( dbg,
           ("NET",
            "nif_get_if_entry -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
#endif
}
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_if_entry_args_index(ErlNifEnv*         env,
                                       const ERL_NIF_TERM eargs,
                                       DWORD*             index)
{
    ERL_NIF_TERM key = atom_index;
    ERL_NIF_TERM eval;
    DWORD        val;
    if (!GET_MAP_VAL(env, eargs, key, &eval)) {
        return FALSE;
    } else {
        if (!IS_NUM(env, eval))
            return FALSE;
        if (!GET_UINT(env, eval, &val))
            return FALSE;
        *index = val;
        return TRUE;
    }
}
#endif
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_if_entry_args_debug(ErlNifEnv*         env,
                                       const ERL_NIF_TERM eargs)
{
    return get_debug(env, eargs);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_get_if_entry(ErlNifEnv* env,
                               BOOLEAN_T  dbg,
                               DWORD      index)
{
    DWORD        ret;
    MIB_IFROW    ifRow;
    ERL_NIF_TERM eifRow, result;
    NDBG2( dbg,
           ("NET", "nif_get_if_entry -> entry with"
            "\r\n   index: %d\r\n", index) );
    sys_memzero(&ifRow, sizeof(ifRow));
    ifRow.dwIndex = index;
    ret = GetIfEntry(&ifRow);
    switch (ret) {
    case NO_ERROR:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> success") );
        eifRow = enet_if_row_encode(env, dbg, &ifRow);
        result = esock_make_ok2(env, eifRow);
        break;
    case ERROR_CAN_NOT_COMPLETE:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: can-not-complete") );
        result = esock_make_error(env, atom_can_not_complete);
        break;
    case ERROR_INVALID_DATA:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: invalid-data") );
        result = esock_make_error(env, atom_invalid_data);
        break;
    case ERROR_INVALID_PARAMETER:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: invalid-parameter") );
        result = esock_make_error(env, atom_invalid_parameter);
        break;
    case ERROR_NOT_FOUND:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: not-found") );
        result = esock_make_error(env, esock_atom_not_found);
        break;
    case ERROR_FILE_NOT_FOUND:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: file-not-found") );
        result = esock_make_error(env, esock_atom_file_not_found);
        break;
    case ERROR_NOT_SUPPORTED:
        NDBG2( dbg, ("NET", "nif_get_if_entry -> error: not-supported") );
        result = esock_make_error(env, atom_not_supported);
        break;
    default:
        NDBG2( dbg,
               ("NET", "nif_get_if_entry -> error: %d\r\n", ret) );
        result = esock_make_error(env, MKI(env, ret));
        break;
    }
    NDBG2( dbg,
           ("NET", "nif_get_if_entry -> done when:"
            "\r\n   result: %T\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_if_row_encode(ErlNifEnv* env,
                                BOOLEAN_T  dbg,
                                MIB_IFROW* rowP)
{
    ERL_NIF_TERM eName, eIndex, eType, eMtu, eSpeed, ePhuysAddr, eAdminStatus;
    ERL_NIF_TERM eOperStatus, eLastChange, eInOctets, eInUcastPkts;
    ERL_NIF_TERM eInNUcastPkts, eInDiscards, eInError, eInUnknownProtos;
    ERL_NIF_TERM eOutOcts, eOutUcastPkts, eOutNUcastPkts, eOutDiscards;
    ERL_NIF_TERM eOutErrors, eOutQLen, eDescr;
    ERL_NIF_TERM erow;
    NDBG2( dbg, ("NET", "enet_if_row_encode -> entry\r\n") );
    eName            = encode_wchar(env, rowP->wszName);
    eIndex           = MKUI(env, rowP->dwIndex);
    eType            = encode_if_type(env, rowP->dwType);
    eMtu             = MKUI(env, rowP->dwMtu);
    eSpeed           = MKUI(env, rowP->dwSpeed);
    ePhuysAddr       = encode_if_row_phys_address(env,
                                                  rowP->dwPhysAddrLen,
                                                  rowP->bPhysAddr);
    eAdminStatus     = encode_if_admin_status(env, rowP->dwAdminStatus);
    eOperStatus      = encode_internal_if_oper_status(env, rowP->dwOperStatus);
    eLastChange      = MKUI(env, rowP->dwLastChange);
    eInOctets        = MKUI(env, rowP->dwInOctets);
    eInUcastPkts     = MKUI(env, rowP->dwInUcastPkts);
    eInNUcastPkts    = MKUI(env, rowP->dwInNUcastPkts);
    eInDiscards      = MKUI(env, rowP->dwInDiscards);
    eInError         = MKUI(env, rowP->dwInErrors);
    eInUnknownProtos = MKUI(env, rowP->dwInUnknownProtos);
    eOutOcts         = MKUI(env, rowP->dwOutOctets);
    eOutUcastPkts    = MKUI(env, rowP->dwOutUcastPkts);
    eOutNUcastPkts   = MKUI(env, rowP->dwOutNUcastPkts);
    eOutDiscards     = MKUI(env, rowP->dwOutDiscards);
    eOutErrors       = MKUI(env, rowP->dwOutErrors);
    eOutQLen         = MKUI(env, rowP->dwOutQLen);
    eDescr           = encode_if_row_description(env,
                                                 rowP->dwDescrLen,
                                                 rowP->bDescr);
    {
        ERL_NIF_TERM keys[] = {esock_atom_name,
                               atom_index,
                               esock_atom_type,
                               esock_atom_mtu,
                               atom_speed,
                               atom_phys_addr,
                               atom_admin_status,
                               atom_internal_oper_status,
                               atom_last_change,
                               atom_in_octets,
                               atom_in_ucast_pkts,
                               atom_in_nucast_pkts,
                               atom_in_discards,
                               atom_in_errors,
                               atom_in_unknown_protos,
                               atom_out_octets,
                               atom_out_ucast_pkts,
                               atom_out_nucast_pkts,
                               atom_out_discards,
                               atom_out_errors,
                               atom_out_qlen,
                               atom_description};
        ERL_NIF_TERM vals[] = {eName,
                               eIndex,
                               eType,
                               eMtu,
                               eSpeed,
                               ePhuysAddr,
                               eAdminStatus,
                               eOperStatus,
                               eLastChange,
                               eInOctets,
                               eInUcastPkts,
                               eInNUcastPkts,
                               eInDiscards,
                               eInError,
                               eInUnknownProtos,
                               eOutOcts,
                               eOutUcastPkts,
                               eOutNUcastPkts,
                               eOutDiscards,
                               eOutErrors,
                               eOutQLen,
                               eDescr};
        size_t       numKeys = NUM(keys);
        size_t       numVals = NUM(vals);
        ESOCK_ASSERT( numKeys == numVals );
        ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, &erow) );
    }
    NDBG2( dbg,
           ("NET", "enet_if_row_encode -> done with:"
            "\r\n   result: %T"
            "\r\n", erow) );
    return erow;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_if_type(ErlNifEnv* env,
                            DWORD      type)
{
    ERL_NIF_TERM   etype;
    switch (type) {
    case IF_TYPE_OTHER:
        etype = atom_other;
        break;
    case IF_TYPE_ETHERNET_CSMACD:
        etype = atom_ethernet_csmacd;
        break;
    case IF_TYPE_ISO88025_TOKENRING:
        etype = atom_iso88025_tokenring;
        break;
    case IF_TYPE_FDDI:
        etype = atom_fddi;
        break;
    case IF_TYPE_PPP:
        etype = atom_ppp;
        break;
    case IF_TYPE_SOFTWARE_LOOPBACK:
        etype = atom_software_loopback;
        break;
    case IF_TYPE_ATM:
        etype = atom_atm;
        break;
    case IF_TYPE_IEEE80211:
        etype = atom_ieee80211;
        break;
    case IF_TYPE_TUNNEL:
        etype = atom_tunnel;
        break;
    case IF_TYPE_IEEE1394:
        etype = atom_ieee1394;
        break;
    case IF_TYPE_IEEE80216_WMAN:
        etype = atom_ieee80216_wman;
        break;
    case IF_TYPE_WWANPP:
        etype = atom_wwanpp;
        break;
    case IF_TYPE_WWANPP2:
        etype = atom_wwanpp2;
        break;
    default:
        etype = MKUI(env, type);
        break;
    }
    return etype;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_if_row_description(ErlNifEnv* env,
                                       DWORD      len,
                                       UCHAR*     buf)
{
    ERL_NIF_TERM edesc;
    UCHAR*       tmp = MALLOC(len + 1);
    ESOCK_ASSERT( tmp != NULL );
    sys_memcpy(tmp, buf, len);
    tmp[len] = 0;
    edesc = MKS(env, tmp);
    FREE(tmp);
    return edesc;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_if_admin_status(ErlNifEnv* env,
                                    DWORD      status)
{
    ERL_NIF_TERM estatus;
    if (status) {
        estatus = esock_atom_enabled;
    } else {
        estatus = esock_atom_disabled;
    }
    return estatus;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_internal_if_oper_status(ErlNifEnv* env,
                                            DWORD      status)
{
    ERL_NIF_TERM estatus;
    switch (status) {
    case IF_OPER_STATUS_NON_OPERATIONAL:
        estatus = atom_non_operational;
        break;
    case IF_OPER_STATUS_UNREACHABLE:
        estatus = atom_unreachable;
        break;
    case IF_OPER_STATUS_DISCONNECTED:
        estatus = atom_disconnected;
        break;
    case IF_OPER_STATUS_CONNECTING:
        estatus = esock_atom_connecting;
        break;
    case IF_OPER_STATUS_CONNECTED:
        estatus = esock_atom_connected;
        break;
    case IF_OPER_STATUS_OPERATIONAL:
        estatus = atom_operational;
        break;
    default:
        estatus = MKUI(env, status);
        break;
    }
    return estatus;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_if_row_phys_address(ErlNifEnv* env,
                                        DWORD      len,
                                        UCHAR*     buf)
{
    return encode_uchar(env, len, buf);
}
#endif
static
ERL_NIF_TERM nif_get_interface_info(ErlNifEnv*         env,
                                    int                argc,
                                    const ERL_NIF_TERM argv[])
{
#if !defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#else
    ERL_NIF_TERM result, eargs;
    BOOLEAN_T    dbg;
    NDBG( ("NET", "nif_get_interface_info -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !IS_MAP(env, argv[0])) {
        return enif_make_badarg(env);
    }
    eargs = argv[0];
    dbg    = enet_get_interface_info_args_debug(env, eargs);
    result = enet_get_interface_info(env, dbg);
    NDBG2( dbg,
           ("NET",
            "nif_get_interface_info -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
#endif
}
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_interface_info_args_debug(ErlNifEnv*         env,
                                             const ERL_NIF_TERM eargs)
{
    return get_debug(env, eargs);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_get_interface_info(ErlNifEnv* env,
                                     BOOLEAN_T  dbg)
{
    int                i;
    DWORD              ret;
    unsigned long      infoSize = 4 * 1024;
    IP_INTERFACE_INFO* infoP    = (IP_INTERFACE_INFO*) MALLOC(infoSize);
    ERL_NIF_TERM       eret, einfo, result;
    for (i = 17;  i;  i--) {
        NDBG2( dbg,
               ("NET", "enet_get_interface_info -> try get info with: "
                "\r\n   infoSize: %d"
                "\r\n", infoSize) );
        ret   = GetInterfaceInfo(infoP, &infoSize);
        NDBG2( dbg,
               ("NET", "enet_get_interface_info -> "
                "get-info result: %d (%d)\r\n", ret, infoSize) );
        if (ret == NO_ERROR) {
            break;
        } else if (ret == ERROR_INSUFFICIENT_BUFFER) {
            infoP = REALLOC(infoP, infoSize);
            continue;
        } else {
            i = 0;
        }
    }
    NDBG2( dbg,
           ("NET", "enet_get_interface_info -> "
            "done when get info counter: %d\r\n", i) );
    if (! i) {
        NDBG2( dbg,
               ("NET", "enet_get_interface_info -> "
                "try encode error (%d)\r\n", ret) );
        FREE(infoP);
        switch (ret) {
        case ERROR_INSUFFICIENT_BUFFER:
            eret = atom_insufficient_buffer;
            break;
        case ERROR_INVALID_PARAMETER:
            eret = atom_invalid_parameter;
            break;
        case ERROR_NO_DATA:
            eret = atom_no_data;
            break;
        case ERROR_NOT_SUPPORTED:
            eret = atom_not_supported;
            break;
        default:
            eret = MKI(env, ret);
            break;
        }
        result = esock_make_error(env, eret);
    } else {
        NDBG2( dbg,
               ("NET", "enet_get_interface_info -> try encode info\r\n") );
        einfo  = enet_interface_info_encode(env, dbg, infoP);
        result = esock_make_ok2(env, einfo);
        FREE(infoP);
    }
    NDBG2( dbg,
           ("NET", "enet_get_interface_info -> done with:"
            "\r\n   result: %T"
            "\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_interface_info_encode(ErlNifEnv*         env,
                                        BOOLEAN_T          dbg,
                                        IP_INTERFACE_INFO* infoP)
{
    ERL_NIF_TERM result;
    LONG         num = infoP->NumAdapters;
    NDBG2( dbg,
           ("NET", "enet_interface_info_encode -> entry with"
            "\r\n   num: %d"
            "\r\n", num) );
    if (num > 0) {
        ERL_NIF_TERM* array = MALLOC(num * sizeof(ERL_NIF_TERM));
        LONG          i     = 0;
        while (i < num) {
            ERL_NIF_TERM entry;
            NDBG2( dbg,
                   ("NET", "enet_interface_info_encode -> "
                    "try encode adapter %d"
                    "\r\n", i) );
            encode_adapter_index_map(env, dbg, &infoP->Adapter[i], &entry);
            array[i] = entry;
            i++;
        }
        result = MKLA(env, array, num);
        FREE(array);
    } else {
        result = MKEL(env);
    }
    NDBG2( dbg,
           ("NET", "enet_get_interface_info -> done with:"
            "\r\n   result: %T"
            "\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
void encode_adapter_index_map(ErlNifEnv*            env,
                              BOOLEAN_T             dbg,
                              IP_ADAPTER_INDEX_MAP* adapterP,
                              ERL_NIF_TERM*         eadapter)
{
    ERL_NIF_TERM eindex = MKI(env, adapterP->Index);
    ERL_NIF_TERM ename  = encode_adapter_index_map_name(env, adapterP->Name);
    ERL_NIF_TERM map;
    NDBG2( dbg,
           ("NET", "encode_adapter_index_map -> map fields: "
            "\r\n   index: %T"
            "\r\n   name:  %T"
            "\r\n", eindex, ename) );
    make_adapter_index_map(env, eindex, ename, &map);
    NDBG2( dbg,
           ("NET", "encode_adapter_index_map -> encoded map: %T\r\n", map) );
    *eadapter = map;
}
#endif
static
ERL_NIF_TERM nif_get_ip_address_table(ErlNifEnv*         env,
                                      int                argc,
                                      const ERL_NIF_TERM argv[])
{
#if !defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#else
    ERL_NIF_TERM result, eargs;
    BOOLEAN_T    dbg, sort;
    NDBG( ("NET", "nif_get_ip_address_table -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !IS_MAP(env, argv[0])) {
        return enif_make_badarg(env);
    }
    eargs = argv[0];
    sort   = enet_get_ip_address_table_args_sort(env, eargs);
    dbg    = enet_get_ip_address_table_args_debug(env, eargs);
    result = enet_get_ip_address_table(env, sort, dbg);
    NDBG2( dbg,
           ("NET",
            "nif_get_ip_address_table -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
#endif
}
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_ip_address_table_args_debug(ErlNifEnv*         env,
                                               const ERL_NIF_TERM eargs)
{
    return get_debug(env, eargs);
}
#endif
#if defined(__WIN32__)
static
BOOLEAN_T enet_get_ip_address_table_args_sort(ErlNifEnv*         env,
                                              const ERL_NIF_TERM eargs)
{
    ERL_NIF_TERM sort = MKA(env, "sort");
    return esock_get_bool_from_map(env, eargs, sort, FALSE);
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_get_ip_address_table(ErlNifEnv* env,
                                       BOOLEAN_T  sort,
                                       BOOLEAN_T  dbg)
{
    int                i;
    DWORD              ret;
    unsigned long      tabSize    = 16*sizeof(MIB_IPADDRROW);
    MIB_IPADDRTABLE*   ipAddrTabP = (MIB_IPADDRTABLE*) MALLOC(tabSize);
    ERL_NIF_TERM       eret, etable, result;
    for (i = 17;  i;  i--) {
        NDBG2( dbg,
               ("NET", "enet_get_ip_address_table -> [%d] try get table with: "
                "\r\n   sort:    %s"
                "\r\n   tabSize: %d"
                "\r\n", i, B2S(sort), tabSize) );
        ret = GetIpAddrTable(ipAddrTabP, &tabSize, sort);
        NDBG2( dbg,
               ("NET", "enet_get_ip_address_table -> "
                "get-tab result: %d (%d)\r\n", ret, tabSize) );
        ipAddrTabP = REALLOC(ipAddrTabP, tabSize);
        if (ret == NO_ERROR) break;
        if (ret == ERROR_INSUFFICIENT_BUFFER) continue;
        i = 0;
    }
    NDBG2( dbg,
           ("NET", "enet_get_ip_address_table -> "
            "done when get-tab counter: %d\r\n", i) );
    if (! i) {
        NDBG2( dbg,
               ("NET",
                "enet_get_ip_address_table -> try transform error\r\n") );
        FREE(ipAddrTabP);
        switch (ret) {
        case ERROR_INSUFFICIENT_BUFFER:
            eret = atom_insufficient_buffer;
            break;
        case ERROR_INVALID_PARAMETER:
            eret = atom_invalid_parameter;
            break;
        case ERROR_NOT_SUPPORTED:
            eret = atom_not_supported;
            break;
        default:
            eret = MKI(env, ret);
            break;
        }
        result = esock_make_error(env, eret);
    } else {
        NDBG2( dbg,
               ("NET",
                "enet_get_ip_address_table -> try transform table\r\n") );
        etable = enet_get_ip_address_table_encode(env, dbg, ipAddrTabP);
        result = esock_make_ok2(env, etable);
        FREE(ipAddrTabP);
    }
    NDBG2( dbg,
           ("NET", "enet_get_ip_address_table -> done with:"
            "\r\n   result: %T"
            "\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM enet_get_ip_address_table_encode(ErlNifEnv*       env,
                                              BOOLEAN_T        dbg,
                                              MIB_IPADDRTABLE* tabP)
{
    ERL_NIF_TERM result;
    LONG         num = tabP->dwNumEntries;
    NDBG2( dbg,
           ("NET", "enet_get_ip_address_table_encode -> entry with"
            "\r\n   num: %d"
            "\r\n", num) );
    if (num > 0) {
        ERL_NIF_TERM* array = MALLOC(num * sizeof(ERL_NIF_TERM));
        LONG          i     = 0;
        while (i < num) {
            ERL_NIF_TERM entry;
            NDBG2( dbg,
                   ("NET", "enet_interface_info_encode -> "
                    "try encode ip-address-row %d"
                    "\r\n", i) );
            entry = encode_ip_address_row(env, dbg, &tabP->table[i]);
            array[i] = entry;
            i++;
        }
        result = MKLA(env, array, num);
        FREE(array);
    } else {
        result = MKEL(env);
    }
    NDBG2( dbg,
           ("NET", "enet_get_ip_address_table -> done with:"
            "\r\n   result: %T"
            "\r\n", result) );
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_ip_address_row(ErlNifEnv*     env,
                                   BOOLEAN_T      dbg,
                                   MIB_IPADDRROW* rowP)
{
    ERL_NIF_TERM eaddr      = encode_ip_address_row_addr(env,
                                                         dbg, "Addr",
                                                         rowP->dwAddr);
    ERL_NIF_TERM eindex     = MKUL(env, rowP->dwIndex);
    ERL_NIF_TERM emask      = encode_ip_address_row_addr(env,
                                                         dbg, "Mask",
                                                         rowP->dwMask);
    ERL_NIF_TERM eBCastAddr = encode_ip_address_row_addr(env,
                                                         dbg, "BCaseAddr",
                                                         rowP->dwBCastAddr);
    ERL_NIF_TERM eReasmSize = MKUL(env, rowP->dwReasmSize);
    ERL_NIF_TERM map;
    NDBG2( dbg,
           ("NET", "encode_ipAddress_row_map -> map fields: "
            "\r\n   address:    %T"
            "\r\n   index:      %T"
            "\r\n   mask:       %T"
            "\r\n   bcas-addr:  %T"
            "\r\n   reasm-size: %T"
            "\r\n", eaddr, eindex, emask, eBCastAddr, eReasmSize) );
    make_ip_address_row(env, eaddr, eindex, emask, eBCastAddr, eReasmSize, &map);
    NDBG2( dbg,
           ("NET", "encode_ip_address_row -> encoded map: %T\r\n", map) );
    return map;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_ip_address_row_addr(ErlNifEnv*  env,
                                        BOOLEAN_T   dbg,
                                        const char* descr,
                                        DWORD       addr)
{
    struct in_addr a;
    ERL_NIF_TERM   ea;
    NDBG2( dbg,
           ("NET",
            "encode_ip_address_row_addr -> entry with: "
            "\r\n   %s: %lu\r\n", descr, addr) );
    a.s_addr = addr;
    esock_encode_in_addr(env, &a, &ea);
    return ea;
}
#endif
#if defined(__WIN32__)
static
void make_ip_address_row(ErlNifEnv*    env,
                         ERL_NIF_TERM  eaddr,
                         ERL_NIF_TERM  eindex,
                         ERL_NIF_TERM  emask,
                         ERL_NIF_TERM  eBCastAddr,
                         ERL_NIF_TERM  eReasmSize,
                         ERL_NIF_TERM* iar)
{
    ERL_NIF_TERM keys[]  = {esock_atom_addr,
                            atom_index,
                            atom_mask,
                            atom_bcast_addr,
                            atom_reasm_size};
    ERL_NIF_TERM vals[]  = {eaddr, eindex, emask, eBCastAddr, eReasmSize};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, iar) );
}
#endif
static
ERL_NIF_TERM nif_getservbyname(ErlNifEnv*         env,
                               int                argc,
                               const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM result, ename, eproto;
    BOOLEAN_T    dbg = FALSE;
    NDBG( ("NET", "nif_getservbyname -> entry (%d)\r\n", argc) );
    if (argc != 2)
        return enif_make_badarg(env);
    ename  = argv[0];
    eproto = argv[1];
    NDBG2( dbg,
           ("NET",
            "nif_getservbyname -> args: "
            "\r\n   ename:  %T"
            "\r\n   eproto: %T"
            "\r\n", ename, eproto) );
    result = enet_getservbyname(env, ename, eproto);
    NDBG2( dbg,
           ("NET",
            "nif_getservbyname -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
}
static
ERL_NIF_TERM enet_getservbyname(ErlNifEnv*   env,
                                ERL_NIF_TERM ename,
                                ERL_NIF_TERM eproto)
{
    char            name[256];
    char            proto[256];
    struct servent* srv;
    short           port;
    if (0 >= GET_STR(env, ename, name, sizeof(name)))
        return esock_make_error(env, esock_atom_einval);
    if (0 >= GET_STR(env, eproto, proto, sizeof(proto)))
        return esock_make_error(env, esock_atom_einval);
    if ( strcmp(proto, "any") == 0 )
        srv = net_getservbyname(name, NULL);
    else
        srv = net_getservbyname(name, proto);
    if (srv == NULL)
        return esock_make_error(env, esock_atom_einval);
    port = net_ntohs(srv->s_port);
    return esock_make_ok2(env, MKI(env, port));
}
static
ERL_NIF_TERM nif_getservbyport(ErlNifEnv*         env,
                               int                argc,
                               const ERL_NIF_TERM argv[])
{
    ERL_NIF_TERM result, eport, eproto;
    BOOLEAN_T    dbg = FALSE;
    NDBG2( dbg, ("NET", "nif_getservbyport -> entry (%d)\r\n", argc) );
    if (argc != 2)
        return enif_make_badarg(env);
    eport  = argv[0];
    eproto = argv[1];
    NDBG2( dbg,
           ("NET",
            "nif_getservbyport -> args: "
            "\r\n   eport:  %T"
            "\r\n   eproto: %T"
            "\r\n", eport, eproto) );
    result = enet_getservbyport(env, dbg, eport, eproto);
    NDBG2( dbg,
           ("NET",
            "nif_getservbyport -> done when result: "
            "\r\n   %T\r\n", result) );
    return result;
}
static
ERL_NIF_TERM enet_getservbyport(ErlNifEnv*   env,
                                BOOLEAN_T      dbg,
                                ERL_NIF_TERM eport,
                                ERL_NIF_TERM eproto)
{
    char            proto[256];
    struct servent* srv;
    unsigned int    port;
    ERL_NIF_TERM ename, result;
    NDBG2( dbg, ("NET", "enet_getservbyport -> try 'get' port\r\n") );
    if (0 >= GET_UINT(env, eport, &port))
        return esock_make_error(env, esock_atom_einval);
    NDBG2( dbg, ("NET", "enet_getservbyport -> (pre htons) port: %u\r\n", port) );
    port = net_htons(port);
    NDBG2( dbg, ("NET", "enet_getservbyport -> (post htons) port: %u\r\n", port) );
    NDBG2( dbg, ("NET", "enet_getservbyport -> try 'get' proto\r\n") );
    if (0 >= GET_STR(env, eproto, proto, sizeof(proto)))
        return esock_make_error(env, esock_atom_einval);
    NDBG2( dbg, ("NET", "enet_getservbyport -> proto: %s\r\n", proto) );
    NDBG2( dbg, ("NET", "enet_getservbyport -> check proto\r\n") );
    if ( strcmp(proto, "any") == 0 ) {
        srv = net_getservbyport(port, NULL);
    } else {
        srv = net_getservbyport(port, proto);
    }
    if (srv == NULL) {
        NDBG2( dbg, ("NET", "enet_getservbyport -> failed get servent\r\n") );
        result = esock_make_error(env, esock_atom_einval);
    } else {
        unsigned int len = strlen(srv->s_name);
        NDBG2( dbg, ("NET", "enet_getservbyport -> make string (term) with length %d\r\n", len) );
        ename = MKSL(env, srv->s_name, len);
        result = esock_make_ok2(env, ename);
    }
    NDBG2( dbg, ("NET", "enet_getservbyport -> done with"
                 "\r\n   result: %T"
                 "\r\n", result) );
    return result;
}
static
ERL_NIF_TERM nif_if_name2index(ErlNifEnv*         env,
                               int                argc,
                               const ERL_NIF_TERM argv[])
{
#if defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#elif defined(HAVE_IF_NAMETOINDEX)
    ERL_NIF_TERM eifn, result;
    char         ifn[IF_NAMESIZE+1];
    NDBG( ("NET", "nif_if_name2index -> entry (%d)\r\n", argc) );
    if (argc != 1) {
        return enif_make_badarg(env);
    }
    eifn = argv[0];
    NDBG( ("NET",
           "nif_if_name2index -> "
           "\r\n   Ifn: %T"
           "\r\n", argv[0]) );
    if (0 >= GET_STR(env, eifn, ifn, sizeof(ifn)))
        return esock_make_error(env, esock_atom_einval);
    result = enet_if_name2index(env, ifn);
    NDBG( ("NET", "nif_if_name2index -> done when result: %T\r\n", result) );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
#if !defined(__WIN32__) && defined(HAVE_IF_NAMETOINDEX)
static
ERL_NIF_TERM enet_if_name2index(ErlNifEnv* env,
                            char*      ifn)
{
    unsigned int idx;
    NDBG( ("NET", "enet_if_name2index -> entry with ifn: %s\r\n", ifn) );
    idx = if_nametoindex(ifn);
    NDBG( ("NET", "enet_if_name2index -> idx: %d\r\n", idx) );
    if (idx == 0) {
        int save_errno = get_errno();
        NDBG( ("NET", "nif_name2index -> failed: %d\r\n", save_errno) );
        return esock_make_error_errno(env, save_errno);
    } else {
         return esock_make_ok2(env, MKI(env, idx));
    }
}
#endif
static
ERL_NIF_TERM nif_if_index2name(ErlNifEnv*         env,
                               int                argc,
                               const ERL_NIF_TERM argv[])
{
#if defined(__WIN32__)
    return enif_raise_exception(env, MKA(env, "notsup"));
#elif defined(HAVE_IF_INDEXTONAME)
    ERL_NIF_TERM result;
    unsigned int idx;
    NDBG( ("NET", "nif_if_index2name -> entry (%d)\r\n", argc) );
    if ((argc != 1) ||
        !GET_UINT(env, argv[0], &idx)) {
        return enif_make_badarg(env);
    }
    NDBG( ("NET", "nif_index2name -> "
           "\r\n   Idx: %T"
           "\r\n", argv[0]) );
    result = enet_if_index2name(env, idx);
    NDBG( ("NET", "nif_if_index2name -> done when result: %T\r\n", result) );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
#if !defined(__WIN32__) && defined(HAVE_IF_INDEXTONAME)
static
ERL_NIF_TERM enet_if_index2name(ErlNifEnv*   env,
                            unsigned int idx)
{
    ERL_NIF_TERM result;
    char*        ifn = MALLOC(IF_NAMESIZE+1);
    if (ifn == NULL)
        return enif_make_badarg(env);
    if (NULL != if_indextoname(idx, ifn)) {
        result = esock_make_ok2(env, MKS(env, ifn));
    } else {
        result = esock_make_error(env, atom_enxio);
    }
    FREE(ifn);
    return result;
}
#endif
static
ERL_NIF_TERM nif_if_names(ErlNifEnv*         env,
                          int                argc,
                          const ERL_NIF_TERM argv[])
{
#if defined(__WIN32__) || (defined(__ANDROID__) && (__ANDROID_API__ < 24))
    return enif_raise_exception(env, MKA(env, "notsup"));
#elif defined(HAVE_IF_NAMEINDEX) && defined(HAVE_IF_FREENAMEINDEX)
    ERL_NIF_TERM result;
    NDBG( ("NET", "nif_if_names -> entry (%d)\r\n", argc) );
    if (argc != 0) {
        return enif_make_badarg(env);
    }
    result = enet_if_names(env);
    NDBG( ("NET", "nif_if_names -> done when result: %T\r\n", result) );
    return result;
#else
    return esock_make_error(env, esock_atom_enotsup);
#endif
}
#if !defined(__WIN32__) && !(defined(__ANDROID__) && (__ANDROID_API__ < 24))
#if defined(HAVE_IF_NAMEINDEX) && defined(HAVE_IF_FREENAMEINDEX)
static
ERL_NIF_TERM enet_if_names(ErlNifEnv* env)
{
    ERL_NIF_TERM         result;
    struct if_nameindex* ifs = if_nameindex();
    NDBG( ("NET", "enet_if_names -> ifs: 0x%lX\r\n", ifs) );
    if (ifs == NULL) {
        result = esock_make_error_errno(env, get_errno());
    } else {
        unsigned int len = enet_if_names_length(ifs);
        NDBG( ("NET", "enet_if_names -> len: %d\r\n", len) );
        if (len > 0) {
            ERL_NIF_TERM* array = MALLOC(len * sizeof(ERL_NIF_TERM));
            unsigned int  i;
            for (i = 0; i < len; i++) {
                array[i] = MKT2(env,
                                MKI(env, ifs[i].if_index),
                                MKS(env, ifs[i].if_name));
            }
            result = esock_make_ok2(env, MKLA(env, array, len));
            FREE(array);
        } else {
            result = esock_make_ok2(env, enif_make_list(env, 0));
        }
    }
    if (ifs != NULL)
        if_freenameindex(ifs);
    return result;
}
static
unsigned int enet_if_names_length(struct if_nameindex* p)
{
    unsigned int len = 0;
    BOOLEAN_T    done =  FALSE;
    while (!done) {
        NDBG( ("NET", "enet_if_names_length -> %d: "
               "\r\n   if_index: %d"
               "\r\n   if_name:  0x%lX"
               "\r\n", len, p[len].if_index, p[len].if_name) );
        if ((p[len].if_index == 0) && (p[len].if_name == NULL))
            done = TRUE;
        else
            len++;
    }
    return len;
}
#endif
#endif
static
ERL_NIF_TERM encode_sockaddr(ErlNifEnv*       env,
                             struct sockaddr* sa)
{
    ERL_NIF_TERM esa;
    if (sa != NULL) {
        esock_encode_sockaddr(env, (ESockAddress*) sa, -1, &esa);
    } else {
        esa = esock_atom_undefined;
    }
    return esa;
}
static
BOOLEAN_T decode_nameinfo_flags(ErlNifEnv*         env,
                                const ERL_NIF_TERM eflags,
                                int*               flags)
{
    BOOLEAN_T result;
    if (IS_ATOM(env, eflags)) {
        NDBG( ("NET", "decode_nameinfo_flags -> is atom (%T)\r\n", eflags) );
        if (COMPARE(eflags, esock_atom_undefined) == 0) {
            *flags = 0;
            result = TRUE;
        } else {
            result = FALSE;
        }
    } else if (IS_LIST(env, eflags)) {
        NDBG( ("NET", "decode_nameinfo_flags -> is list\r\n") );
        result = decode_nameinfo_flags_list(env, eflags, flags);
    } else {
        result = FALSE;
    }
    NDBG( ("NET", "decode_nameinfo_flags -> result: %s\r\n", B2S(result)) );
    return result;
}
static
BOOLEAN_T decode_nameinfo_flags_list(ErlNifEnv*         env,
                                     const ERL_NIF_TERM eflags,
                                     int*               flags)
{
    ERL_NIF_TERM elem, tail, list = eflags;
    int          tmp = 0;
    BOOLEAN_T    done = FALSE;
    while (!done) {
        if (GET_LIST_ELEM(env, list, &elem, &tail)) {
            if (COMPARE(elem, atom_namereqd) == 0) {
                tmp |= NI_NAMEREQD;
            } else if (COMPARE(elem, esock_atom_dgram) == 0) {
                tmp |= NI_DGRAM;
            } else if (COMPARE(elem, atom_nofqdn) == 0) {
                tmp |= NI_NOFQDN;
            } else if (COMPARE(elem, atom_numerichost) == 0) {
                tmp |= NI_NUMERICHOST;
            } else if (COMPARE(elem, atom_numericserv) == 0) {
                tmp |= NI_NUMERICSERV;
#if defined(NI_IDN)
            } else if (COMPARE(elem, atom_idn) == 0) {
                tmp |= NI_IDN;
#endif
            } else {
                NDBG( ("NET", "decode_nameinfo_flags_list -> "
                       "invalid flag: %T\r\n", elem) );
                return FALSE;
            }
            list = tail;
        } else {
            done = TRUE;
        }
    }
    *flags = tmp;
    return TRUE;
}
static
BOOLEAN_T decode_addrinfo_string(ErlNifEnv*         env,
                                 const ERL_NIF_TERM eString,
                                 char**             stringP)
{
    BOOLEAN_T result;
    if (IS_ATOM(env, eString)) {
        if (COMPARE(eString, esock_atom_undefined) == 0) {
            *stringP = NULL;
            result   = TRUE;
        } else {
            *stringP = NULL;
            result   = FALSE;
        }
    } else {
        result = esock_decode_string(env, eString, stringP);
    }
    return result;
}
static
ERL_NIF_TERM encode_address_infos(ErlNifEnv*       env,
                                  struct addrinfo* addrInfo)
{
    ERL_NIF_TERM result;
    unsigned int len = address_info_length(addrInfo);
    NDBG( ("NET", "encode_address_infos -> len: %d\r\n", len) );
    if (len > 0) {
        ERL_NIF_TERM*    array = MALLOC(len * sizeof(ERL_NIF_TERM));
        unsigned int     i     = 0;
        struct addrinfo* p     = addrInfo;
        while (i < len) {
            array[i] = encode_address_info(env, p);
            p = p->ai_next;
            i++;
        }
        result = MKLA(env, array, len);
        FREE(array);
    } else {
        result = MKEL(env);
    }
    NDBG( ("NET", "encode_address_infos -> result: "
           "\r\n   %T\r\n", result) );
    return result;
}
static
unsigned int address_info_length(struct addrinfo* addrInfoP)
{
    unsigned int     len = 1;
    struct addrinfo* tmp;
    BOOLEAN_T        done = FALSE;
    tmp = addrInfoP;
    while (!done) {
        if (tmp->ai_next != NULL) {
            len++;
            tmp = tmp->ai_next;
        } else {
            done = TRUE;
        }
    }
    return len;
}
static
ERL_NIF_TERM encode_address_info(ErlNifEnv*       env,
                                 struct addrinfo* addrInfoP)
{
    ERL_NIF_TERM fam, type, proto, addr, addrInfo;
    fam   = encode_address_info_family(env, addrInfoP->ai_family);
    type  = encode_address_info_type(env,   addrInfoP->ai_socktype);
    proto = MKI(env, addrInfoP->ai_protocol);
    esock_encode_sockaddr(env,
                          (ESockAddress*) addrInfoP->ai_addr,
                          addrInfoP->ai_addrlen,
                          &addr);
    make_address_info(env, fam, type, proto, addr, &addrInfo);
    return addrInfo;
}
static
ERL_NIF_TERM encode_address_info_family(ErlNifEnv* env,
                                        int        family)
{
    ERL_NIF_TERM efam;
    esock_encode_domain(env, family, &efam);
    return efam;
}
static
ERL_NIF_TERM encode_address_info_type(ErlNifEnv* env,
                                      int        socktype)
{
    ERL_NIF_TERM etype;
    ERL_NIF_TERM zero = MKI(env, 0);
    esock_encode_type(env, socktype, &etype);
    if (IS_IDENTICAL(zero, etype))
        return esock_atom_any;
    else
        return etype;
}
static
void make_address_info(ErlNifEnv*    env,
                       ERL_NIF_TERM  fam,
                       ERL_NIF_TERM  sockType,
                       ERL_NIF_TERM  proto,
                       ERL_NIF_TERM  addr,
                       ERL_NIF_TERM* ai)
{
    ERL_NIF_TERM keys[]  = {esock_atom_family,
                            esock_atom_socktype,
                            esock_atom_protocol,
                            esock_atom_addr};
    ERL_NIF_TERM vals[]  = {fam, sockType, proto, addr};
    size_t       numKeys = NUM(keys);
    ESOCK_ASSERT( numKeys == NUM(vals) );
    ESOCK_ASSERT( MKMA(env, keys, vals, numKeys, ai) );
}
#ifdef HAVE_SETNS
#if !defined(__WIN32__)
static
BOOLEAN_T change_network_namespace(char* netns, int* cns, int* err)
{
    int save_errno;
    int current_ns = 0;
    int new_ns     = 0;
    NDBG( ("NET", "change_network_namespace -> entry with"
            "\r\n   new ns: %s", netns) );
    if (netns != NULL) {
        current_ns = open("/proc/self/ns/net", O_RDONLY);
        if (current_ns == -1) {
            *cns = current_ns;
            *err = get_errno();
            return FALSE;
        }
        new_ns = open(netns, O_RDONLY);
        if (new_ns == -1) {
            save_errno = get_errno();
            while (close(current_ns) == -1 &&
                   get_errno() == EINTR);
            *cns = -1;
            *err = save_errno;
            return FALSE;
        }
        if (setns(new_ns, CLONE_NEWNET) != 0) {
            save_errno = get_errno();
            while ((close(new_ns) == -1) &&
                   (get_errno() == EINTR));
            while ((close(current_ns) == -1) &&
                   (get_errno() == EINTR));
            *cns = -1;
            *err = save_errno;
            return FALSE;
        } else {
            while ((close(new_ns) == -1) &&
                   (get_errno() == EINTR));
            *cns = current_ns;
            *err = 0;
            return TRUE;
        }
    } else {
        *cns = -1;
        *err = 0;
        return TRUE;
    }
}
static
BOOLEAN_T restore_network_namespace(int ns, int* err)
{
    int save_errno;
    NDBG( ("NET", "restore_network_namespace -> entry with"
            "\r\n   ns: %d", ns) );
    if (ns != -1) {
        if (setns(ns, CLONE_NEWNET) != 0) {
            save_errno = get_errno();
            while (close(ns) == -1 &&
                   get_errno() == EINTR);
            *err = save_errno;
            return FALSE;
        } else {
            while (close(ns) == -1 &&
                   get_errno() == EINTR);
            *err = 0;
            return TRUE;
        }
  }
  *err = 0;
  return TRUE;
}
#endif
#endif
static
ErlNifFunc net_funcs[] =
{
    {"nif_info",      0, nif_info,      0},
    {"nif_command",   1, nif_command,   0},
    {"nif_gethostname",      0, nif_gethostname,   0},
    {"nif_getnameinfo",      2, nif_getnameinfo,   0},
    {"nif_getaddrinfo",      3, nif_getaddrinfo,   0},
    {"nif_getifaddrs",       1, nif_getifaddrs,       ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_get_adapters_addresses", 1, nif_get_adapters_addresses, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_get_if_entry",       1, nif_get_if_entry,       ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_get_interface_info", 1, nif_get_interface_info, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_get_ip_address_table", 1, nif_get_ip_address_table, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_getservbyname", 2, nif_getservbyname, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_getservbyport", 2, nif_getservbyport, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"nif_if_name2index",    1, nif_if_name2index, 0},
    {"nif_if_index2name",    1, nif_if_index2name, 0},
    {"nif_if_names",         0, nif_if_names,      0}
};
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_wchar(ErlNifEnv* env, WCHAR* name)
{
    ERL_NIF_TERM result;
    int          len = WideCharToMultiByte(CP_UTF8, 0,
                                           name, -1,
                                           NULL, 0, NULL, NULL);
    if (!len) {
        result = esock_atom_undefined;
    } else {
        char* buf = (char*) MALLOC(len+1);
        if (0 == WideCharToMultiByte(CP_UTF8, 0,
                                     name, -1,
                                     buf, len, NULL, NULL)) {
            DWORD error = GetLastError();
            switch (error) {
            case ERROR_INSUFFICIENT_BUFFER:
                result = atom_insufficient_buffer;
                break;
            case ERROR_INVALID_FLAGS:
                result = atom_invalid_flags;
                break;
            case ERROR_INVALID_PARAMETER:
                result = atom_invalid_parameter;
                break;
            case ERROR_NO_UNICODE_TRANSLATION:
                result = atom_no_uniconde_traslation;
                break;
            default:
                result = MKI(env, error);
                break;
            }
        } else {
            result = MKS(env, buf);
        }
        FREE(buf);
    }
    return result;
}
#endif
#if defined(__WIN32__)
static
ERL_NIF_TERM encode_uchar(ErlNifEnv* env,
                          DWORD      len,
                          UCHAR*     buf)
{
    ERL_NIF_TERM   ebuf;
    unsigned char* p;
    p = enif_make_new_binary(env, len, &ebuf);
    ESOCK_ASSERT( p != NULL );
    sys_memcpy(p, buf, len);
    return ebuf;
}
#endif
static
BOOLEAN_T get_debug(ErlNifEnv*   env,
                    ERL_NIF_TERM map)
{
    ERL_NIF_TERM debug = MKA(env, "debug");
    return esock_get_bool_from_map(env, map, debug, NET_NIF_DEBUG_DEFAULT);
}
static
int on_load(ErlNifEnv* env, void** priv_data, ERL_NIF_TERM load_info)
{
#if !defined(__WIN32__)
    data.debug = get_debug(env, load_info);
    NDBG( ("NET", "on_load -> entry\r\n") );
#endif
#define LOCAL_ATOM_DECL(A) atom_##A = MKA(env, #A)
LOCAL_ATOMS
LOCAL_ERROR_REASON_ATOMS
#undef LOCAL_ATOM_DECL
    net = enif_open_resource_type_x(env,
                                    "net",
                                    &netInit,
                                    ERL_NIF_RT_CREATE,
                                    NULL);
#if !defined(__WIN32__)
    NDBG( ("NET", "on_load -> done\r\n") );
#endif
    return !net;
}
ERL_NIF_INIT(prim_net, net_funcs, on_load, NULL, NULL, NULL)
#endif